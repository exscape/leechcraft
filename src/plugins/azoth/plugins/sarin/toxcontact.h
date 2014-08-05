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

#include <QObject>
#include <interfaces/azoth/iclentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Sarin
{
	class ToxAccount;

	class ToxContact : public QObject
					 , public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry)

		const QByteArray Pubkey_;
		ToxAccount * const Acc_;

		QString PublicName_;

		EntryStatus Status_;
	public:
		ToxContact (const QByteArray& pubkey, ToxAccount *account);

		QObject* GetQObject () override;
		QObject* GetParentAccount () const override;
		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;

		QString GetEntryName () const override;
		void SetEntryName (const QString& name) override;
		QString GetEntryID () const override;
		QString GetHumanReadableID () const override;
		QStringList Groups () const override;
		void SetGroups (const QStringList& groups) override;

		QStringList Variants () const override;

		QObject* CreateMessage (IMessage::Type, const QString&, const QString&) override;
		QList<QObject*> GetAllMessages () const override;
		void PurgeMessages (const QDateTime&) override;

		void SetChatPartState (ChatPartState, const QString&) override;

		EntryStatus GetStatus (const QString&) const override;
		QImage GetAvatar () const override;
		void ShowInfo () override;
		QList<QAction*> GetActions () const override;
		QMap<QString, QVariant> GetClientInfo (const QString&) const override;

		void MarkMsgsRead () override;
		void ChatTabClosed () override;

		void SetStatus (const EntryStatus&);
	signals:
		void gotMessage (QObject*) override;
		void statusChanged (const EntryStatus&, const QString&) override;
		void availableVariantsChanged (const QStringList&) override;
		void avatarChanged (const QImage&) override;
		void nameChanged (const QString&) override;
		void groupsChanged (const QStringList&) override;
		void chatPartStateChanged (const ChatPartState&, const QString&) override;
		void permsChanged () override;
		void entryGenerallyChanged () override;
	};
}
}
}