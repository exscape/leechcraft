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

#include "mailtab.h"
#include <QToolBar>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QFileDialog>
#include <QToolButton>
#include <util/util.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "storage.h"
#include "mailtreedelegate.h"
#include "mailmodel.h"
#include "viewcolumnsmanager.h"
#include "accountfoldermanager.h"
#include "vmimeconversions.h"

namespace LeechCraft
{
namespace Snails
{
	MailTab::MailTab (const TabClassInfo& tc, QObject *pmt, QWidget *parent)
	: QWidget (parent)
	, TabToolbar_ (new QToolBar)
	, MsgToolbar_ (new QToolBar)
	, TabClass_ (tc)
	, PMT_ (pmt)
	, MailSortFilterModel_ (new QSortFilterProxyModel (this))
	{
		Ui_.setupUi (this);
		//Ui_.MailTreeLay_->insertWidget (0, MsgToolbar_);

		Ui_.MailView_->settings ()->setAttribute (QWebSettings::DeveloperExtrasEnabled, true);

		auto colMgr = new ViewColumnsManager (Ui_.MailTree_->header ());
		colMgr->SetStretchColumn (1);
		colMgr->SetDefaultWidths ({
				"Typical sender name and surname",
				{},
				QDateTime::currentDateTime ().toString (),
				Util::MakePrettySize (999 * 1024) + "  "
			});

		Ui_.AccountsTree_->setModel (Core::Instance ().GetAccountsModel ());

		MailSortFilterModel_->setDynamicSortFilter (true);
		MailSortFilterModel_->setSortRole (MailModel::MailRole::Sort);
		MailSortFilterModel_->sort (2, Qt::DescendingOrder);
		Ui_.MailTree_->setItemDelegate (new MailTreeDelegate (this));
		Ui_.MailTree_->setModel (MailSortFilterModel_);

		connect (Ui_.AccountsTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleCurrentAccountChanged (QModelIndex)));
		connect (Ui_.MailTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleMailSelected (QModelIndex)));

