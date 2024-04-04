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
 * @file CarlaVehicleControl.cpp
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

#include "CarlaVehicleControl.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

carla_msgs::msg::CarlaVehicleControl::CarlaVehicleControl()
{
    // m_header com.eprosima.fastdds.idl.parser.typecode.StructTypeCode@2e6a8155

    // m_throttle com.eprosima.idl.parser.typecode.PrimitiveTypeCode@6221a451
    m_throttle = 0.0;
    // m_steer com.eprosima.idl.parser.typecode.PrimitiveTypeCode@52719fb6
    m_steer = 0.0;
    // m_brake com.eprosima.idl.parser.typecode.PrimitiveTypeCode@3012646b
    m_brake = 0.0;
    // m_hand_brake com.eprosima.idl.parser.typecode.PrimitiveTypeCode@4a883b15
    m_hand_brake = false;
    // m_reverse com.eprosima.idl.parser.typecode.PrimitiveTypeCode@25641d39
    m_reverse = false;
    // m_gear com.eprosima.idl.parser.typecode.PrimitiveTypeCode@537f60bf
    m_gear = 0;
    // m_manual_gear_shift com.eprosima.idl.parser.typecode.PrimitiveTypeCode@5677323c
    m_manual_gear_shift = false;
    // m_control_priority com.eprosima.idl.parser.typecode.PrimitiveTypeCode@18df8434
    m_control_priority = 0;

}

carla_msgs::msg::CarlaVehicleControl::~CarlaVehicleControl()
{








}

carla_msgs::msg::CarlaVehicleControl::CarlaVehicleControl(
        const CarlaVehicleControl& x)
{
    m_header = x.m_header;
    m_throttle = x.m_throttle;
    m_steer = x.m_steer;
    m_brake = x.m_brake;
    m_hand_brake = x.m_hand_brake;
    m_reverse = x.m_reverse;
    m_gear = x.m_gear;
    m_manual_gear_shift = x.m_manual_gear_shift;
    m_control_priority = x.m_control_priority;
}

carla_msgs::msg::CarlaVehicleControl::CarlaVehicleControl(
        CarlaVehicleControl&& x)
{
    m_header = std::move(x.m_header);
    m_throttle = x.m_throttle;
    m_steer = x.m_steer;
    m_brake = x.m_brake;
    m_hand_brake = x.m_hand_brake;
    m_reverse = x.m_reverse;
    m_gear = x.m_gear;
    m_manual_gear_shift = x.m_manual_gear_shift;
    m_control_priority = x.m_control_priority;
}

carla_msgs::msg::CarlaVehicleControl& carla_msgs::msg::CarlaVehicleControl::operator =(
        const CarlaVehicleControl& x)
{

    m_header = x.m_header;
    m_throttle = x.m_throttle;
    m_steer = x.m_steer;
    m_brake = x.m_brake;
    m_hand_brake = x.m_hand_brake;
    m_reverse = x.m_reverse;
    m_gear = x.m_gear;
    m_manual_gear_shift = x.m_manual_gear_shift;
    m_control_priority = x.m_control_priority;

    return *this;
}

carla_msgs::msg::CarlaVehicleControl& carla_msgs::msg::CarlaVehicleControl::operator =(
        CarlaVehicleControl&& x)
{

    m_header = std::move(x.m_header);
    m_throttle = x.m_throttle;
    m_steer = x.m_steer;
    m_brake = x.m_brake;
    m_hand_brake = x.m_hand_brake;
    m_reverse = x.m_reverse;
    m_gear = x.m_gear;
    m_manual_gear_shift = x.m_manual_gear_shift;
    m_control_priority = x.m_control_priority;

    return *this;
}

bool carla_msgs::msg::CarlaVehicleControl::operator ==(
        const CarlaVehicleControl& x) const
{

    return (m_header == x.m_header && m_throttle == x.m_throttle && m_steer == x.m_steer && m_brake == x.m_brake && m_hand_brake == x.m_hand_brake && m_reverse == x.m_reverse && m_gear == x.m_gear && m_manual_gear_shift == x.m_manual_gear_shift && m_control_priority == x.m_control_priority);
}

