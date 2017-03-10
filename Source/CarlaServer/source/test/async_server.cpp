// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"

#include "carla/server/CarlaServer.h"

#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

enum ErrorCodes {
  InvalidArguments,
  STLException,
  UnknownException,
  ErrorSending,
  ErrorReading
};

static int toInt(const std::string &str) {
  return std::stoi(str);
}

static std::string daytimeString() {
  using namespace std;
  time_t now = time(0);
  std::string str = ctime(&now);
  return str;
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 6) {
      std::cerr << "Usage: server <send-port> <read-port>" << std::endl;
      return InvalidArguments;
    }

	using namespace carla::server;

    // This already starts the two threads.
    CarlaServer server(toInt(argv[1u]), toInt(argv[2u]), toInt(argv[3u]), toInt(argv[4u]), toInt(argv[5u]));

    // Let's simulate the game loop.


	 Color c1;
	c1.red = 255;
	c1.green = 0;
	c1.blue = 0;
	//c1.alpha = 0;

	 Color c2;
	c2.red = 4;
	c2.green = 5;
	c2.blue = 6;
	//c2.alpha = 0;

	 Color c3;
	c3.red = 7;
	c3.green = 8;
	c3.blue = 9;
	//c3.alpha = 0;

	 Color c4;
	c4.red = 10;
	c4.green = 11;
	c4.blue = 12;
	//c4.alpha = 0;

	std::vector<Color> img;
	for (int i=0; i < 1024; ++i) img.push_back(c1);
	
	std::vector<Color> img_2;
	for (int i = 0; i < 1024; ++i) img_2.push_back(c2);

	std::vector<Color> depth_1;
	for (int i = 0; i < 1024; ++i) depth_1.push_back(c3);

	std::vector<Color> depth_2;
	for (int i = 0; i < 1024; ++i) depth_2.push_back(c4);

	Reward_Values testData;
	testData.player_x = 1.0f;
	testData.player_y = 1.0f;
	testData.speed = 100.0f;
	testData.collision_gen = 10.0f;
	testData.collision_ped = 10.0f;
	testData.collision_car = 10.0f;
	testData.intersect = 50.0f;
	testData.inertia_x = 0.5f;
	testData.inertia_y = 0.1f;
	testData.inertia_z = 0.02f;
	testData.timestamp = 1;
	testData.ori_x = 10.0f;
	testData.ori_y = 20.0f;
	testData.ori_z = 30.0f;
	testData.img = img;
	testData.img_2 = img_2;
	testData.depth_1 = depth_1;
	testData.depth_2 = depth_2;


	
	server.sendWorld();

	int mode, scene;
	bool end = false;
	do {
		end = server.tryReadSceneInit(mode, scene);
	}while (!end);

	std::vector<Position> positions;
	std::vector<const float*> pMatrix;

	positions.push_back(Position{ 0.0f, 0.0f });
	positions.push_back(Position{ 1.0f, 2.0f });
	positions.push_back(Position{ 3.0f, 4.0f });

	float list[16] = { 10.0, 10.0,10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0 };

	pMatrix.push_back(list);

	Scene_Values sceneVal = {
		positions,
		pMatrix,
	};

	server.sendSceneValues(sceneVal);
	
	end = false;
	float startPoint, endPoint;
	do {
		end = server.tryReadEpisodeStart(startPoint, endPoint);
	}
	while (!end);

	server.sendEndReset();

    for (;;) {

	  Sleep(50);
      //std::cout << "Sending..." << std::endl;
      auto time = daytimeString();
	 // server.reward = testData;
	  server.sendReward(testData);
      using namespace std::chrono_literals;

	  Sleep(50);
      //std::cout << "Listening..." << std::endl;
	  float steer, gas;
      if (server.tryReadControl(steer, gas)) {
        /*if ((message == "q") || (message == "quit"))
          break;*/
      }
    }

  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return STLException;
  } catch (...) {
    std::cerr << "Unknown exception thrown" << std::endl;
    return UnknownException;
  }
}
