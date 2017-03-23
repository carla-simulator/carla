// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace carla {

  struct Vector2D {
    float x;
    float y;
  };

  struct Vector3D {
    float x;
    float y;
    float z;
  };

  struct Color {
    uint8_t B;
    uint8_t G;
    uint8_t R;
    uint8_t A;
  };

  struct Reward_Values {
    /// Time-stamp of the current frame.
    int32_t timestamp;
    /// World location of the player.
    Vector2D player_location;
    /// Orientation of the player.
    Vector3D player_orientation;
    /// Current acceleration of the player.
    Vector3D player_acceleration;
    /// Forward speed in km/h.
    float forward_speed;
    /// General collision intensity (everything else but pedestrians and cars).
    float collision_general;
    /// Collision intensity with pedestrians.
    float collision_pedestrian;
    /// Collision intensity with other cars.
    float collision_car;
    /// Percentage of the car invading other lanes.
    float intersect_other_lane;
    /// Percentage of the car off-road.
    float intersect_offroad;
    /// Width and height of the images.
    uint32_t image_width, image_height;
    /// RGB images.
    std::vector<Color> image_rgb_0;
    std::vector<Color> image_rgb_1;
    /// Depth images.
    std::vector<Color> image_depth_0;
    std::vector<Color> image_depth_1;
  };

  struct Scene_Values {
    /// Possible world positions to spawn the player.
    std::vector<Vector2D> possible_positions;
    /// Projection matrices of the cameras (1 in mode mono, 2 in stereo).
    std::vector<std::array<float, 16u>> projection_matrices;
  };

  enum class Mode : int8_t {
    MONO,
    STEREO,

    NUMBER_OF_MODES,
    INVALID = -1
  };

  /// Asynchronous TCP server. Uses three ports, one for sending messages
  /// (write), one for receiving messages (read), and one for world level
  /// commands (world).
  ///
  /// Writing and reading are executed in different threads. Each thread has its
  /// own queue of messages.
  class CarlaServer {
  public:

    explicit CarlaServer(uint32_t writePort, uint32_t readPort, uint32_t worldPort);

    ~CarlaServer();

    CarlaServer(const CarlaServer &) = delete;
    CarlaServer &operator=(const CarlaServer &x) = delete;

    /// Initialize the server.
    ///
    /// @param LevelCount Number of levels available.
    bool init(uint32_t LevelCount);

    /// Try to read if the client has selected an scene and mode. Return false
    /// if the queue is empty.
    ///
    /// If returned mode INVALID, ignore scene.
    bool tryReadSceneInit(Mode &mode, uint32_t &scene, bool &readed);

    /// Try to read if the client has selected an end & start point. Return
    /// false if the queue is empty.
    bool tryReadEpisodeStart(uint32_t &startIndex, uint32_t &endIndex, bool &readed);

    /// Try to read the response of the client. Return false if the queue
    /// is empty.
    bool tryReadControl(float &steer, float &throttle, bool &readed);

    bool newEpisodeRequested(bool &newEpisode);

    /// Send values of the current player status.
    bool sendReward(std::unique_ptr<Reward_Values> values);

    /// Send the values of the generated scene.
    bool sendSceneValues(const Scene_Values &values);

    /// Send a signal to the client to notify that the car is ready.
    bool sendEndReset();

    /// return true if client thread is connected
    bool clientConnected();

    /// return true if server thread is connected
    bool serverConnected();

    /// return true if world thread is connected
    bool worldConnected();

    // Returns true if the server needs to restart the communication protocol
    bool needsRestart();

  private:

    class Pimpl;
    const std::unique_ptr<Pimpl> _pimpl;
  };

} // namespace carla