bool carla_msgs::msg::CarlaVehicleControl::operator !=(
        const CarlaVehicleControl& x) const
{
    return !(*this == x);
}

size_t carla_msgs::msg::CarlaVehicleControl::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += std_msgs::msg::Header::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);



    return current_alignment - initial_alignment;
}

size_t carla_msgs::msg::CarlaVehicleControl::getCdrSerializedSize(
        const carla_msgs::msg::CarlaVehicleControl& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += std_msgs::msg::Header::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);



    return current_alignment - initial_alignment;
}

void carla_msgs::msg::CarlaVehicleControl::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{

    scdr << m_header;
    scdr << m_throttle;
    scdr << m_steer;
    scdr << m_brake;
    scdr << m_hand_brake;
    scdr << m_reverse;
    scdr << m_gear;
    scdr << m_manual_gear_shift;
    scdr << m_control_priority;

}

void carla_msgs::msg::CarlaVehicleControl::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{

    dcdr >> m_header;
    dcdr >> m_throttle;
    dcdr >> m_steer;
    dcdr >> m_brake;
    dcdr >> m_hand_brake;
    dcdr >> m_reverse;
    dcdr >> m_gear;
    dcdr >> m_manual_gear_shift;
    dcdr >> m_control_priority;
}

/*!
 * @brief This function copies the value in member header
 * @param _header New value to be copied in member header
 */
void carla_msgs::msg::CarlaVehicleControl::header(
        const std_msgs::msg::Header& _header)
{
    m_header = _header;
}

/*!
 * @brief This function moves the value in member header
 * @param _header New value to be moved in member header
 */
void carla_msgs::msg::CarlaVehicleControl::header(
        std_msgs::msg::Header&& _header)
{
    m_header = std::move(_header);
}

/*!
 * @brief This function returns a constant reference to member header
 * @return Constant reference to member header
 */
const std_msgs::msg::Header& carla_msgs::msg::CarlaVehicleControl::header() const
{
    return m_header;
}

/*!
 * @brief This function returns a reference to member header
 * @return Reference to member header
 */
std_msgs::msg::Header& carla_msgs::msg::CarlaVehicleControl::header()
{
    return m_header;
}
/*!
 * @brief This function sets a value in member throttle
 * @param _throttle New value for member throttle
 */
void carla_msgs::msg::CarlaVehicleControl::throttle(
        float _throttle)
{
    m_throttle = _throttle;
}

/*!
 * @brief This function returns the value of member throttle
 * @return Value of member throttle
 */
float carla_msgs::msg::CarlaVehicleControl::throttle() const
{
    return m_throttle;
}

/*!
 * @brief This function returns a reference to member throttle
 * @return Reference to member throttle
 */
float& carla_msgs::msg::CarlaVehicleControl::throttle()
{
    return m_throttle;
}

/*!
 * @brief This function sets a value in member steer
 * @param _steer New value for member steer
 */
void carla_msgs::msg::CarlaVehicleControl::steer(
        float _steer)
{
    m_steer = _steer;
}

/*!
 * @brief This function returns the value of member steer
 * @return Value of member steer
 */
float carla_msgs::msg::CarlaVehicleControl::steer() const
{
    return m_steer;
}

/*!
 * @brief This function returns a reference to member steer
 * @return Reference to member steer
 */
float& carla_msgs::msg::CarlaVehicleControl::steer()
{
    return m_steer;
}

/*!
 * @brief This function sets a value in member brake
 * @param _brake New value for member brake
 */
void carla_msgs::msg::CarlaVehicleControl::brake(
        float _brake)
{
    m_brake = _brake;
}

/*!
 * @brief This function returns the value of member brake
 * @return Value of member brake
 */
float carla_msgs::msg::CarlaVehicleControl::brake() const
{
    return m_brake;
}

