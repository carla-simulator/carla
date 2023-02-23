#ifndef OSM_RENDERER_H
#define OSM_RENDERER_H

#include <boost/asio.hpp>

#include <string>
#include <vector>
#include <memory>
#include "MapDrawer.h"


class OsmRenderer 
{
private:
  // Boost socket
  boost::asio::io_service io_service;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> SocketAcceptorPtr;
  std::unique_ptr<boost::asio::ip::tcp::socket> SocketPtr;


  // Map Drawer
  std::unique_ptr<MapDrawer> Drawer;

  void RunCmd(std::string Cmd);

  std::vector<std::string> SplitCmd (std::string s, std::string delimiter);

  // Command Handlers
  void RenderMapCmd(std::vector<std::string> CmdArgs, uint8_t* OutMap);
  void ConfigMapCmd(std::vector<std::string> CmdArgs);
  void SendLatLonCmd(std::vector<std::string> CmdArgs);


public:
  std::string GetOsmRendererString() const;

  void InitRenderer();
  void StartLoop();

  void ShutDown();
};

#endif