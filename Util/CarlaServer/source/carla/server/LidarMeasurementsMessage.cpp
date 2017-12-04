
#include "carla/server/LidarMeasurementsMessage.h"

#include <cstring>

#include "carla/Debug.h"
#include "carla/Logging.h"

namespace carla {
namespace server {

  static size_t GetSizeOfLidarPointsCounts(const carla_lidar_measurement& lidar_measurement) {
    return sizeof(uint32_t) * lidar_measurement.channels_count;
  }

  static size_t GetSizeOfLidarPoints(const carla_lidar_measurement& lidar_measurement) {
    size_t total_points = 0;
    for(int i=0; i<lidar_measurement.channels_count; i++)
    {
      total_points += lidar_measurement.points_count_by_channel[i];
    }
    return total_points * 3 * sizeof(double);
  }

  static size_t GetSizeOfBuffer(const_array_view<carla_lidar_measurement> lidar_measurements) {
    size_t total = 0u;
    for (const auto &lidar_measurement : lidar_measurements) {
      total += sizeof(double); // horizontal_angle
      total += 2 * sizeof(uint32_t); // type_of_the_message, channels_count
      total += GetSizeOfLidarPointsCounts(lidar_measurement);
      total += GetSizeOfLidarPoints(lidar_measurement);
    }
    return total;
  }

  static size_t WriteIntToBuffer(unsigned char *buffer, uint32_t size) {
    std::memcpy(buffer, &size, sizeof(uint32_t));
    return sizeof(uint32_t);
  }

  static size_t WriteDoubleToBuffer(unsigned char *buffer, double val) {
    std::memcpy(buffer, &val, sizeof(double));
    return sizeof(double);
  }

  static size_t WriteLidarMeasurementToBuffer(unsigned char *buffer, const carla_lidar_measurement &lidar_measurement) {
    const auto points_counts_size = GetSizeOfLidarPointsCounts(lidar_measurement);
    const auto points_size = GetSizeOfLidarPoints(lidar_measurement);
    DEBUG_ASSERT(lidar_measurement.points_count_by_channel != nullptr);
    DEBUG_ASSERT(lidar_measurement.data != nullptr);
    std::memcpy(buffer, lidar_measurement.points_count_by_channel, points_counts_size);
    std::memcpy(buffer + points_counts_size, lidar_measurement.data, points_size);
    return points_counts_size + points_size;
  }

  size_t LidarMeasurementsMessage::GetSize(const_array_view<carla_lidar_measurement> lidar_measurements) {
    return GetSizeOfBuffer(lidar_measurements);
  }

  size_t LidarMeasurementsMessage::Write(
    const_array_view<carla_lidar_measurement> lidar_measurements,
    unsigned char *buffer
  ) {
    long buffer_size = GetSizeOfBuffer(lidar_measurements);

    auto begin = buffer;
    for (const auto &lidar_measurement : lidar_measurements) {
      begin += WriteDoubleToBuffer(begin, lidar_measurement.horizontal_angle);
      begin += WriteIntToBuffer(begin, 10); // type of lidar message
      begin += WriteIntToBuffer(begin, lidar_measurement.channels_count);
      begin += WriteLidarMeasurementToBuffer(begin, lidar_measurement);
    }
    DEBUG_ASSERT(std::distance(buffer, begin) == buffer_size);
    return buffer_size;
  }

} // namespace server
} // namespace carla
