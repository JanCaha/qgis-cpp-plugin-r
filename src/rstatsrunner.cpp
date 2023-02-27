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

SEXP dfToQGIS( SEXP data )
{
    if ( !Rcpp::is<Rcpp::DataFrame>( data ) )
        return Rcpp::wrap( false );

    Rcpp::DataFrame df = Rcpp::as<Rcpp::DataFrame>( data );

    bool isDdataFrame = df.inherits( "data.frame" );

    if ( !isDdataFrame )
        return Rcpp::wrap( false );

    bool isSf = df.inherits( "sf" );
    bool hasSfColumAttribute = df.hasAttribute( "sf_column" );

    Rcpp::StringVector dfColumnNames = df.names();

    bool prepared = false;
    QgsVectorLayer *resultLayer = nullptr;
    std::unique_ptr<ScopedProgressTask> task;
    QgsFields fields = QgsFields();
    std::string geometryColumnName;

    auto prepareOnMainThread =
        [&geometryColumnName, &fields, &dfColumnNames, &hasSfColumAttribute, &prepared, &df, &task, &resultLayer]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "dfToQGIS",
                    "prepareOnMainThread must be run on the main thread" );

        for ( int i = 0; i < df.ncol(); i++ )
        {

            QgsField field;
            bool addCurrentField = false;
            QString fieldName = QString::fromStdString( Rcpp::as<std::string>( dfColumnNames( i ) ) );

            switch ( TYPEOF( df[i] ) )
            {
                case ( LGLSXP ):
                {
                    field = QgsField( fieldName, QVariant::Bool );
                    addCurrentField = true;
                    break;
                }
                case ( INTSXP ):
                {
                    field = QgsField( fieldName, QVariant::Int );
                    addCurrentField = true;
                    break;
                }
                case ( REALSXP ):
                {
                    field = QgsField( fieldName, QVariant::Double );
                    addCurrentField = true;
                    break;
                }
                case ( STRSXP ):
                {
                    field = QgsField( fieldName, QVariant::String );
                    addCurrentField = true;
                    break;
                }
            }
            if ( addCurrentField )
                fields.append( field );
        }

        Qgis::WkbType wkbType;
        QgsCoordinateReferenceSystem crs;

        if ( hasSfColumAttribute )
        {
            Rcpp::Function st_geometry_type =
                Rcpp::Function( "st_geometry_type", Rcpp::Environment::namespace_env( "sf" ) );
            Rcpp::StringVector geometryTypeList = st_geometry_type( df, Rcpp::Named( "by_geometry" ) = false );
            QString geometryTypeNameString = QString::fromStdString( Rcpp::as<std::string>( geometryTypeList[0] ) );
            wkbType = QgsGeometry::fromWkt( QString( "%1 ()" ).arg( geometryTypeNameString ) ).wkbType();

            Rcpp::Function st_crs = Rcpp::Function( "st_crs", Rcpp::Environment::namespace_env( "sf" ) );
            Rcpp::List crsList = st_crs( df );
            Rcpp::StringVector crsWkt = crsList["wkt"];
            QString wkt = QString::fromStdString( Rcpp::as<std::string>( crsWkt[0] ) );
            crs = QgsCoordinateReferenceSystem::fromWkt( wkt );

            geometryColumnName = Rcpp::as<std::string>( df.attr( "sf_column" ) );
        }
        else
        {
            wkbType = Qgis::WkbType::NoGeometry;
        }

        resultLayer = QgsMemoryProviderUtils::createMemoryLayer( QStringLiteral( "R_layer" ), fields, wkbType, crs );

        task = std::make_unique<ScopedProgressTask>( QObject::tr( "Creating QGIS layer from R dataframe" ), true );
        prepared = true;
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    if ( !prepared )
        return Rcpp::wrap( false );

    Rcpp::StringVector geometries;
    Rcpp::List geometriesWKB;

    if ( isSf && hasSfColumAttribute )
    {
        Rcpp::Function st_as_binary = Rcpp::Function( "st_as_binary", Rcpp::Environment::namespace_env( "sf" ) );
        Rcpp::Function wkb_translate_wkt =
            Rcpp::Function( "wkb_translate_wkt", Rcpp::Environment::namespace_env( "wk" ) );
        SEXP geometryColumnCall = Rf_lang3( R_DollarSymbol, df, Rf_mkString( geometryColumnName.c_str() ) );
        geometries = wkb_translate_wkt( st_as_binary( Rf_eval( geometryColumnCall, R_GlobalEnv ) ) );
    }

    QgsFeatureList features = QgsFeatureList();

    for ( int i = 0; i < df.nrows(); i++ )
    {

        if ( task->isCanceled() )
            break;

        QgsFeature feature;
        QgsAttributes featureAttributes;
        featureAttributes.reserve( fields.count() );

        const double progress = 100 * ( double( i ) / double( df.nrows() ) );
        int currentAttributeField = 0;

        for ( int j = 0; j < df.ncol(); j++ )
        {
            switch ( TYPEOF( df[j] ) )
            {
                case ( LGLSXP ):
                {
                    Rcpp::LogicalVector column = Rcpp::as<Rcpp::LogicalVector>( df( j ) );
                    featureAttributes.insert( currentAttributeField, column( i ) );
                    currentAttributeField++;
                    break;
                }
                case ( INTSXP ):
                {
                    if ( Rcpp::as<std::string>( dfColumnNames( j ) ) == "fid" )
                        break;
                    Rcpp::IntegerVector column = Rcpp::as<Rcpp::IntegerVector>( df( j ) );
                    featureAttributes.insert( currentAttributeField, column( i ) );
                    currentAttributeField++;
                    break;
                }
                case ( REALSXP ):
                {
                    Rcpp::DoubleVector column = Rcpp::as<Rcpp::DoubleVector>( df( j ) );
                    featureAttributes.insert( currentAttributeField, column( i ) );
                    currentAttributeField++;
                    break;
                }
                case ( STRSXP ):
                {
                    Rcpp::StringVector column = Rcpp::as<Rcpp::StringVector>( df( j ) );
                    featureAttributes.insert( currentAttributeField,
                                              QString::fromStdString( Rcpp::as<std::string>( column( i ) ) ) );
                    currentAttributeField++;
                    break;
                }
            }
        }

        feature.setAttributes( featureAttributes );

        if ( hasSfColumAttribute )
        {
            std::string wkt = Rcpp::as<std::string>( geometries[i] );
            QgsGeometry geom = QgsGeometry::fromWkt( QString::fromStdString( wkt ) );
            feature.setGeometry( geom );
        }

        features.append( feature );
        task->setProgress( progress );
    }

    resultLayer->dataProvider()->addFeatures( features );
    QgsProject::instance()->addMapLayer( resultLayer );
    return Rcpp::wrap( true );
}

