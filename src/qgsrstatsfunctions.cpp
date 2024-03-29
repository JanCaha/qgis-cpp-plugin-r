#include <functional>

#include <QString>
#include <QThread>
#include <QVariant>

#include <Rcpp.h>

#include "qgsfield.h"
#include "qgsfields.h"
#include "qgsmemoryproviderutils.h"
#include "qgsproject.h"
#include "qgsproxyprogresstask.h"
#include "qgsvectorlayer.h"

#include "qgsrstatsapplicationwrapper.h"
#include "qgsrstatsfunctions.h"
#include "qgsrstatsmaplayerwrapper.h"
#include "qgsrstatsutils.h"

SEXP QgRstatsFunctions::DollarMapLayer( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj, std::string name )
{
    if ( name == "id" )
    {
        return Rcpp::wrap( obj->id() );
    }
    else if ( name == "featureCount" )
    {
        return obj->featureCount();
    }
    else if ( name == "toDataFrame" )
    {
        std::function<SEXP( bool, bool )> func =
            std::bind( &toDataFrame, obj, std::placeholders::_1, std::placeholders::_2 );
        return Rcpp::InternalFunction( func );
    }
    else if ( name == "readAsSf" )
    {
        return obj->readAsSf();
    }
    else if ( name == "isVectorLayer" )
    {
        return Rcpp::wrap( obj->isVectorLayer() );
    }
    else if ( name == "toSf" )
    {
        return Rcpp::wrap( obj->toDataFrame( true, false ) );
    }
    else if ( name == "tableToDf" )
    {
        return Rcpp::wrap( obj->toDataFrame( false, false ) );
    }
    else if ( name == "isRasterLayer" )
    {
        return Rcpp::wrap( obj->isRasterLayer() );
    }
    else if ( name == "toNumericVector" )
    {
        std::function<SEXP( std::string, bool )> func =
            std::bind( &toNumericVector, obj, std::placeholders::_1, std::placeholders::_2 );
        return Rcpp::InternalFunction( func );
    }
    else
    {
        return NULL;
    }
}

void QgRstatsFunctions::printApplicationWrapper()
{
    Rcpp::print( Rcpp::wrap( QgsRstatsApplicationWrapper::rClassName() ) );
}

void QgRstatsFunctions::printMapLayerWrapper( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj )
{
    Rcpp::print( Rcpp::wrap( QgsRstatsMapLayerWrapper::rClassName() + "(" + obj->id() + ")" ) );
}

// The function which is called when running QGIS$...
SEXP QgRstatsFunctions::Dollar( Rcpp::XPtr<QgsRstatsApplicationWrapper> obj, std::string name )
{
    if ( name == "versionInt" )
    {
        return Rcpp::wrap( obj->version() );
    }
    else if ( name == "activeLayer" )
    {
        return obj->activeLayer();
    }
    else if ( name == "mapLayers" )
    {
        return obj->mapLayers();
    }
    else if ( name == "mapLayerByName" )
    {
        std::function<SEXP( std::string )> func = std::bind( &mapLayerByName, obj, std::placeholders::_1 );
        return Rcpp::InternalFunction( func );
    }
    else if ( name == "projectCrs" )
    {
        return obj->projectCrs();
    }
    else if ( name == "dfToLayer" )
    {
        return Rcpp::InternalFunction( &dfToLayer );
    }
    else
    {
        return NULL;
    }
}

SEXP QgRstatsFunctions::dfToLayer( SEXP data )
{
    if ( !Rcpp::is<Rcpp::DataFrame>( data ) )
        return Rcpp::wrap( false );

    Rcpp::DataFrame df = Rcpp::as<Rcpp::DataFrame>( data );

    bool isDataFrame = df.inherits( "data.frame" );

    if ( !isDataFrame )
        return Rcpp::wrap( false );

    Rcpp::StringVector dfColumnNames = df.names();

    bool prepared = false;
    QgsVectorLayer *resultLayer = nullptr;
    std::unique_ptr<QgsScopedProxyProgressTask> task;
    QgsFields fields = QgsFields();
    std::string geometryColumnName;

    // TODO run everything from main layer

    auto prepareOnMainThread = [&geometryColumnName, &fields, &prepared, &df, &task, &resultLayer]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "dfToQGIS",
                    "prepareOnMainThread must be run on the main thread" );

        QgsRstatsUtils::preparedFieldsFromDf( df, fields );

        Qgis::WkbType wkbType = QgsRstatsUtils::wkbType( df );
        QgsCoordinateReferenceSystem crs = QgsRstatsUtils::crs( df );

        if ( QgsRstatsUtils::hasSfColumn( df ) )
        {
            geometryColumnName = QgsRstatsUtils::geometryColumn( df );
        }

        resultLayer = QgsMemoryProviderUtils::createMemoryLayer( QStringLiteral( "R_layer" ), fields, wkbType, crs );

        task = std::make_unique<QgsScopedProxyProgressTask>( QObject::tr( "Creating QGIS layer from R dataframe" ) );
        prepared = true;
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    if ( !prepared )
        return Rcpp::wrap( false );

    prepared = false;
    auto prepareFeaturesOnMainThread = [&fields, &prepared, &df, &task, &resultLayer]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "dfToQGIS",
                    "prepareOnMainThread must be run on the main thread" );

        Rcpp::StringVector geometries = QgsRstatsUtils::geometries( df );

        QgsFeatureList features = QgsFeatureList();

        for ( int i = 0; i < df.nrows(); i++ )
        {

            QgsFeature feature( fields );

            QgsRstatsUtils::prepareFeature( feature, df, i, geometries );

            features.append( feature );

            const double progress = 100 * ( double( i ) / double( df.nrows() ) );
            task->setProgress( progress );
        }

        resultLayer->dataProvider()->addFeatures( features );
        QgsProject::instance()->addMapLayer( resultLayer );

        prepared = true;
    };

    QMetaObject::invokeMethod( qApp, prepareFeaturesOnMainThread, Qt::BlockingQueuedConnection );

    return Rcpp::wrap( prepared );
}

SEXP QgRstatsFunctions::toDataFrame( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj, bool includeGeometry, bool selectedOnly )
{
    return obj->toDataFrame( includeGeometry, selectedOnly );
}

SEXP tableToDf( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj ) { return obj->toDataFrame( false, false ); }

SEXP toSf( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj ) { return obj->toDataFrame( true, false ); }

SEXP QgRstatsFunctions::toNumericVector( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj, const std::string &field,
                                         bool selectedOnly )
{
    return obj->toNumericVector( field, selectedOnly );
}

SEXP QgRstatsFunctions::mapLayerByName( Rcpp::XPtr<QgsRstatsApplicationWrapper> obj, std::string name )
{
    return obj->mapLayerByName( name );
}

SEXP QgRstatsFunctions::toRaster( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj ) { return obj->toRaster(); }

SEXP QgRstatsFunctions::toTerra( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj ) { return obj->toTerra(); }

SEXP QgRstatsFunctions::toStars( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj ) { return obj->toStars(); }