		FillTabToolbarActions ();
	}

	TabClassInfo MailTab::GetTabClassInfo () const
	{
		return TabClass_;
	}

	QObject* MailTab::ParentMultiTabs ()
	{
		return PMT_;
	}

	void MailTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* MailTab::GetToolBar () const
	{
		return TabToolbar_;
	}

	void MailTab::FillTabToolbarActions ()
	{
		QAction *fetch = new QAction (tr ("Fetch new mail"), this);
		fetch->setProperty ("ActionIcon", "mail-receive");
		TabToolbar_->addAction (fetch);
		connect (fetch,
				SIGNAL (triggered ()),
				this,
				SLOT (handleFetchNewMail ()));
		TabToolbar_->addAction (fetch);

		TabToolbar_->addSeparator ();

		MsgReply_ = new QAction (tr ("Reply..."), this);
		MsgReply_->setProperty ("ActionIcon", "mail-reply-sender");
		connect (MsgReply_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleReply ()));
		TabToolbar_->addAction (MsgReply_);

		MsgAttachments_ = new QMenu (tr ("Attachments"));
		MsgAttachments_->setIcon (Core::Instance ().GetProxy ()->
					GetIconThemeManager ()->GetIcon ("mail-attachment"));
		TabToolbar_->addAction (MsgAttachments_->menuAction ());

		TabToolbar_->addSeparator ();

		MsgCopy_ = new QMenu (tr ("Copy messages"));
		connect (MsgCopy_,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleCopyMessages (QAction*)));

		MsgCopyButton_ = new QToolButton;
		MsgCopyButton_->setMenu (MsgCopy_);
		MsgCopyButton_->setProperty ("ActionIcon", "edit-copy");
		MsgCopyButton_->setPopupMode (QToolButton::InstantPopup);
		TabToolbar_->addWidget (MsgCopyButton_);

		MsgMarkUnread_ = new QAction (tr ("Mark as unread"), this);
		MsgMarkUnread_->setProperty ("ActionIcon", "mail-mark-unread");
		connect (MsgMarkUnread_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMarkMsgUnread ()));
		TabToolbar_->addAction (MsgMarkUnread_);

		MsgRemove_ = new QAction (tr ("Delete messages"), this);
		MsgRemove_->setProperty ("ActionIcon", "list-remove");
		connect (MsgRemove_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleRemoveMsgs ()));
		TabToolbar_->addAction (MsgRemove_);

		SetMsgActionsEnabled (false);
	}

	QList<QByteArray> MailTab::GetSelectedIds () const
	{
		const auto& rows = Ui_.MailTree_->selectionModel ()->selectedRows ();

		QList<QByteArray> ids;
		for (const auto& index : rows)
			ids << index.data (MailModel::MailRole::ID).toByteArray ();

		const auto& currentId = Ui_.MailTree_->currentIndex ()
				.data (MailModel::MailRole::ID).toByteArray ();
		if (!currentId.isEmpty () && !ids.contains (currentId))
			ids << currentId;

		return ids;
	}

	void MailTab::SetMsgActionsEnabled (bool enable)
	{
		for (auto act : { MsgReply_, MsgMarkUnread_, MsgRemove_ })
			act->setEnabled (enable);
		MsgCopyButton_->setEnabled (enable);
	}

	QList<Folder> MailTab::GetActualFolders () const
	{
		if (!CurrAcc_)
			return {};

		auto folders = CurrAcc_->GetFolderManager ()->GetFolders ();
		return folders;
	}

	void MailTab::handleCurrentAccountChanged (const QModelIndex& idx)
	{
		if (CurrAcc_)
		{
			disconnect (CurrAcc_.get (),
					0,
					this,
					0);
			disconnect (CurrAcc_->GetFolderManager (),
					0,
					this,
					0);
		}

		CurrAcc_ = Core::Instance ().GetAccount (idx);
		handleFoldersUpdated ();

		if (!CurrAcc_)
			return;

		connect (CurrAcc_.get (),
				SIGNAL (messageBodyFetched (Message_ptr)),
				this,
				SLOT (handleMessageBodyFetched (Message_ptr)));

		MailSortFilterModel_->setSourceModel (CurrAcc_->GetMailModel ());
		MailSortFilterModel_->setDynamicSortFilter (true);

		if (Ui_.TagsTree_->selectionModel ())
			Ui_.TagsTree_->selectionModel ()->deleteLater ();
		Ui_.TagsTree_->setModel (CurrAcc_->GetFoldersModel ());

		connect (Ui_.TagsTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleCurrentTagChanged (QModelIndex)));

		const auto fm = CurrAcc_->GetFolderManager ();
		connect (fm,
				SIGNAL (foldersUpdated ()),
				this,
				SLOT (handleFoldersUpdated ()));
	}

	namespace
	{
		QString HTMLize (const QList<QPair<QString, QString>>& adds)
		{
			QStringList result;

			for (const auto& pair : adds)
			{
				const bool hasName = !pair.first.isEmpty ();

				QString thisStr;

				if (hasName)
					thisStr += "<span style='address_name'>" + pair.first + "</span> &lt;";

				thisStr += QString ("<span style='address_email'><a href='mailto:%1'>%1</a></span>")
						.arg (pair.second);

				if (hasName)
					thisStr += '>';

				result << thisStr;
			}

			return result.join (", ");
		}
	}

	void MailTab::handleCurrentTagChanged (const QModelIndex& sidx)
	{
		CurrAcc_->ShowFolder (sidx);
		handleMailSelected ({});
	}

	namespace
	{
		QString GetStyle ()
		{
			const auto& palette = qApp->palette ();

			auto result = Core::Instance ().GetMsgViewTemplate ();
			result.replace ("$WindowText", palette.color (QPalette::ColorRole::WindowText).name ());
			result.replace ("$Window", palette.color (QPalette::ColorRole::Window).name ());
			result.replace ("$Base", palette.color (QPalette::ColorRole::Base).name ());
			result.replace ("$Text", palette.color (QPalette::ColorRole::Text).name ());
			return result;
		}
	}

	void MailTab::handleMailSelected (const QModelIndex& sidx)
	{
		if (!CurrAcc_)
		{
			SetMsgActionsEnabled (false);
			Ui_.MailView_->setHtml ({});
			return;
		}

		CurrMsg_.reset ();

		if (!sidx.isValid ())
		{
			SetMsgActionsEnabled (false);
			Ui_.MailView_->setHtml ({});
			return;
		}

		const auto& idx = MailSortFilterModel_->mapToSource (sidx);
		const auto& id = idx.sibling (idx.row (), 0).data (MailModel::MailRole::ID).toByteArray ();
		const auto& folder = CurrAcc_->GetMailModel ()->GetCurrentFolder ();

		Message_ptr msg;
		try
		{
			msg = Core::Instance ().GetStorage ()->LoadMessage (CurrAcc_.get (), folder, id);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to load message"
					<< CurrAcc_->GetID ().toHex ()
					<< id.toHex ()
					<< e.what ();

			SetMsgActionsEnabled (false);
			const QString& html = tr ("<h2>Unable to load mail</h2><em>%1</em>").arg (e.what ());
			Ui_.MailView_->setHtml (html);
			return;
		}

		SetMsgActionsEnabled (true);

		if (!msg->IsFullyFetched ())
			CurrAcc_->FetchWholeMessage (msg);
		else if (!msg->IsRead ())
			CurrAcc_->SetReadStatus (true, { id }, folder);

		QString html = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">";
		html += "<html xmlns='http://www.w3.org/1999/xhtml'><head><title>Message</title><style>";
		html += GetStyle ();
		html += "</style></head><body><div class='header'>";
		auto addField = [&html] (const QString& cssClass, const QString& name, const QString& text)
		{
			if (!text.isEmpty ())
				html += "<span class='field " + cssClass + "'><span class='fieldName'>" +
						name + "</span>: " + text + "</span><br />";
		};

		addField ("subject", tr ("Subject"), msg->GetSubject ());
		addField ("from", tr ("From"), HTMLize ({ msg->GetAddress (Message::Address::From) }));
		addField ("to", tr ("To"), HTMLize (msg->GetAddresses (Message::Address::To)));
		addField ("cc", tr ("Copy"), HTMLize (msg->GetAddresses (Message::Address::Cc)));
		addField ("bcc", tr ("Blind copy"), HTMLize (msg->GetAddresses (Message::Address::Bcc)));
		addField ("date", tr ("Date"), msg->GetDate ().toString ());

		const auto& htmlBody = msg->IsFullyFetched () ?
				msg->GetHTMLBody () :
				"<em>" + tr ("Fetching the message...") + "</em>";

		html += "</div><div class='body'>";
		html += htmlBody.isEmpty () ?
				"<pre>" + Qt::escape (msg->GetBody ()) + "</pre>" :
				htmlBody;
		html += "</div>";
		html += "</body></html>";

		Ui_.MailView_->setHtml (html);

		MsgAttachments_->clear ();
		MsgAttachments_->setEnabled (!msg->GetAttachments ().isEmpty ());
		for (const auto& att : msg->GetAttachments ())
		{
			const auto& name = att.GetName () + " (" + Util::MakePrettySize (att.GetSize ()) + ")";
			const auto act = MsgAttachments_->addAction (name,
					this,
					SLOT (handleAttachment ()));
			act->setProperty ("Snails/MsgId", id);
			act->setProperty ("Snails/AttName", att.GetName ());
			act->setProperty ("Snails/Folder", folder);
		}

		CurrMsg_ = msg;
	}

	void MailTab::handleFoldersUpdated ()
	{
		MsgCopy_->clear ();

		if (!CurrAcc_)
			return;

		for (const auto& folder : GetActualFolders ())
		{
			const auto& icon = GetFolderIcon (folder.Type_);
			const auto act = MsgCopy_->addAction (icon, folder.Path_.join ("/"));
			act->setProperty ("Snails/FolderPath", folder.Path_);
		}
	}

	void MailTab::handleReply ()
	{
		if (!CurrAcc_ || !CurrMsg_)
			return;

		Core::Instance ().PrepareReplyTab (CurrMsg_, CurrAcc_);
	}

	void MailTab::handleCopyMessages (QAction *action)
	{
		if (!CurrAcc_)
			return;

		const auto& folderPath = action->property ("Snails/FolderPath").toStringList ();
		if (folderPath.isEmpty ())
			return;

		const auto& ids = GetSelectedIds ();
		CurrAcc_->CopyMessages (ids, CurrAcc_->GetMailModel ()->GetCurrentFolder (), { folderPath });
	}

	void MailTab::handleMarkMsgUnread ()
	{
		if (!CurrAcc_)
			return;

		const auto& ids = GetSelectedIds ();
		CurrAcc_->SetReadStatus (false, ids, CurrAcc_->GetMailModel ()->GetCurrentFolder ());
	}

	void MailTab::handleRemoveMsgs ()
	{
		if (!CurrAcc_)
			return;

		const auto& ids = GetSelectedIds ();
		CurrAcc_->DeleteMessages (ids, CurrAcc_->GetMailModel ()->GetCurrentFolder ());
	}

	void MailTab::handleAttachment ()
	{
		if (!CurrAcc_)
			return;

		const auto& name = sender ()->property ("Snails/AttName").toString ();

		const auto& path = QFileDialog::getSaveFileName (0,
				tr ("Save attachment"),
				QDir::homePath () + '/' + name);
		if (path.isEmpty ())
			return;

		const auto& id = sender ()->property ("Snails/MsgId").toByteArray ();
		const auto& folder = sender ()->property ("Snails/Folder").toStringList ();

		const auto& msg = Core::Instance ().GetStorage ()->LoadMessage (CurrAcc_.get (), folder, id);
		CurrAcc_->FetchAttachment (msg, name, path);
	}

	void MailTab::handleFetchNewMail ()
	{
		Storage *st = Core::Instance ().GetStorage ();
		for (auto acc : Core::Instance ().GetAccounts ())
			acc->Synchronize (st->HasMessagesIn (acc.get ()) ?
						Account::FetchNew:
						Account::FetchAll);
	}

	void MailTab::handleMessageBodyFetched (Message_ptr msg)
	{
		const QModelIndex& cur = Ui_.MailTree_->currentIndex ();
		if (cur.data (MailModel::MailRole::ID).toByteArray () != msg->GetFolderID ())
			return;

		handleMailSelected (cur);
	}
}
}
