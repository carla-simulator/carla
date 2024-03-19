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
 * @file CarlaWeatherParameters.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace {
char dummy;
}  // namespace
#endif  // _WIN32

#include "CarlaWeatherParameters.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

carla_msgs::msg::CarlaWeatherParameters::CarlaWeatherParameters()
{
    // m_cloudiness com.eprosima.idl.parser.typecode.PrimitiveTypeCode@23e84203
    m_cloudiness = 0.0;
    // m_precipitation com.eprosima.idl.parser.typecode.PrimitiveTypeCode@19932c16
    m_precipitation = 0.0;
    // m_precipitation_deposits com.eprosima.idl.parser.typecode.PrimitiveTypeCode@73eb439a
    m_precipitation_deposits = 0.0;
    // m_wind_intensity com.eprosima.idl.parser.typecode.PrimitiveTypeCode@514646ef
    m_wind_intensity = 0.0;
    // m_fog_density com.eprosima.idl.parser.typecode.PrimitiveTypeCode@305ffe9e
    m_fog_density = 0.0;
    // m_fog_distance com.eprosima.idl.parser.typecode.PrimitiveTypeCode@302c971f
    m_fog_distance = 0.0;
    // m_wetness com.eprosima.idl.parser.typecode.PrimitiveTypeCode@1972e513
    m_wetness = 0.0;
    // m_sun_azimuth_angle com.eprosima.idl.parser.typecode.PrimitiveTypeCode@7ae0a9ec
    m_sun_azimuth_angle = 0.0;
    // m_sun_altitude_angle com.eprosima.idl.parser.typecode.PrimitiveTypeCode@35841320
    m_sun_altitude_angle = 0.0;

}

carla_msgs::msg::CarlaWeatherParameters::~CarlaWeatherParameters()
{








}

carla_msgs::msg::CarlaWeatherParameters::CarlaWeatherParameters(
        const CarlaWeatherParameters& x)
{
    m_cloudiness = x.m_cloudiness;
    m_precipitation = x.m_precipitation;
    m_precipitation_deposits = x.m_precipitation_deposits;
    m_wind_intensity = x.m_wind_intensity;
    m_fog_density = x.m_fog_density;
    m_fog_distance = x.m_fog_distance;
    m_wetness = x.m_wetness;
    m_sun_azimuth_angle = x.m_sun_azimuth_angle;
    m_sun_altitude_angle = x.m_sun_altitude_angle;
}

carla_msgs::msg::CarlaWeatherParameters::CarlaWeatherParameters(
        CarlaWeatherParameters&& x)
{
    m_cloudiness = x.m_cloudiness;
    m_precipitation = x.m_precipitation;
    m_precipitation_deposits = x.m_precipitation_deposits;
    m_wind_intensity = x.m_wind_intensity;
    m_fog_density = x.m_fog_density;
    m_fog_distance = x.m_fog_distance;
    m_wetness = x.m_wetness;
    m_sun_azimuth_angle = x.m_sun_azimuth_angle;
    m_sun_altitude_angle = x.m_sun_altitude_angle;
}

carla_msgs::msg::CarlaWeatherParameters& carla_msgs::msg::CarlaWeatherParameters::operator =(
        const CarlaWeatherParameters& x)
{

    m_cloudiness = x.m_cloudiness;
    m_precipitation = x.m_precipitation;
    m_precipitation_deposits = x.m_precipitation_deposits;
    m_wind_intensity = x.m_wind_intensity;
    m_fog_density = x.m_fog_density;
    m_fog_distance = x.m_fog_distance;
    m_wetness = x.m_wetness;
    m_sun_azimuth_angle = x.m_sun_azimuth_angle;
    m_sun_altitude_angle = x.m_sun_altitude_angle;

    return *this;
}

carla_msgs::msg::CarlaWeatherParameters& carla_msgs::msg::CarlaWeatherParameters::operator =(
        CarlaWeatherParameters&& x)
{

    m_cloudiness = x.m_cloudiness;
    m_precipitation = x.m_precipitation;
    m_precipitation_deposits = x.m_precipitation_deposits;
    m_wind_intensity = x.m_wind_intensity;
    m_fog_density = x.m_fog_density;
    m_fog_distance = x.m_fog_distance;
    m_wetness = x.m_wetness;
    m_sun_azimuth_angle = x.m_sun_azimuth_angle;
    m_sun_altitude_angle = x.m_sun_altitude_angle;

    return *this;
}

