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

#include "certsmodel.h"
#include <QDateTime>
#include <QStringList>
#include <QBrush>
#include <QApplication>
#include <QPalette>
#include <QtDebug>

namespace LeechCraft
{
namespace CertMgr
{
	CertsModel::CertsModel (QObject *parent)
	: QAbstractItemModel { parent }
	{
	}

	QModelIndex CertsModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return {};

		if (!parent.isValid ())
			return createIndex (row, column, -1);

		if (static_cast<int> (parent.internalId ()) != -1)
			return {};

		auto id = static_cast<quint32> (parent.row ());
		return createIndex (row, column, id);
	}

	QModelIndex CertsModel::parent (const QModelIndex& child) const
	{
		if (!child.isValid ())
			return {};

		auto id = static_cast<int> (child.internalId ());
		if (id == -1)
			return {};

		return index (id, 0, {});
	}

	int CertsModel::columnCount (const QModelIndex&) const
	{
		return 1;
	}

	int CertsModel::rowCount (const QModelIndex& parent) const
	{
		if (!parent.isValid ())
			return Issuer2Certs_.size ();

		const auto id = static_cast<int> (parent.internalId ());
		return id == -1 ?
				Issuer2Certs_.value (parent.row ()).second.size () :
				0;
	}

	namespace
	{
		QString MakeTooltip (const QSslCertificate& cert)
		{
			QString result;

			auto add = [&result, &cert] (const QString& name, const QString& val) -> void
			{
				if (val.isEmpty ())
					return;

				result += "<tr><td align='right'>" + name + ": </td>";
				result += "<td>" + val + "</td></tr>";
			};

#if QT_VERSION < 0x050000
			auto addStdFields = [&add] (std::function<QString (QSslCertificate::SubjectInfo)> getter) -> void
			{
#else
			auto addStdFields = [&add] (std::function<QStringList (QSslCertificate::SubjectInfo)> listGetter) -> void
			{
				const auto getter = [listGetter] (QSslCertificate::SubjectInfo key)
						{ return listGetter (key).join ("; "); };
#endif
				add (CertsModel::tr ("Organization"),
						getter (QSslCertificate::Organization));
				add (CertsModel::tr ("Unit"),
						getter (QSslCertificate::OrganizationalUnitName));
				add (CertsModel::tr ("Common name"),
						getter (QSslCertificate::CommonName));
				add (CertsModel::tr ("Locality"),
						getter (QSslCertificate::LocalityName));
				add (CertsModel::tr ("State"),
						getter (QSslCertificate::StateOrProvinceName));
				add (CertsModel::tr ("Country"),
						getter (QSslCertificate::CountryName));
			};

			result += "<strong>" + CertsModel::tr ("Issuee") + ":</strong>";
			result += "<table style='border: none'>";
			addStdFields ([&cert] (QSslCertificate::SubjectInfo info)
						{ return cert.subjectInfo (info); });

			add (CertsModel::tr ("Serial number"), cert.serialNumber ());
			result += "</table><br />";

			result += "<strong>" + CertsModel::tr ("Issuer") + ":</strong>";
			result += "<table style='border: none'>";
			addStdFields ([&cert] (QSslCertificate::SubjectInfo info)
						{ return cert.issuerInfo (info); });
			result += "</table><br />";

			result += "<strong>" + CertsModel::tr ("Dates") + ":</strong>";
			result += "<table style='border: none'>";
			add (CertsModel::tr ("Valid since"),
					QLocale {}.toString (cert.effectiveDate (), QLocale::ShortFormat));
			add (CertsModel::tr ("Valid until"),
					QLocale {}.toString (cert.expiryDate (), QLocale::ShortFormat));
			result += "</table><br />";

#if QT_VERSION < 0x050000
			const auto& subjs = cert.alternateSubjectNames ();
#else
			const auto& subjs = cert.subjectAlternativeNames ();
#endif
			if (!subjs.isEmpty ())
			{
				result += "<strong>" + CertsModel::tr ("Alternate names") + ":</strong>";

				for (auto key : subjs.keys ())
				{
					QString name;
					switch (key)
					{
					case QSsl::DnsEntry:
						name = CertsModel::tr ("DNS");
						break;
					case QSsl::EmailEntry:
						name = CertsModel::tr ("Email");
						break;
					}

					add (name, QStringList { subjs.values (key) }.join ("; "));
				}
			}

			return result;
		}
	}

	QVariant CertsModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		const auto id = static_cast<int> (index.internalId ());

		if (id == -1)
			switch (role)
			{
			case Qt::DisplayRole:
				return { Issuer2Certs_.value (index.row ()).first };
			default:
				return {};
			}

		const auto& cert = Issuer2Certs_.value (id).second.value (index.row ());

