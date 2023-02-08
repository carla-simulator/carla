#ifndef OSM_RENDERER_H
#define OSM_RENDERER_H

#include <string>
#include <netinet/in.h>
#include <vector>
#include <memory>
#include "MapDrawer.h"

//using namespace std;


class OsmRenderer 
{
private:
  // Socket
  bool isStarted = false;
  int RendererSocketfd;
  sockaddr_in Address;

  // Map Drawer
  std::unique_ptr<MapDrawer> Drawer;

  void RunCmd(int ConnectionSocket, char* Cmd);

  std::vector<std::string> SplitCmd (std::string s, std::string delimiter);

  // Command Handlers
  void RenderMapCmd(std::vector<std::string> CmdArgs, uint8_t* OutMap);
  void ConfigMapCmd(std::vector<std::string> CmdArgs);


public:
  std::string GetOsmRendererString() const;

  int InitRenderer();
  int StartLoop();

  void ShutDown();
};

#endif