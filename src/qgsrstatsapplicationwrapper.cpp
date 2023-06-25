#include <QList>
#include <QMap>
#include <QMetaObject>
#include <QString>
#include <QThread>

#include <RcppCommon.h>

#include "qgsmaplayer.h"
#include "qgsproject.h"

#include "qgsrstatsapplicationwrapper.h"
#include "qgsrstatsfunctions.h"
#include "qgsrstatsmaplayerwrapper.h"

int QgsRstatsApplicationWrapper::version() const { return Qgis::versionInt(); }

SEXP QgsRstatsApplicationWrapper::activeLayer() const
{
    QgsMapLayer *mapLayer;

    auto prepareOnMainThread = [&mapLayer, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "activeLayer",
                    "QGIS$activeLayer must be run on the main thread" );

        mapLayer = mIface->activeLayer();
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    Rcpp::XPtr<QgsRstatsMapLayerWrapper> res( new QgsRstatsMapLayerWrapper( mapLayer ) );
    res.attr( "class" ) = QgsRstatsMapLayerWrapper::rClassName();
    return res;
}

SEXP QgsRstatsApplicationWrapper::mapLayers()
{
    std::vector<std::string> layersNames;

    auto prepareOnMainThread = [&layersNames]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "mapLayers",
                    "QGIS$mapLayers must be run on the main thread" );

        QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();

        for ( QString name : layers.keys() )
        {
            layersNames.push_back( name.toStdString() );
        }
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    if ( layersNames.empty() )
    {
        return R_NilValue;
    }

    return Rcpp::wrap( layersNames );
}

SEXP QgsRstatsApplicationWrapper::mapLayerByName( std::string layerName )
{
    QgsMapLayer *mapLayer;

    auto prepareOnMainThread = [&mapLayer, &layerName]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "mapLayerByName",
                    "QGIS$mapLayerByName(layerName) must be run on the main thread" );

        QList<QgsMapLayer *> mapLayers = QgsProject::instance()->mapLayersByName( QString::fromStdString( layerName ) );

        mapLayer = mapLayers.at( 0 );
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    if ( !mapLayer )
        return R_NilValue;

    Rcpp::XPtr<QgsRstatsMapLayerWrapper> res( new QgsRstatsMapLayerWrapper( mapLayer ) );
    res.attr( "class" ) = QgsRstatsMapLayerWrapper::rClassName();
    return res;
}

SEXP QgsRstatsApplicationWrapper::projectCrs()
{
    Rcpp::Function st_crs( "st_crs", Rcpp::Environment::namespace_env( "sf" ) );
    SEXP result;

    auto prepareOnMainThread = [&st_crs, &result]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "projectCrs",
                    "QGIS$projectCrs must be run on the main thread" );

        result = st_crs( QgsProject::instance()->crs().toWkt().toStdString() );
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    return result;
}

std::string QgsRstatsApplicationWrapper::rClassName() { return "QGIS"; }

Rcpp::CharacterVector QgsRstatsApplicationWrapper::functions()
{
    Rcpp::CharacterVector ret;
    ret.push_back( "version" );
    ret.push_back( "activeLayer" );
    ret.push_back( "mapLayers" );
    ret.push_back( "mapLayerByName(layerName)" );
    ret.push_back( "projectCrs" );
    ret.push_back( "dfToLayer(df)" );
    return ret;
}

Rcpp::XPtr<QgsRstatsApplicationWrapper> QgsRstatsApplicationWrapper::instance( std::shared_ptr<QgisInterface> iface )
{
    Rcpp::XPtr<QgsRstatsApplicationWrapper> qgiswrapper( new QgsRstatsApplicationWrapper() );
    qgiswrapper->mIface = iface;
    qgiswrapper.attr( "class" ) = QgsRstatsApplicationWrapper::rClassName();
    return qgiswrapper;
}
