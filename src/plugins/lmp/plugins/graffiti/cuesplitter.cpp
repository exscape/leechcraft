/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "cuesplitter.h"
#include <QTime>
#include <QFile>
#include <QtDebug>
#include <QTimer>
#include <QDir>
#include <QProcess>

#ifdef Q_OS_UNIX
#include <sys/time.h>
#include <sys/resource.h>
#endif

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	namespace
	{
		struct Track
		{
			int Index_;

			QTime StartPos_;
			QTime EndPos_;

			QString Title_;
			QString Performer_;
		};

		struct File
		{
			QString Filename_;
			QList<Track> Tracks_;

			bool IsValid () const
			{
				for (const auto& track : Tracks_)
					if (!track.StartPos_.isValid ())
						return false;
				return true;
			}
		};

		struct Cue
		{
			QString Performer_;
			QString Album_;

			int Date_;
			QString Genre_;

			QList<File> Files_;

			bool IsValid () const
			{
				for (const auto& file : Files_)
					if (!file.IsValid ())
						return false;
				return true;
			}
		};

		QString ChopQuotes (QString str)
		{
			if (str.startsWith ('\"'))
				str = str.mid (1);
			if (str.endsWith ('\"'))
				str.chop (1);
			return str;
		}

		void HandleREM (QByteArray rem, Cue& result)
		{
			rem = rem.mid (4);
			if (rem.startsWith ("GENRE "))
				result.Genre_ = rem.mid (6);
			else if (rem.startsWith ("DATE "))
				result.Date_ = rem.mid (5).toInt ();
		}

		template<typename Iter>
		Iter HandleTrack (Iter i, Iter end, File& file, const Cue& cue)
		{
			const auto& trackLine = i->trimmed ();
			if (!trackLine.startsWith ("TRACK "))
				return end;

			Track track;
			track.Performer_ = cue.Performer_;
			track.Index_ = trackLine.split (' ').value (1).toInt ();

			++i;

			while (i != end)
			{
				const auto& line = i->trimmed ();
				if (line.startsWith ("TRACK "))
					break;

				const auto firstSpace = line.indexOf (' ');
				const auto& command = line.left (firstSpace);

				auto textVal = [&line, firstSpace] () { return ChopQuotes (line.mid (firstSpace + 1).trimmed ()); };
				if (command == "TITLE")
					track.Title_ = textVal ();
				else if (command == "PERFORMER")
					track.Performer_ = textVal ();
				else if (command == "INDEX" && line.mid (firstSpace + 1, 2) == "01")
				{
					const auto& components = line.mid (firstSpace + 4, 8).trimmed ().split (':');
					if (components.size () == 3)
						track.StartPos_.setHMS (0,
								components.at (0).toInt (),
								components.at (1).toInt (),
								1000. / 75. * components.at (2).toInt ());
				}

				++i;
			}

			file.Tracks_ << track;

			return i;
		}

		template<typename Iter>
		Iter HandleFile (const QByteArray& line, Iter i, Iter end, Cue& result)
		{
			const auto startQuote = line.indexOf ('"');
			const auto endQuote = line.lastIndexOf ('"');

			File file;
			file.Filename_ = QString::fromUtf8 (line.mid (startQuote + 1, endQuote - startQuote - 1));

			while (i != end)
			{
				if (i->startsWith ("FILE "))
					break;

				i = HandleTrack (i, end, file, result);
			}

			Track *prevTrack = 0;
			int index = 1;
			for (auto& track : file.Tracks_)
			{
				if (prevTrack)
					prevTrack->EndPos_ = track.StartPos_;

				prevTrack = &track;
				if (!track.Index_)
					track.Index_ = index++;
			}

			result.Files_ << file;

			return i;
		}

		Cue ParseCue (const QString& cue)
		{
			QFile file (cue);
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to parse"
						<< cue;
				return Cue ();
			}

			Cue result;
			const auto& lines = file.readAll ().split ('\n');
			for (auto i = lines.begin (), end= lines.end (); i != end; )
			{
				const auto& line = i->trimmed ();

				if (line.startsWith ("REM "))
				{
					HandleREM (line, result);
					++i;
				}
				else if (line.startsWith ("PERFORMER "))
				{
					result.Performer_ = ChopQuotes (QString::fromUtf8 (line.mid (10)));
					++i;
				}
				else if (line.startsWith ("TITLE "))
				{
					result.Album_ = ChopQuotes (QString::fromUtf8 (line.mid (6)));
					++i;
				}
				else if (line.startsWith ("FILE "))
					i = HandleFile (line.mid (5), i + 1, end, result);
			}
			return result;
		}
	}

	CueSplitter::CueSplitter (const QString& cueFile, const QString& dir, QObject *parent)
	: QObject (parent)
	, CueFile_ (cueFile)
	, Dir_ (dir)
	{
		QTimer::singleShot (0,
				this,
				SLOT (split ()));
	}

	void CueSplitter::split ()
	{
		const auto& cue = ParseCue (CueFile_);
		qDebug () << cue.IsValid () << cue.Album_ << cue.Performer_ << cue.Date_;
		for (const auto& file : cue.Files_)
		{
			qDebug () << "\t" << file.Filename_;
			for (const auto& track : file.Tracks_)
			{
				qDebug () << "\t\t" << track.Index_ << track.Title_ << track.StartPos_ << track.EndPos_;
			}
		}

		if (!cue.IsValid ())
		{
			emit error (tr ("Cue file is invalid"));
			deleteLater ();
			return;
		}

		auto makeFilename = [&cue] (const Track& track) -> QString
		{
			auto filename = QString::number (track.Index_);
			if (!cue.Performer_.isEmpty ())
				filename += " - " + cue.Performer_;
			if (!cue.Album_.isEmpty ())
				filename += " - " + cue.Album_;
			if (!track.Title_.isEmpty ())
				filename += " - " + track.Title_;
			return filename += ".flac";
		};

		for (const auto& file : cue.Files_)
		{
			const QDir dir (Dir_);
			if (!dir.exists (file.Filename_))
			{
				qWarning () << Q_FUNC_INFO
						<< file.Filename_
						<< "not found";
				emit error (tr ("No such file %1.").arg (file.Filename_));
				continue;
			}

			const auto& path = dir.absoluteFilePath (file.Filename_);

			for (const auto& track : file.Tracks_)
				SplitQueue_.append ({
						path,
						dir.absoluteFilePath (makeFilename (track)),
						track.Index_,
						track.StartPos_,
						track.EndPos_,
						track.Performer_,
						cue.Album_,
						track.Title_,
						cue.Date_,
						cue.Genre_
					});
		}

		scheduleNext ();
	}

	void CueSplitter::scheduleNext ()
	{
		if (SplitQueue_.isEmpty ())
		{
			deleteLater ();
			emit finished ();
			return;
		}

		const auto& item = SplitQueue_.takeFirst ();

		auto makeTime = [] (const QTime& time)
		{
			return QString ("%1:%2.%3")
					.arg (time.hour () * 60 + time.minute ())
					.arg (time.second ())
					.arg (time.msec () / 10);
		};

		QStringList args { "-8" };
		if (item.From_ != QTime (0, 0))
			args << ("--skip=" + makeTime (item.From_));
		if (item.To_.isValid ())
			args << ("--until=" + makeTime (item.To_));

		auto addTag = [&args] (const QString& name, const QString& value)
		{
			if (!value.isEmpty ())
				args << ("--tag=" + name + "=" + value);
		};
		addTag ("ARTIST", item.Artist_);
		addTag ("ALBUM", item.Album_);
		addTag ("TITLE", item.Title_);
		addTag ("TRACKNUMBER", QString::number (item.Index_));
		addTag ("GENRE", item.Genre_);
		addTag ("DATE", item.Date_ > 0 ? QString::number (item.Date_) : QString ());

		args << item.SourceFile_ << "-o" << item.TargetFile_;

		auto process = new QProcess (this);
		process->start ("flac", args);

		connect (process,
				SIGNAL (finished (int)),
				this,
				SLOT (handleProcessFinished (int)));
		connect (process,
				SIGNAL (error (QProcess::ProcessError)),
				this,
				SLOT (handleProcessError ()));

#ifdef Q_OS_UNIX
		setpriority (PRIO_PROCESS, process->pid (), 19);
#endif
	}

	void CueSplitter::handleProcessFinished (int code)
	{
		sender ()->deleteLater ();
		scheduleNext ();
	}

	void CueSplitter::handleProcessError ()
	{
		auto process = qobject_cast<QProcess*> (sender ());
		process->deleteLater ();

		const auto& errorString = tr ("Failed to start recoder: %1.")
				.arg (process->errorString ());

		if (!EmittedErrors_.contains (errorString))
		{
			emit error (errorString);
			EmittedErrors_ << errorString;
		}

		scheduleNext ();
	}
}
}
}
