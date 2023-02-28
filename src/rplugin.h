#ifndef RPLUGIN_H
#define RPLUGIN_H

#include <QAction>
#include <QApplication>

#include "qgis.h"
#include "qgisinterface.h"
#include "qgisplugin.h"

#include "gui/rstatsconsole.h"
#include "rstatsrunner.h"

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
        std::shared_ptr<RStatsRunner> mRStatsRunner = nullptr;
        std::shared_ptr<RStatsConsole> mRConsole = nullptr;
        std::shared_ptr<QAction> mOpenConsole = nullptr;
};

static const QString sName = QStringLiteral( "R Plugin" );
static const QString sDescription = QStringLiteral( "Plugin that allows running R directly in QGIS" );
static const QString sCategory = QStringLiteral( "Vector" );
static const QString sPluginVersion = QStringLiteral( "0.1" );
static const QgisPlugin::PluginType sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = QStringLiteral( ":/rplugin/icons/R_logo.svg" );

#endif // CPPTOOLSPLUGIN_H
