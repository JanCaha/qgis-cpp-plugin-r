#ifndef QGSRSTATSMAPLAYERWRAPPER_H
#define QGSRSTATSMAPLAYERWRAPPER_H

#include <RcppCommon.h>
#include <Rcpp.h>

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

    Rcpp::DataFrame asDataFrame( bool selectedOnly ) const;

    Rcpp::NumericVector toNumericVector( const std::string &fieldName, bool selectedOnly );

    SEXP readAsSf();

    bool isVectorLayer() const;

    bool isRasterLayer() const;

    SEXP toRaster();

    SEXP toTerra();

    SEXP toStars();

    QgsRasterLayer *rasterLayer() const;

    QgsVectorLayer *vectorLayer() const;

    static Rcpp::CharacterVector functions();
    static std::string rClassName();
    static std::string s3FunctionForClass( std::string functionName );

  private:
    QString mLayerId;

    SEXP toRasterDataObject( RasterPackage rasterPackage );

    QgsMapLayer *mapLayer() const;

};

#endif // QGSRSTATSMAPLAYERWRAPPER_H
