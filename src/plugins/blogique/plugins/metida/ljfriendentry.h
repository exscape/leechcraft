/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include <memory>
#include <QObject>
#include <QUrl>
#include <QDateTime>
#include <QColor>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJFriendEntry;
	typedef std::shared_ptr<LJFriendEntry> LJFriendEntry_ptr;

	class LJFriendEntry : public QObject
	{
		Q_OBJECT

		QUrl AvatarUrl_;
		QString FullName_;
		QString UserName_;
		uint GroupMask_;
		QColor BGColor_;
		QColor FGColor_;
		QString Birthday_;
		bool FriendOf_;
		bool MyFriend_;
	public:
		LJFriendEntry (QObject *parent = 0);

		void SetAvatarUrl (const QUrl& url);
		QUrl GetAvatarurl () const;
		void SetFullName (const QString& fullName);
		QString GetFullName () const;
		void SetUserName (const QString& userName);
		QString GetUserName () const;
		void SetGroupMask (int groupmask);
		uint GetGroupMask () const;
		void SetBGColor (const QString& name);
		QColor GetBGColor () const;
		void SetFGColor (const QString& name);
		QColor GetFGColor () const;
		void SetBirthday (const QString& date);
		QString GetBirthday () const;
		void SetFriendOf (bool friendOf);
		bool GetFriendOf () const;
		void SetMyFriend (bool myFriend);
		bool GetMyFriend () const;

		QByteArray Serialize () const;
		static LJFriendEntry_ptr Deserialize (const QByteArray& data);
		
		bool operator== (const LJFriendEntry& entry) const;
	};
}
}
}
