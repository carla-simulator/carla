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

    int imageWidth = 512, imageHeight = 512;

    std::vector<unsigned char> img;
    img.resize(imageWidth * imageHeight * 4);
    for (int i = 0; i < imageHeight; ++i)
      for (int e = 0; e < imageWidth; ++e) {
        img[4 * imageWidth * i + 4 * e + 0] = 255 * !(e & i);
        img[4 * imageWidth * i + 4 * e + 1] = e ^ i;
        img[4 * imageWidth * i + 4 * e + 2] = e | i;
        img[4 * imageWidth * i + 4 * e + 3] = 255;
      }

    std::vector<unsigned char> img_2;
    img_2.resize(imageWidth * imageHeight * 4);
    for (int i = 0; i < imageHeight; ++i)
      for (int e = 0; e < imageWidth; ++e) {
        img_2[4 * imageWidth * i + 4 * e + 0] = 255 * !(e & i);
        img_2[4 * imageWidth * i + 4 * e + 1] = e ^ i;
        img_2[4 * imageWidth * i + 4 * e + 2] = e | i;
        img_2[4 * imageWidth * i + 4 * e + 3] = 255;
      }

    std::vector<unsigned char> depth_1;
    depth_1.resize(imageWidth * imageHeight * 4);
    for (int i = 0; i < imageHeight; ++i)
      for (int e = 0; e < imageWidth; ++e) {
        depth_1[4 * imageWidth * i + 4 * e + 0] = 255 * !(e & i);
        depth_1[4 * imageWidth * i + 4 * e + 1] = e ^ i;
        depth_1[4 * imageWidth * i + 4 * e + 2] = e | i;
        depth_1[4 * imageWidth * i + 4 * e + 3] = 255;
      }

    std::vector<unsigned char> depth_2;
    depth_2.resize(imageWidth * imageHeight * 4);
    for (int i = 0; i < imageHeight; ++i)
      for (int e = 0; e < imageWidth; ++e) {
        depth_2[4 * imageWidth * i + 4 * e + 0] = 255 * !(e & i);
        depth_2[4 * imageWidth * i + 4 * e + 1] = e ^ i;
        depth_2[4 * imageWidth * i + 4 * e + 2] = e | i;
        depth_2[4 * imageWidth * i + 4 * e + 3] = 255;
      }

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
    testData.img_height = imageHeight;
    testData.img_width = imageWidth;
    testData.img = img;
    testData.img_2 = img_2;
    testData.depth_1 = depth_1;
    testData.depth_2 = depth_2;


    std::cout << "Server send World" << std::endl;
    server.sendWorld();

    int mode, scene;
    bool end = false;

    std::cout << "Server wait scene init" << std::endl;
    do {
      end = server.tryReadSceneInit(mode, scene);
    } while (!end);

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

    std::cout << "Server send positions" << std::endl;

    server.sendSceneValues(sceneVal);

    end = false;
    size_t startPoint, endPoint;

    std::cout << "Server wait new episode" << std::endl;

    do {

      end = server.tryReadEpisodeStart(startPoint, endPoint);
    } while (!end);

    std::cout << "Server send end reset" << std::endl;

    server.sendEndReset();

    float steer, gas;
    for (;;) {

      if (server.tryReadEpisodeStart(startPoint, endPoint)) {
        std::cout << "------> RESET <------" << std::endl;
        std::cout << " --> Start: " << startPoint << " End: " << endPoint << std::endl;
        server.sendEndReset();
      }
      else {

        if (server.tryReadControl(steer, gas)) {
          std::cout << "Steer: " << steer << "Gas: " << gas << std::endl;
        }

       server.sendReward(testData);

      }

      Sleep(100);

    }

  }
  catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return STLException;
  }
  catch (...) {
    std::cerr << "Unknown exception thrown" << std::endl;
    return UnknownException;
  }
}


//TODO:
//pmatrix float 16
//start_index size_t 
