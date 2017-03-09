#include "Carla.h"
#include "Protocol.h"

#include "CarlaServer.h"

namespace carla {
	namespace server {
		Protocol::Protocol(carla::server::CarlaServer *server) {
			_server = server;
		}

		Protocol::~Protocol() {

		}

		Reward Protocol::LoadReward(const Reward_Values &values) {
			Reward reward;
			reward.set_collision_car(values.collision_car);
			reward.set_collision_gen(values.collision_gen);
			reward.set_collision_ped(values.collision_ped);

			//IMAGE 1
			std::string* image = reward.add_image();
			std::vector<Color> img = values.img;

			for (int i = 0; i < img.size(); ++i) {
				image->append((const char*)img[i].red);
				image->append((const char*)img[i].green);
				image->append((const char*)img[i].blue);
				image->append((const char*)img[i].alpha);
			}


			if (_server->GetMode() == STEREO) {

				//IMAGE 2
				image = reward.add_image();
				img = values.img_2;

				for (int i = 0; i < img.size(); ++i) {
					image->append((const char*)img[i].red);
					image->append((const char*)img[i].green);
					image->append((const char*)img[i].blue);
					image->append((const char*)img[i].alpha);
				}

				//DEPTH 1
				image = reward.add_image();
				img = values.depth_1;

				for (int i = 0; i < img.size(); ++i) {
					image->append((const char*)img[i].red);
					image->append((const char*)img[i].green);
					image->append((const char*)img[i].blue);
					image->append((const char*)img[i].alpha);
				}

				//DEPTH 2
				image = reward.add_image();
				img = values.depth_2;

				for (int i = 0; i < img.size(); ++i) {
					image->append((const char*)img[i].red);
					image->append((const char*)img[i].green);
					image->append((const char*)img[i].blue);
					image->append((const char*)img[i].alpha);
				}
			}

			reward.set_inertia_x(values.inertia_x);
			reward.set_inertia_y(values.inertia_y);
			reward.set_inertia_z(values.inertia_z);
			reward.set_intersect(values.intersect);
			reward.set_ori_x(values.ori_x);
			reward.set_ori_y(values.ori_y);
			reward.set_ori_z(values.ori_z);
			reward.set_player_y(values.player_y);
			reward.set_player_x(values.player_x);
			reward.set_speed(values.speed);
			reward.set_timestamp(values.timestamp);

			return reward;
		}

		Scene Protocol::LoadScene(const Scene_Values &values) {
			Scene scene;
			Scene::Position* point;
			std::vector<Position> positions = values._possible_Positions;
			for (int i = 0; i < positions.size(); ++i) {
				point = scene.add_position();
				point->set_pos_x(positions[i].x);
				point->set_pos_y(positions[i].y);
			}


			if (_server->GetMode() == STEREO) {
				Scene::Projection_Matrix* matrix;
				std::vector<const float*> projection_matrix = values._projection_Matrix;
				for (int i = 0; i < projection_matrix.size(); ++i) {
					matrix = scene.add_camera_matrix();
					for (int e = 0; e < 16; ++e) matrix->add_cam_param(projection_matrix[i][e]);
				}
			}

			return scene;
		}

		World Protocol::LoadWorld() {
			World world;
			world.add_modes(_server->GetModesCount());
			world.add_scenes(_server->GetModesCount());
			return world;
		}
	}
}
