#include "Carla.h"
#include "Protocol.h"

#include "Server.h"
#include "lodepng.h"

#include <iostream>

namespace carla {
	namespace server {

		Protocol::Protocol(carla::server::Server *server) {
			_server = server;
		}

		Protocol::~Protocol() {

		}

		Reward Protocol::LoadReward(const Reward_Values &values) {
			Reward reward;
			reward.set_collision_car(values.collision_car);
			reward.set_collision_gen(values.collision_general);
			reward.set_collision_ped(values.collision_pedestrian);
      reward.set_acceleration_x(values.player_acceleration.x);
      reward.set_acceleration_y(values.player_acceleration.y);
      reward.set_acceleration_z(values.player_acceleration.z);
      reward.set_sidewalk_intersect(values.intersect_offroad);
      reward.set_ori_x(values.player_orientation.x);
      reward.set_ori_y(values.player_orientation.y);
      reward.set_ori_z(values.player_orientation.z);
      reward.set_player_y(values.player_location.y);
      reward.set_player_x(values.player_location.x);
      reward.set_speed(values.foreward_speed);
      reward.set_timestamp(values.timestamp);

      lodepng::State state;

			//IMAGE 1
      //std::string image = reward.add_image();

			std::vector<unsigned char> color_img = values.image_rgb_0;

      std::vector<unsigned char> img;
      img.resize(color_img.size()*4);

      for(int i = 0; i < color_img.size(); ++i){
        img[4*i] = color_img[i].R;
        img[4*i + 1] = color_img[i].G;
        img[4*i + 2] = color_img[i].B;
        img[4*i + 3] = color_img[i].A;
      }

      std::vector<unsigned char> png;
      unsigned error = lodepng::encode(png, img, values.image_width, values.image_height, state);


      if (!error) {
        reward.add_image(std::string(png.begin(), png.end()));
        //for (int i = 0; i < png.size(); ++i) {
        //  (*image) += png[i];
        //  //image->append(GetBytes((img[i].green)));
        //  //image->append(GetBytes((img[i].blue)));
        //  //image->append(GetBytes((img[i].alpha)));
        //}
      }
      else std::cout << "No se pudo cargar img" << std::endl;




			if (_server->GetMode() == STEREO) {

				//IMAGE 2
				//image = reward.add_image();
			color_img = values.image_rgb_1;
      img.clear();
      img.resize(color_img.size()*4);

      for(int i = 0; i < color_img.size(); ++i){
        img[4*i] = color_img[i].R;
        img[4*i + 1] = color_img[i].G;
        img[4*i + 2] = color_img[i].B;
        img[4*i + 3] = color_img[i].A;
      }

        png.clear();
        error = lodepng::encode(png, img, values.image_width, values.image_height, state);

        reward.add_image(std::string(png.begin(), png.end()));

				//for (int i = 0; i < png.size(); ++i) {

				//	//(*image) += png[i];
				//	/*image->append(GetBytes((img[i].red)));
				//	image->append(GetBytes((img[i].green)));
				//	image->append(GetBytes((img[i].blue)));*/
				//	//image->append(GetBytes((img[i].alpha)));
				//}

				//DEPTH 1
				//image = reward.add_depth();
				color_img = values.image_depth_0;

        img.clear();
        img.resize(color_img.size()*4);

        for(int i = 0; i < color_img.size(); ++i){
          img[4*i] = color_img[i].R;
          img[4*i + 1] = color_img[i].G;
          img[4*i + 2] = color_img[i].B;
          img[4*i + 3] = color_img[i].A;
        }


        png.clear();
        error = lodepng::encode(png, img, values.image_width, values.image_height, state);
        reward.add_image(std::string(png.begin(), png.end()));
				//for (int i = 0; i < png.size(); ++i) {
				//	//(*image) += png[i];
				//	/*image->append(GetBytes((img[i].red)));
				//	image->append(GetBytes((img[i].green)));
				//	image->append(GetBytes((img[i].blue)));*/
				//	//image->append(GetBytes((img[i].alpha)));
				//}

				//DEPTH 2
				//image = reward.add_depth();
				color_img = values.image_depth_1;

        img.clear();
        img.resize(color_img.size()*4);

        for(int i = 0; i < color_img.size(); ++i){
          img[4*i] = color_img[i].R;
          img[4*i + 1] = color_img[i].G;
          img[4*i + 2] = color_img[i].B;
          img[4*i + 3] = color_img[i].A;
        }


        png.clear();
        error = lodepng::encode(png, img, values.image_width, values.image_height, state);
        reward.add_image(std::string(png.begin(), png.end()));
				//for (int i = 0; i < png.size(); ++i) {
				//	//(*image) += png[i];
				//	/*image->append(GetBytes((img[i].red)));
				//	image->append(GetBytes((img[i].green)));
				//	image->append(GetBytes((img[i].blue)));*/
				//	//image->append(GetBytes((img[i].alpha)));
				//}
			}



			return reward;
		}

		Scene Protocol::LoadScene(const Scene_Values &values) {
			Scene scene;
			Scene::Position* point;
			std::vector<Vector2D> positions = values.possible_Positions;
			for (int i = 0; i < positions.size(); ++i) {
				point = scene.add_position();
				point->set_pos_x(positions[i].x);
				point->set_pos_y(positions[i].y);
			}


			if (_server->GetMode() == STEREO) {
				Scene::Projection_Matrix* matrix;
				std::vector<atd::array<float,16>> projection_matrix = values.projection_Matrix;
				for (int i = 0; i < projection_matrix.size(); ++i) {
					matrix = scene.add_camera_matrix();
					for (int e = 0; e < 16; ++e) matrix->add_cam_param(projection_matrix[i][e]);
				}
			}

			return scene;
		}

		World Protocol::LoadWorld(const int modes, const int scenes) {
			World world;
			world.add_modes(modes);
			world.add_scenes(scenes);
			return world;
		}
	}
}
