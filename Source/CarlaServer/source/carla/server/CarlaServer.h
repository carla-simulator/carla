// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CarlaCommunication.h"

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace carla {
namespace server {


	struct Color {
		std::uint8_t red;
		std::uint8_t green;
		std::uint8_t blue;
		std::uint8_t alpha;
	};

	struct Position {
		float x, y;
	};

	struct Reward_Values {
		float player_x, player_y;
		float speed;
		float collision_gen, collision_ped, collision_car;
		float intersect;
		float inertia_x, inertia_y, inertia_z;
		std::int32_t timestamp;
		float ori_x = 0, ori_y, ori_z;
		std::vector<Color> img;
		std::vector<Color> img_2;
		std::vector<Color> depth_1;
		std::vector<Color> depth_2;
	};

	struct Scene_Values {
		std::vector<Position> _possible_Positions;
		std::vector<const float*> _projection_Matrix;
	};

	enum Mode {
		MONO = 0,
		STEREO = 1
	};
  /// Asynchronous TCP server. Uses two ports, one for sending messages (write)
  /// and one for receiving messages (read).
  ///
  /// Writing and reading are executed in two different threads. Each thread has
  /// its own queue of messages.
  ///
  /// Note that a new socket is created for every connection (every write and
  /// read).
  class CARLA_API CarlaServer : private NonCopyable {
  public:

    /// Starts two threads for writing and reading.
    explicit CarlaServer(int writePort, int readPort, int worldPort, int modesCount, int scenesCount);

    ~CarlaServer();

    ///// Send values of the current player status
    void sendReward( const Reward_Values &values);

	//// Send the values of the generated scene
	void sendSceneValues( const Scene_Values &values);

	//// Send a signal to the client to notify that the car is ready
	void sendEndReset();

	void sendWord();

    ///// Try to read the response of the client. Return false if the queue
    ///// is empty.
    bool tryReadControl(float &steer, float &gas);
	
	////Try to read if the client has selected an scene and mode. Return false if the queue is empty
	bool tryReadSceneInit(int &mode, int &scene);

	////Try to read if the client has selected an end & start point. Return false if the queue is empty
	bool tryReadEpisodeStart(float &start_index, float &end_index);



	int GetModesCount() const;
	int GetScenesCount() const;

	void setMode(Mode mode);
	Mode GetMode() const;

	void SetScene(int scene);
	int GetScene() const;

	void SetReset(bool reset);
	bool Reset() const;


  private:

	
	std::mutex _mutex;

	std::atomic<Mode> _mode = MONO;
	std::atomic_int _scene;
	std::atomic_bool _reset;

	const int _modes;
	const int _scenes;

	const std::unique_ptr<CarlaCommunication> _communication;

	const std::unique_ptr<Protocol> _proto;

  };

} // namespace server
} // namespace carla
