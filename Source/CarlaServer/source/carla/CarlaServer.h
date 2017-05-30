// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
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

  enum ImageType : uint8_t {
    IMAGE,
    SCENE_FINAL,
    DEPTH,
    SEMANTIC_SEG,
  };

  struct Image {
    Image();
    ~Image();
    std::vector<Color> image;
    ImageType type;
    uint32_t width, height;
  };

  struct Control_Values {

    float steer;
    float gas;
    float brake;
    bool hand_brake;
    bool reverse;
  };

  struct Reward_Values {
    Reward_Values();
    ~Reward_Values();
    /// Time-stamp of the current frame, in milliseconds as given by the OS.
    int32_t platform_timestamp;
    /// In-game time-stamp, milliseconds elapsed since the beginning of the
    /// current level.
    int32_t game_timestamp;
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
    /// Captured images.
    std::vector<Image> images;
  };

  struct Scene_Values {
    Scene_Values();
    ~Scene_Values();
    /// Possible world positions to spawn the player.
    std::vector<Vector2D> possible_positions;
    /// Projection matrices of the cameras.
    //std::vector<std::array<float, 16u>> projection_matrices;
  };

  /// Asynchronous TCP server. Uses three ports, one for sending messages
  /// (write), one for receiving messages (read), and one for world level
  /// commands (world).
  ///
  /// Writing and reading are executed in different threads. Each thread has its
  /// own queue of messages.
  ///
  /// Functions return false if the server needs restart.
  class CarlaServer {
  public:

    explicit CarlaServer(uint32_t writePort, uint32_t readPort, uint32_t worldPort);

    ~CarlaServer();

    CarlaServer(const CarlaServer &) = delete;
    CarlaServer &operator=(const CarlaServer &x) = delete;

    /// Initialize the server.
    ///
    /// @param LevelCount Number of levels available.
    //bool init(uint32_t levelCount);

    /// Try to read if the client has selected an scene and mode. Return false
    /// if the queue is empty.
    //bool tryReadSceneInit(uint32_t &scene, bool &readed);

    /// Try to read if the client has selected an end & start point. Return
    /// false if the queue is empty.
    bool tryReadEpisodeStart(uint32_t &startIndex, uint32_t &endIndex, bool &readed);

    /// Try to read the response of the client. Return false if the queue
    /// is empty.
    bool tryReadControl(Control_Values &control, bool &readed);

    bool newEpisodeRequested(std::string &init_file, bool &readed);

    /// Send values of the current player status.
    ///
    /// @warning This function takes ownership of the @a values object.
    bool sendReward(Reward_Values *values);

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
