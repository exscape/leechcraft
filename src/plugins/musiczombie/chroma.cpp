/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "chroma.h"
#include <stdexcept>
#include <memory>
#include <QString>
#include <QByteArray>
#include <QtDebug>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audioconvert.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

namespace LeechCraft
{
namespace MusicZombie
{
	QMutex Chroma::CodecMutex_ (QMutex::NonRecursive);
	QMutex Chroma::RegisterMutex_ (QMutex::NonRecursive);

	Chroma::Chroma ()
	: Ctx_ (chromaprint_new (CHROMAPRINT_ALGORITHM_DEFAULT))
	{
		QMutexLocker locker (&RegisterMutex_);
		av_register_all ();
	}

	Chroma::~Chroma ()
	{
		chromaprint_free (Ctx_);
	}

	Chroma::Result Chroma::operator() (const QString& filename)
	{
		std::shared_ptr<AVFormatContext> formatCtx;
		{
			AVFormatContext *formatCtxRaw = nullptr;
			if (avformat_open_input (&formatCtxRaw, filename.toLatin1 ().constData (), nullptr, nullptr))
				throw std::runtime_error ("error opening file");

			formatCtx.reset (formatCtxRaw,
					[] (AVFormatContext *ctx) { avformat_close_input (&ctx); });
		}

		{
			QMutexLocker locker (&CodecMutex_);
			if (avformat_find_stream_info (formatCtx.get (), nullptr) < 0)
				throw std::runtime_error ("could not find stream");
		}

		AVCodec *codec = nullptr;
		const auto streamIndex = av_find_best_stream (formatCtx.get (), AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
		if (streamIndex < 0)
			throw std::runtime_error ("could not find audio stream");

		auto stream = formatCtx->streams [streamIndex];

		bool codecOpened = false;

		std::shared_ptr<AVCodecContext> codecCtx (stream->codec,
				[&codecOpened] (AVCodecContext *ctx) { if (codecOpened) avcodec_close (ctx); });
		{
			QMutexLocker locker (&CodecMutex_);
			if (avcodec_open2 (codecCtx.get (), codec, nullptr) < 0)
				throw std::runtime_error ("couldn't open the codec");
		}
		codecOpened = true;

		if (codecCtx->channels <= 0)
			throw std::runtime_error ("no channels found");

		std::shared_ptr<SwrContext> swr;
		if (codecCtx->sample_fmt != AV_SAMPLE_FMT_S16)
		{
			swr.reset (swr_alloc (), [] (SwrContext *ctx) { if (ctx) swr_free (&ctx); });
			av_opt_set_int (swr.get (), "in_channel_layout", codecCtx->channel_layout, 0);
			av_opt_set_int (swr.get (), "out_channel_layout", codecCtx->channel_layout,  0);
			av_opt_set_int (swr.get (), "in_sample_rate", codecCtx->sample_rate, 0);
			av_opt_set_int (swr.get (), "out_sample_rate", codecCtx->sample_rate, 0);
			av_opt_set_sample_fmt (swr.get (), "in_sample_fmt", codecCtx->sample_fmt, 0);
			av_opt_set_sample_fmt (swr.get (), "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
			swr_init (swr.get ());
		}

		AVPacket packet;
		av_init_packet (&packet);

		const int maxLength = 120;
		auto remaining = maxLength * codecCtx->channels * codecCtx->sample_rate;
		chromaprint_start (Ctx_, codecCtx->sample_rate, codecCtx->channels);

		std::shared_ptr<AVFrame> frame (av_frame_alloc (),
				[] (AVFrame *frame) { av_frame_free (&frame); });
		auto maxDstNbSamples = 0;

		uint8_t *dstData [1] = { nullptr };
		std::shared_ptr<void> dstDataGuard (nullptr,
				[&dstData] (void*) { if (dstData [0]) av_freep (&dstData [0]); });
		while (true)
		{
			if (av_read_frame (formatCtx.get (), &packet) < 0)
				break;

			std::shared_ptr<void> guard (nullptr,
					[&packet] (void*) { if (packet.data) av_free_packet (&packet); });

			if (packet.stream_index != streamIndex)
				continue;

			av_frame_unref (frame.get ());
			int gotFrame = false;
			auto consumed = avcodec_decode_audio4 (codecCtx.get (), frame.get (), &gotFrame, &packet);

			if (consumed < 0 || !gotFrame)
				continue;

			uint8_t **data = nullptr;
			if (swr)
			{
				if (frame->nb_samples > maxDstNbSamples)
				{
					if (dstData [0])
						av_freep (&dstData [0]);
					int linesize = 0;
					if (av_samples_alloc (dstData, &linesize, codecCtx->channels, frame->nb_samples, AV_SAMPLE_FMT_S16, 1) < 0)
						throw std::runtime_error ("cannot allocate memory for resampling");
				}

				if (swr_convert (swr.get (), dstData, frame->nb_samples, const_cast<const uint8_t**> (frame->data), frame->nb_samples) < 0)
					throw std::runtime_error ("cannot resample audio");

				data = dstData;
			}
			else
				data = frame->data;

			auto length = std::min (remaining, frame->nb_samples * codecCtx->channels);
			if (!chromaprint_feed (Ctx_, data [0], length))
				throw std::runtime_error ("cannot feed data");

			bool finished = false;
			if (maxLength)
			{
				remaining -= length;
				if (remaining <= 0)
					finished = true;
			}
			if (finished)
				break;
		}

		if (!chromaprint_finish (Ctx_))
			throw std::runtime_error ("fingerprint calculation failed");

		char *fingerprint = 0;
		if (!chromaprint_get_fingerprint (Ctx_, &fingerprint))
			throw std::runtime_error ("unable to get fingerprint");

		QByteArray result (fingerprint);
		chromaprint_dealloc (fingerprint);

		const double divideFactor = 1. / av_q2d (stream->time_base);
		const double duration = stream->duration / divideFactor;

		return { result, static_cast<int> (duration) };
	}
}
}
