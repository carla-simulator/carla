#ifndef MAP_DRAWE_H
#define MAP_DRAWE_H

#include <vector>
#include <string>
#include <memory>

#include "MapRasterizer.h"

#include <osmscout/GeoCoord.h>
#include <osmscout/Database.h>
#include <osmscoutmap/MapService.h>
#include <osmscout/BasemapDatabase.h>
#if defined(_MSC_VER)
    #include <osmscout/projection/MercatorProjection.h>
#endif
class MapDrawer
{
public:
    void PreLoad(std::vector<std::string>& Args);

    void Draw(std::uint8_t* OutMap, osmscout::GeoCoord Coords, double ZoomValue);

    inline const int GetImgSizeSqr()
    {
        return Size * Size;
    };

    osmscout::GeoCoord GetBottomLeftCoord();
    osmscout::GeoCoord GetTopRightCoord();

private:
    // Rastering
    std::unique_ptr<MapRasterizer> Rasterizer;

    // Arguments
    std::string DataBasePath;
    std::string StyleSheetPath;
    int Size;

    // Entities
    osmscout::DatabaseParameter DbParameter;
    osmscout::DatabaseRef Database;
    osmscout::MapServiceRef MapService;
    osmscout::StyleConfigRef StyleSheet;

    osmscout::MercatorProjection Projection;
    osmscout::MapData MapData;
    osmscout::MapParameter DrawParameter;
    osmscout::AreaSearchParameter SearchParameter;
    //GeoCoord CenterCoords;

    void LoadDatabaseData();
    void SetDrawParameters();

    void DrawMap(std::uint8_t* OutMap);

};

#endif