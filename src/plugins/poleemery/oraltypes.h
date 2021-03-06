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

#pragma once

#define BOOST_RESULT_OF_USE_DECLTYPE

#include <type_traits>
#include <boost/fusion/include/at_c.hpp>

namespace LeechCraft
{
namespace Poleemery
{
namespace oral
{
	template<typename T>
	struct PKey
	{
		typedef T value_type;

		T Val_;

		PKey ()
		: Val_ ()
		{
		}

		PKey (T val)
		: Val_ (val)
		{
		}

		PKey& operator= (T val)
		{
			Val_ = val;
			return *this;
		}

		operator value_type () const
		{
			return Val_;
		}
	};

	template<typename T>
	struct Unique
	{
		typedef T value_type;

		T Val_;

		Unique ()
		: Val_ ()
		{
		}

		Unique (T val)
		: Val_ (val)
		{
		}

		Unique& operator= (T val)
		{
			Val_ = val;
			return *this;
		}

		operator value_type () const
		{
			return Val_;
		}
	};

	namespace detail
	{
		template<typename T>
		struct IsPKey : std::false_type {};

		template<typename U>
		struct IsPKey<PKey<U>> : std::true_type {};
	}

	template<typename Seq, int Idx>
	struct References
	{
		typedef typename std::decay<typename boost::fusion::result_of::at_c<Seq, Idx>::type>::type member_type;
		static_assert (detail::IsPKey<member_type>::value, "References<> element must refer to a PKey<> element");

		typedef typename member_type::value_type value_type;
		value_type Val_;

		References ()
		: Val_ ()
		{
		}

		References (value_type t)
		: Val_ (t)
		{
		}

		template<typename T>
		References (const PKey<T>& key)
		: Val_ (static_cast<T> (key))
		{
		}

		References& operator= (const value_type& val)
		{
			Val_ = val;
			return *this;
		}

		template<typename T>
		References& operator= (const PKey<T>& key)
		{
			Val_ = key;
			return *this;
		}

		operator value_type () const
		{
			return Val_;
		}
	};
}
}
}
