#ifndef MAPLAYERWRAPPER_H
#define MAPLAYERWRAPPER_H

#include <Rcpp.h>

#include "QString"

#include "qgisinterface.h"
#include "qgsmaplayer.h"

class MapLayerWrapper
{
    public:
        enum RasterPackage
        {
            raster,
            stars,
            terra
        };

        MapLayerWrapper( const std::shared_ptr<QgsMapLayer> layer = nullptr,
                         std::shared_ptr<QgisInterface> iface = nullptr );

        std::string id() const;
        long long featureCount() const;

        Rcpp::LogicalVector isRasterLayer();
        Rcpp::LogicalVector isVectorLayer();

        Rcpp::DataFrame toDataFrame( bool selectedOnly );
        Rcpp::NumericVector toNumericVector( const std::string &fieldName, bool selectedOnly );
        SEXP toSf();
        SEXP toRaster();
        SEXP toTerra();
        SEXP toStars();

    private:
        QString mLayerId;
        std::shared_ptr<QgisInterface> mIface;

        SEXP toRasterDataObject( RasterPackage rasterPackage );
        QgsMapLayer *mapLayer();
};

#endif