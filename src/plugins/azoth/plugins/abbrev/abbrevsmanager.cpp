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

#include "abbrevsmanager.h"
#include <QSettings>
#include <QCoreApplication>
#include <interfaces/azoth/iprovidecommands.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Abbrev
{
	AbbrevsManager::AbbrevsManager (QObject *parent)
	: QObject { parent }
	{
		Load ();
	}

	void AbbrevsManager::Add (const Abbreviation& abbrev)
	{
		if (std::any_of (Abbrevs_.begin (), Abbrevs_.end (),
				[&abbrev] (const Abbreviation& other)
					{ return other.Pattern_ == abbrev.Pattern_; }))
			throw CommandException { tr ("Abbreviation with this pattern already exists.") };

		if (abbrev.Pattern_.isEmpty ())
			throw CommandException { tr ("Abbeviation pattern is empty.") };

		if (abbrev.Expansion_.isEmpty ())
			throw CommandException { tr ("Abbeviation expansion is empty.") };

		const auto pos = std::lower_bound (Abbrevs_.begin (), Abbrevs_.end (), abbrev,
				[] (const Abbreviation& left, const Abbreviation& right)
					{ return left.Pattern_.size () > right.Pattern_.size (); });
		Abbrevs_.insert (pos, abbrev);

		Save ();
	}

	const QList<Abbreviation>& AbbrevsManager::List () const
	{
		return Abbrevs_;
	}

	void AbbrevsManager::Remove (int index)
	{
		if (index < 0 || index >= Abbrevs_.size ())
			return;

		Abbrevs_.removeAt (index);
		Save ();
	}

	namespace
	{
		bool IsBadChar (const QChar& c)
		{
			return c.isLetter ();
		}
	}

	QString AbbrevsManager::Process (QString text) const
	{
		int cyclesCount = 0;
		while (true)
		{
			auto result = text;
			for (const auto& abbrev : Abbrevs_)
				if (result.contains (abbrev.Pattern_))
				{
					bool changed = false;
					auto pos = 0;
					while ((pos = result.indexOf (abbrev.Pattern_, pos)) != -1)
					{
						const auto afterAbbrevPos = pos + abbrev.Pattern_.size ();
						if ((!pos || !IsBadChar (result.at (pos - 1))) &&
							(afterAbbrevPos >= result.size () || !IsBadChar (result.at (afterAbbrevPos))))
						{
							result.replace (pos, abbrev.Pattern_.size (), abbrev.Expansion_);
							pos += abbrev.Expansion_.size ();
							changed = true;
						}
						else
							pos += abbrev.Pattern_.size ();
					}
					if (changed)
						break;
				}

			if (result == text)
				break;

			text = result;
			if (++cyclesCount >= 1024)
				throw CommandException { tr ("Too much expansions during abbreviations application. Check your rules.") };
		}

		return text;
	}

	void AbbrevsManager::Load ()
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Abbrev" };
		settings.beginGroup ("Abbrevs");
		Abbrevs_ = settings.value ("Abbreviations").value<decltype (Abbrevs_)> ();
		settings.endGroup ();
	}

	void AbbrevsManager::Save () const
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Abbrev" };
		settings.beginGroup ("Abbrevs");
		settings.setValue ("Abbreviations", QVariant::fromValue (Abbrevs_));
		settings.endGroup ();
	}
}
}
}