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
  std::cout << "┌ Waiting Command..." << std::endl;
  int ConnectionSocket = accept(RendererSocketfd, (struct sockaddr*)&Address, (socklen_t*)&AddrLen);
  if(ConnectionSocket < 0)
  {
    throw runtime_error("Connection not accepted " + ConnectionSocket);
  }
  while(true)
  {
    
    char Buffer[BUFFER_SIZE] = {};
    size_t ReadBytes = read(ConnectionSocket, Buffer, BUFFER_SIZE);
    std::cout << LOG_PRFX << "Received message: " << Buffer << std::endl;

    RunCmd(ConnectionSocket, Buffer);

    std::cout << "└ End of Command." << std::endl << "┌ Waiting Command..." << std::endl;
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
    std::uint8_t* RenderedMap = new uint8_t[Drawer->GetImgSizeSqr() * 4];
    RenderMapCmd(CmdVector, RenderedMap);

    if(send(ConnectionSocket, RenderedMap, (Drawer->GetImgSizeSqr() * 4 * sizeof(uint8_t)), 0) < 0)
    {
      std::cerr << LOG_PRFX << " ⛔️ ERROR Sending map to client: " << errno << " :: " << strerror(errno) << std::endl;
    }
    else{
      std::cout << LOG_PRFX << " ✅ SUCCESS! Bitmap sent correctly!" << std::endl;
    }
  }
  else if(CmdType == "-C")// Configuration Command
  {
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

void OsmRenderer::RenderMapCmd(vector<string> CmdArgs, uint8_t* OutMap)
{
  std::cout << LOG_PRFX << "Rendering map at [" << stof(CmdArgs[R_CMD_LATITUDE]) << ", "
    << stof(CmdArgs[R_CMD_LONGITUDE]) << "] with zoom: " << CmdArgs[R_CMD_ZOOM] << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  osmscout::GeoCoord Coord(stof(CmdArgs[R_CMD_LATITUDE]), stof(CmdArgs[R_CMD_LONGITUDE]));
  Drawer->Draw(OutMap, Coord, stod(CmdArgs[R_CMD_ZOOM]));
  auto stop = std::chrono::high_resolution_clock::now();

  auto ElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << LOG_PRFX << "Elapsed Rendering time: " << ElapsedTime.count() << "ms." << std::endl;
}

void OsmRenderer::ConfigMapCmd(vector<string> CmdArgs)
{
  std::cout << LOG_PRFX << "Configuring Renderer:: DATABASE:" 
      << CmdArgs[C_CMD_DATABASE_PATH] << " STYLESHEET: "
      << CmdArgs[C_CMD_STYLESHEET_PATH] << " SIZE: " << CmdArgs[C_CMD_IMG_SIZE]<< std::endl;
  Drawer = new MapDrawer(CmdArgs);

  if(!Drawer)
  {
    std::cout << LOG_PRFX << "--> ERROR creating Drawer"  << std::endl;
    return;
  }
}
