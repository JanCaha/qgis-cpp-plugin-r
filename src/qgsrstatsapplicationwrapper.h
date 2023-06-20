#ifndef QGSRSTATAPPLICATIONWRAPPER_H
#define QGSRSTATAPPLICATIONWRAPPER_H

#include <Rcpp.h>

#include "qgisinterface.h"

class QgsRstatsApplicationWrapper
{
  public:
    int version() const;

    SEXP activeLayer() const;
    SEXP mapLayers();
    SEXP mapLayerByName( std::string layerName );

    SEXP projectCrs();

    static std::string rClassName();
    static Rcpp::CharacterVector functions();
    static Rcpp::XPtr<QgsRstatsApplicationWrapper> instance(std::shared_ptr<QgisInterface> iface);
  
  private: 
    QgsRstatsApplicationWrapper();
    std::shared_ptr<QgisInterface> mIface = nullptr;
};

#endif // QGSRSTATAPPLICATIONWRAPPER_H
