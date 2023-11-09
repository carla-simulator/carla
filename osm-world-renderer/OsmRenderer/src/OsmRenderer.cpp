#include "OsmRenderer.h"

#include "OsmRendererMacros.h"
#include "MapDrawer.h"

#include <stdexcept>
#include <iostream>
#include <string.h>
#include <chrono>
#include <stdlib.h> 

#define PORT 5000
#define BUFFER_SIZE 1024

using namespace std;

namespace Asio = boost::asio;
using AsioStreamBuf = boost::asio::streambuf;
using AsioTCP = boost::asio::ip::tcp;
using AsioSocket = boost::asio::ip::tcp::socket;
using AsioAcceptor = boost::asio::ip::tcp::acceptor;
using AsioEndpoint = boost::asio::ip::tcp::endpoint;


string OsmRenderer::GetOsmRendererString() const
{
  return "Renderer speaking here";
}

void OsmRenderer::InitRenderer() 
{
  SocketAcceptorPtr = make_unique<AsioAcceptor>(io_service, AsioEndpoint(AsioTCP::v4(), PORT));
  SocketPtr = make_unique<AsioSocket>(io_service);
}

void OsmRenderer::StartLoop()
{
  std::cout << "┌ Waiting Command..." << std::endl;
  SocketAcceptorPtr->accept(*SocketPtr);
  if(!SocketPtr->is_open())
  {
    throw runtime_error("Connection not accepted. Socket is not opened.");
  }
  while(true)
  {
    AsioStreamBuf Buffer;
    Asio::read(*SocketPtr, Buffer, Asio::transfer_at_least(2));

    string BufferStr = Asio::buffer_cast<const char*>(Buffer.data());

    std::cout << LOG_PRFX << "Received message: " << BufferStr << std::endl;

    RunCmd(BufferStr);
  }
}

void OsmRenderer::ShutDown()
{
  // TODO Shutdown socket and clear pointers
}

void OsmRenderer::RunCmd(string Cmd)
{
  string CmdStr = Cmd;
  vector<string> CmdVector = SplitCmd(CmdStr, " ");
  
  string CmdType = CmdVector[CMD_INDEX];

  if(CmdType == "-R")     // Render Command
  {
    std::unique_ptr<std::uint8_t> RenderedMap = std::unique_ptr<std::uint8_t>(new uint8_t[Drawer->GetImgSizeSqr() * 4]);
    RenderMapCmd(CmdVector, RenderedMap.get());

    std::cout << LOG_PRFX << "Sending image data: " << (Drawer->GetImgSizeSqr() * 4 * sizeof(uint8_t)) << " bytes" << std::endl;
    Asio::write(*SocketPtr, Asio::buffer(RenderedMap.get(), (Drawer->GetImgSizeSqr() * 4 * sizeof(uint8_t))));

    // TODO: delete RenderedMap after is sent
  }
  else if(CmdType == "-C")// Configuration Command
  {
    ConfigMapCmd(CmdVector);
  }
  else if(CmdType == "-X")// Exit command
  {
    std::cout << "└ Bye!" << std::endl;
    exit(EXIT_SUCCESS);
  }
  else if(CmdType == "-L")
  {
    SendLatLonCmd(CmdVector);
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
  Drawer = std::make_unique<MapDrawer>();
  Drawer->PreLoad(CmdArgs);

  if(!Drawer)
  {
    std::cout << LOG_PRFX << "--> ERROR creating Drawer"  << std::endl;
    return;
  }
}

void OsmRenderer::SendLatLonCmd(vector<string> CmdArgs)
{
  osmscout::GeoCoord TopRightCoord = Drawer->GetTopRightCoord();
  osmscout::GeoCoord BottomLeftCoord = Drawer->GetBottomLeftCoord();

  std::cout << LOG_PRFX << "TOP: " << TopRightCoord.GetLat() << " -- " << TopRightCoord.GetLon() << std::endl;
  std::cout << LOG_PRFX << "BOTTOM: " << BottomLeftCoord.GetLat() << " -- " << BottomLeftCoord.GetLon() << std::endl;

  string CoordsStr = to_string(TopRightCoord.GetLat()) + "&" + to_string(TopRightCoord.GetLon()) + 
      "&" + to_string(BottomLeftCoord.GetLat()) + "&" + to_string(BottomLeftCoord.GetLon()) + "&";

  std::cout << LOG_PRFX << "Sending [" << CoordsStr << "] Size: " << CoordsStr.size() << std::endl;
  
  Asio::write(*SocketPtr, Asio::buffer(CoordsStr));
}
