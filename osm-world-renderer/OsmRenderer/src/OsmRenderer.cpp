#include "OsmRenderer.h"

#include "OsmRendererMacros.h"
#include "MapDrawer.h"

#include <sys/socket.h>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <chrono>

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
    //RenderMapCmd(CmdVector);
  }
  else if(CmdType == "-C")// Configuration Command
  {
    //const char* message = "Configuration Command";
    //send(ConnectionSocket, message, strlen(message), 0);
    ConfigMapCmd(CmdVector);
    // TODO: Delete these lines or move them to Render Command handling
    std::uint8_t* RenderedMap;
    RenderMapCmd(CmdVector, RenderedMap);
    //size_t BufferSize = sizeof(RenderedMap) / sizeof(RenderedMap[0]);
    std::cout << "===> " << ConnectionSocket << " " << Drawer->GetImgSizeSqr() << std::endl;
    for(size_t i = 0; i < Drawer->GetImgSizeSqr(); i++ )
    {
     // std::cout << std::dec << RenderedMap[i] << " ";
    }
    if(send(ConnectionSocket, (const char*)RenderedMap, (Drawer->GetImgSizeSqr() * 4 * sizeof(uint8_t)), 0) < 0)
    //if(send(ConnectionSocket, (const char*)RenderedMap, (Drawer->GetImgSizeSqr() * sizeof(uint8_t)), 0) < 0)
    //if(send(ConnectionSocket, (const char*)RenderedMap, BufferSize, 0) < 0)
    {
      std::cerr << " ⛔️ ERROR Sending map to client: " << errno << " :: " << strerror(errno) << std::endl;
    }
    else{
      std::cout << " ✅ SUCCESS! Bitmap sent correctly!" << std::endl;
    }
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

void OsmRenderer::RenderMapCmd(vector<string> CmdArgs, uint8_t* OutMap)
{
  // TODO: Move to RenderMapCmd
  auto start = std::chrono::high_resolution_clock::now();
  osmscout::GeoCoord Coord(40.415, -3.702);                 // TODO: Lat and Lon info from command
  Drawer->Draw(OutMap, Coord, 100000);  // TODO: Zoom info from command
  auto stop = std::chrono::high_resolution_clock::now();
  auto ElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "Elapsed Rendering time: " << ElapsedTime.count() << "ms." << std::endl;
  //return RenderedMap;
}

void OsmRenderer::ConfigMapCmd(vector<string> CmdArgs)
{
  std::cout << "Creating drawer......." << std::endl;
  Drawer = new MapDrawer(CmdArgs);

  if(!Drawer)
  {
    std::cout << "--> ERROR creating Drawer"  << std::endl;
    return;
  }
}
