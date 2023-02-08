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

//using namespace std; // TODO Remove


class MapDrawer
{
public:
    void PreLoad(std::vector<std::string>& Args);

    void Draw(std::uint8_t* OutMap, osmscout::GeoCoord Coords, double ZoomValue);

    inline const int GetImgSizeSqr()
    {
        return Size * Size;
    }; 

private:
    // Rastering
    std::unique_ptr<MapRasterizer> Rasterizer;

    // Arguments
    std::string DataBasePath;
    std::string StyleSheetPath;
    int Size;
    //size_t Latitude;
    //size_t Longitude;
    //size_t Zoom;

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