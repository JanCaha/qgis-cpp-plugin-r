#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsprocessingregistry.h"

#include <QFile>
#include <QMenu>

#include "rplugin.h"

RPlugin::RPlugin( QgisInterface *iface )
    : QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType ), mIface( iface )
{
    mOpenConsole = std::make_shared<QAction>( "Open R Console", this );
    mOpenConsole->setIcon( QIcon( sPluginIcon ) );

    connect( mOpenConsole.get(), &QAction::triggered, this, &RPlugin::showConsole );

    mIface->pluginMenu()->addAction( mOpenConsole.get() );
}

void RPlugin::showConsole()
{
    prepareForUse();
    mRConsole->show();
    mRConsole->raise();
}

void RPlugin::prepareForUse()
{
    if ( !mRStatsRunner )
    {
        mRStatsRunner = std::make_shared<RStatsRunner>( mIface );
        mRConsole = std::make_shared<QgsRStatsConsole>( mIface->mainWindow(), mRStatsRunner, mIface );
    }
}

void RPlugin::initGui() { prepareForUse(); }

void RPlugin::unload()
{
    mIface->pluginMenu()->removeAction( mOpenConsole.get() );
    mOpenConsole.reset();
    mRConsole.reset();
    mRStatsRunner.reset();
}

// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin *classFactory( QgisInterface *qgisInterfacePointer )
{
    return new RPlugin( qgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN const QString *name() { return &sName; }

// Return the description
QGISEXTERN const QString *description() { return &sDescription; }

// Return the category
QGISEXTERN const QString *category() { return &sCategory; }

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type() { return sPluginType; }

// Return the version number for the plugin
QGISEXTERN const QString *version() { return &sPluginVersion; }

QGISEXTERN const QString *icon() { return &sPluginIcon; }

// Delete ourself
QGISEXTERN void unload( QgisPlugin *pluginPointer ) { delete pluginPointer; }
