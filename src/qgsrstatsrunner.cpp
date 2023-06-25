#include <QDir>
#include <QFile>
#include <QString>
#include <QVariant>

#include <RInside.h>
#include <Rcpp.h>
#include <RcppCommon.h>

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmemoryproviderutils.h"
#include "qgsproviderregistry.h"
#include "qgsproxyprogresstask.h"
#include "qgsrasterlayer.h"
#include "qgstaskmanager.h"
#include "qgsvariantutils.h"

#include "qgsrstatsmaplayerwrapper.h"
#include "qgsrstatsrunner.h"
#include "qgsvectorlayerfeatureiterator.h"

QgsRStatsRunner::QgsRStatsRunner( std::shared_ptr<QgisInterface> iface ) : mIface( iface )
{
    mSession = std::make_unique<QgsRStatsSession>( mIface );
    mSession->moveToThread( &mSessionThread );
    mSessionThread.start();

    connect( mSession.get(), &QgsRStatsSession::consoleMessage, this, &QgsRStatsRunner::consoleMessage );
    connect( mSession.get(), &QgsRStatsSession::showMessage, this, &QgsRStatsRunner::showMessage );
    connect( mSession.get(), &QgsRStatsSession::errorOccurred, this, &QgsRStatsRunner::errorOccurred );
    connect( mSession.get(), &QgsRStatsSession::busyChanged, this, &QgsRStatsRunner::busyChanged );
    connect( mSession.get(), &QgsRStatsSession::commandFinished, this, &QgsRStatsRunner::commandFinished );
}

QgsRStatsRunner::~QgsRStatsRunner()
{
    // todo -- gracefully shut down session!
    mSessionThread.quit();
    mSessionThread.wait();
}

void QgsRStatsRunner::execCommand( const QString &command )
{
    // todo result handling...
    QMetaObject::invokeMethod( mSession.get(), "execCommand", Qt::QueuedConnection, Q_ARG( QString, command ) );
}

bool QgsRStatsRunner::busy() const { return mSession->busy(); }

void QgsRStatsRunner::showStartupMessage()
{
    QMetaObject::invokeMethod( mSession.get(), "showStartupMessage", Qt::QueuedConnection );
}

void QgsRStatsRunner::setLibraryPath()
{
    QMetaObject::invokeMethod( mSession.get(), "setLibraryPath", Qt::QueuedConnection );
}

void QgsRStatsRunner::emptyRMemory() { mSession->emptyRMemory(); }
