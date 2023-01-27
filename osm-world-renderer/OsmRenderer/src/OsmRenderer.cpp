#include "../include/OsmRenderer.h"

#include "../include/OsmRendererMacros.h"
#include "../include/MapDrawer.h"

#include <sys/socket.h>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <unistd.h>

//#include "osmscoutmapsvg/MapPainterSVG.h"

#define PORT 5000
#define BUFFER_SIZE 1024

string OsmRenderer::GetOsmRendererString() const
{
  return "Renderer speaking here";
}

int OsmRenderer::InitRenderer() 
{
  // Open socket
  if((RendererSocketfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    throw runtime_error("Error opening the renderer socket");
  }
  // Attach socket to port 
  int opt = 1;
  if(setsockopt(RendererSocketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
  {
    throw runtime_error("Error setting renderer socket opt");
  }
  Address.sin_family = AF_INET;
  Address.sin_addr.s_addr = INADDR_ANY;
  Address.sin_port = htons(PORT);

  if(bind(RendererSocketfd, (struct sockaddr*)&Address, sizeof(Address)) < 0)
  {
    throw runtime_error("Error binding renderer socket to port " + PORT);
  }

  if(listen(RendererSocketfd, 3) < 0 )
  {
    throw runtime_error("Error listening to port " + PORT);
  }

  return RendererSocketfd;
}

int OsmRenderer::StartLoop()
{
  const int AddrLen = sizeof(Address);
  while(true)
  {
    std::cout << "┌ Waiting Command..." << std::endl;
    int ConnectionSocket = accept(RendererSocketfd, (struct sockaddr*)&Address, (socklen_t*)&AddrLen);
    if(ConnectionSocket < 0)
    {
      throw runtime_error("Connection not accepted " + ConnectionSocket);
    }

    char Buffer[BUFFER_SIZE] = {};
    read(ConnectionSocket, Buffer, BUFFER_SIZE);
    std::cout << "└ Received message: " << Buffer << std::endl;
    //char* message = "Hello darling";
    //send(ConnectionSocket, message, strlen(message), 0);
    //write(ConnectionSocket, message, strlen(message));
    RunCmd(ConnectionSocket, Buffer);
    close(ConnectionSocket);
  }
  return 0;
}

void OsmRenderer::ShutDown()
{
  //close(RendererSocketfd);
  shutdown(RendererSocketfd, SHUT_RDWR);
}

void OsmRenderer::RunCmd(int ConnectionSocket, char* Cmd)
{
  string CmdStr = Cmd;
  vector<string> CmdVector = SplitCmd(CmdStr, " ");
  
  string CmdType = CmdVector[CMD_INDEX];

  if(CmdType == "-R")     // Render Command
  {
    //const char* message = "Render Command";
    //send(ConnectionSocket, message, strlen(message), 0);
    RenderMapCmd(CmdVector);
  }
  else if(CmdType == "-C")// Configuration Command
  {
    //const char* message = "Configuration Command";
    //send(ConnectionSocket, message, strlen(message), 0);
    ConfigMapCmd(CmdVector);
  }
}

vector<string> OsmRenderer::SplitCmd (string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

void OsmRenderer::RenderMapCmd(vector<string> CmdArgs)
{
  /*for(auto i : CmdArgs)
  {
    std::cout << i << std::endl;
  }*/

}

void OsmRenderer::ConfigMapCmd(vector<string> CmdArgs)
{
  Drawer = new MapDrawer(CmdArgs);

  if(!Drawer)
  {
    std::cout << "--> ERROR creating Drawer"  << std::endl;
    return;
  }

  osmscout::GeoCoord Coord(40.415, -3.702);
  Drawer->Draw(Coord, 100000);

}
