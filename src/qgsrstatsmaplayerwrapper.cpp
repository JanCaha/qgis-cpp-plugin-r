#include <Rcpp.h>
#include <RcppCommon.h>

#include <QString>
#include <QThread>

#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayerfeatureiterator.h"

#include "scopedprogresstask.h"
#include "qgsrstatsmaplayerwrapper.h"
#include "qgsrstatsutils.h"

QgsRstatsMapLayerWrapper::QgsRstatsMapLayerWrapper( const QgsMapLayer *layer )
{
    auto idOnMainThread = [&layer, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "idOnMainThread",
                    "idOnMainThread must be run on the main thread" );
        mLayerId = layer ? layer->id() : QString();
    };
    QMetaObject::invokeMethod( qApp, idOnMainThread, Qt::BlockingQueuedConnection );
}

std::string QgsRstatsMapLayerWrapper::id() const { return mLayerId.toStdString(); }

SEXP QgsRstatsMapLayerWrapper::featureCount() const
{
    if ( isRasterLayer() )
        return R_NilValue;

    long long res = -1;
    auto countOnMainThread = [&res, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "featureCount",
                    "featureCount must be run on the main thread" );

        if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( mLayerId ) )
        {
            if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
            {
                res = vl->featureCount();
            }
        }
    };

    QMetaObject::invokeMethod( qApp, countOnMainThread, Qt::BlockingQueuedConnection );

    return Rcpp::wrap( res );
}

Rcpp::DataFrame QgsRstatsMapLayerWrapper::toDataFrame( bool selectedOnly ) const
{
    Rcpp::DataFrame result = Rcpp::DataFrame();

    bool prepared = false;
    QgsFields fields;
    long long featureCount = -1;
    std::unique_ptr<QgsVectorLayerFeatureSource> source;
    std::unique_ptr<ScopedProgressTask> task;
    QgsFeatureIds selectedFeatureIds;

    auto prepareSourceFeatureCountOnMainThread =
        [&prepared, &fields, &featureCount, &source, &task, selectedOnly, &selectedFeatureIds, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "toDataFrame",
                    "toDataFrame must be run on the main thread" );

        prepared = false;

        if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( mLayerId ) )
        {
            if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
            {
                fields = vlayer->fields();
                source = std::make_unique<QgsVectorLayerFeatureSource>( vlayer );
                if ( selectedOnly )
                {
                    selectedFeatureIds = vlayer->selectedFeatureIds();
                    featureCount = selectedFeatureIds.size();
                }
                else
                {
                    featureCount = vlayer->featureCount();
                }
            }
        }

        prepared = true;

        task = std::make_unique<ScopedProgressTask>( QObject::tr( "Creating R dataframe" ), true );
    };

    QMetaObject::invokeMethod( qApp, prepareSourceFeatureCountOnMainThread, Qt::BlockingQueuedConnection );

    if ( !prepared )
        return result;

    QList<int> attributesToFetch;

    auto prepareAttributesOnMainThread = [&attributesToFetch, &result, &fields, &featureCount]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "toDataFrame",
                    "toDataFrame must be run on the main thread" );

        for ( int index = 0; index < fields.count(); ++index )
        {
            const QgsField field = fields.at( index );

            if ( QgsRstatsUtils::canConvertToRcpp( field ) )
            {
                result.push_back( QgsRstatsUtils::fieldToRcppVector( field, featureCount ),
                                  field.name().toStdString() );
                attributesToFetch.append( index );
            }
        }
    };

    QMetaObject::invokeMethod( qApp, prepareAttributesOnMainThread, Qt::BlockingQueuedConnection );

    if ( selectedOnly && selectedFeatureIds.empty() )
        return result;

    QgsFeature feature;
    QgsFeatureRequest req;
    req.setFlags( QgsFeatureRequest::NoGeometry );
    req.setSubsetOfAttributes( attributesToFetch );
    if ( selectedOnly )
        req.setFilterFids( selectedFeatureIds );

    auto prepareFeaturesOnMainThread = [&source, &result, &req, &feature, &featureCount, &task]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "toDataFrame",
                    "toDataFrame must be run on the main thread" );

        QgsFeatureIterator it = source->getFeatures( req );
        std::size_t featureNumber = 0;

        int prevProgress = 0;
        while ( it.nextFeature( feature ) )
        {
            const int progress = 100 * static_cast<double>( featureNumber ) / featureCount;
            if ( progress > prevProgress )
            {
                task->setProgress( progress );
                prevProgress = progress;
            }

            if ( task->isCanceled() )
                break;

            QgsRstatsUtils::addFeatureToDf( feature, featureNumber, result );
            featureNumber++;
        }
    };

    QMetaObject::invokeMethod( qApp, prepareFeaturesOnMainThread, Qt::BlockingQueuedConnection );

    return result;
}

