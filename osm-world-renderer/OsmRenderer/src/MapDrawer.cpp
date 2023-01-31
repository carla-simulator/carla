#include "MapDrawer.h"
#include "OsmRendererMacros.h"
#include "MapRasterizer.h"

#include "osmscoutmapsvg/MapPainterSVG.h"

#include <iostream>
//#include <fstream>
#include <sstream>
#include <list>

MapDrawer::MapDrawer(vector<string> Args)
{
  // Open Database
  DataBasePath = Args[C_CMD_DATABASE_PATH];
  StyleSheetPath = Args[C_CMD_STYLESHEET_PATH];
  Size = stoi(Args[C_CMD_IMG_SIZE]);

  Database = std::make_shared<osmscout::Database>(DbParameter);

  if(!Database->Open(DataBasePath))
  {
    std::cerr << "ERROR Opening Database in " << DataBasePath << std::endl;
  }

  MapService = std::make_shared<osmscout::MapService>(Database);

  StyleSheet = std::make_shared<osmscout::StyleConfig>(Database->GetTypeConfig());
  if(!StyleSheet->Load(StyleSheetPath))
  {
    std::cerr << "ERROR Opening Stylesheet in " << DataBasePath << std::endl;
  }

  Rasterizer = new MapRasterizer();
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