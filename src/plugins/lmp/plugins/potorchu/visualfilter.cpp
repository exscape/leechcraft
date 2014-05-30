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

#include "visualfilter.h"
#include <QtDebug>
#include <QWidget>
#include <gst/gst.h>
#include <libprojectM/projectM.hpp>
#include <util/lmp/gstutil.h>
#include "viswidget.h"
#include "visscene.h"

namespace LeechCraft
{
namespace LMP
{
namespace Potorchu
{
	VisualFilter::VisualFilter (const QByteArray& effectId, const ILMPProxy_ptr& proxy)
	: EffectId_ { effectId }
	, LmpProxy_ { proxy }
	, Widget_ { new VisWidget }
	, Scene_ { new VisScene }
	, Elem_ { gst_bin_new (nullptr) }
	, Tee_ { gst_element_factory_make ("tee", nullptr) }
	, AudioQueue_ { gst_element_factory_make ("queue", nullptr) }
	, ProbeQueue_ { gst_element_factory_make ("queue", nullptr) }
	, Converter_ { gst_element_factory_make ("audioconvert", nullptr) }
	, FakeSink_ { gst_element_factory_make ("fakesink", nullptr) }
	{
		gst_bin_add_many (GST_BIN (Elem_), Tee_, AudioQueue_, ProbeQueue_, Converter_, FakeSink_, nullptr);

		auto teeTemplate = gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (Tee_), "src%d");

		auto teeAudioPad = gst_element_request_pad (Tee_, teeTemplate, nullptr, nullptr);
		auto audioPad = gst_element_get_static_pad (AudioQueue_, "sink");
		gst_pad_link (teeAudioPad, audioPad);
		gst_object_unref (audioPad);

		GstUtil::AddGhostPad (Tee_, Elem_, "sink");
		GstUtil::AddGhostPad (AudioQueue_, Elem_, "src");

		gst_element_link (ProbeQueue_, Converter_);
		const auto caps = gst_caps_new_simple ("audio/x-raw-int",
				"width", G_TYPE_INT, 16,
				"signed", G_TYPE_BOOLEAN, true,
				nullptr);
		gst_element_link_filtered (Converter_, FakeSink_, caps);
		gst_caps_unref (caps);

		auto teeProbePad = gst_element_request_pad (Tee_, teeTemplate, nullptr, nullptr);
		auto streamPad = gst_element_get_static_pad (ProbeQueue_, "sink");
		gst_pad_link (teeProbePad, streamPad);
		gst_object_unref (streamPad);

		Widget_->resize (512, 512);
		Widget_->show ();
		Widget_->setScene (Scene_.get ());
		Widget_->SetFps (60);

		connect (Widget_.get (),
				SIGNAL (redrawRequested ()),
				Scene_.get (),
				SLOT (update ()));
		connect (Scene_.get (),
				SIGNAL (redrawing ()),
				this,
				SLOT (updateFrame ()));
		//proxy->GetGuiProxy ()->AddCurrentSongTab (tr ("Visualization"), Widget_.get ());

		connect (Widget_.get (),
				SIGNAL (nextVis ()),
				this,
				SLOT (handleNextVis ()));
		connect (Widget_.get (),
				SIGNAL (prevVis ()),
				this,
				SLOT (handlePrevVis ()));

		gst_pad_add_buffer_probe (gst_element_get_static_pad (Converter_, "src"),
				G_CALLBACK (+[] (GstPad*, GstBuffer *buf, VisualFilter *filter)
					{ filter->HandleBuffer (buf); }),
				this);
	}

	QByteArray VisualFilter::GetEffectId () const
	{
		return EffectId_;
	}

	QByteArray VisualFilter::GetInstanceId () const
	{
		return EffectId_;
	}

	IFilterConfigurator* VisualFilter::GetConfigurator () const
	{
		return nullptr;
	}

	GstElement* VisualFilter::GetElement () const
	{
		return Elem_;
	}

	void VisualFilter::InitProjectM ()
	{
		/*
    struct Settings {
        int meshX;
        int meshY;
        int fps;
        int textureSize;
        int windowWidth;
        int windowHeight;
        std::string presetURL;
        std::string titleFontURL;
        std::string menuFontURL;
        int smoothPresetDuration;
        int presetDuration;
        float beatSensitivity;
        bool aspectCorrection;
        float easterEgg;
        bool shuffleEnabled;
	bool softCutRatingsEnabled;
    };
	*/
		projectM::Settings settings
		{
			32,
			24,
			60,
			512,
			512,
			512,
			"/usr/share/projectM/presets",
			"/usr/share/fonts/droid/DroidSans.ttf",
			"/usr/share/fonts/droid/DroidSans.ttf",
			5,
			15,
			0,
			false,
			false,
			true,
			false
		};
		ProjectM_.reset (new projectM { settings });
	}

	void VisualFilter::HandleBuffer (GstBuffer *buffer)
	{
		const auto samples = GST_BUFFER_SIZE (buffer) / sizeof (short) / 2;
		const auto data = reinterpret_cast<short*> (GST_BUFFER_DATA (buffer));

		ProjectM_->pcm ()->addPCM16Data (data, samples);
	}

	void VisualFilter::SetVisualizer ()
	{
	}

	void VisualFilter::updateFrame ()
	{
		if (!ProjectM_)
			InitProjectM ();

		ProjectM_->renderFrame ();
	}

	void VisualFilter::handleNextVis ()
	{
	}

	void VisualFilter::handlePrevVis ()
	{
	}
}
}
}
