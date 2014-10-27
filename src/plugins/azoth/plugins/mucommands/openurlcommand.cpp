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

#include "openurlcommand.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <QStringList>
#include <QUrl>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iproxyobject.h>

namespace LeechCraft
{
namespace Azoth
{
namespace MuCommands
{
	namespace
	{
		QStringList GetAllUrls (IProxyObject *azothProxy, ICLEntry *entry, QObject *since = nullptr)
		{
			QStringList urls;

			const auto& allMsgs = entry->GetAllMessages ();

			const auto begin = since ?
					std::find_if (allMsgs.begin (), allMsgs.end (),
							[since] (IMessage *msg) { return msg->GetQObject () == since; }) :
					allMsgs.begin ();
			for (auto i = begin == allMsgs.end () ? allMsgs.begin () : begin;
					i != allMsgs.end (); ++i)
			{
				const auto msg = *i;
				switch (msg->GetMessageType ())
				{
				case IMessage::Type::ChatMessage:
				case IMessage::Type::MUCMessage:
					break;
				default:
					continue;
				}

				urls += azothProxy->GetFormatterProxy ().FindLinks (msg->GetBody ());
			}

			urls.removeDuplicates ();

			return urls;
		}
	}

	StringCommandResult ListUrls (IProxyObject *azothProxy, ICLEntry *entry, const QString&)
	{
		const auto& urls = GetAllUrls (azothProxy, entry);
		const auto& body = urls.isEmpty () ?
				QObject::tr ("Sorry, no links found, chat more!") :
				QObject::tr ("Found links:") + "<ol><li>" + urls.join ("</li><li>") + "</li></ol>";
		return { true, body };
	}

	namespace
	{
		using UrlIndex_t = int;

		struct UrlRange
		{
			boost::optional<int> Start_;
			boost::optional<int> End_;
		};

		struct SinceLast {};

		struct JustLast {};

		using RxableRanges_t = boost::variant<SinceLast, UrlRange>;

		using RegExpStr_t = std::string;

		struct UrlComposite
		{
			RxableRanges_t Range_;
			boost::optional<RegExpStr_t> Pat_;
		};

		using OpenUrlParams_t = boost::variant<UrlIndex_t, UrlComposite, RegExpStr_t, JustLast>;
	}
}
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Azoth::MuCommands::UrlRange,
		(boost::optional<int>, Start_)
		(boost::optional<int>, End_));

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Azoth::MuCommands::UrlComposite,
		(LeechCraft::Azoth::MuCommands::RxableRanges_t, Range_)
		(boost::optional<std::string>, Pat_));

namespace LeechCraft
{
namespace Azoth
{
namespace MuCommands
{
	namespace
	{
		namespace ascii = boost::spirit::ascii;
		namespace qi = boost::spirit::qi;
		namespace phoenix = boost::phoenix;

		template<typename Iter>
		struct Parser : qi::grammar<Iter, OpenUrlParams_t ()>
		{
			qi::rule<Iter, SinceLast ()> SinceLast_;
			qi::rule<Iter, OpenUrlParams_t ()> Start_;
			qi::rule<Iter, UrlIndex_t ()> Index_;
			qi::rule<Iter, UrlComposite ()> RegExp_;
			qi::rule<Iter, RegExpStr_t ()> RegExpPat_;
			qi::rule<Iter, RxableRanges_t ()> RxableRanges_;
			qi::rule<Iter, UrlRange ()> Range_;
			qi::rule<Iter, JustLast ()> JustLast_;

			Parser ()
			: Parser::base_type { Start_ }
			{
				Index_ = qi::int_;
				SinceLast_ = qi::lit ("last") > qi::attr (SinceLast {});
				Range_ = -(qi::int_) >> qi::lit (':') >> -(qi::int_);
				RxableRanges_ = SinceLast_ | Range_;
				RegExpPat_ = qi::lit ("rx ") >> +qi::char_;
				RegExp_ = RxableRanges_ >> -(qi::lit (' ') >> RegExpPat_);
				JustLast_ = qi::attr (JustLast {});

				Start_ = (RegExpPat_ | RegExp_ | Index_ | JustLast_) >> qi::eoi;
			}
		};

		struct ParseError {};

		template<typename Iter>
		OpenUrlParams_t ParseCommand (Iter begin, Iter end)
		{
			OpenUrlParams_t res;
			if (!qi::parse (begin, end, Parser<Iter> {}, res))
				throw ParseError {};
			return res;
		}

