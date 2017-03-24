// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include <carla/CarlaServer.h>

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

static uint32_t toInt(const std::string &str) {
  return std::stoi(str);
}

static std::string daytimeString() {
  using namespace std;
  time_t now = time(0);
  std::string str = ctime(&now);
  return str;
}

static std::vector<carla::Color> makeImage(uint32_t width, uint32_t height) {
  // Xisco's magic image generator.
  std::vector<unsigned char> img(width * height * 4);
  for (int i = 0; i < height; ++i) {
   for (int e = 0; e < width; ++e) {
     img[4 * width * i + 4 * e + 0] = 255 * !(e & i);
     img[4 * width * i + 4 * e + 1] = e ^ i;
     img[4 * width * i + 4 * e + 2] = e | i;
     img[4 * width * i + 4 * e + 3] = 255;
   }
  }

  std::vector<carla::Color> image(width * height);
  size_t i = 0u;
  for (carla::Color &color : image) {
    color.R = img[i++];
    color.G = img[i++];
    color.B = img[i++];
    color.A = img[i++];
  }
  return image;
}

std::unique_ptr<carla::Reward_Values> makeReward(){

  auto reward = std::make_unique<carla::Reward_Values>();

  const uint32_t imageWidth = 512u;
    const uint32_t imageHeight = 512u;

    reward->player_location = {1.0f, 1.0f};
    reward->player_orientation = {1.0f, 1.0f};
    reward->player_acceleration = {1.0f, 1.0f};
    reward->forward_speed = 100.0f;
    reward->collision_general = 10.0f;
    reward->collision_pedestrian = 10.0f;
    reward->collision_car = 10.0f;
    reward->intersect_other_lane = 0.5f;
    reward->intersect_offroad = 0.5f;
    reward->image_width = imageWidth;
    reward->image_height = imageHeight;
    reward->image_rgb_0 = makeImage(imageWidth, imageHeight);
    reward->image_rgb_1 = makeImage(imageWidth, imageHeight);
    reward->image_depth_0 = makeImage(imageWidth, imageHeight);
    reward->image_depth_1 = makeImage(imageWidth, imageHeight);

    static decltype(carla::Reward_Values::timestamp) timestamp = 0u;
    reward->timestamp = timestamp++;

    return reward;
}


int main(int argc, char *argv[]) {
  try {
    if (argc != 4) {
      std::cerr << "Usage: server <world-port> <write-port> <read-port>" << std::endl;
      return InvalidArguments;
    }
    const uint32_t worldPort = toInt(argv[1u]);
    const uint32_t writePort = toInt(argv[2u]);
    const uint32_t readPort  = toInt(argv[3u]);
    // const uint32_t mode      = toInt(argv[4u]);
    // const uint32_t scene     = toInt(argv[5u]);

    // This already starts the two threads.
    carla::CarlaServer server(writePort, readPort, worldPort);

    // Let's simulate the game loop.

    

    for (;;){
      if (server.init(1u)){

        {
          carla::Mode mode;
          uint32_t scene;
          bool error = false, readed = false;
          do{
            error = !server.tryReadSceneInit(mode, scene, readed);
          }while(!readed && !error);

          if (error) std::cerr << "ERROR while sending SceneValues" << std::endl;
          else std::cout << "Received: mode = " << (mode == carla::Mode::MONO ? "MONO" : "STEREO")
                    << ", scene = " << scene << std::endl;
        }


          carla::Scene_Values sceneValues;


          sceneValues.possible_positions.push_back({0.0f, 0.0f});
          sceneValues.possible_positions.push_back({1.0f, 2.0f});
          sceneValues.possible_positions.push_back({3.0f, 4.0f});
          const std::array<float, 16u> pMatrix = {{ 10.0 }};
          sceneValues.projection_matrices.push_back(pMatrix);

         /*  std::cout << "POSSIBLE POSITIONS "<< std::endl;
           for (int i=0; i<sceneValues.possible_positions.size(); ++i){
            std::cout << "   x: " << sceneValues.possible_positions[i].x << " y: " << sceneValues.possible_positions[i].y << std::endl;
          }*/


          if (!server.sendSceneValues(sceneValues)) std::cerr << "ERROR while sending SceneValues" << std::endl;


          std::cout << "New episode" << std::endl;

          {

            uint32_t start, end;
            bool error = false, readed = false;
            do{
              error = !server.tryReadEpisodeStart(start, end, readed);
            }while (!readed && !error);

            if (error) std::cerr << "ERROR while reading EpisodeStart" << std::endl;
            else std::cout << "Received: startIndex = " << start << ", endIndex = " << end << std::endl;

          }

        if (!server.sendEndReset()) std::cerr << "ERROR while sending EndReset" << std::endl;

        while (true) {
          float steer, gas;
          bool newEpisode = false;
          if (!server.newEpisodeRequested(newEpisode)){
            std::cerr << "ERROR while checking for newEpisode request" << std::endl;
            break;
          }

          if (newEpisode){
            std::cout << "-------- NEW EPISODE --------" << std::endl;
            if (!server.sendSceneValues(sceneValues)){
              std::cerr << "ERROR while sending SceneValues" << std::endl;
              break;
            }

            std::cout << "Waiting Episode Start" << std::endl;

            uint32_t startPoint, endPoint;
            bool error = false, readed = false;
            do{
              error = !server.tryReadEpisodeStart(startPoint, endPoint, readed);
            }while (!readed && !error);

            if (error) {
              std::cerr << "ERROR while reading EpisodeStart" << std::endl;
              break;
            }
            else{
              std::cout << "--> Start: " << startPoint << " End: " << endPoint << " <--" << std::endl;
              server.sendEndReset();
            }

          }else {

            bool error = false, readed = false;
            if (!server.tryReadControl(steer, gas, readed)){
              std::cerr << "ERROR while reading Control" << std::endl;
              break;
            }
            else if (readed)
              std::cout << "CONTROL -->  gas: " << gas << " steer: " << steer << std::endl;


            

            if (!server.sendReward(makeReward())) {
              std::cerr << "ERROR while sending Reward" << std::endl;
              break;
            }
            
          }
        }

      std::cout << " -----  RESTARTING -----" <<  std::endl;
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

//TODO:
//pmatrix float 16
//start_index size_t
