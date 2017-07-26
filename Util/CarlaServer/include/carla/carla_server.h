/* CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings. */

#ifndef CARLA_CARLASERVER_H
#define CARLA_CARLASERVER_H

#include <stdint.h>

#ifndef CARLA_SERVER_API
#  define CARLA_SERVER_API extern
#endif // CARLA_SERVER_API

#ifdef __cplusplus
extern "C" {
#endif

  /* ======================================================================== */
  /* -- Basic types --------------------------------------------------------- */
  /* ======================================================================== */

  struct carla_vector3d {
    float x;
    float y;
    float z;
  };

  struct carla_image {
    uint32_t width;
    uint32_t height;
    uint32_t type;
    const uint32_t *data;
  };

  /* ======================================================================== */
  /* -- carla_request_new_episode ------------------------------------------- */
  /* ======================================================================== */

  /** @warning the underlying char array is statically allocated inside
    * CarlaServer, it might be deleted on subsequent requests of new episodes,
    * therefore for a given CarlaServer carla_read_request_new_episode
    * is NOT thread-safe.
    *
    * Do NOT delete the char array.
    */
  struct carla_request_new_episode {
    const char *ini_file;
    uint32_t ini_file_length;
  };

  /* ======================================================================== */
  /* -- carla_scene_description --------------------------------------------- */
  /* ======================================================================== */

  struct carla_scene_description {
    /** Collection of the initial player start locations. */
    const struct carla_vector3d *player_start_locations;
    uint32_t number_of_player_start_locations;
  };

  /* ======================================================================== */
  /* -- carla_episode_start ------------------------------------------------- */
  /* ======================================================================== */

  struct carla_episode_start {
    uint32_t player_start_location_index;
  };

  /* ======================================================================== */
  /* -- carla_episode_ready ------------------------------------------------- */
  /* ======================================================================== */

  struct carla_episode_ready {
    bool ready;
  };

  /* ======================================================================== */
  /* -- carla_control ------------------------------------------------------- */
  /* ======================================================================== */

  struct carla_control {
    float steer;
    float throttle;
    float brake;
    bool hand_brake;
    bool reverse;
  };

  /* ======================================================================== */
  /* -- carla_player_measurements ------------------------------------------- */
  /* ======================================================================== */

  struct carla_player_measurements {
    /** World location of the player. */
    struct carla_vector3d location;
    /** Orientation of the player. */
    struct carla_vector3d orientation;
    /** Current acceleration of the player. */
    struct carla_vector3d acceleration;
    /** Forward speed in km/h. */
    float forward_speed;
    /** Collision intensity with other vehicles. */
    float collision_vehicles;
    /** Collision intensity with pedestrians. */
    float collision_pedestrians;
    /** General collision intensity (everything else but pedestrians and vehicles). */
    float collision_other;
    /** Percentage of the car invading other lanes. */
    float intersection_otherlane;
    /** Percentage of the car off-road. */
    float intersection_offroad;
  };

  /* ======================================================================== */
  /* -- carla_measurements -------------------------------------------------- */
  /* ======================================================================== */

  struct carla_measurements {
    /** Time-stamp of the current frame, in milliseconds as given by the OS. */
    uint32_t platform_timestamp;
    /** In-game time-stamp, milliseconds elapsed since the beginning of the current level. */
    uint32_t game_timestamp;
    /** Player measurements. */
    struct carla_player_measurements player_measurements;
  };

  /* ======================================================================== */
  /* -- CARLA server -------------------------------------------------------- */
  /* ======================================================================== */

  /** CARLA Server
    *
    * int32_t as return type indicates the error code of the operation, it
    * matches boost::asio::error::basic_errors. A value of 0 indicates success.
    *
    * Most of the functions have a time-out, typically blocking this thread
    * until the corresponding asynchronous operation is completed or the time-
    * out is met. Set a time-out of 0 to get a non-blocking call.
    */

  typedef void* CarlaServerPtr;

  CARLA_SERVER_API const int32_t CARLA_SERVER_SUCCESS;
  CARLA_SERVER_API const int32_t CARLA_SERVER_TRY_AGAIN;
  CARLA_SERVER_API const int32_t CARLA_SERVER_TIMED_OUT;
  CARLA_SERVER_API const int32_t CARLA_SERVER_OPERATION_ABORTED;

  /* -- Creation and destruction -------------------------------------------- */

  /** Create a CARLA server instance. */
  CARLA_SERVER_API CarlaServerPtr carla_make_server();

  /** Destroy a CARLA server instance (disconnects all running servers
    * associated with this instance).
    */
  CARLA_SERVER_API void carla_free_server(CarlaServerPtr self);

  /* -- Connecting and disconnecting ---------------------------------------- */

  /** Connect the CARLA world server.
    *
    * The time-out in this functions sets the time-out used for all the
    * subsequent networking communications with the given instance.
    *
    * The non-blocking version returns immediately while the blocking version
    * waits until the connection is established successfully.
    */
  CARLA_SERVER_API int32_t carla_server_connect(
      CarlaServerPtr self,
      uint32_t world_port,
      uint32_t server_timeout_milliseconds);
  CARLA_SERVER_API int32_t carla_server_connect_non_blocking(
      CarlaServerPtr self,
      uint32_t world_port,
      uint32_t server_timeout_milliseconds);

  /** Signal the world server to disconnect. */
  CARLA_SERVER_API void carla_disconnect_server(CarlaServerPtr self);

  /* -- Write and read functions -------------------------------------------- */

  /** If the new episode request is received, blocks until the agent server is
    * terminated.
    */
  CARLA_SERVER_API int32_t carla_read_request_new_episode(
      CarlaServerPtr self,
      carla_request_new_episode &values,
      uint32_t timeout_milliseconds);

  CARLA_SERVER_API int32_t carla_write_scene_description(
      CarlaServerPtr self,
      const carla_scene_description &values,
      uint32_t timeout_milliseconds);

  CARLA_SERVER_API int32_t carla_read_episode_start(
      CarlaServerPtr self,
      carla_episode_start &values,
      uint32_t timeout_milliseconds);

  /** This launches the agent server. */
  CARLA_SERVER_API int32_t carla_write_episode_ready(
      CarlaServerPtr self,
      const carla_episode_ready &values,
      const uint32_t timeout);

  /** Return values:
    *   CARLA_SERVER_SUCCESS A value was readed.
    *   CARLA_SERVER_TRY_AGAIN Nothing received yet.
    *   CARLA_SERVER_OPERATION_ABORTED Agent server is missing.
    */
  CARLA_SERVER_API int32_t carla_read_control(
      CarlaServerPtr self,
      carla_control &values,
      uint32_t timeout_milliseconds);

  /** Return values:
    *   CARLA_SERVER_SUCCESS Value was posted for sending.
    *   CARLA_SERVER_OPERATION_ABORTED Agent server is missing.
    */
  CARLA_SERVER_API int32_t carla_write_measurements(
      CarlaServerPtr self,
      const carla_measurements &values,
      const struct carla_image *images,
      uint32_t number_of_images);

#ifdef __cplusplus
}
#endif

#endif /* CARLA_CARLASERVER_H */
