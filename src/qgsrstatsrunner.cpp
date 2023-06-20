#include "qgsrstatsrunner.h"

#include <RcppCommon.h>
#include <Rcpp.h>
#include <RInside.h>

#include "qgslogger.h"
#include "qgsvariantutils.h"
#include "qgstaskmanager.h"
#include "qgsproxyprogresstask.h"
#include <QVariant>
#include <QString>
#include <QFile>
#include <QDir>

#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsrasterlayer.h"
#include "qgsmemoryproviderutils.h"
#include "qgsrstatsmaplayerwrapper.h"


QgsRStatsRunner::QgsRStatsRunner(std::shared_ptr<QgisInterface> iface): mIface(iface)
{
  mSession = std::make_unique<QgsRStatsSession>(mIface);
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
  QMetaObject::invokeMethod( mSession.get(), "execCommand", Qt::QueuedConnection,
                             Q_ARG( QString, command ) );
}

bool QgsRStatsRunner::busy() const
{
  return mSession->busy();
}

void QgsRStatsRunner::showStartupMessage()
{
  QMetaObject::invokeMethod( mSession.get(), "showStartupMessage", Qt::QueuedConnection );
}

void QgsRStatsRunner::setLibraryPath()
{
  QMetaObject::invokeMethod( mSession.get(), "setLibraryPath", Qt::QueuedConnection );
}

void QgsRStatsRunner::emptyRMemory()
{
  mSession->emptyRMemory();
}
