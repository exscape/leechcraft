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

#include "rosenthal.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"
#include "knowndictsmanager.h"
#include "checker.h"

namespace LeechCraft
{
namespace Rosenthal
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("rosenthal");

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"rosenthalsettings.xml");

		connect (SettingsDialog_.get (),
				SIGNAL (pushButtonClicked (QString)),
				this,
				SLOT (handlePushButtonClicked (QString)));

		KnownMgr_ = new KnownDictsManager;
		SettingsDialog_->SetDataSource ("Dictionaries", KnownMgr_->GetModel ());
		SettingsDialog_->SetDataSource ("PrimaryLanguage", KnownMgr_->GetEnabledModel ());
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Rosenthal";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Rosenthal";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Spellchecker service module for other plugins to use.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	ISpellChecker_ptr Plugin::CreateSpellchecker ()
	{
		return std::shared_ptr<Checker> (new Checker (KnownMgr_));
	}

	void Plugin::handlePushButtonClicked (const QString& name)
	{
		if (name != "InstallDicts")
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown button"
					<< name;
			return;
		}

		auto e = Util::MakeEntity ("ListPackages",
				QString (),
				FromUserInitiated,
				"x-leechcraft/package-manager-action");
		e.Additional_ ["Tags"] = QStringList ("dicts");

		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_rosenthal, LeechCraft::Rosenthal::Plugin);
