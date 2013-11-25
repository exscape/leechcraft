/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <functional>
#include <QObject>
#include <QHash>
#include <interfaces/core/icoreproxy.h>

class QStandardItem;

namespace LeechCraft
{
namespace Util
{
namespace SvcAuth
{
	class VkAuthManager;
}

class QueueManager;
enum class QueuePriority;
}

namespace TouchStreams
{
	class AlbumsManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		const qlonglong UserID_ = -1;

		Util::SvcAuth::VkAuthManager * const AuthMgr_;
		Util::QueueManager * const Queue_;
		QList<QPair<std::function<void (QString)>, Util::QueuePriority>> RequestQueue_;

		struct AlbumInfo
		{
			qlonglong ID_;
			QString Name_;
			QStandardItem *Item_;
		};
		QHash<qlonglong, AlbumInfo> Albums_;

		QStandardItem * const AlbumsRootItem_;

		quint32 TracksCount_ = 0;
	public:
		AlbumsManager (Util::SvcAuth::VkAuthManager*, Util::QueueManager*, ICoreProxy_ptr, QObject* = 0);
		AlbumsManager (qlonglong, Util::SvcAuth::VkAuthManager*, Util::QueueManager*, ICoreProxy_ptr, QObject* = 0);
		~AlbumsManager ();

		QStandardItem* GetRootItem () const;
		qlonglong GetUserID () const;
		quint32 GetTracksCount () const;

		QStandardItem* RefreshItems (const QList<QStandardItem*>&);
	private:
		bool HandleAlbums (const QVariant&);
		bool HandleTracks (const QVariant&);
	public slots:
		void refetchAlbums ();
		void handleAlbumsFetched ();
		void handleTracksFetched ();
	signals:
		void finished (AlbumsManager*);
	};
}
}