/*!
 * @brief This function returns a reference to member brake
 * @return Reference to member brake
 */
float& carla_msgs::msg::CarlaVehicleControl::brake()
{
    return m_brake;
}

/*!
 * @brief This function sets a value in member hand_brake
 * @param _hand_brake New value for member hand_brake
 */
void carla_msgs::msg::CarlaVehicleControl::hand_brake(
        bool _hand_brake)
{
    m_hand_brake = _hand_brake;
}

/*!
 * @brief This function returns the value of member hand_brake
 * @return Value of member hand_brake
 */
bool carla_msgs::msg::CarlaVehicleControl::hand_brake() const
{
    return m_hand_brake;
}

/*!
 * @brief This function returns a reference to member hand_brake
 * @return Reference to member hand_brake
 */
bool& carla_msgs::msg::CarlaVehicleControl::hand_brake()
{
    return m_hand_brake;
}

/*!
 * @brief This function sets a value in member reverse
 * @param _reverse New value for member reverse
 */
void carla_msgs::msg::CarlaVehicleControl::reverse(
        bool _reverse)
{
    m_reverse = _reverse;
}

/*!
 * @brief This function returns the value of member reverse
 * @return Value of member reverse
 */
bool carla_msgs::msg::CarlaVehicleControl::reverse() const
{
    return m_reverse;
}

/*!
 * @brief This function returns a reference to member reverse
 * @return Reference to member reverse
 */
bool& carla_msgs::msg::CarlaVehicleControl::reverse()
{
    return m_reverse;
}

/*!
 * @brief This function sets a value in member gear
 * @param _gear New value for member gear
 */
void carla_msgs::msg::CarlaVehicleControl::gear(
        int32_t _gear)
{
    m_gear = _gear;
}

/*!
 * @brief This function returns the value of member gear
 * @return Value of member gear
 */
int32_t carla_msgs::msg::CarlaVehicleControl::gear() const
{
    return m_gear;
}

/*!
 * @brief This function returns a reference to member gear
 * @return Reference to member gear
 */
int32_t& carla_msgs::msg::CarlaVehicleControl::gear()
{
    return m_gear;
}

/*!
 * @brief This function sets a value in member manual_gear_shift
 * @param _manual_gear_shift New value for member manual_gear_shift
 */
void carla_msgs::msg::CarlaVehicleControl::manual_gear_shift(
        bool _manual_gear_shift)
{
    m_manual_gear_shift = _manual_gear_shift;
}

/*!
 * @brief This function returns the value of member manual_gear_shift
 * @return Value of member manual_gear_shift
 */
bool carla_msgs::msg::CarlaVehicleControl::manual_gear_shift() const
{
    return m_manual_gear_shift;
}

/*!
 * @brief This function returns a reference to member manual_gear_shift
 * @return Reference to member manual_gear_shift
 */
bool& carla_msgs::msg::CarlaVehicleControl::manual_gear_shift()
{
    return m_manual_gear_shift;
}

/*!
 * @brief This function sets a value in member control_priority
 * @param _control_priority New value for member control_priority
 */
void carla_msgs::msg::CarlaVehicleControl::control_priority(
        uint8_t _control_priority)
{
    m_control_priority = _control_priority;
}

/*!
 * @brief This function returns the value of member control_priority
 * @return Value of member control_priority
 */
uint8_t carla_msgs::msg::CarlaVehicleControl::control_priority() const
{
    return m_control_priority;
}

/*!
 * @brief This function returns a reference to member control_priority
 * @return Reference to member control_priority
 */
uint8_t& carla_msgs::msg::CarlaVehicleControl::control_priority()
{
    return m_control_priority;
}


size_t carla_msgs::msg::CarlaVehicleControl::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t current_align = current_alignment;



    return current_align;
}

bool carla_msgs::msg::CarlaVehicleControl::isKeyDefined()
{
    return false;
}

void carla_msgs::msg::CarlaVehicleControl::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
             
}