SEXP MapLayerWrapperId( Rcpp::XPtr<MapLayerWrapper> obj ) { return Rcpp::wrap( obj->id() ); }

SEXP MapLayerWrapperFeatureCount( Rcpp::XPtr<MapLayerWrapper> obj ) { return Rcpp::wrap( obj->featureCount() ); }

SEXP MapLayerWrapperToDataFrame( Rcpp::XPtr<MapLayerWrapper> obj, bool selectedOnly )
{
    return obj->toDataFrame( selectedOnly );
}

SEXP MapLayerWrapperToNumericVector( Rcpp::XPtr<MapLayerWrapper> obj, const std::string &field, bool selectedOnly )
{
    return obj->toNumericVector( field, selectedOnly );
}

SEXP MapLayerWrapperToSf( Rcpp::XPtr<MapLayerWrapper> obj ) { return obj->toSf(); }

SEXP MapLayerWrapperByName( std::string name )
{
    QList<QgsMapLayer *> layers = QgsProject::instance()->mapLayersByName( QString::fromStdString( name ) );
    if ( !layers.empty() )
    {
        std::shared_ptr<QgsMapLayer> mapLayer;
        mapLayer.reset( layers.at( 0 ) );
        return Rcpp::XPtr<MapLayerWrapper>( new MapLayerWrapper( mapLayer ) );
    }

    return nullptr;
}

