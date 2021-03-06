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

#include "birthdaynotifier.h"
#include <QTimer>
#include <QIcon>
#include <util/util.h>
#include <util/xpc/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/an/constants.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/imetainfoentry.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace BirthdayNotifier
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_birthdaynotifier");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothbirthdaynotifiersettings.xml");
		XmlSettingsManager::Instance ().RegisterObject ("NotifyNTimesPerDay", this, "notifyNTimesPerDaySettingsChanged");

		CheckTimer_ = new QTimer (this);

		connect (CheckTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkDates ()));

		notifyNTimesPerDaySettingsChanged ();
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (10000,
				this,
				SLOT (checkDates ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.BirthdayNotifier";
	}

	void Plugin::Release ()
	{
		CheckTimer_->stop ();
	}

	QString Plugin::GetName () const
	{
		return "Azoth Birthday Notifier";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Notifies about birthdays of your buddies.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/azoth/birthdaynotifier/resources/images/birthdaynotifier.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::NotifyBirthday (ICLEntry *entry, int days)
	{
		const auto& hrId = entry->GetEntryName ();

		const QString& notify = days ?
				tr ("%1 has birthday in %n day(s)!", 0, days).arg (hrId) :
				tr ("%1 has birthday today!").arg (hrId);
		auto e = Util::MakeNotification (tr ("Birthday reminder"), notify, PInfo_);

		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = GetUniqueID ();
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] = AN::CatOrganizer;
		e.Additional_ ["org.LC.AdvNotifications.EventID"] = "org.LC.Plugins.Azoth.BirthdayNotifier.Birthday/" + entry->GetEntryID ();
		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (hrId);

		e.Additional_ ["org.LC.AdvNotifications.EventType"] = AN::TypeOrganizerEventDue;
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.Count"] = 1;

		const auto& px = QPixmap::fromImage (entry->GetAvatar ());
		if (!px.isNull ())
			e.Additional_ ["NotificationPixmap"] = px;

		emit gotEntity (e);
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	void Plugin::checkDates ()
	{
		if (!XmlSettingsManager::Instance ().property ("NotifyEnabled").toBool ())
			return;

		const auto& ranges = XmlSettingsManager::Instance ()
				.property ("NotificationDays").toString ().split (',', QString::SkipEmptyParts);

		QList<int> allowedDays;
		for (const auto& range : ranges)
		{
			if (!range.contains ('-'))
			{
				bool ok = false;
				const int day = range.toInt (&ok);
				if (ok)
					allowedDays << day;
				continue;
			}

			const auto& ends = range.split ('-', QString::SkipEmptyParts);
			if (ends.size () != 2)
				continue;

			bool bOk = false, eOk = false;
			const int begin = ends.at (0).toInt (&bOk);
			const int end = ends.at (1).toInt (&eOk);
			if (!bOk || !eOk)
				continue;

			for (int i = begin; i <= end; ++i)
				allowedDays << i;
		}

		const auto& today = QDate::currentDate ();

		for (auto accObj : AzothProxy_->GetAllAccounts ())
		{
			auto acc = qobject_cast<IAccount*> (accObj);
			auto extSelf = qobject_cast<IExtSelfInfoAccount*> (accObj);
			for (auto entryObj : acc->GetCLEntries ())
			{
				auto entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry || entry->GetEntryType () != ICLEntry::EntryType::Chat)
					continue;

				if (extSelf && extSelf->GetSelfContact () == entryObj)
					continue;

				auto meta = qobject_cast<IMetaInfoEntry*> (entryObj);
				if (!meta)
					continue;

				auto dt = meta->GetMetaInfo (IMetaInfoEntry::DataField::BirthDate).toDate ();
				if (!dt.isValid ())
					continue;

				dt.setDate (today.year (), dt.month (), dt.day ());
				if (!dt.isValid ())
					continue;

				int days = today.daysTo (dt);
				if (days < 0)
				{
					dt.setDate (today.year () + 1, dt.month (), dt.day ());
					if (!dt.isValid ())
						continue;
					days = today.daysTo (dt);
				}

				if (allowedDays.contains (days))
					NotifyBirthday (entry, days);
			}
		}
	}

	void Plugin::notifyNTimesPerDaySettingsChanged ()
	{
		static const int kMSecPerDay = 24 * 60 * 60 * 1000;

		CheckTimer_->stop ();
		const int interval = XmlSettingsManager::Instance ().property ("NotifyNTimesPerDay").toInt ();
		const int timeOutMSec = kMSecPerDay / (interval ? interval : 1);
		CheckTimer_->start (timeOutMSec);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_birthdaynotifier, LeechCraft::Azoth::BirthdayNotifier::Plugin);
