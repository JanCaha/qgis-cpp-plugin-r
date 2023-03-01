#include <Rcpp.h>
#include <RcppCommon.h>

#include <QDir>
#include <QFile>
#include <QString>
#include <QVariant>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmemoryproviderutils.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsproxyprogresstask.h"
#include "qgsrasterlayer.h"
#include "qgstaskmanager.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include "maplayerwrapper.h"
#include "qgisapplicationrwrapper.h"
#include "rstatsrunner.h"
#include "scopedprogresstask.h"

RStatsRunner::RStatsRunner( std::shared_ptr<QgisInterface> iface ) : mIface( iface )
{
    mSession = std::make_unique<RStatsSession>( mIface );
    mSession->moveToThread( &mSessionThread );
    mSessionThread.start();

    connect( mSession.get(), &RStatsSession::consoleMessage, this, &RStatsRunner::consoleMessage );
    connect( mSession.get(), &RStatsSession::showMessage, this, &RStatsRunner::showMessage );
    connect( mSession.get(), &RStatsSession::errorOccurred, this, &RStatsRunner::errorOccurred );
    connect( mSession.get(), &RStatsSession::busyChanged, this, &RStatsRunner::busyChanged );
    connect( mSession.get(), &RStatsSession::commandFinished, this, &RStatsRunner::commandFinished );
}

RStatsRunner::~RStatsRunner()
{
    // todo -- gracefully shut down session!
    mSessionThread.quit();
    mSessionThread.wait();
}

void RStatsRunner::execCommand( const QString &command )
{
    // todo result handling...
    QMetaObject::invokeMethod( mSession.get(), "execCommand", Qt::QueuedConnection, Q_ARG( QString, command ) );
}

bool RStatsRunner::busy() const { return mSession->busy(); }

void RStatsRunner::showStartupMessage()
{
    QMetaObject::invokeMethod( mSession.get(), "showStartupMessage", Qt::QueuedConnection );
}

void RStatsRunner::emptyRMemory() { mSession->emptyRMemory(); }