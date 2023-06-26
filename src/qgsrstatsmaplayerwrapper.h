#ifndef QGSRSTATSMAPLAYERWRAPPER_H
#define QGSRSTATSMAPLAYERWRAPPER_H

#include <Rcpp.h>
#include <RcppCommon.h>

#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

class QgsRstatsMapLayerWrapper
{
    public:
        enum RasterPackage
        {
            raster,
            stars,
            terra
        };

        QgsRstatsMapLayerWrapper( const QgsMapLayer *layer = nullptr );

        std::string id() const;

        SEXP featureCount() const;

        Rcpp::DataFrame toDataFrame( bool includeGeometry, bool selectedOnly ) const;

        Rcpp::NumericVector toNumericVector( const std::string &fieldName, bool selectedOnly );

        SEXP readAsSf();

        bool isVectorLayer() const;

        bool isRasterLayer() const;

        SEXP toRaster();

        SEXP toTerra();

        SEXP toStars();

        static Rcpp::CharacterVector functions();
        static std::string rClassName();
        static std::string s3FunctionForClass( std::string functionName );

    private:
        QString mLayerId;

        SEXP toRasterDataObject( RasterPackage rasterPackage );
};

#endif // QGSRSTATSMAPLAYERWRAPPER_H
