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

#include "structures.h"

namespace LeechCraft
{
namespace XProxy
{
	Proxy::operator QNetworkProxy () const
	{
		return { Type_, Host_, static_cast<quint16> (Port_), User_, Pass_ };
	}

	namespace
	{
		template<typename Class, typename G, typename... Gs>
		struct IsLessImpl
		{
			bool operator() (const Class& left, const Class& right, G g, Gs... gs) const
			{
				if (left.*g != right.*g)
					return left.*g < right.*g;

				return IsLessImpl<Class, Gs...> {} (left, right, gs...);
			}
		};

		template<typename Class, typename G>
		struct IsLessImpl<Class, G>
		{
			bool operator() (const Class& left, const Class& right, G g) const
			{
				if (left.*g != right.*g)
					return left.*g < right.*g;

				return false;
			}
		};

		template<typename Class, typename... Gs>
		bool IsLess (const Class& left, const Class& right, Gs... gs)
		{
			return IsLessImpl<Class, Gs...> {} (left, right, gs...);
		}
	}

	bool operator< (const Proxy& left, const Proxy& right)
	{
		return IsLess (left, right, &Proxy::Type_, &Proxy::Port_, &Proxy::Host_, &Proxy::User_, &Proxy::Pass_);
	}

	bool operator== (const Proxy& left, const Proxy& right)
	{
		return !(left < right) && !(right < left);
	}

	QDataStream& operator<< (QDataStream& out, const Proxy& p)
	{
		out << static_cast<quint8> (1);
		out << static_cast<qint8> (p.Type_)
			<< p.Host_
			<< p.Port_
			<< p.User_
			<< p.Pass_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Proxy& p)
	{
		quint8 ver = 0;
		in >> ver;
		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return in;
		}

		qint8 type = 0;
		in >> type
			>> p.Host_
			>> p.Port_
			>> p.User_
			>> p.Pass_;
		p.Type_ = static_cast<QNetworkProxy::ProxyType> (type);

		return in;
	}

	QDataStream& operator<< (QDataStream& out, const ReqTarget& t)
	{
		out << static_cast<quint8> (2);
		out << t.Host_
			<< t.Port_
			<< t.Protocols_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, ReqTarget& t)
	{
		quint8 ver = 0;
		in >> ver;
		if (ver < 1 || ver > 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return in;
		}

		if (ver == 1)
		{
			QRegExp rx;
			in >> rx;
			t.Host_ = Util::RegExp { rx.pattern (), rx.caseSensitivity () };
		}
		else
			in >> t.Host_;

		in >> t.Port_
			>> t.Protocols_;
		return in;
	}
}
}