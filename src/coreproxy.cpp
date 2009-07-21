#include "coreproxy.h"
#include "core.h"
#include "mainwindow.h"
#include "xmlsettingsmanager.h"
#include "skinengine.h"
#include "tagsmanager.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

CoreProxy::CoreProxy (QObject *parent)
: QObject (parent)
{
}

QNetworkAccessManager* CoreProxy::GetNetworkAccessManager () const
{
	return Core::Instance ().GetNetworkAccessManager ();
}

const IShortcutProxy* CoreProxy::GetShortcutProxy () const
{
	return Core::Instance ().GetShortcutProxy ();
}

QTreeView* CoreProxy::GetMainView () const
{
	return Core::Instance ().GetReallyMainWindow ()->GetMainView ();
}

QModelIndex CoreProxy::MapToSource (const QModelIndex& index) const
{
	return Core::Instance ().MapToSource (index);
}

BaseSettingsManager* CoreProxy::GetSettingsManager () const
{
	return XmlSettingsManager::Instance ();
}

QMainWindow* CoreProxy::GetMainWindow () const
{
	return Core::Instance ().GetReallyMainWindow ();
}

QIcon CoreProxy::GetIcon (const QString& icon, const QString& iconOff) const
{
	return SkinEngine::Instance ().GetIcon (icon, iconOff);
}

ITagsManager* CoreProxy::GetTagsManager () const
{
	return &TagsManager::Instance ();
}

int CoreProxy::GetID ()
{
	int i = 0;
	while (true)
		if (!UsedIDs_.Val ().contains (++i))
		{
			UsedIDs_.Val () << i;
			return i;
		}
	throw std::runtime_error ("ID pool exhausted");
}

void CoreProxy::FreeID (int id)
{
	if (UsedIDs_.Val ().removeAll (id) != 1)
		throw std::runtime_error (QString ("The ID being freed wasn't reserved %1")
				.arg (id).toStdString ().c_str ());
}

#define LC_DEFINE_REGISTER(a) \
void CoreProxy::RegisterHook (LeechCraft::HookSignature<LeechCraft::a>::Signature_t functor) \
{ \
	Core::Instance ().RegisterHook (functor); \
}
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
	LC_EXPANDER ((HIDDownloadFinishedNotification)
			(HIDNetworkAccessManagerCreateRequest));
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_DEFINE_REGISTER
