/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "operationpropsdialog.h"
#include <QStringListModel>
#include <QtDebug>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/itagsmanager.h>
#include "structures.h"
#include "core.h"
#include "accountsmanager.h"
#include "operationsmanager.h"
#include "currenciesmanager.h"

namespace LeechCraft
{
namespace Poleemery
{
	OperationPropsDialog::OperationPropsDialog (QWidget *parent)
	: QDialog (parent)
	, Accounts_ (Core::Instance ().GetAccsManager ()->GetAccounts ())
	{
		Ui_.setupUi (this);
		Ui_.DateEdit_->setDateTime (QDateTime::currentDateTime ());

		for (const auto& acc : Accounts_)
			Ui_.AccsBox_->addItem (acc.Name_);

		auto completer = new Util::TagsCompleter (Ui_.Categories_, Ui_.Categories_);
		const auto& cats = Core::Instance ().GetOpsManager ()->GetKnownCategories ().toList ();
		completer->OverrideModel (new QStringListModel (cats, completer));
		Ui_.Categories_->AddSelector ();

		const auto& entries = Core::Instance ().GetOpsManager ()->GetAllEntries ();
		for (const auto& entry : entries)
			switch (entry->GetType ())
			{
			case EntryType::Receipt:
				ReceiptNames_ << entry->Name_;
				break;
			case EntryType::Expense:
				ExpenseNames_ << entry->Name_;
				ShopNames_ << std::dynamic_pointer_cast<ExpenseEntry> (entry)->Shop_;
				break;
			}

		Ui_.AmountCurrency_->addItems (Core::Instance ()
				.GetCurrenciesManager ()->GetEnabledCurrencies ());

		ReceiptNames_.removeDuplicates ();
		std::sort (ReceiptNames_.begin (), ReceiptNames_.end ());
		ExpenseNames_.removeDuplicates ();
		std::sort (ExpenseNames_.begin (), ExpenseNames_.end ());
		ShopNames_.removeDuplicates ();
		std::sort (ShopNames_.begin (), ShopNames_.end ());

		Ui_.Shop_->addItems (QStringList (QString ()) + ShopNames_);

		on_ExpenseEntry__released ();

		if (!Accounts_.isEmpty ())
			on_AccsBox__currentIndexChanged (0);
	}

	EntryType OperationPropsDialog::GetEntryType () const
	{
		return Ui_.ExpenseEntry_->isChecked () ?
				EntryType::Expense :
				EntryType::Receipt;
	}

	EntryBase_ptr OperationPropsDialog::GetEntry () const
	{
		const auto& acc = Accounts_.at (Ui_.AccsBox_->currentIndex ());
		const auto accId = acc.ID_;

		auto curMgr = Core::Instance ().GetCurrenciesManager ();

		const auto currency = Ui_.AmountCurrency_->currentText ();
		const auto amount = curMgr->Convert (currency, acc.Currency_, Ui_.Amount_->value ());

		const auto& name = Ui_.Name_->currentText ();
		const auto& descr = Ui_.Description_->text ();
		auto dt = Ui_.DateEdit_->dateTime ();
		auto time = dt.time ();
		dt.setTime ({ time.hour (), time.minute () });

		switch (GetEntryType ())
		{
		case EntryType::Receipt:
			return std::make_shared<ReceiptEntry> (accId, amount, name, descr, dt);
		case EntryType::Expense:
			return std::make_shared<ExpenseEntry> (accId, amount, name, descr, dt,
					Ui_.CountBox_->value (),
					Ui_.Shop_->currentText (),
					Core::Instance ().GetCoreProxy ()->
							GetTagsManager ()->Split (Ui_.Categories_->text ()));
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown entry type";
		return {};
	}

	void OperationPropsDialog::on_AccsBox__currentIndexChanged (int index)
	{
		const auto& accCur = Accounts_.at (index).Currency_;
		const auto pos = Ui_.AmountCurrency_->findText (accCur);
		if (pos >= 0)
			Ui_.AmountCurrency_->setCurrentIndex (pos);
	}

	void OperationPropsDialog::on_ExpenseEntry__released ()
	{
		Ui_.Name_->clear ();
		Ui_.Name_->addItems (QStringList (QString ()) + ExpenseNames_);
		Ui_.PagesStack_->setCurrentWidget (Ui_.ExpensePage_);
	}

	void OperationPropsDialog::on_ReceiptEntry__released ()
	{
		Ui_.Name_->clear ();
		Ui_.Name_->addItems (QStringList (QString ()) + ReceiptNames_);
		Ui_.PagesStack_->setCurrentWidget (Ui_.ReceiptPage_);
	}
}
}
