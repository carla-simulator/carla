#ifndef OSM_RENDERER_H
#define OSM_RENDERER_H

#include <string>
#include <netinet/in.h>

using namespace std;

class OsmRenderer 
{
private:
  bool isStarted = false;

  int RendererSocketfd;

  sockaddr_in Address;

public:
  string GetOsmRendererString() const;

  int InitRenderer();
  int StartLoop();

  void ShutDown();
};

#endif