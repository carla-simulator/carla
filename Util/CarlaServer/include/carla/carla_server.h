/* Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
 * de Barcelona (UAB).
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

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

  struct carla_rotation3d {
    float pitch;
    float yaw;
    float roll;
  };

  struct carla_transform {
    /** Location in meters. */
    struct carla_vector3d location;
    /** Unit vector pointing "forward". */
    struct carla_vector3d orientation;
    /** Rotation angles in degrees. */
    struct carla_rotation3d rotation;
  };

  struct carla_bounding_box {
    struct carla_transform transform;
    struct carla_vector3d extent;
  };

  /* ======================================================================== */
  /* -- agents -------------------------------------------------------------- */
  /* ======================================================================== */

#define CARLA_SERVER_AGENT_UNKNOWN            0u
#define CARLA_SERVER_AGENT_VEHICLE           10u
#define CARLA_SERVER_AGENT_PEDESTRIAN        20u
#define CARLA_SERVER_AGENT_SPEEDLIMITSIGN    30u
#define CARLA_SERVER_AGENT_TRAFFICLIGHT      40u

#define CARLA_SERVER_AGENT_TRAFFICLIGHT_GREEN   41u
#define CARLA_SERVER_AGENT_TRAFFICLIGHT_YELLOW  42u
#define CARLA_SERVER_AGENT_TRAFFICLIGHT_RED     43u

  /** @todo This is a bit tricky:
    * - If type is a traffic light, box extent and forward speed are ignored.
    * - If type is a speed limit sign, box extent is ignored. Forward speed is speed limit.
    * - If type is a vehicle or a pedestrian, every field is valid.
    */
  struct carla_agent {
    uint32_t id;
    uint32_t type;
    struct carla_transform transform;
    struct carla_bounding_box bounding_box;
    float forward_speed;
  };

  /* ======================================================================== */
  /* -- sensors ------------------------------------------------------------- */
  /* ======================================================================== */

#define CARLA_SERVER_SENSOR_UNKNOWN                  0u
#define CARLA_SERVER_CAMERA                        101u
#define CARLA_SERVER_LIDAR_RAY_CAST                102u

  struct carla_sensor_definition {
    /** Id of the sensor. */
    uint32_t id;
    /** Type of the sensor (one of the above defines). */
    uint32_t type;
    /** Display name of the sensor. */
    const char *name;
  };

  struct carla_sensor_data {
    uint32_t id;
    const void *header;
    uint32_t header_size;
    const void *data;
    uint32_t data_size;
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
    /** Display name of the current map. */
    const char *map_name;
    /** Collection of the initial player start locations. */
    const struct carla_transform *player_start_spots;
    uint32_t number_of_player_start_spots;
    /** Definitions of the sensors present in the scene. */
    const struct carla_sensor_definition *sensors;
    uint32_t number_of_sensors;
  };

  /* ======================================================================== */
  /* -- carla_episode_start ------------------------------------------------- */
  /* ======================================================================== */

  struct carla_episode_start {
    uint32_t player_start_spot_index;
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
    /** World transform of the player. */
    struct carla_transform transform;
    /** Bounding box of the player. */
    struct carla_bounding_box bounding_box;
    /** Current acceleration of the player. */
    struct carla_vector3d acceleration;
    /** Forward speed in m/s. */
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
    /** Vehicle's AI control that would apply this frame. */
    struct carla_control autopilot_control;
  };

  /* ======================================================================== */
  /* -- carla_measurements -------------------------------------------------- */
  /* ======================================================================== */

  struct carla_measurements {
    /** Frame counter. */
    uint32_t frame_number;
    /** Time-stamp of the current frame, in milliseconds as given by the OS. */
    uint32_t platform_timestamp;
    /** In-game time-stamp, milliseconds elapsed since the beginning of the current level. */
    uint32_t game_timestamp;
    /** Player measurements. */
    struct carla_player_measurements player_measurements;
    /** Non-player agents. */
    const struct carla_agent *non_player_agents;
    uint32_t number_of_non_player_agents;
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
  CARLA_SERVER_API int32_t carla_write_sensor_data(
      CarlaServerPtr self,
      const carla_sensor_data &data);

  /** Return values:
    *   CARLA_SERVER_SUCCESS Value was posted for sending.
    *   CARLA_SERVER_OPERATION_ABORTED Agent server is missing.
    */
  CARLA_SERVER_API int32_t carla_write_measurements(
      CarlaServerPtr self,
      const carla_measurements &values);

#ifdef __cplusplus
}
#endif

#endif /* CARLA_CARLASERVER_H */
