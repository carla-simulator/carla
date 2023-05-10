#include "carla/sensor/data/LivoxLidarMeasurement.h"
#include "carla/sensor/s11n/LivoxLidarSerializer.h"

namespace carla
{
    namespace sensor
    {
        namespace s11n
        {

            SharedPtr<SensorData> LivoxLidarSerializer::Deserialize(RawData &&data)
            {
                return SharedPtr<data::LivoxLidarMeasurement>(
                    new data::LivoxLidarMeasurement{std::move(data)});
            }

        } // namespace s11n
    }     // namespace sensor
} // namespace carla