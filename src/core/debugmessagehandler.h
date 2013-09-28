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

#ifndef DEBUGMESSAGEHANDLER_H
#define DEBUGMESSAGEHANDLER_H
#include <QtGlobal>
#include <QMutex>

namespace DebugHandler
{
	/** Simple debug message handler. Writes the message of type type
	 * to the logs. The logs are contained in ~/.leechcraft and have
	 * following names for different message types:
	 * - QtDebugMsg -> debug.log
	 * - QtWarningMsg -> warning.log
	 * - QtCriticalMsg -> critical.log
	 * - QtFatalMsg -> fatal.log
	 *
	 * @param[in] type The type of the message.
	 * @param[in] message The message to print.
	 *
	 * @sa backtraced
	 */
#ifndef USE_QT5
	void simple (QtMsgType type, const char *message);
#else
	void simple (QtMsgType type, const QMessageLogContext& context, const QString& msg);
#endif
	/** Debug message handler which prints backtraces for all messages
	 * except QtDebugMsg ones. This is the only difference from the
	 * simple() debug message handler. Refer to simple() documentation
	 * for more information.
	 *
	 * @param[in] type The type of the message.
	 * @param[in] message The message to print.
	 *
	 * @sa simple
	 */
#ifndef USE_QT5
	void backtraced (QtMsgType type, const char *message);
#else
	void backtraced (QtMsgType type, const QMessageLogContext& context, const QString& msg);
#endif
};

#endif