SEXP MapLayerWrapperToRaster( Rcpp::XPtr<MapLayerWrapper> obj ) { return obj->toRaster(); }

SEXP MapLayerWrapperToTerra( Rcpp::XPtr<MapLayerWrapper> obj ) { return obj->toTerra(); }

SEXP MapLayerWrapperToStars( Rcpp::XPtr<MapLayerWrapper> obj ) { return obj->toStars(); }

SEXP MapLayerWrapperIsVectorLayer( Rcpp::XPtr<MapLayerWrapper> obj ) { return obj->isVectorLayer(); }

SEXP MapLayerWrapperIsRasterLayer( Rcpp::XPtr<MapLayerWrapper> obj ) { return obj->isRasterLayer(); }

SEXP MapLayerWrapperDollar( Rcpp::XPtr<MapLayerWrapper> obj, std::string name )
{
    if ( name == "id" )
    {
        return Rcpp::wrap( obj->id() );
    }
    else
    {
        return NULL;
    }
}

// The function which is called when running QGIS$...
SEXP Dollar( Rcpp::XPtr<QgisApplicationRWrapper> obj, std::string name )
{
    if ( name == "versionInt" )
    {
        return Rcpp::wrap( obj->version() );
    }
    else if ( name == "activeLayer" )
    {
        return obj->activeLayer();
    }
    else if ( name == "mapLayerByName" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperByName );
    }
    else if ( name == "layerId" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperId );
    }
    else if ( name == "featureCount" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperFeatureCount );
    }
    else if ( name == "toDataFrame" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToDataFrame );
    }
    else if ( name == "toNumericVector" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToNumericVector );
    }
    else if ( name == "toSf" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToSf );
    }
    else if ( name == "dfToQGIS" )
    {
        return Rcpp::InternalFunction( &dfToQGIS );
    }
    else if ( name == "isVectorLayer" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperIsVectorLayer );
    }
    else if ( name == "isRasterLayer" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperIsRasterLayer );
    }
    else if ( name == "toRaster" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToRaster );
    }
    else if ( name == "toTerra" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToTerra );
    }
    else if ( name == "toStars" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToStars );
    }
    else if ( name == "dfToQGIS" )
    {
        return Rcpp::InternalFunction( &dfToQGIS );
    }
    else if ( name == "isVectorLayer" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperIsVectorLayer );
    }
    else if ( name == "isRasterLayer" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperIsRasterLayer );
    }
    else if ( name == "toRaster" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToRaster );
    }
    else if ( name == "toTerra" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToTerra );
    }
    else if ( name == "toStars" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToStars );
    }
    else if ( name == "dfToQGIS" )
    {
        return Rcpp::InternalFunction( &dfToQGIS );
    }
    else if ( name == "isVectorLayer" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperIsVectorLayer );
    }
    else if ( name == "isRasterLayer" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperIsRasterLayer );
    }
    else if ( name == "toRaster" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToRaster );
    }
    else if ( name == "toTerra" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToTerra );
    }
    else if ( name == "toStars" )
    {
        return Rcpp::InternalFunction( &MapLayerWrapperToStars );
    }
    else
    {
        return NULL;
    }
}

// The function listing the elements of QGIS
Rcpp::CharacterVector Names( Rcpp::XPtr<QgisApplicationRWrapper> )
{
    Rcpp::CharacterVector ret;
    ret.push_back( "versionInt" );
    ret.push_back( "activeLayer" );
    ret.push_back( "layerId" );
    ret.push_back( "featureCount" );
    ret.push_back( "mapLayerByName" );
    ret.push_back( "toDataFrame" );
    ret.push_back( "toNumericVector" );
    ret.push_back( "toSf" );
    ret.push_back( "toRaster" );
    ret.push_back( "toTerra" );
    ret.push_back( "toStars" );
    ret.push_back( "isVectorLayer" );
    ret.push_back( "isRasterLayer" );
    ret.push_back( "dfToQGIS" );
    return ret;
}

//
// QgsRStatsRunner
//

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
