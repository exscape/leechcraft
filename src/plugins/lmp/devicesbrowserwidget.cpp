/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "devicesbrowserwidget.h"
#include <util/models/flattenfiltermodel.h>
#include <util/util.h>
#include <interfaces/iremovabledevmanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include "core.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class MountableFlattener : public Util::FlattenFilterModel
		{
		public:
			MountableFlattener (QObject *parent)
			: Util::FlattenFilterModel (parent)
			{
			}

			QVariant data (const QModelIndex& index, int role) const
			{
				if (role != Qt::DisplayRole)
					return Util::FlattenFilterModel::data (index, role);

				const auto& mounts = index.data (DeviceRoles::MountPoints).toStringList ();
				const auto& mountText = mounts.isEmpty () ?
						tr ("not mounted") :
						tr ("mounted at %1").arg (mounts.join ("; "));

				const auto& size = index.data (DeviceRoles::TotalSize).toLongLong ();
				return QString ("%1 (%2, %3), %4")
						.arg (index.data (DeviceRoles::VisibleName).toString ())
						.arg (Util::MakePrettySize (size))
						.arg (index.data (DeviceRoles::DevFile).toString ())
						.arg (mountText);
			}
		protected:
			bool IsIndexAccepted (const QModelIndex& child) const
			{
				return child.data (DeviceRoles::IsMountable).toBool ();
			}
		};
	}

	DevicesBrowserWidget::DevicesBrowserWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	void DevicesBrowserWidget::InitializeDevices ()
	{
		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		const auto& mgrs = pm->GetAllCastableTo<IRemovableDevManager*> ();
		if (mgrs.isEmpty ())
		{
			setEnabled (false);
			return;
		}

		auto flattener = new MountableFlattener (this);
		flattener->SetSource (mgrs.at (0)->GetDevicesModel ());
		Ui_.DevicesSelector_->setModel (flattener);

		Ui_.DevicesSelector_->setCurrentIndex (-1);
	}
}
}
