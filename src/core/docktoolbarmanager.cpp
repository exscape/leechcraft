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

#include "docktoolbarmanager.h"
#include <QToolBar>
#include <QDockWidget>
#include <QActionGroup>
#include <QTimer>
#include <util/defaulthookproxy.h>
#include "mainwindow.h"
#include "core.h"
#include "coreinstanceobject.h"
#include "coreplugin2manager.h"
#include "dockmanager.h"

namespace LeechCraft
{
	DockToolbarManager::DockToolbarManager (MainWindow *win, DockManager *dock)
	: QObject (win)
	, Win_ (win)
	, DockManager_ (dock)
	{
		auto instanceObj = Core::Instance ().GetCoreInstanceObject ();
		instanceObj->GetCorePluginManager ()->RegisterHookable (this);

		auto init = [this, win] (Qt::DockWidgetArea area) -> void
		{
			auto bar = win->GetDockListWidget (area);
			bar->setFloatable (false);
			bar->setMovable (false);
			bar->hide ();
			bar->setToolButtonStyle (Qt::ToolButtonIconOnly);
			Area2Info_ [area].Bar_ = bar;
		};
		init (Qt::LeftDockWidgetArea);
		init (Qt::RightDockWidgetArea);
		init (Qt::TopDockWidgetArea);
		init (Qt::BottomDockWidgetArea);
	}

	void DockToolbarManager::AddDock (QDockWidget *dw, Qt::DockWidgetArea area)
	{
		const auto& areaInfo = Area2Info_ [area];

		auto bar = areaInfo.Bar_;
		if (!bar)
			return;

		auto toggleAct = dw->toggleViewAction ();
		if (bar->actions ().contains (toggleAct))
		{
			qWarning () << Q_FUNC_INFO
					<< "double-adding"
					<< dw
					<< dw->windowTitle ();
			return;
		}

		emit hookAddingDockAction (IHookProxy_ptr (new Util::DefaultHookProxy),
				Win_, toggleAct, area);
		bar->addAction (toggleAct);
		if (bar->actions ().size () >= 2)
		{
			Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
			emit hookDockBarWillBeShown (proxy, Win_, bar, area);
			if (!proxy->IsCancelled ())
				bar->show ();
		}

		Action2Widget_ [toggleAct] = dw;

		connect (dw,
				SIGNAL (dockLocationChanged (Qt::DockWidgetArea)),
				this,
				SLOT (updateDockLocation (Qt::DockWidgetArea)));
		connect (toggleAct,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleActionToggled (bool)));

		if (toggleAct->isChecked ())
		{
			UpdateActionGroup (toggleAct, true);
			QTimer::singleShot (0, dw, SLOT (show ()));
		}
	}

	void DockToolbarManager::RemoveDock (QDockWidget *dw)
	{
		auto toggleAct = dw->toggleViewAction ();
		disconnect (dw,
				SIGNAL (dockLocationChanged (Qt::DockWidgetArea)),
				this,
				SLOT (updateDockLocation (Qt::DockWidgetArea)));
		disconnect (toggleAct,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleActionToggled (bool)));

		HandleDockDestroyed (dw, toggleAct);
	}

	/* Both dw and act can be already dead and gone here.
	 */
	void DockToolbarManager::HandleDockDestroyed (QDockWidget *dw, QAction *act)
	{
		IHookProxy_ptr proxy (new Util::DefaultHookProxy);
		for (auto& info : Area2Info_)
		{
			if (info.Bar_->actions ().contains (act))
			{
				emit hookRemovingDockAction (proxy, Win_, act, info.Area_);
				info.Bar_->removeAction (act);
			}

			auto remaining = info.Bar_->actions ();
			if (remaining.size () < 2)
				info.Bar_->hide ();

			QList<QAction*> forceActions;
			for (auto dock : DockManager_->GetForcefullyClosed ())
				forceActions << dock->toggleViewAction ();

			for (auto i = remaining.begin (); i != remaining.end (); )
				if (forceActions.contains (*i))
					i = remaining.erase (i);
				else
					++i;

			if (!remaining.isEmpty ())
				QTimer::singleShot (0,
						Action2Widget_ [remaining.last ()],
						SLOT (show ()));
		}

		Action2Widget_.remove (act);
	}

	void DockToolbarManager::UpdateActionGroup (QAction *action, bool enabled)
	{
		if (!enabled)
			return;

		const auto& assocs = action->associatedWidgets ();

		for (auto& info : Area2Info_)
		{
			if (!assocs.contains (info.Bar_))
				continue;

			for (auto otherAct : info.Bar_->actions ())
				if (otherAct != action && otherAct->isChecked ())
				{
					auto dw = Action2Widget_ [otherAct];
					dw->hide ();
				}
		}
	}

	void DockToolbarManager::updateDockLocation (Qt::DockWidgetArea area)
	{
		auto dw = qobject_cast<QDockWidget*> (sender ());
		RemoveDock (dw);
		AddDock (dw, area);
	}

	void DockToolbarManager::handleActionToggled (bool enabled)
	{
		UpdateActionGroup (qobject_cast<QAction*> (sender ()), enabled);
	}
}