Rcpp::NumericVector QgsRstatsMapLayerWrapper::toNumericVector( const std::string &fieldName, bool selectedOnly )
{
    Rcpp::NumericVector result;

    bool prepared = false;
    QgsFields fields;
    long long featureCount = -1;
    std::unique_ptr<QgsVectorLayerFeatureSource> source;
    std::unique_ptr<ScopedProgressTask> task;
    QgsFeatureIds selectedFeatureIds;

    auto prepareOnMainThread =
        [&prepared, &fields, &featureCount, &source, &task, selectedOnly, &selectedFeatureIds, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "toDataFrame",
                    "prepareOnMainThread must be run on the main thread" );

        prepared = false;
        if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( mLayerId ) )
        {
            if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
            {
                fields = vlayer->fields();
                source = std::make_unique<QgsVectorLayerFeatureSource>( vlayer );
                if ( selectedOnly )
                {
                    selectedFeatureIds = vlayer->selectedFeatureIds();
                    featureCount = selectedFeatureIds.size();
                }
                else
                {
                    featureCount = vlayer->featureCount();
                }
            }
        }
        prepared = true;

        task = std::make_unique<ScopedProgressTask>( QObject::tr( "Creating R dataframe" ), true );
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    if ( !prepared )
        return result;

    const int fieldIndex = fields.lookupField( QString::fromStdString( fieldName ) );
    if ( fieldIndex < 0 )
        return result;

    const QgsField field = fields.at( fieldIndex );
    if ( !( field.type() == QVariant::Double || field.type() == QVariant::Int ) )
        return result;

    result = Rcpp::NumericVector( featureCount, 0 );
    if ( selectedOnly && selectedFeatureIds.empty() )
        return result;

    std::size_t i = 0;

    QgsFeature feature;
    QgsFeatureRequest req;
    req.setFlags( QgsFeatureRequest::NoGeometry );
    req.setSubsetOfAttributes( { fieldIndex } );
    if ( selectedOnly )
        req.setFilterFids( selectedFeatureIds );

    QgsFeatureIterator it = source->getFeatures( req );

    int prevProgress = 0;
    while ( it.nextFeature( feature ) )
    {
        const int progress = 100 * static_cast<double>( i ) / featureCount;
        if ( progress > prevProgress )
        {
            task->setProgress( progress );
            prevProgress = progress;
        }

        if ( task->isCanceled() )
            break;

        result[i] = feature.attribute( fieldIndex ).toDouble();
        i++;
    }

    return result;
}

SEXP QgsRstatsMapLayerWrapper::readAsSf()
{
    if ( !isVectorLayer() )
    {
        return R_NilValue;
    }

    bool prepared = false;
    QString path;
    QString layerName;

    auto prepareOnMainThread = [&prepared, &path, &layerName, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "readAsSf",
                    "prepareOnMainThread must be run on the main thread" );

        prepared = false;
        if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( mLayerId ) )
        {
            if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
            {
                if ( vlayer->dataProvider()->name() == QStringLiteral( "ogr" ) )
                {
                    const QVariantMap parts =
                        QgsProviderRegistry::instance()->decodeUri( layer->dataProvider()->name(), layer->source() );
                    path = parts[QStringLiteral( "path" )].toString();
                    layerName = parts[QStringLiteral( "layerName" )].toString();
                    prepared = true;
                }
            }
        }
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );
    if ( !prepared )
        return R_NilValue;

    if ( path.isEmpty() )
        return R_NilValue;

    Rcpp::Function st_read( "st_read", Rcpp::Environment::namespace_env( "sf" ) );

    return st_read( path.toStdString(), layerName.toStdString() );
}

bool QgsRstatsMapLayerWrapper::isVectorLayer() const
{
    bool prepared;
    bool isVectorLayer = false;

    auto prepareOnMainThread = [&isVectorLayer, &prepared, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "isVectorLayer",
                    "prepareOnMainThread must be run on the main thread" );

        prepared = false;
        if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( mLayerId ) )
        {
            if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
            {
                Q_UNUSED( vlayer );
                isVectorLayer = true;
            }
        }
        prepared = true;
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );
    if ( !prepared )
        return false;

    return isVectorLayer;
}

