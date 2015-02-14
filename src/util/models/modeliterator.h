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

#include <QModelIndex>
#include "modelsconfig.h"

namespace LeechCraft
{
namespace Util
{
	class UTIL_MODELS_API ModelIterator
	{
		QAbstractItemModel * const Model_;
		const QModelIndex Parent_;

		int Row_;
		int Col_;
	public:
		enum class Direction
		{
			Rows,
			Cols
		};
	private:
		const Direction Dir_;
	public:
		ModelIterator (QAbstractItemModel *model, int row, int col = 0,
				Direction dir = Direction::Rows, const QModelIndex& parent = QModelIndex ());

		ModelIterator& operator++ ();
		ModelIterator operator++ (int);
		ModelIterator& operator-- ();
		ModelIterator operator-- (int);
		ModelIterator& operator+= (int);
		ModelIterator& operator-= (int);
		int operator- (const ModelIterator& other) const;

		bool operator== (const ModelIterator& other) const;
		bool operator!= (const ModelIterator& other) const;

		QModelIndex operator* () const;

		int GetRow () const;
		int GetCol () const;
	private:
		int& GetIncrementable ();
	};
}
}

namespace std
{
	template<>
	struct iterator_traits<LeechCraft::Util::ModelIterator>
	{
		typedef QModelIndex value_type;
		typedef int difference_type;

		typedef random_access_iterator_tag iterator_category;
	};
}
