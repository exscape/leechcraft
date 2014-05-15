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

#pragma once

#include <QPushButton>
#include <QColor>
#include "guiconfig.h"

namespace LeechCraft
{
namespace Util
{
	/** @brief A button for choosing a color.
	 *
	 * This class provides a button that can be used to choose a color.
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API ColorButton : public QPushButton
	{
		Q_OBJECT

		QColor Color_;
	public:
		/** @brief Constructs the button with the given \em parent.
		 *
		 * @param[in] parent The parent widget for the button.
		 */
		ColorButton (QWidget *parent = 0);

		/** @brief Returns the current color represented by this button.
		 *
		 * The default value for the color is black.
		 *
		 * @return The currently set color.
		 *
		 * @sa SetColor()
		 */
		QColor GetColor () const;

		/** @brief Sets the color represented by this button.
		 *
		 * Sets the \em color of this button and emits the colorChanged()
		 * signal if the color has been changed.
		 *
		 * @param[in] color The new color to be represented by this
		 * button.
		 *
		 * @sa SetColor(), colorChanged()
		 */
		void SetColor (const QColor& color);
	private slots:
		void handleSelector ();
	signals:
		/** @brief Emitted when the color is changed.
		 *
		 * @param[out] color The new color represented by this button.
		 *
		 * @sa SetColor()
		 */
		void colorChanged (const QColor& color);
	};
}
}