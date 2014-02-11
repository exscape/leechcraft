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

#include "serverhistorywidget.h"
#include <QtDebug>
#include <interfaces/azoth/ihaveserverhistory.h>
#include "proxyobject.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	ServerHistoryWidget::ServerHistoryWidget (QObject *account, QWidget *parent)
	: QWidget { parent }
	, Toolbar_ { new QToolBar { this } }
	, AccObj_ { account }
	, IHSH_ { qobject_cast<IHaveServerHistory*> (account) }
	{
		Ui_.setupUi (this);

		if (!IHSH_)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IHaveServerHistory"
					<< account;
			return;
		}

		Ui_.ContactsView_->setModel (IHSH_->GetServerContactsModel ());

		connect (AccObj_,
				SIGNAL (serverHistoryFetched (QModelIndex, int, SrvHistMessages_t)),
				this,
				SLOT (handleFetched (QModelIndex, int, SrvHistMessages_t)));

		auto prevAct = Toolbar_->addAction (tr ("Previous page"),
				this, SLOT (navigatePrevious ()));
		prevAct->setProperty ("ActionIcon", "go-previous");

		auto nextAct = Toolbar_->addAction (tr ("Next page"),
				this, SLOT (navigateNext ()));
		nextAct->setProperty ("ActionIcon", "go-next");
	}

	void ServerHistoryWidget::SetTabInfo (QObject *plugin, const TabClassInfo& tc)
	{
		PluginObj_ = plugin;
		TC_ = tc;
	}

	TabClassInfo ServerHistoryWidget::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* ServerHistoryWidget::ParentMultiTabs ()
	{
		return PluginObj_;
	}

	void ServerHistoryWidget::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* ServerHistoryWidget::GetToolBar () const
	{
		return Toolbar_;
	}

	int ServerHistoryWidget::GetReqMsgCount () const
	{
		return std::max (20, FirstMsgCount_);
	}

	void ServerHistoryWidget::handleFetched (const QModelIndex& index, int offset, const SrvHistMessages_t& messages)
	{
		if (index != Ui_.ContactsView_->currentIndex ())
			return;

		if (FirstMsgCount_ == -1)
			FirstMsgCount_ = messages.size ();

		Ui_.MessagesView_->clear ();

		const auto& bgColor = palette ().color (QPalette::Base);
		const auto& colors = Core::Instance ().GenerateColors ("hash", bgColor);

		QString preNick = XmlSettingsManager::Instance ().property ("PreNickText").toString ();
		QString postNick = XmlSettingsManager::Instance ().property ("PostNickText").toString ();
		preNick.replace ('<', "&lt;");
		postNick.replace ('<', "&lt;");

		for (const auto& message : messages)
		{
			const auto& color = Core::Instance ().GetNickColor (message.Nick_, colors);

			auto msgText = message.Body_;
			ProxyObject {}.FormatLinks (msgText);
			msgText.replace ('\n', "<br/>");

			QString html = "[" + message.TS_.toString () + "] " + preNick;
			html += "<font color='" + color + "'>" + message.Nick_ + "</font> ";
			html += postNick + ' ' + msgText;

			html.prepend (QString ("<font color='#") +
					(message.Dir_ == IMessage::DIn ? "0000dd" : "dd0000") +
					"'>");
			html += "</font>";

			Ui_.MessagesView_->append (html);
		}
	}

	void ServerHistoryWidget::on_ContactsView__activated (const QModelIndex& index)
	{
		CurrentOffset_ = 0;
		FirstMsgCount_ = -1;
		IHSH_->FetchServerHistory (index, CurrentOffset_, 50);
	}

	void ServerHistoryWidget::navigatePrevious ()
	{
		CurrentOffset_ += GetReqMsgCount ();
		IHSH_->FetchServerHistory (Ui_.ContactsView_->currentIndex (),
				CurrentOffset_, GetReqMsgCount ());
	}

	void ServerHistoryWidget::navigateNext ()
	{
		CurrentOffset_ = std::max (CurrentOffset_ - GetReqMsgCount (), 0);
		IHSH_->FetchServerHistory (Ui_.ContactsView_->currentIndex (),
				CurrentOffset_, GetReqMsgCount ());
	}
}
}