		OpenUrlParams_t ParseCommand (const QString& cmd)
		{
			const auto& unicode = cmd.section (' ', 1).toUtf8 ();
			return ParseCommand (unicode.begin (), unicode.end ());
		}

		struct RxableRangesVisitor : public boost::static_visitor<QStringList>
		{
			IProxyObject * const AzothProxy_;
			ICLEntry * const Entry_;

			RxableRangesVisitor (IProxyObject *azothProxy, ICLEntry *entry)
			: AzothProxy_ { azothProxy }
			, Entry_ { entry }
			{
			}

			QStringList operator() (const SinceLast&) const
			{
				const auto last = AzothProxy_->GetFirstUnreadMessage (Entry_->GetQObject ());
				return GetAllUrls (AzothProxy_, Entry_, last);
			}

			QStringList operator() (const UrlRange& range) const
			{
				const auto& allUrls = GetAllUrls (AzothProxy_, Entry_);
				if (allUrls.isEmpty ())
					return {};

				auto begin = boost::get_optional_value_or (range.Start_, 1);
				auto end = boost::get_optional_value_or (range.End_, allUrls.size ());

				if (!begin || !end)
					throw StringCommandResult
					{
						true,
						QObject::tr ("Indexes cannot be equal to zero.")
					};

				begin = begin > 0 ? (begin - 1) : (allUrls.size () + begin);
				end = end > 0 ? (end - 1) : (allUrls.size () + end);

				if (begin > end)
					throw StringCommandResult
					{
						true,
						QObject::tr ("Begin index should not be greater than end index.")
					};

				if (end >= allUrls.size ())
					throw StringCommandResult
					{
						true,
						QObject::tr ("End index is out of bounds of the URLs list.")
					};

				QStringList result;
				for (auto i = begin; i <= end; ++i)
					result << allUrls.value (i);
				return result;
			}
		};

		struct ParseResultVisitor : public boost::static_visitor<CommandResult_t>
		{
			IEntityManager * const IEM_;
			IProxyObject * const AzothProxy_;
			const TaskParameters Params_;
			ICLEntry * const Entry_;

			ParseResultVisitor (IEntityManager *iem,
					IProxyObject *azothProxy, ICLEntry *entry, TaskParameters params)
			: IEM_ { iem }
			, AzothProxy_ { azothProxy }
			, Params_ { params }
			, Entry_ { entry }
			{
			}

			CommandResult_t operator() (UrlIndex_t idx) const
			{
				return (*this) (UrlComposite { UrlRange { idx, idx }, {} } );
			}

			CommandResult_t operator() (const JustLast&) const
			{
				return (*this) (-1);
			}

			CommandResult_t operator() (const RegExpStr_t& rxStr) const
			{
				return (*this) ({ UrlRange {}, rxStr });
			}

			CommandResult_t operator() (const UrlComposite& rx) const
			{
				QStringList urls;
				try
				{
					urls = boost::apply_visitor (RxableRangesVisitor { AzothProxy_, Entry_ }, rx.Range_);
				}
				catch (const StringCommandResult& res)
				{
					return res;
				}

				if (rx.Pat_)
					urls = urls.filter (QRegExp { QString::fromStdString (*rx.Pat_) });

				for (const auto& url : urls)
				{
					if (url.isEmpty ())
						continue;

					const auto& entity = Util::MakeEntity (QUrl::fromEncoded (url.toUtf8 ()),
							{},
							Params_ | FromUserInitiated);
					IEM_->HandleEntity (entity);
				}

				return true;
			}
		};
	}

	CommandResult_t OpenUrl (const ICoreProxy_ptr& coreProxy, IProxyObject *azothProxy,
			ICLEntry *entry, const QString& text, TaskParameters params)
	{
		OpenUrlParams_t parseResult;
		try
		{
			parseResult = ParseCommand (text);
		}
		catch (const ParseError&)
		{
			return StringCommandResult
			{
				true,
				QObject::tr ("Unable to parse `%1`.")
					.arg ("<em>" + text + "</em>")
			};
		}

		return boost::apply_visitor (ParseResultVisitor
				{
					coreProxy->GetEntityManager (),
					azothProxy,
					entry,
					params
				},
				parseResult);
	}
}
}
}
