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

#include "manager.h"
#include <QFile>
#include <QDataStream>
#include <QSslSocket>
#include <QtDebug>
#include <util/util.h>
#include <util/sys/paths.h>
#include "certsmodel.h"

namespace LeechCraft
{
namespace CertMgr
{
	Manager::Manager ()
	: SystemCertsModel_ { new CertsModel { this } }
	, LocalCertsModel_ { new CertsModel { this } }
	{
		const auto& systems = QSslSocket::systemCaCertificates ();
		QSet<QByteArray> serializedCerts;
		for (const auto& cert : systems)
		{
			const auto& pem = cert.toPem ();
			if (serializedCerts.contains (pem))
				continue;

			serializedCerts << pem;
			Defaults_ << cert;
		}

		LoadLocals ();
		LoadBlacklist ();

		for (const auto& cert : Blacklist_)
			SystemCertsModel_->SetBlacklisted (cert, true);

		RegenAllowed ();

		ResetSocketDefault ();

		SystemCertsModel_->ResetCerts (Defaults_);
		LocalCertsModel_->ResetCerts (Locals_);
	}

	int Manager::AddCerts (const QList<QSslCertificate>& certs)
	{
		int added = 0;
		for (const auto& cert : certs)
		{
			if (cert.isNull ())
				continue;

			if (Defaults_.contains (cert) || Locals_.contains (cert))
				continue;

			Locals_ << cert;
			LocalCertsModel_->AddCert (cert);
			++added;
		}

		if (!added)
			return 0;

		SaveLocals ();
		ResetSocketDefault ();

		return added;
	}

	void Manager::RemoveCert (const QSslCertificate& cert)
	{
		if (!Locals_.removeOne (cert))
			return;

		LocalCertsModel_->RemoveCert (cert);

		SaveLocals ();
		ResetSocketDefault ();
	}

	QAbstractItemModel* Manager::GetSystemModel () const
	{
		return SystemCertsModel_;
	}

	QAbstractItemModel* Manager::GetLocalModel () const
	{
		return LocalCertsModel_;
	}

	const QList<QSslCertificate>& Manager::GetLocalCerts () const
	{
		return Locals_;
	}

	const QList<QSslCertificate>& Manager::GetDefaultCerts () const
	{
		return Defaults_;
	}

	bool Manager::IsBlacklisted (const QSslCertificate& cert) const
	{
		return Blacklist_.contains (cert);
	}

	void Manager::ToggleBlacklist (const QSslCertificate& cert, bool blacklist)
	{
		if (cert.isNull ())
			return;

		if ((blacklist && Blacklist_.contains (cert)) ||
			(!blacklist && !Blacklist_.removeAll (cert)))
			return;

		if (blacklist)
			Blacklist_ << cert;

		SystemCertsModel_->SetBlacklisted (cert, blacklist);

		RegenAllowed ();
		ResetSocketDefault ();
	}

	void Manager::RegenAllowed ()
	{
		AllowedDefaults_.clear ();

		for (const auto& item : Defaults_)
			if (!Blacklist_.contains (item))
				AllowedDefaults_ << item;
	}

	void Manager::ResetSocketDefault ()
	{
		QSslSocket::setDefaultCaCertificates (AllowedDefaults_ + Locals_);
	}

	namespace
	{
		void Save (const QString& filename, const QList<QSslCertificate>& certs)
		{
			const auto& path = Util::CreateIfNotExists ("certmgr").absoluteFilePath (filename);

			QFile file { path };
			if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open file"
						<< path
						<< file.errorString ();
				return;
			}

			QDataStream stream { &file };
			for (const auto& cert : certs)
				stream << cert.toPem ();
		}

		void Load (const QString& filename, QList<QSslCertificate>& certs)
		{
			const auto& path = Util::CreateIfNotExists ("certmgr").absoluteFilePath (filename);

			QFile file { path };
			if (!file.exists ())
				return;

			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open file"
						<< path
						<< file.errorString ();
				return;
			}

			QDataStream stream { &file };

			while (!stream.atEnd ())
			{
				QByteArray ba;
				stream >> ba;

				certs << QSslCertificate::fromData (ba, QSsl::Pem);
			}
		}
	}

	void Manager::SaveBlacklist () const
	{
		Save ("blacklist", Blacklist_);
	}

	void Manager::LoadBlacklist ()
	{
		Load ("blacklist", Blacklist_);
	}

	void Manager::SaveLocals () const
	{
		Save ("locals", Locals_);
	}

	void Manager::LoadLocals ()
	{
		Load ("locals", Locals_);
	}
}
}
