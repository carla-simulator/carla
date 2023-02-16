#include "MapDrawer.h"
#include "OsmRendererMacros.h"
#include "MapRasterizer.h"

#include "osmscoutmapsvg/MapPainterSVG.h"

#include <iostream>
//#include <fstream>
#include <sstream>
#include <list>

using namespace std;

void MapDrawer::PreLoad(vector<string>& Args)
{
  // Open Database
  std::cout << "Begining" << std::endl;
  DataBasePath = Args[C_CMD_DATABASE_PATH];
  StyleSheetPath = Args[C_CMD_STYLESHEET_PATH];
  std::cout << "Before stoi" << std::endl;
  Size = stoi(Args[C_CMD_IMG_SIZE]);

  Database = std::make_shared<osmscout::Database>(DbParameter);

  std::cout << "Before opening database" << std::endl;

  if(!Database->Open(DataBasePath))
  {
    std::cerr << "ERROR Opening Database in " << DataBasePath << std::endl;
  }

  std::cout << "After opening database" << std::endl;

  MapService = std::make_shared<osmscout::MapService>(Database);

  StyleSheet = std::make_shared<osmscout::StyleConfig>(Database->GetTypeConfig());
  if(!StyleSheet->Load(StyleSheetPath))
  {
    std::cerr << "ERROR Opening Stylesheet in " << DataBasePath << std::endl;
  }

  Rasterizer = std::make_unique<MapRasterizer>();
}

void MapDrawer::Draw(std::uint8_t* OutMap, osmscout::GeoCoord Coords, double ZoomValue)
{
  osmscout::Magnification Zoom;
  Zoom.SetMagnification(ZoomValue);
  Projection.Set(Coords, Zoom, 96.0f, Size, Size);
  
  LoadDatabaseData();
  SetDrawParameters();

  DrawMap(OutMap);
}

osmscout::GeoCoord MapDrawer::GetBottomLeftCoord()
{
  osmscout::GeoCoord PixelCoord;
  if(Projection.PixelToGeo(0, Size-1, PixelCoord))
    return PixelCoord;
  else
    return osmscout::GeoCoord(0,0);
}

osmscout::GeoCoord MapDrawer::GetTopRightCoord()
{
  osmscout::GeoCoord PixelCoord;
  if(Projection.PixelToGeo(Size-1, 0, PixelCoord))
    return PixelCoord;
  else
    return osmscout::GeoCoord(0,0);
}

void MapDrawer::LoadDatabaseData()
{
  // Load Database
  std::list<osmscout::TileRef> Tiles;

  MapData.ClearDBData();

  MapService->LookupTiles(Projection, Tiles);
  MapService->LoadMissingTileData(SearchParameter, *StyleSheet, Tiles);
  MapService->AddTileDataToMapData(Tiles, MapData);
  MapService->GetGroundTiles(Projection, MapData.groundTiles);

}

void MapDrawer::SetDrawParameters()
{
  DrawParameter.SetFontName(DEFAULT_FONT_FILE);

  //DrawParameter.SetFontSize(args.fontSize);
  DrawParameter.SetRenderSeaLand(true);
  DrawParameter.SetRenderUnknowns(false);
  DrawParameter.SetRenderBackground(false);
  //DrawParameter.SetRenderContourLines(args.renderContourLines);
  //DrawParameter.SetRenderHillShading(args.renderHillShading);

  //DrawParameter.SetIconMode(args.iconMode);
  //DrawParameter.SetIconPaths(args.iconPaths);

  //DrawParameter.SetDebugData(args.debug);
  //DrawParameter.SetDebugPerformance(args.debug);

  DrawParameter.SetLabelLineMinCharCount(15);
  DrawParameter.SetLabelLineMaxCharCount(30);
  DrawParameter.SetLabelLineFitToArea(true);
}

void MapDrawer::DrawMap(std::uint8_t* OutMap)
{
  std::stringstream OutSvgStream;
  if (!OutSvgStream) {
    std::cerr << "Cannot open '" << "' for writing!" << std::endl;
    return;
  }

  osmscout::MapPainterSVG Painter(StyleSheet);
  Painter.DrawMap(Projection, DrawParameter, MapData, OutSvgStream);

  Rasterizer->RasterizeSVG(OutMap, OutSvgStream.str(), Size);
}