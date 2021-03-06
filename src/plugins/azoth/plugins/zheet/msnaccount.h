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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_MSNACCOUNT_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_MSNACCOUNT_H
#include <QObject>
#include <QSet>
#include <interfaces/an/ianemitter.h>
#include <msn/passport.h>
#include <msn/util.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iextselfinfoaccount.h>

namespace MSN
{
	class NotificationServerConnection;
	class Buddy;
	class Message;
}

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class MSNProtocol;
	class Callbacks;
	class MSNAccountConfigWidget;
	class MSNBuddyEntry;
	class SBManager;
	class GroupManager;
	class TransferManager;

	class MSNAccount : public QObject
					 , public IAccount
					 , public IExtSelfInfoAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount LeechCraft::Azoth::IExtSelfInfoAccount)

		MSNProtocol *Proto_;

		QString Name_;
		MSN::Passport Passport_;
		QString Server_;
		int Port_;

		Callbacks *CB_;
		MSN::NotificationServerConnection *Conn_;
		SBManager *SB_;
		GroupManager *GroupManager_;
		TransferManager *TM_;

		EntryStatus PendingStatus_;
		bool Connecting_;
		EntryStatus CurrentStatus_;

		QHash<QString, MSNBuddyEntry*> Entries_;
		QHash<QString, MSNBuddyEntry*> CID2Entry_;

		QSet<QString> BL_;

		QString OurFriendlyName_;

		QAction *ActionManageBL_;
	public:
		MSNAccount (const QString&, MSNProtocol* = 0);
		void Init ();

		QByteArray Serialize () const;
		static MSNAccount* Deserialize (const QByteArray&, MSNProtocol*);

		void FillConfig (MSNAccountConfigWidget*);

		MSN::NotificationServerConnection* GetNSConnection () const;
		SBManager* GetSBManager () const;
		GroupManager* GetGroupManager () const;

		MSNBuddyEntry* GetBuddy (const QString&) const;
		MSNBuddyEntry* GetBuddyByCID (const QString&) const;

		QSet<QString> GetBL () const;
		void RemoveFromBL (const QString&);

		// IAccount
		QObject* GetQObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString& name);
		QByteArray GetAccountID () const;
		QList<QAction*> GetActions () const;
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&, const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;

		// IExtSelfInfoAccount
		QObject* GetSelfContact () const;
		QImage GetSelfAvatar () const;
		QIcon GetAccountIcon () const;
	private slots:
		void handleConnected ();
		void handleWeChangedState (State);
		void handleGotOurFriendlyName (const QString&);
		void handleBuddyChangedStatus (const QString&, State);
		void handleGotBuddies (const QList<MSN::Buddy*>&);
		void handleRemovedBuddy (const QString&, const QString&);
		void handleRemovedBuddy (MSN::ContactList, const QString&);
		void handleGotMessage (const QString&, MSN::Message*);
		void handleGotNudge (const QString&);

		void handleInitialEmailNotification (int, int);
		void handleNewEmailNotification (const QString&, const QString&);

		void handleManageBL ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void accountRenamed (const QString&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void mucInvitationReceived (const QVariantMap&,
				const QString&, const QString&);

		void accountSettingsChanged ();
	};
}
}
}

#endif
