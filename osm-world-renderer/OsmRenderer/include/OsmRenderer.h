#ifndef OSM_RENDERER_H
#define OSM_RENDERER_H

#include <string>
#include <netinet/in.h>
#include <vector>

using namespace std;

class MapDrawer;

class OsmRenderer 
{
private:
  // Socket
  bool isStarted = false;
  int RendererSocketfd;
  sockaddr_in Address;

  // Map Drawer
  MapDrawer* Drawer;

  void RunCmd(int ConnectionSocket, char* Cmd);

  vector<string> SplitCmd (string s, string delimiter);

  // Command Handlers
  void RenderMapCmd(vector<string> CmdArgs);
  void ConfigMapCmd(vector<string> CmdArgs);

  // Map Rendering functions


public:
  string GetOsmRendererString() const;

  int InitRenderer();
  int StartLoop();

  void ShutDown();
};

#endif