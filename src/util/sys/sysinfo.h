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

#include <QPair>
#include "sysconfig.h"

class QString;

namespace LeechCraft
{
namespace Util
{
namespace SysInfo
{
	/** @brief Returns a string of OS name and version joined together.
	 *
	 * @return The name and version of OS running LeechCraft.
	 */
	UTIL_SYS_API QString GetOSName ();

	struct OSInfo
	{
		QString Name_;
		QString Version_;

		QString Flavour_;

		UTIL_SYS_API OSInfo (const QString& name, const QString& version);

		UTIL_SYS_API OSInfo (const QString& flavour, const QString& name, const QString& version);
	};

	/** @brief Returns more precise information about OS name and version.
	 *
	 * @return A structure OSInfo consisting of OS name, version and other
	 * fields.
	 */
	UTIL_SYS_API OSInfo GetOSInfo ();
}
}
}
