#ifndef QGSRSTATSUTILS_H
#define QGSRSTATSUTILS_H

#include <Rcpp.h>

#include "qgis.h"

class QgsField;
class QgsFeature;
class QgsFields;
class QgsCoordinateReferenceSystem;

class QgsRstatsUtils
{
  public:
    static SEXP fieldToRcppVector( QgsField field, std::size_t featureCount );
    static bool canConvertToRcpp( QgsField field );
    static void addFeatureToDf( QgsFeature feature, std::size_t featureNumber, Rcpp::DataFrame &df );

    static void preparedFieldsFromDf(Rcpp::DataFrame &df, QgsFields &fields);
    static Qgis::WkbType wkbType(Rcpp::DataFrame &df);
    static QgsCoordinateReferenceSystem crs(Rcpp::DataFrame &df);
    static std::string geometryColumn(Rcpp::DataFrame &df);
    static bool hasSfColumn(Rcpp::DataFrame &df);
    static bool isSf(Rcpp::DataFrame &df);
    static Rcpp::StringVector geometries(Rcpp::DataFrame &df);
    static void prepareFeature(QgsFeature &feature, Rcpp::DataFrame &df, int row, Rcpp::StringVector &geometries);
};

#endif // QGSRSTATSUTILS_H
