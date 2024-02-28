// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file CarlaWeatherParameters.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAWEATHERPARAMETERS_H_
#define _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAWEATHERPARAMETERS_H_


#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <bitset>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(CarlaWeatherParameters_SOURCE)
#define CarlaWeatherParameters_DllAPI __declspec( dllexport )
#else
#define CarlaWeatherParameters_DllAPI __declspec( dllimport )
#endif // CarlaWeatherParameters_SOURCE
#else
#define CarlaWeatherParameters_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define CarlaWeatherParameters_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima


namespace carla_msgs {
    namespace msg {
        /*!
         * @brief This class represents the structure CarlaWeatherParameters defined by the user in the IDL file.
         * @ingroup CARLAWEATHERPARAMETERS
         */
        class CarlaWeatherParameters
        {
        public:

            /*!
             * @brief Default constructor.
             */
            eProsima_user_DllExport CarlaWeatherParameters();

            /*!
             * @brief Default destructor.
             */
            eProsima_user_DllExport ~CarlaWeatherParameters();

            /*!
             * @brief Copy constructor.
             * @param x Reference to the object carla_msgs::msg::CarlaWeatherParameters that will be copied.
             */
            eProsima_user_DllExport CarlaWeatherParameters(
                    const CarlaWeatherParameters& x);

            /*!
             * @brief Move constructor.
             * @param x Reference to the object carla_msgs::msg::CarlaWeatherParameters that will be copied.
             */
            eProsima_user_DllExport CarlaWeatherParameters(
                    CarlaWeatherParameters&& x);

            /*!
             * @brief Copy assignment.
             * @param x Reference to the object carla_msgs::msg::CarlaWeatherParameters that will be copied.
             */
            eProsima_user_DllExport CarlaWeatherParameters& operator =(
                    const CarlaWeatherParameters& x);

            /*!
             * @brief Move assignment.
             * @param x Reference to the object carla_msgs::msg::CarlaWeatherParameters that will be copied.
             */
            eProsima_user_DllExport CarlaWeatherParameters& operator =(
                    CarlaWeatherParameters&& x);

            /*!
             * @brief Comparison operator.
             * @param x carla_msgs::msg::CarlaWeatherParameters object to compare.
             */
            eProsima_user_DllExport bool operator ==(
                    const CarlaWeatherParameters& x) const;

            /*!
             * @brief Comparison operator.
             * @param x carla_msgs::msg::CarlaWeatherParameters object to compare.
             */
            eProsima_user_DllExport bool operator !=(
                    const CarlaWeatherParameters& x) const;

            /*!
             * @brief This function sets a value in member cloudiness
             * @param _cloudiness New value for member cloudiness
             */
            eProsima_user_DllExport void cloudiness(
                    float _cloudiness);

            /*!
             * @brief This function returns the value of member cloudiness
             * @return Value of member cloudiness
             */
            eProsima_user_DllExport float cloudiness() const;

            /*!
             * @brief This function returns a reference to member cloudiness
             * @return Reference to member cloudiness
             */
            eProsima_user_DllExport float& cloudiness();

            /*!
             * @brief This function sets a value in member precipitation
             * @param _precipitation New value for member precipitation
             */
            eProsima_user_DllExport void precipitation(
                    float _precipitation);

            /*!
             * @brief This function returns the value of member precipitation
             * @return Value of member precipitation
             */
            eProsima_user_DllExport float precipitation() const;

            /*!
             * @brief This function returns a reference to member precipitation
             * @return Reference to member precipitation
             */
            eProsima_user_DllExport float& precipitation();

            /*!
             * @brief This function sets a value in member precipitation_deposits
             * @param _precipitation_deposits New value for member precipitation_deposits
             */
            eProsima_user_DllExport void precipitation_deposits(
                    float _precipitation_deposits);

            /*!
             * @brief This function returns the value of member precipitation_deposits
             * @return Value of member precipitation_deposits
             */
            eProsima_user_DllExport float precipitation_deposits() const;

            /*!
             * @brief This function returns a reference to member precipitation_deposits
             * @return Reference to member precipitation_deposits
             */
            eProsima_user_DllExport float& precipitation_deposits();

            /*!
             * @brief This function sets a value in member wind_intensity
             * @param _wind_intensity New value for member wind_intensity
             */
            eProsima_user_DllExport void wind_intensity(
                    float _wind_intensity);

            /*!
             * @brief This function returns the value of member wind_intensity
             * @return Value of member wind_intensity
             */
            eProsima_user_DllExport float wind_intensity() const;

