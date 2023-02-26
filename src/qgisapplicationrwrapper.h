#ifndef APPLICATIONRWRAPPER_H
#define APPLICATIONRWRAPPER_H

#include "qgisinterface.h"
#include "qgsapplication.h"

#include "maplayerwrapper.h"
#include "qgisapplicationrwrapper.h"

class QgisApplicationRWrapper
{
    public:
        QgisApplicationRWrapper( std::shared_ptr<QgisInterface> iface ) { mIface = iface; }

        int version() const { return Qgis::versionInt(); }

        SEXP activeLayer() const
        {
            std::shared_ptr<QgsMapLayer> mapLayer;
            mapLayer.reset( mIface->activeLayer() );
            Rcpp::XPtr<MapLayerWrapper> res( new MapLayerWrapper( mapLayer ) );
            res.attr( "class" ) = "MapLayerWrapper";
            return res;
        }

    private:
        std::shared_ptr<QgisInterface> mIface;
};

#endif