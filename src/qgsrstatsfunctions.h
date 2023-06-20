#ifndef QGRSSTATSFUNCTIONS_H
#define QGRSSTATSFUNCTIONS_H

#include <RcppCommon.h>
#include <Rcpp.h>

#include "qgsrstatsapplicationwrapper.h"
#include "qgsrstatsmaplayerwrapper.h"

class QgRstatsFunctions
{
  public:
    static SEXP Dollar( Rcpp::XPtr<QgsRstatsApplicationWrapper> obj, std::string name );
    static SEXP DollarMapLayer( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj, std::string name );

    static SEXP mapLayerByName( Rcpp::XPtr<QgsRstatsApplicationWrapper> obj, std::string name );
    static SEXP dfToLayer( SEXP data );

    static void printMapLayerWrapper( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj );
    static void printApplicationWrapper();

    static SEXP readAsSf( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj );

    static SEXP asDataFrame( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj, bool selectedOnly );
    static SEXP toNumericVector( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj, const std::string &field, bool selectedOnly );

    static SEXP toRaster( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj );
    static SEXP toTerra( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj );
    static SEXP toStars( Rcpp::XPtr<QgsRstatsMapLayerWrapper> obj );
};

#endif // QGRSSTATSFUNCTIONS_H