bool carla_msgs::msg::CarlaWeatherParameters::operator ==(
        const CarlaWeatherParameters& x) const
{

    return (m_cloudiness == x.m_cloudiness && m_precipitation == x.m_precipitation && m_precipitation_deposits == x.m_precipitation_deposits && m_wind_intensity == x.m_wind_intensity && m_fog_density == x.m_fog_density && m_fog_distance == x.m_fog_distance && m_wetness == x.m_wetness && m_sun_azimuth_angle == x.m_sun_azimuth_angle && m_sun_altitude_angle == x.m_sun_altitude_angle);
}

bool carla_msgs::msg::CarlaWeatherParameters::operator !=(
        const CarlaWeatherParameters& x) const
{
    return !(*this == x);
}

size_t carla_msgs::msg::CarlaWeatherParameters::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);



    return current_alignment - initial_alignment;
}

size_t carla_msgs::msg::CarlaWeatherParameters::getCdrSerializedSize(
        const carla_msgs::msg::CarlaWeatherParameters& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);



    return current_alignment - initial_alignment;
}

void carla_msgs::msg::CarlaWeatherParameters::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{

    scdr << m_cloudiness;
    scdr << m_precipitation;
    scdr << m_precipitation_deposits;
    scdr << m_wind_intensity;
    scdr << m_fog_density;
    scdr << m_fog_distance;
    scdr << m_wetness;
    scdr << m_sun_azimuth_angle;
    scdr << m_sun_altitude_angle;

}

void carla_msgs::msg::CarlaWeatherParameters::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{

    dcdr >> m_cloudiness;
    dcdr >> m_precipitation;
    dcdr >> m_precipitation_deposits;
    dcdr >> m_wind_intensity;
    dcdr >> m_fog_density;
    dcdr >> m_fog_distance;
    dcdr >> m_wetness;
    dcdr >> m_sun_azimuth_angle;
    dcdr >> m_sun_altitude_angle;
}

/*!
 * @brief This function sets a value in member cloudiness
 * @param _cloudiness New value for member cloudiness
 */
void carla_msgs::msg::CarlaWeatherParameters::cloudiness(
        float _cloudiness)
{
    m_cloudiness = _cloudiness;
}

/*!
 * @brief This function returns the value of member cloudiness
 * @return Value of member cloudiness
 */
float carla_msgs::msg::CarlaWeatherParameters::cloudiness() const
{
    return m_cloudiness;
}

/*!
 * @brief This function returns a reference to member cloudiness
 * @return Reference to member cloudiness
 */
float& carla_msgs::msg::CarlaWeatherParameters::cloudiness()
{
    return m_cloudiness;
}

/*!
 * @brief This function sets a value in member precipitation
 * @param _precipitation New value for member precipitation
 */
void carla_msgs::msg::CarlaWeatherParameters::precipitation(
        float _precipitation)
{
    m_precipitation = _precipitation;
}

/*!
 * @brief This function returns the value of member precipitation
 * @return Value of member precipitation
 */
float carla_msgs::msg::CarlaWeatherParameters::precipitation() const
{
    return m_precipitation;
}

/*!
 * @brief This function returns a reference to member precipitation
 * @return Reference to member precipitation
 */
float& carla_msgs::msg::CarlaWeatherParameters::precipitation()
{
    return m_precipitation;
}

/*!
 * @brief This function sets a value in member precipitation_deposits
 * @param _precipitation_deposits New value for member precipitation_deposits
 */
void carla_msgs::msg::CarlaWeatherParameters::precipitation_deposits(
        float _precipitation_deposits)
{
    m_precipitation_deposits = _precipitation_deposits;
}

/*!
 * @brief This function returns the value of member precipitation_deposits
 * @return Value of member precipitation_deposits
 */
float carla_msgs::msg::CarlaWeatherParameters::precipitation_deposits() const
{
    return m_precipitation_deposits;
}

/*!
 * @brief This function returns a reference to member precipitation_deposits
 * @return Reference to member precipitation_deposits
 */
float& carla_msgs::msg::CarlaWeatherParameters::precipitation_deposits()
{
    return m_precipitation_deposits;
}

/*!
 * @brief This function sets a value in member wind_intensity
 * @param _wind_intensity New value for member wind_intensity
 */
void carla_msgs::msg::CarlaWeatherParameters::wind_intensity(
        float _wind_intensity)
{
    m_wind_intensity = _wind_intensity;
}

/*!
 * @brief This function returns the value of member wind_intensity
 * @return Value of member wind_intensity
 */
