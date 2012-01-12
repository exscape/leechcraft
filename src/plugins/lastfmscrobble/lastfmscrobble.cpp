/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "lastfmscrobble.h"
#include <QIcon>
#include <QByteArray>
#include <interfaces/core/icoreproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/entitytesthandleresult.h>
#include "lastfmsubmitter.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"lastfmscrobblesettings.xml");
		LFSubmitter_ = new LastFMSubmitter (this);
		
		QList<QByteArray> propNames;
		propNames << "lastfm.login";
		propNames << "lastfm.password";

		XmlSettingsManager::Instance ().RegisterObject (propNames,
				this, "handleSubmitterInit");
	}
	
	void Plugin::SecondInit ()
	{
	}
	
	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Lastfmscrobble";
	}
	
	QString Plugin::GetName () const
	{
		return "Lastfmscrobble";
	}
	
	QString Plugin::GetInfo () const
	{
		return tr ("Last.fm Scrobbler");
	}
	
	void Plugin::Release ()
	{
	}
	
	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
		
	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		return entity.Mime_ == "x-leechcraft/now-playing-track-info" ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}
	
	void Plugin::Handle (Entity entity)
	{
		if (entity.Entity_.toString () == "SUBMIT")
		{
			LFSubmitter_->submit();
			return;
		}
		
		MediaMeta meta;
		
		meta.Album_ =  entity.Additional_["Album"].toString ();
		meta.Artist_ = entity.Additional_["Artist"].toString ();
		meta.Date_ = entity.Additional_["Date"].toString ();
		meta.Genre_ = entity.Additional_["Genre"].toString ();
		meta.Length_ = entity.Additional_["Length"].toInt ();
		meta.Title_ = entity.Additional_["Title"].toString ();
		meta.TrackNumber_ = entity.Additional_["TrackNumber"].toInt ();
		
		LFSubmitter_->sendTrack(meta);
	}
		
	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}
	
	void Plugin::handleSubmitterInit ()
	{
		LFSubmitter_->SetUsername (XmlSettingsManager::Instance ()
				.property ("lastfm.login").toString ());
		LFSubmitter_->SetPassword (XmlSettingsManager::Instance ()
				.property ("lastfm.password").toString ());

		LFSubmitter_->Init (Proxy_->GetNetworkAccessManager ());
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_lastfmscrobble, LeechCraft::Lastfmscrobble::Plugin);