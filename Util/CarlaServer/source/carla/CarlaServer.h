// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <cstdint>

namespace carla {

  // ===========================================================================
  // -- Basic types ------------------------------------------------------------
  // ===========================================================================

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

  // ===========================================================================
  // -- Image ------------------------------------------------------------------
  // ===========================================================================

  class Image {
  public:

    ~Image();

    /// Creates an uninitialized image of the given size.
    void make(ImageType type, uint32_t width, uint32_t height);

    ImageType type() const {
      return _type;
    }

    uint32_t width() const {
      return _width;
    }

    uint32_t height() const {
      return _height;
    }

    uint32_t size() const {
      return _width * _height;
    }

    bool empty() const {
      return size() == 0u;
    }

    Color &operator[](uint32_t index) {
      return _image[index];
    }

    const Color &operator[](uint32_t index) const {
      return _image[index];
    }

    Color *begin() {
      return _image;
    }

    const Color *begin() const {
      return _image;
    }

    Color *end() {
      return _image + size();
    }

    const Color *end() const {
      return _image + size();
    }

  private:

    ImageType _type;

    uint32_t _width = 0u;

    uint32_t _height = 0u;

    Color *_image = nullptr;
  };

  // ===========================================================================
  // -- Collection -------------------------------------------------------------
  // ===========================================================================

  /// Basic dynamic array. To avoid using STL containers. Collection<char> is
  /// always null terminated.
  //
  // All dynamic memory management done in the cpp file.
  template <typename T>
  class Collection {
  public:

    Collection() = default;

    explicit Collection(uint32_t count);

    ~Collection();

    Collection(const Collection &) = delete;
    Collection &operator=(const Collection &) = delete;

    void clear();

    void clearAndResize(uint32_t count);

    /// Copy the contents of the array @a other till @a size.
    void copyContents(const T *other, uint32_t size);

    /// Takes over the ownership of the array @a other, and sets size to
    /// @a size.
    void takeOver(T *other, uint32_t size);

    T &operator[](uint32_t index) {
      return _array[index];
    }

    const T &operator[](uint32_t index) const {
      return _array[index];
    }

    uint32_t size() const {
      return _size;
    }

    T *begin() {
      return _array;
    }

    const T *begin() const {
      return _array;
    }

    T *end() {
      return _array + size();
    }

    const T *end() const {
      return _array + size();
    }

    const T *data() const {
      return _array;
    }

  private:

    T *_array = nullptr;

    uint32_t _size = 0u;
  };

  // ===========================================================================
  // -- CarlaString ------------------------------------------------------------
  // ===========================================================================

  using CarlaString = Collection<char>;

  // ===========================================================================
  // -- Control_Values ---------------------------------------------------------
  // ===========================================================================

  struct Control_Values {
    float steer;
    float gas;
    float brake;
    bool hand_brake;
    bool reverse;
  };

  // ===========================================================================
  // -- Reward_Values ----------------------------------------------------------
  // ===========================================================================

  struct Reward_Values {
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
    Collection<Image> images;
  };

  // ===========================================================================
  // -- Scene_Values -----------------------------------------------------------
  // ===========================================================================

  struct Scene_Values {
    /// Possible world positions to spawn the player.
    Collection<Vector2D> possible_positions;
  };

  // ===========================================================================
  // -- CarlaServer ------------------------------------------------------------
  // ===========================================================================

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
    CarlaServer &operator=(const CarlaServer &) = delete;

    /// Try to read if the client has selected an end & start point. Return
    /// false if the queue is empty.
    bool tryReadEpisodeStart(uint32_t &startIndex, uint32_t &endIndex, bool &readed);

    /// Try to read the response of the client. Return false if the queue
    /// is empty.
    bool tryReadControl(Control_Values &control, bool &readed);

    bool newEpisodeRequested(CarlaString &init_file, bool &readed);

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
    Pimpl * const _pimpl;
  };

} // namespace carla