bool QgsRstatsMapLayerWrapper::isRasterLayer() const
{
    bool prepared;
    bool isRasterLayer = false;

    auto prepareOnMainThread = [&isRasterLayer, &prepared, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "isRasterLayer",
                    "prepareOnMainThread must be run on the main thread" );

        prepared = false;
        if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( mLayerId ) )
        {
            if ( QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer ) )
            {
                Q_UNUSED( rlayer );
                isRasterLayer = true;
            }
        }
        prepared = true;
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );
    if ( !prepared )
        return false;

    return isRasterLayer;
}

enum RasterPackage
{
    raster,
    stars,
    terra
};

SEXP QgsRstatsMapLayerWrapper::toRaster() { return this->toRasterDataObject( RasterPackage::raster ); }

SEXP QgsRstatsMapLayerWrapper::toTerra() { return this->toRasterDataObject( RasterPackage::terra ); }

SEXP QgsRstatsMapLayerWrapper::toStars() { return this->toRasterDataObject( RasterPackage::stars ); }

QgsMapLayer *QgsRstatsMapLayerWrapper::mapLayer() const
{
    QgsMapLayer *mapLayer;

    auto prepareOnMainThread = [&mapLayer, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "toDataFrame",
                    "prepareOnMainThread must be run on the main thread" );

        mapLayer = QgsProject::instance()->mapLayer( mLayerId );
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    return mapLayer;
}

QgsRasterLayer *QgsRstatsMapLayerWrapper::rasterLayer() const
{
    QgsRasterLayer *rlayer = nullptr;

    auto prepareOnMainThread = [&rlayer, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "rasterLayer",
                    "prepareOnMainThread must be run on the main thread" );

        rlayer = QgsProject::instance()->mapLayer<QgsRasterLayer *>( mLayerId );
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    return rlayer;
}

QgsVectorLayer *QgsRstatsMapLayerWrapper::vectorLayer() const
{
    QgsVectorLayer *vlayer = nullptr;

    auto prepareOnMainThread = [&vlayer, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "rasterLayer",
                    "prepareOnMainThread must be run on the main thread" );

        vlayer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( mLayerId );
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    return vlayer;
}

SEXP QgsRstatsMapLayerWrapper::toRasterDataObject( RasterPackage rasterPackage )
{
    if ( !this->isRasterLayer() )
        return R_NilValue;

    bool prepared = false;
    QString rasterPath;

    auto prepareOnMainThread = [&rasterPath, &prepared, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "toRaster",
                    "prepareOnMainThread must be run on the main thread" );

        if ( QgsRasterLayer *rlayer = rasterLayer() )
        {
            rasterPath = rlayer->dataProvider()->dataSourceUri();
        }

        prepared = true;
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );

    if ( !prepared )
        return R_NilValue;

    if ( rasterPath.isEmpty() )
        return R_NilValue;

    switch ( rasterPackage )
    {
        case RasterPackage::raster:
        {
            Rcpp::Function raster( "raster", Rcpp::Environment::namespace_env( "raster" ) );
            return raster( rasterPath.toStdString() );
        }
        case RasterPackage::terra:
        {
            Rcpp::Function rast( "rast", Rcpp::Environment::namespace_env( "terra" ) );
            return rast( rasterPath.toStdString() );
        }
        case RasterPackage::stars:
        {
            Rcpp::Function read_stars( "read_stars", Rcpp::Environment::namespace_env( "stars" ) );
            return read_stars( rasterPath.toStdString() );
        }
        default:
            return Rcpp::wrap( rasterPath.toStdString() );
    }
}

std::string QgsRstatsMapLayerWrapper::rClassName() { return "QgsMapLayerWrapper"; }

Rcpp::CharacterVector QgsRstatsMapLayerWrapper::functions()
{
    Rcpp::CharacterVector ret;
    ret.push_back( "id" );
    ret.push_back( "featureCount" );
    ret.push_back( "asDataFrame(onlySelected)" );
    ret.push_back( "toNumericVector(fieldName, onlySelected)" );
    ret.push_back( "readAsSf()" );
    return ret;
}

std::string QgsRstatsMapLayerWrapper::s3FunctionForClass( std::string functionName )
{
    return functionName + "." + rClassName();
}
