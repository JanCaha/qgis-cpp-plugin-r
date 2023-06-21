#ifndef RPLUGIN_H
#define RPLUGIN_H

#include <QAction>
#include <QApplication>

#include "qgis.h"
#include "qgisinterface.h"
#include "qgisplugin.h"

#include "qgsrstatsconsole.h"
#include "qgsrstatsrunner.h"

class RPlugin : public QObject, public QgisPlugin
{
        Q_OBJECT

    public:
        explicit RPlugin( QgisInterface *iface );
        void initGui() override;
        void unload() override;
        void prepareForUse();
        void showConsole();

    private:
        std::shared_ptr<QgisInterface> mIface = nullptr;
        std::shared_ptr<QgsRStatsRunner> mRStatsRunner = nullptr;
        std::shared_ptr<QgsRStatsConsole> mRConsole = nullptr;
        std::shared_ptr<QAction> mOpenConsole = nullptr;
        std::shared_ptr<QgsRStatsSettingsOptionsFactory> mRSettingsFactory = nullptr;
};

static const QString sName = QStringLiteral( "R Console Plugin" );
static const QString sDescription =
    QStringLiteral( "Plugin that allows running R directly in QGIS in form or R Console." );
static const QString sCategory = QStringLiteral( "Vector" );
static const QString sPluginVersion = QStringLiteral( "0.1" );
static const QgisPlugin::PluginType sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = QStringLiteral( ":/rplugin/icons/R_logo.svg" );

#endif // CPPTOOLSPLUGIN_H
