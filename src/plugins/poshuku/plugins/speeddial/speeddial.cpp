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

#include "speeddial.h"
#include <vector>
#include <QHash>
#include <QUrl>
#include <QtConcurrentRun>
#include <interfaces/poshuku/iproxyobject.h>
#include <interfaces/poshuku/istoragebackend.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace SpeedDial
{
	void Plugin::Init (ICoreProxy_ptr)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.SpeedDial";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku SpeedDial";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds a special speed dial page.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	void Plugin::initPlugin (QObject *object)
	{
		PoshukuProxy_ = qobject_cast<IProxyObject*> (object);
	}

	namespace
	{
		typedef QList<QPair<QString, QString>> TopList_t;

		double GetScore (const QDateTime& then, const QDateTime& now)
		{
			return std::log (std::max (then.daysTo (now) + 1, 1));
		}

		void Compactify (std::vector<std::pair<QString, int>>& vec, size_t count)
		{
			while (vec.size () > count)
			{
				const auto& last = vec.back ();
				vec.pop_back ();

				const auto pos = std::find_if (vec.begin (), vec.end (),
						[last] (decltype (*vec.cbegin ()) item)
							{ return last.first.startsWith (item.first); });
				if (pos != vec.end ())
					pos->second += last.second;
			}
		}

		TopList_t GetTopUrls (const IStorageBackend_ptr& sb, size_t count)
		{
			history_items_t items;
			sb->LoadHistory (items);

			const auto& now = QDateTime::currentDateTime ();

			QHash<QString, double> url2score;
			for (const auto& item : items)
				url2score [item.URL_] += GetScore (item.DateTime_, now);

			std::vector<std::pair<QString, int>> vec;
			for (auto i = url2score.begin (), end = url2score.end (); i != end; ++i)
				vec.emplace_back (i.key (), i.value ());

			std::sort (vec.begin (), vec.end (),
					[] (decltype (*vec.cbegin ()) left, decltype (*vec.cbegin ()) right)
						{ return left.second > right.second; });

			std::sort (vec.begin (), vec.end (),
					[] (decltype (*vec.cbegin ()) left, decltype (*vec.cbegin ()) right)
						{ return left.second > right.second; });

			TopList_t result;
			for (size_t i = 0; i < std::min (vec.size (), count); ++i)
			{
				const auto& url = vec [i].first;

				const auto& item = std::find_if (items.begin (), items.end (),
						[&url] (const HistoryItem& item) { return item.URL_ == url; });

				result.append ({ url, item->Title_ });
			}
			return result;
		}
	}

	void Plugin::hookBrowserWidgetInitialized (LeechCraft::IHookProxy_ptr proxy,
			QWebView *view,
			QObject *browserWidget)
	{
		QtConcurrent::run ([this] () -> QList<QPair<QString, QString>>
				{
					const auto& sb = PoshukuProxy_->CreateStorageBackend ();

					const auto& items = GetTopUrls (sb, 8);
					return items;
				});
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_speeddial, LeechCraft::Poshuku::SpeedDial::Plugin);