            /*!
             * @brief This function returns a reference to member wind_intensity
             * @return Reference to member wind_intensity
             */
            eProsima_user_DllExport float& wind_intensity();

            /*!
             * @brief This function sets a value in member fog_density
             * @param _fog_density New value for member fog_density
             */
            eProsima_user_DllExport void fog_density(
                    float _fog_density);

            /*!
             * @brief This function returns the value of member fog_density
             * @return Value of member fog_density
             */
            eProsima_user_DllExport float fog_density() const;

            /*!
             * @brief This function returns a reference to member fog_density
             * @return Reference to member fog_density
             */
            eProsima_user_DllExport float& fog_density();

            /*!
             * @brief This function sets a value in member fog_distance
             * @param _fog_distance New value for member fog_distance
             */
            eProsima_user_DllExport void fog_distance(
                    float _fog_distance);

            /*!
             * @brief This function returns the value of member fog_distance
             * @return Value of member fog_distance
             */
            eProsima_user_DllExport float fog_distance() const;

            /*!
             * @brief This function returns a reference to member fog_distance
             * @return Reference to member fog_distance
             */
            eProsima_user_DllExport float& fog_distance();

            /*!
             * @brief This function sets a value in member wetness
             * @param _wetness New value for member wetness
             */
            eProsima_user_DllExport void wetness(
                    float _wetness);

            /*!
             * @brief This function returns the value of member wetness
             * @return Value of member wetness
             */
            eProsima_user_DllExport float wetness() const;

            /*!
             * @brief This function returns a reference to member wetness
             * @return Reference to member wetness
             */
            eProsima_user_DllExport float& wetness();

            /*!
             * @brief This function sets a value in member sun_azimuth_angle
             * @param _sun_azimuth_angle New value for member sun_azimuth_angle
             */
            eProsima_user_DllExport void sun_azimuth_angle(
                    float _sun_azimuth_angle);

            /*!
             * @brief This function returns the value of member sun_azimuth_angle
             * @return Value of member sun_azimuth_angle
             */
            eProsima_user_DllExport float sun_azimuth_angle() const;

            /*!
             * @brief This function returns a reference to member sun_azimuth_angle
             * @return Reference to member sun_azimuth_angle
             */
            eProsima_user_DllExport float& sun_azimuth_angle();

            /*!
             * @brief This function sets a value in member sun_altitude_angle
             * @param _sun_altitude_angle New value for member sun_altitude_angle
             */
            eProsima_user_DllExport void sun_altitude_angle(
                    float _sun_altitude_angle);

            /*!
             * @brief This function returns the value of member sun_altitude_angle
             * @return Value of member sun_altitude_angle
             */
            eProsima_user_DllExport float sun_altitude_angle() const;

            /*!
             * @brief This function returns a reference to member sun_altitude_angle
             * @return Reference to member sun_altitude_angle
             */
            eProsima_user_DllExport float& sun_altitude_angle();


            /*!
             * @brief This function returns the maximum serialized size of an object
             * depending on the buffer alignment.
             * @param current_alignment Buffer alignment.
             * @return Maximum serialized size.
             */
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief This function returns the serialized size of a data depending on the buffer alignment.
             * @param data Data which is calculated its serialized size.
             * @param current_alignment Buffer alignment.
             * @return Serialized size.
             */
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const carla_msgs::msg::CarlaWeatherParameters& data,
                    size_t current_alignment = 0);


            /*!
             * @brief This function serializes an object using CDR serialization.
             * @param cdr CDR serialization object.
             */
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            /*!
             * @brief This function deserializes an object using CDR serialization.
             * @param cdr CDR serialization object.
             */
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);



            /*!
             * @brief This function returns the maximum serialized size of the Key of an object
             * depending on the buffer alignment.
             * @param current_alignment Buffer alignment.
             * @return Maximum serialized size.
             */
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief This function tells you if the Key has been defined for this type
             */
            eProsima_user_DllExport static bool isKeyDefined();

            /*!
             * @brief This function serializes the key members of an object using CDR serialization.
             * @param cdr CDR serialization object.
             */
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:

            float m_cloudiness;
            float m_precipitation;
            float m_precipitation_deposits;
            float m_wind_intensity;
            float m_fog_density;
            float m_fog_distance;
            float m_wetness;
            float m_sun_azimuth_angle;
            float m_sun_altitude_angle;
        };
    } // namespace msg
} // namespace carla_msgs

#endif // _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAWEATHERPARAMETERS_H_