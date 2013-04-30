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

#pragma once

#include <QWizard>
#include <interfaces/core/icoreproxy.h>

class QAuthenticator;
class QNetworkReply;
class QNetworkAccessManager;

namespace LeechCraft
{
struct Entity;

namespace Dolozhee
{
	class ChooseUserPage;
	class ReportTypePage;
	class BugReportPage;
	class FeatureRequestPage;

	class ReportWizard : public QWizard
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		QNetworkAccessManager *NAM_;
		ChooseUserPage *ChooseUser_;
		ReportTypePage *ReportType_;
		BugReportPage *BugReportPage_;
		FeatureRequestPage *FRPage_;
		bool FirstAuth_;
	public:
		enum PageID
		{
			ChooseUser,
			UserStatus,
			ReportType,
			BugDetails,
			FeatureDetails,
			Final
		};

		ReportWizard (ICoreProxy_ptr, QWidget* = 0);

		QNetworkAccessManager* GetNAM () const;
		QNetworkReply* PostRequest (const QString&, const QByteArray&, const QByteArray& = "application/xml");

		ChooseUserPage* GetChooseUserPage () const;
		ReportTypePage* GetReportTypePage () const;
		BugReportPage* GetBugReportPage () const;
		FeatureRequestPage* GetFRPage () const;
	private slots:
		void handleAuthenticationRequired (QNetworkReply*, QAuthenticator*);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
