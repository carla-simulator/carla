#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

void lock(const char *oldpath, const char *newpath) {
  while (symlink(oldpath, newpath) == -1) {
    continue;
  }
}

int main() {

  std::string data_sync_dir = "./../carla-veins-data/";
  int veh_id = 0;
  int c = 0;

  std::string sensor_data = "[{\"id\": " + std::to_string(veh_id) + ", \"position\": [1, 2]}]";
  std::string sensor_data_file_name = std::to_string(veh_id) + "_sensor.json";
  std::string sensor_lock_file_name = std::to_string(veh_id) + "_sensor.json.lock";

  std::string packet_data = "[{\"id\": " + std::to_string(veh_id) + ", \"sender\": " + std::to_string(veh_id + 1) + "}]";
  std::string packet_data_file_name = std::to_string(veh_id) + "_packet.json";
  std::string packet_lock_file_name = std::to_string(veh_id) + "_packet.json.lock";

  while (true) {
    c = c + 1;
    sensor_data = "[{\"id\": " + std::to_string(veh_id) + ", \"position\": [1, 2], \"c\": " + std::to_string(c) + "}]";
    packet_data = "[{\"id\": " + std::to_string(veh_id) + ", \"sender\": " + std::to_string(veh_id + 1) + ", \"c\": " + std::to_string(c) + "}]";

    sleep(1);
    lock((data_sync_dir + packet_data_file_name).c_str(), (data_sync_dir + packet_lock_file_name).c_str());
    std::ofstream ofs(data_sync_dir + packet_data_file_name, std::ios::in | std::ios::ate);
    ofs << packet_data << std::endl;
    ofs.close();
    unlink((data_sync_dir + packet_lock_file_name).c_str());

    lock((data_sync_dir + sensor_data_file_name).c_str(), (data_sync_dir + sensor_lock_file_name).c_str());
    std::ifstream ifs(data_sync_dir + sensor_data_file_name);
    std::string buf;
    std::string data;
    while (!ifs.eof()) {
      std::getline(ifs, buf);
      std::cout << buf << std::endl;
      data += buf + "\n";
    }
    // std::cout << data << std::endl;
    ifs.close();

    std::ofstream ofs2(data_sync_dir + sensor_data_file_name);
    ofs2.close();
    unlink((data_sync_dir + sensor_lock_file_name).c_str());
  }
  return 0;
}
