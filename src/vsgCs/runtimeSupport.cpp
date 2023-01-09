#include "runtimeSupport.h"

#include <Cesium3DTilesSelection/registerAllTileContentTypes.h>

namespace vsgCs {
    void startup()
    {
        Cesium3DTilesSelection::registerAllTileContentTypes();
    }
}
