/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_SIDEBAR_SBWIDGET_H
#define PLUGINS_SIDEBAR_SBWIDGET_H
#include <QWidget>
#include "ui_sbwidget.h"

class QToolButton;

namespace LeechCraft
{
namespace Sidebar
{
	class SBWidget : public QWidget
	{
		Q_OBJECT

		Ui::SBWidget Ui_;
		const QSize IconSize_;

		QMap<QAction*, QToolButton*> CurTab2Button_;
	public:
		SBWidget (QWidget* = 0);

		void AddTabOpenAction (QAction*);
		void AddQLAction (QAction*);
		void AddCurTabAction (QAction*);
		void RemoveCurTabAction (QAction*);
	};
}
}

#endif
