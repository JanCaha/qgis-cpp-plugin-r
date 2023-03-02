#include "QThread"

#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include "maplayerwrapper.h"
#include "scopedprogresstask.h"

MapLayerWrapper::MapLayerWrapper( const QgsMapLayer *layer ) : mLayerId( layer ? layer->id() : QString() ) {}

std::string MapLayerWrapper::id() const { return mLayerId.toStdString(); }

QgsMapLayer *MapLayerWrapper::mapLayer() const { return QgsProject::instance()->mapLayer( mLayerId ); }

long long MapLayerWrapper::featureCount() const
{
    long long res = -1;
    auto countOnMainThread = [&res, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "featureCount",
                    "featureCount must be run on the main thread" );

        if ( QgsMapLayer *layer = mapLayer() )
        {
            if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
            {
                res = vl->featureCount();
            }
        }
    };

    QMetaObject::invokeMethod( qApp, countOnMainThread, Qt::BlockingQueuedConnection );
    return res;
}

Rcpp::DataFrame MapLayerWrapper::toDataFrame( bool selectedOnly )
{
    Rcpp::DataFrame result = Rcpp::DataFrame();

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

    QList<int> attributesToFetch;
    for ( int index = 0; index < fields.count(); ++index )
    {
        Rcpp::RObject column;
        const QgsField field = fields.at( index );

        switch ( field.type() )
        {
            case QVariant::Bool:
            {
                column = Rcpp::LogicalVector( featureCount );
                break;
            }
            case QVariant::Int:
            {
                column = Rcpp::IntegerVector( featureCount );
                break;
            }
            case QVariant::Double:
            {
                column = Rcpp::DoubleVector( featureCount );
                break;
            }
            case QVariant::LongLong:
            {
                column = Rcpp::DoubleVector( featureCount );
                break;
            }
            case QVariant::String:
            {
                column = Rcpp::StringVector( featureCount );
                break;
            }

            default:
                continue;
        }

        result.push_back( column, field.name().toStdString() );
        attributesToFetch.append( index );
    }

    if ( selectedOnly && selectedFeatureIds.empty() )
        return result;

    QgsFeature feature;
    QgsFeatureRequest req;
    req.setFlags( QgsFeatureRequest::NoGeometry );
    req.setSubsetOfAttributes( attributesToFetch );
    if ( selectedOnly )
        req.setFilterFids( selectedFeatureIds );

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

        int settingColumn = 0;

        const QgsAttributes attributes = feature.attributes();
        const QVariant *attributeData = attributes.constData();

        for ( int i = 0; i < fields.count(); i++, attributeData++ )
        {
            QgsField field = fields.at( i );

            switch ( field.type() )
            {
                case QVariant::Bool:
                {
                    Rcpp::LogicalVector column = result[settingColumn];
                    column[featureNumber] = attributeData->toBool();
                    break;
                }
                case QVariant::Int:
                {
                    Rcpp::IntegerVector column = result[settingColumn];
                    column[featureNumber] = attributeData->toInt();
                    break;
                }
                case QVariant::LongLong:
                {
                    Rcpp::DoubleVector column = result[settingColumn];
                    bool ok;
                    double val = attributeData->toDouble( &ok );
                    if ( ok )
                        column[featureNumber] = val;
                    else
                        column[featureNumber] = R_NaReal;
                    break;
                }
                case QVariant::Double:
                {
                    Rcpp::DoubleVector column = result[settingColumn];
                    column[featureNumber] = attributeData->toDouble();
                    break;
                }
                case QVariant::String:
                {
                    Rcpp::StringVector column = result[settingColumn];
                    column[featureNumber] = attributeData->toString().toStdString();
                    break;
                }

                default:
                    continue;
            }
            settingColumn++;
        }
        featureNumber++;
    }
    return result;
}

Rcpp::NumericVector MapLayerWrapper::toNumericVector( const std::string &fieldName, bool selectedOnly )
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

SEXP MapLayerWrapper::toSf()
{
    bool prepared = false;
    QString path;
    QString layerName;
    auto prepareOnMainThread = [&prepared, &path, &layerName, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "toSf",
                    "prepareOnMainThread must be run on the main thread" );

        prepared = false;
        if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( mLayerId ) )
        {
            if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
            {
                if ( vlayer->dataProvider()->name() != QStringLiteral( "ogr" ) )
                    return;

                const QVariantMap parts =
                    QgsProviderRegistry::instance()->decodeUri( layer->dataProvider()->name(), layer->source() );
                path = parts[QStringLiteral( "path" )].toString();
                layerName = parts[QStringLiteral( "layerName" )].toString();
                prepared = true;
            }
        }
    };

    QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );
    if ( !prepared )
        return R_NilValue;

    if ( path.isEmpty() )
        return R_NilValue;

    Rcpp::Function st_read( "st_read" );

    return st_read( path.toStdString(), layerName.toStdString() );
}

Rcpp::LogicalVector MapLayerWrapper::isVectorLayer()
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

Rcpp::LogicalVector MapLayerWrapper::isRasterLayer()
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

SEXP MapLayerWrapper::toRaster() { return this->toRasterDataObject( RasterPackage::raster ); }

SEXP MapLayerWrapper::toTerra() { return this->toRasterDataObject( RasterPackage::terra ); }

SEXP MapLayerWrapper::toStars() { return this->toRasterDataObject( RasterPackage::stars ); }

SEXP MapLayerWrapper::toRasterDataObject( RasterPackage rasterPackage )
{
    if ( !this->isRasterLayer()( 0 ) )
        return R_NilValue;

    bool prepared = false;
    QString rasterPath;

    auto prepareOnMainThread = [&rasterPath, &prepared, this]
    {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "toRaster",
                    "prepareOnMainThread must be run on the main thread" );

        if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( mLayerId ) )
        {
            if ( QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer ) )
            {
                rasterPath = rlayer->dataProvider()->dataSourceUri();
            }
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