		switch (role)
		{
		case Qt::DisplayRole:
		{
			const auto& name = cert.subjectInfo (QSslCertificate::CommonName);
			const auto& org = cert.subjectInfo (QSslCertificate::Organization);

			if (!name.isEmpty () && !org.isEmpty ())
				return QString ("%1 (%2)")
#if QT_VERSION < 0x050000
						.arg (name)
						.arg (org);
#else
						.arg (name.join ("; "))
						.arg (org.join ("; "));
#endif
			else if (!name.isEmpty ())
				return name;
			else
				return org;
		}
		case Qt::ToolTipRole:
			return MakeTooltip (cert);
		case Qt::ForegroundRole:
		{
			const auto colorGroup = Blacklisted_.contains (cert) ?
					QPalette::Disabled :
					QPalette::Normal;
			return QBrush { qApp->palette ().color (colorGroup, QPalette::Text) };
		}
		case CertificateRole:
			return QVariant::fromValue (cert);
		default:
			return {};
		}
	}

	void CertsModel::AddCert (const QSslCertificate& cert)
	{
		const auto pos = CreateListPosForCert (cert);

		const auto& parentIndex = index (std::distance (Issuer2Certs_.begin (), pos), 0, {});

		beginInsertRows (parentIndex, pos->second.size (), pos->second.size ());
		pos->second << cert;
		endInsertRows ();
	}

	void CertsModel::RemoveCert (const QSslCertificate& cert)
	{
		const auto pos = GetListPosForCert (cert);
		if (pos == Issuer2Certs_.end ())
			return;

		const auto certIdx = pos->second.indexOf (cert);
		if (certIdx == -1)
			return;

		const auto& parentIndex = index (std::distance (Issuer2Certs_.begin (), pos), 0, {});

		beginRemoveRows (parentIndex, certIdx, certIdx);
		pos->second.removeAt (certIdx);
		endRemoveRows ();
	}

	void CertsModel::ResetCerts (const QList<QSslCertificate>& certs)
	{
		beginResetModel ();

		Issuer2Certs_.clear ();

		for (const auto& cert : certs)
			CreateListForCert (cert) << cert;

		endResetModel ();
	}

	QModelIndex CertsModel::FindCertificate (const QSslCertificate& cert) const
	{
		const auto listPos = GetListPosForCert (cert);
		if (listPos == Issuer2Certs_.end ())
			return {};

		const auto certIdx = listPos->second.indexOf (cert);
		if (certIdx == -1)
			return {};

		const auto& parent = index (std::distance (Issuer2Certs_.begin (), listPos), 0, {});
		return index (certIdx, 0, parent);
	}

	void CertsModel::SetBlacklisted (const QSslCertificate& cert, bool blacklisted)
	{
		if (blacklisted)
			Blacklisted_ << cert;
		else
			Blacklisted_.removeAll (cert);

		const auto& idx = FindCertificate (cert);
		if (!idx.isValid ())
			return;

		emit dataChanged (idx, idx);
	}

	namespace
	{
		QString GetIssuerName (const QSslCertificate& cert)
		{
			auto issuer = cert.issuerInfo (QSslCertificate::Organization);
			if (issuer.isEmpty ())
				issuer = cert.issuerInfo (QSslCertificate::CommonName);

#if QT_VERSION < 0x050000
			return issuer;
#else
			return issuer.join ("; ");
#endif
		}
	}

	auto CertsModel::GetListPosForCert (const QSslCertificate& cert) -> CertsDict_t::iterator
	{
		const auto& issuer = GetIssuerName (cert);

		const auto pos = std::lower_bound (Issuer2Certs_.begin (), Issuer2Certs_.end (), issuer,
				[] (decltype (Issuer2Certs_.at (0)) item, const QString& issuer)
					{ return QString::localeAwareCompare (item.first.toLower (), issuer.toLower ()) < 0; });

		if (pos == Issuer2Certs_.end ())
			return pos;

		return pos->first.toLower () == issuer.toLower () ?
				pos :
				Issuer2Certs_.end ();
	}

	auto CertsModel::GetListPosForCert (const QSslCertificate& cert) const -> CertsDict_t::const_iterator
	{
		const auto& issuer = GetIssuerName (cert);

		const auto pos = std::lower_bound (Issuer2Certs_.begin (), Issuer2Certs_.end (), issuer,
				[] (decltype (Issuer2Certs_.at (0)) item, const QString& issuer)
					{ return QString::localeAwareCompare (item.first.toLower (), issuer.toLower ()) < 0; });

		if (pos == Issuer2Certs_.end ())
			return pos;

		return pos->first.toLower () == issuer.toLower () ?
				pos :
				Issuer2Certs_.end ();
	}

	auto CertsModel::CreateListPosForCert (const QSslCertificate& cert) -> CertsDict_t::iterator
	{
		const auto& issuer = GetIssuerName (cert);

		auto pos = std::lower_bound (Issuer2Certs_.begin (), Issuer2Certs_.end (), issuer,
				[] (decltype (Issuer2Certs_.at (0)) item, const QString& issuer)
					{ return QString::localeAwareCompare (item.first.toLower (), issuer.toLower ()) < 0; });

		if (pos != Issuer2Certs_.end () && pos->first.toLower () == issuer.toLower ())
			return pos;

		const auto row = std::distance (Issuer2Certs_.begin (), pos);
		beginInsertRows ({}, row, row);
		pos = Issuer2Certs_.insert (pos, { issuer, {} });
		endInsertRows ();

		return pos;
	}

	QList<QSslCertificate>& CertsModel::CreateListForCert (const QSslCertificate& cert)
	{
		return CreateListPosForCert (cert)->second;
	}
}
}