float carla_msgs::msg::CarlaWeatherParameters::wind_intensity() const
{
    return m_wind_intensity;
}

/*!
 * @brief This function returns a reference to member wind_intensity
 * @return Reference to member wind_intensity
 */
float& carla_msgs::msg::CarlaWeatherParameters::wind_intensity()
{
    return m_wind_intensity;
}

/*!
 * @brief This function sets a value in member fog_density
 * @param _fog_density New value for member fog_density
 */
void carla_msgs::msg::CarlaWeatherParameters::fog_density(
        float _fog_density)
{
    m_fog_density = _fog_density;
}

/*!
 * @brief This function returns the value of member fog_density
 * @return Value of member fog_density
 */
float carla_msgs::msg::CarlaWeatherParameters::fog_density() const
{
    return m_fog_density;
}

/*!
 * @brief This function returns a reference to member fog_density
 * @return Reference to member fog_density
 */
float& carla_msgs::msg::CarlaWeatherParameters::fog_density()
{
    return m_fog_density;
}

/*!
 * @brief This function sets a value in member fog_distance
 * @param _fog_distance New value for member fog_distance
 */
void carla_msgs::msg::CarlaWeatherParameters::fog_distance(
        float _fog_distance)
{
    m_fog_distance = _fog_distance;
}

/*!
 * @brief This function returns the value of member fog_distance
 * @return Value of member fog_distance
 */
float carla_msgs::msg::CarlaWeatherParameters::fog_distance() const
{
    return m_fog_distance;
}

/*!
 * @brief This function returns a reference to member fog_distance
 * @return Reference to member fog_distance
 */
float& carla_msgs::msg::CarlaWeatherParameters::fog_distance()
{
    return m_fog_distance;
}

/*!
 * @brief This function sets a value in member wetness
 * @param _wetness New value for member wetness
 */
void carla_msgs::msg::CarlaWeatherParameters::wetness(
        float _wetness)
{
    m_wetness = _wetness;
}

/*!
 * @brief This function returns the value of member wetness
 * @return Value of member wetness
 */
float carla_msgs::msg::CarlaWeatherParameters::wetness() const
{
    return m_wetness;
}

/*!
 * @brief This function returns a reference to member wetness
 * @return Reference to member wetness
 */
float& carla_msgs::msg::CarlaWeatherParameters::wetness()
{
    return m_wetness;
}

/*!
 * @brief This function sets a value in member sun_azimuth_angle
 * @param _sun_azimuth_angle New value for member sun_azimuth_angle
 */
void carla_msgs::msg::CarlaWeatherParameters::sun_azimuth_angle(
        float _sun_azimuth_angle)
{
    m_sun_azimuth_angle = _sun_azimuth_angle;
}

/*!
 * @brief This function returns the value of member sun_azimuth_angle
 * @return Value of member sun_azimuth_angle
 */
float carla_msgs::msg::CarlaWeatherParameters::sun_azimuth_angle() const
{
    return m_sun_azimuth_angle;
}

/*!
 * @brief This function returns a reference to member sun_azimuth_angle
 * @return Reference to member sun_azimuth_angle
 */
float& carla_msgs::msg::CarlaWeatherParameters::sun_azimuth_angle()
{
    return m_sun_azimuth_angle;
}

/*!
 * @brief This function sets a value in member sun_altitude_angle
 * @param _sun_altitude_angle New value for member sun_altitude_angle
 */
void carla_msgs::msg::CarlaWeatherParameters::sun_altitude_angle(
        float _sun_altitude_angle)
{
    m_sun_altitude_angle = _sun_altitude_angle;
}

/*!
 * @brief This function returns the value of member sun_altitude_angle
 * @return Value of member sun_altitude_angle
 */
float carla_msgs::msg::CarlaWeatherParameters::sun_altitude_angle() const
{
    return m_sun_altitude_angle;
}

/*!
 * @brief This function returns a reference to member sun_altitude_angle
 * @return Reference to member sun_altitude_angle
 */
float& carla_msgs::msg::CarlaWeatherParameters::sun_altitude_angle()
{
    return m_sun_altitude_angle;
}


size_t carla_msgs::msg::CarlaWeatherParameters::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t current_align = current_alignment;



    return current_align;
}

bool carla_msgs::msg::CarlaWeatherParameters::isKeyDefined()
{
    return false;
}

void carla_msgs::msg::CarlaWeatherParameters::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
             
}


