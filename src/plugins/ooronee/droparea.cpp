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

#include "droparea.h"
#include <QCursor>
#if QT_VERSION < 0x050000
#include <QGraphicsSceneDragDropEvent>
#endif
#include <QMimeData>

namespace LeechCraft
{
namespace Ooronee
{
#if QT_VERSION < 0x050000
	DropArea::DropArea (QDeclarativeItem *parent)
	: QDeclarativeItem { parent }
#else
	DropArea::DropArea (QQuickItem *parent)
	: QQuickItem { parent }
#endif
	{
		SetAcceptingDrops (true);
	}

	bool DropArea::GetAcceptingDrops () const
	{
#if QT_VERSION < 0x050000
		return acceptDrops ();
#else
		return flags () & ItemAcceptsDrops;
#endif
	}

	void DropArea::SetAcceptingDrops (bool accepting)
	{
		if (accepting == GetAcceptingDrops ())
			return;

#if QT_VERSION < 0x050000
		setAcceptDrops (accepting);
#else
		setFlag (ItemAcceptsDrops, accepting);
#endif
		emit acceptingDropsChanged (accepting);
	}

#if QT_VERSION < 0x050000
	void DropArea::dragEnterEvent (QGraphicsSceneDragDropEvent *event)
#else
	void DropArea::dragEnterEvent (QDragEnterEvent *event)
#endif
	{
		auto data = event->mimeData ();
		if (!(data->hasImage () || data->hasText ()))
			return;

		event->acceptProposedAction ();
		setCursor (Qt::DragCopyCursor);

		emit dragEntered (data->hasImage () ?
				data->imageData () :
				data->text ());
	}

#if QT_VERSION < 0x050000
	void DropArea::dragLeaveEvent (QGraphicsSceneDragDropEvent*)
#else
	void DropArea::dragLeaveEvent (QDragLeaveEvent*)
#endif
	{
		unsetCursor ();
		emit dragLeft ();
	}

#if QT_VERSION < 0x050000
	void DropArea::dropEvent (QGraphicsSceneDragDropEvent *event)
#else
	void DropArea::dropEvent (QDropEvent *event)
#endif
	{
		unsetCursor ();

		const auto data = event->mimeData ();
		emit dataDropped (data->hasImage () ?
				data->imageData () :
				data->text ());
	}
}
}
