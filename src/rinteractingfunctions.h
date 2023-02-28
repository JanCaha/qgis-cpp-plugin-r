#ifndef RINTERACTINGFUNCTIONS_H
#define RINTERACTINGFUNCTIONS_H

#include <Rcpp.h>
#include <RcppCommon.h>

#include "maplayerwrapper.h"
#include "qgisapplicationrwrapper.h"

SEXP dfToQGIS( SEXP data );

SEXP MapLayerWrapperId( Rcpp::XPtr<MapLayerWrapper> obj );

SEXP MapLayerWrapperFeatureCount( Rcpp::XPtr<MapLayerWrapper> obj );

SEXP MapLayerWrapperToDataFrame( Rcpp::XPtr<MapLayerWrapper> obj, bool selectedOnly );

SEXP MapLayerWrapperToNumericVector( Rcpp::XPtr<MapLayerWrapper> obj, const std::string &field, bool selectedOnly );

SEXP MapLayerWrapperToSf( Rcpp::XPtr<MapLayerWrapper> obj );

SEXP MapLayerWrapperByName( std::string name );

SEXP MapLayerWrapperToRaster( Rcpp::XPtr<MapLayerWrapper> obj );

SEXP MapLayerWrapperToTerra( Rcpp::XPtr<MapLayerWrapper> obj );

SEXP MapLayerWrapperToStars( Rcpp::XPtr<MapLayerWrapper> obj );

SEXP MapLayerWrapperIsVectorLayer( Rcpp::XPtr<MapLayerWrapper> obj );

SEXP MapLayerWrapperIsRasterLayer( Rcpp::XPtr<MapLayerWrapper> obj );

SEXP MapLayerWrapperDollar( Rcpp::XPtr<MapLayerWrapper> obj, std::string name );

// The function which is called when running QGIS$...
SEXP Dollar( Rcpp::XPtr<QgisApplicationRWrapper> obj, std::string name );

// The function listing the elements of QGIS
Rcpp::CharacterVector Names( Rcpp::XPtr<QgisApplicationRWrapper> );

#endif