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

static void makeImage(uint32_t width, uint32_t height, carla::Image &image) {
  // Xisco's magic image generator.
  std::vector<unsigned char> img(width * height * 4);
  for (uint32_t i = 0; i < height; ++i) {
   for (uint32_t e = 0; e < width; ++e) {
     img[4 * width * i + 4 * e + 0] = rand() % 256;//255 * !(e & i);
     img[4 * width * i + 4 * e + 1] = rand() % 256;//e ^ i;
     img[4 * width * i + 4 * e + 2] = rand() % 256;//e | i;
     img[4 * width * i + 4 * e + 3] = 255;
   }
  }

  // Convert to carla::Image.
  image.make(carla::ImageType::IMAGE, width, height);
  size_t i = 0u;
  for (carla::Color &color : image) {
    color.R = img[i++];
    color.G = img[i++];
    color.B = img[i++];
    color.A = img[i++];
  }
}

std::unique_ptr<carla::Reward_Values> makeReward() {
  auto reward = std::make_unique<carla::Reward_Values>();

  const uint32_t imageWidth = 800u;
  const uint32_t imageHeight = 600u;

  reward->player_location = {1.0f, 1.0f};
  reward->player_orientation = {1.0f, 0.0f, 0.0f};
  reward->player_acceleration = {1.0f, 0.0f, 0.0f};
  reward->forward_speed = 100.0f;
  reward->collision_general = 10.0f;
  reward->collision_pedestrian = 10.0f;
  reward->collision_car = 10.0f;
  reward->intersect_other_lane = 0.5f;
  reward->intersect_offroad = 0.5f;
  reward->game_timestamp = 0u;

  reward->images.clearAndResize(4u);
  for (carla::Image &img : reward->images) {
    makeImage(imageWidth, imageHeight, img);
  }
/*
  reward->image_rgb_0 = makeImage(imageWidth, imageHeight);
  reward->image_rgb_1 = makeImage(imageWidth, imageHeight);
  reward->image_depth_0 = makeImage(imageWidth, imageHeight);
  reward->image_depth_1 = makeImage(imageWidth, imageHeight);
*/
  static decltype(carla::Reward_Values::platform_timestamp) timestamp = 0u;

  reward->platform_timestamp = timestamp++;

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

    // This already starts the two threads.
    carla::CarlaServer server(writePort, readPort, worldPort);

    // Let's simulate the game loop.
    carla::CarlaString file;
    bool read;


    for (;;) {
      if (server.newEpisodeRequested(file, read) && read) {

        carla::Scene_Values sceneValues;

        sceneValues.possible_positions.clearAndResize(3u);
        sceneValues.possible_positions[0u] = {0.0f, 0.0f};
        sceneValues.possible_positions[1u] = {1.0f, 2.0f};
        sceneValues.possible_positions[2u] = {3.0f, 4.0f};

        //const std::array<float, 16u> pMatrix = {{ 10.0 }};
        //for (int i = 0; i < 100; ++i) sceneValues.projection_matrices.push_back(pMatrix);

        if (!server.sendSceneValues(sceneValues)) {
          std::cerr << "ERROR while sending SceneValues" << std::endl;
        }

        std::cout << "New episode" << std::endl;
        {
          uint32_t start, end;
          bool error = false, readed = false;
          do {
            error = !server.tryReadEpisodeStart(start, end, readed);
          } while (!readed && !error);

          if (error) {
            std::cerr << "ERROR while reading EpisodeStart" << std::endl;
          } else {
            std::cout << "Received: startIndex = " << start << ", endIndex = " << end << std::endl;
          }
        }

        if (!server.sendEndReset()) {
          std::cerr << "ERROR while sending EndReset" << std::endl;
        }

        while (true) {
          //float steer, gas;
          bool readed = false;
          carla::CarlaString newConfigFile;
          if (!server.newEpisodeRequested(newConfigFile, readed)){
            std::cerr << "ERROR while checking for newEpisode request" << std::endl;
            break;
          }

          if (readed){
            std::cout << "-------- NEW EPISODE --------" << std::endl;
            if (!server.sendSceneValues(sceneValues)){
              std::cerr << "ERROR while sending SceneValues" << std::endl;
              break;
            }

            uint32_t startPoint, endPoint;
            bool error = false, readed = false;
            do {
              error = !server.tryReadEpisodeStart(startPoint, endPoint, readed);

            } while (!readed && !error);

            if (error) {
              std::cerr << "ERROR while reading EpisodeStart" << std::endl;
              break;
            } else {
              std::cout << "--> Start: " << startPoint << " End: " << endPoint << " <--" << std::endl;
              server.sendEndReset();
            }

          } else {

            bool readed = false;
            carla::Control_Values control;
            if (!server.tryReadControl(control, readed)){
              std::cerr << "ERROR while reading Control" << std::endl;
              break;
            } else if (readed) {
              std::cout << "CONTROL -->  gas: " << control.gas << " steer: " << control.steer <<
              " brake: " << control.brake << " hand_brake: " << (control.hand_brake ? "true" : "false") << " reverse: " << (control.reverse ? "true" : "false") << std::endl;
            }

              if (!server.sendReward(makeReward().release())) {
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

