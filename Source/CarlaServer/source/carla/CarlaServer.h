// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <array>
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
    uint8 B;
    uint8 G;
    uint8 R;
    uint8 A;
  };

  struct Reward_Values {
    /// Time-stamp of the current frame.
    int32 timestamp;
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
    int32 image_width, image_height;
    /// RGB images.
    std::vector<Color> image_rgb_0;
    std::vector<Color> image_rgb_1;
    /// Depth images.
    std::vector<Color> image_depth_1;
    std::vector<Color> image_depth_2;
  };

  struct Scene_Values {
    /// Possible world positions to spawn the player.
    std::vector<Vector2D> possible_Positions;
    /// Projection matrices of the cameras (1 in mode mono, 2 in stereo).
    std::vector<std::array<float, 16u>> projection_Matrices;
  };

  enum class Mode : int8 {
    MONO = 0,
    STEREO = 1,

    NUMBER_OF_MODES
  };

  /// Asynchronous TCP server. Uses three ports, one for sending messages
  /// (write), one for receiving messages (read), and one for world level
  /// commands (world).
  ///
  /// Writing and reading are executed in different threads. Each thread has its
  /// own queue of messages.
  class CarlaServer {
  public:

    explicit CarlaServer(uint32 writePort, uint32 readPort, uint32 worldPort);

    ~CarlaServer();

    CarlaServer(const CarlaServer &) = delete;
    CarlaServer &operator=(const CarlaServer &x) = delete;

    /// Initialize the server.
    ///
    /// @param LevelCount Number of levels available.
    void init(uint32 LevelCount);

    /// Try to read if the client has selected an scene and mode. Return false
    /// if the queue is empty.
    bool tryReadSceneInit(Mode &mode, int &scene);

    /// Try to read if the client has selected an end & start point. Return
    /// false if the queue is empty.
    bool tryReadEpisodeStart(uint32 &startIndex, uint32 &endIndex);

    /// Try to read the response of the client. Return false if the queue
    /// is empty.
    bool tryReadControl(float &steer, float &throttle);

    /// Send values of the current player status.
    void sendReward(const Reward_Values &values);

    /// Send the values of the generated scene.
    void sendSceneValues(const Scene_Values &values);

    /// Send a signal to the client to notify that the car is ready.
    void sendEndReset();

  private:

    class Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
  };

} // namespace carla
