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
 * @file AckermannDrive.cpp
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

#include "AckermannDrive.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

ackermann_msgs::msg::AckermannDrive::AckermannDrive()
{
    // m_steering_angle com.eprosima.idl.parser.typecode.PrimitiveTypeCode@2654635
    m_steering_angle = 0.0;
    // m_steering_angle_velocity com.eprosima.idl.parser.typecode.PrimitiveTypeCode@737a135b
    m_steering_angle_velocity = 0.0;
    // m_speed com.eprosima.idl.parser.typecode.PrimitiveTypeCode@687ef2e0
    m_speed = 0.0;
    // m_acceleration com.eprosima.idl.parser.typecode.PrimitiveTypeCode@15dcfae7
    m_acceleration = 0.0;
    // m_jerk com.eprosima.idl.parser.typecode.PrimitiveTypeCode@3da05287
    m_jerk = 0.0;

}

ackermann_msgs::msg::AckermannDrive::~AckermannDrive()
{




}

ackermann_msgs::msg::AckermannDrive::AckermannDrive(
        const AckermannDrive& x)
{
    m_steering_angle = x.m_steering_angle;
    m_steering_angle_velocity = x.m_steering_angle_velocity;
    m_speed = x.m_speed;
    m_acceleration = x.m_acceleration;
    m_jerk = x.m_jerk;
}

ackermann_msgs::msg::AckermannDrive::AckermannDrive(
        AckermannDrive&& x)
{
    m_steering_angle = x.m_steering_angle;
    m_steering_angle_velocity = x.m_steering_angle_velocity;
    m_speed = x.m_speed;
    m_acceleration = x.m_acceleration;
    m_jerk = x.m_jerk;
}

ackermann_msgs::msg::AckermannDrive& ackermann_msgs::msg::AckermannDrive::operator =(
        const AckermannDrive& x)
{

    m_steering_angle = x.m_steering_angle;
    m_steering_angle_velocity = x.m_steering_angle_velocity;
    m_speed = x.m_speed;
    m_acceleration = x.m_acceleration;
    m_jerk = x.m_jerk;

    return *this;
}

ackermann_msgs::msg::AckermannDrive& ackermann_msgs::msg::AckermannDrive::operator =(
        AckermannDrive&& x)
{

    m_steering_angle = x.m_steering_angle;
    m_steering_angle_velocity = x.m_steering_angle_velocity;
    m_speed = x.m_speed;
    m_acceleration = x.m_acceleration;
    m_jerk = x.m_jerk;

    return *this;
}

bool ackermann_msgs::msg::AckermannDrive::operator ==(
        const AckermannDrive& x) const
{

    return (m_steering_angle == x.m_steering_angle && m_steering_angle_velocity == x.m_steering_angle_velocity && m_speed == x.m_speed && m_acceleration == x.m_acceleration && m_jerk == x.m_jerk);
}

bool ackermann_msgs::msg::AckermannDrive::operator !=(
        const AckermannDrive& x) const
{
    return !(*this == x);
}

size_t ackermann_msgs::msg::AckermannDrive::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);



    return current_alignment - initial_alignment;
}

size_t ackermann_msgs::msg::AckermannDrive::getCdrSerializedSize(
        const ackermann_msgs::msg::AckermannDrive& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);



    return current_alignment - initial_alignment;
}

void ackermann_msgs::msg::AckermannDrive::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{

    scdr << m_steering_angle;
    scdr << m_steering_angle_velocity;
    scdr << m_speed;
    scdr << m_acceleration;
    scdr << m_jerk;

}

void ackermann_msgs::msg::AckermannDrive::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{

    dcdr >> m_steering_angle;
    dcdr >> m_steering_angle_velocity;
    dcdr >> m_speed;
    dcdr >> m_acceleration;
    dcdr >> m_jerk;
}

/*!
 * @brief This function sets a value in member steering_angle
 * @param _steering_angle New value for member steering_angle
 */
void ackermann_msgs::msg::AckermannDrive::steering_angle(
        float _steering_angle)
{
    m_steering_angle = _steering_angle;
}

/*!
 * @brief This function returns the value of member steering_angle
 * @return Value of member steering_angle
 */
float ackermann_msgs::msg::AckermannDrive::steering_angle() const
{
    return m_steering_angle;
}

/*!
 * @brief This function returns a reference to member steering_angle
 * @return Reference to member steering_angle
 */
float& ackermann_msgs::msg::AckermannDrive::steering_angle()
{
    return m_steering_angle;
}

/*!
 * @brief This function sets a value in member steering_angle_velocity
 * @param _steering_angle_velocity New value for member steering_angle_velocity
 */
void ackermann_msgs::msg::AckermannDrive::steering_angle_velocity(
        float _steering_angle_velocity)
{
    m_steering_angle_velocity = _steering_angle_velocity;
}

/*!
 * @brief This function returns the value of member steering_angle_velocity
 * @return Value of member steering_angle_velocity
 */
float ackermann_msgs::msg::AckermannDrive::steering_angle_velocity() const
{
    return m_steering_angle_velocity;
}

/*!
 * @brief This function returns a reference to member steering_angle_velocity
 * @return Reference to member steering_angle_velocity
 */
float& ackermann_msgs::msg::AckermannDrive::steering_angle_velocity()
{
    return m_steering_angle_velocity;
}

/*!
 * @brief This function sets a value in member speed
 * @param _speed New value for member speed
 */
void ackermann_msgs::msg::AckermannDrive::speed(
        float _speed)
{
    m_speed = _speed;
}

/*!
 * @brief This function returns the value of member speed
 * @return Value of member speed
 */
float ackermann_msgs::msg::AckermannDrive::speed() const
{
    return m_speed;
}

/*!
 * @brief This function returns a reference to member speed
 * @return Reference to member speed
 */
float& ackermann_msgs::msg::AckermannDrive::speed()
{
    return m_speed;
}

/*!
 * @brief This function sets a value in member acceleration
 * @param _acceleration New value for member acceleration
 */
void ackermann_msgs::msg::AckermannDrive::acceleration(
        float _acceleration)
{
    m_acceleration = _acceleration;
}

/*!
 * @brief This function returns the value of member acceleration
 * @return Value of member acceleration
 */
float ackermann_msgs::msg::AckermannDrive::acceleration() const
{
    return m_acceleration;
}

/*!
 * @brief This function returns a reference to member acceleration
 * @return Reference to member acceleration
 */
float& ackermann_msgs::msg::AckermannDrive::acceleration()
{
    return m_acceleration;
}

/*!
 * @brief This function sets a value in member jerk
 * @param _jerk New value for member jerk
 */
void ackermann_msgs::msg::AckermannDrive::jerk(
        float _jerk)
{
    m_jerk = _jerk;
}

/*!
 * @brief This function returns the value of member jerk
 * @return Value of member jerk
 */
float ackermann_msgs::msg::AckermannDrive::jerk() const
{
    return m_jerk;
}

/*!
 * @brief This function returns a reference to member jerk
 * @return Reference to member jerk
 */
float& ackermann_msgs::msg::AckermannDrive::jerk()
{
    return m_jerk;
}


size_t ackermann_msgs::msg::AckermannDrive::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t current_align = current_alignment;



    return current_align;
}

bool ackermann_msgs::msg::AckermannDrive::isKeyDefined()
{
    return false;
}

void ackermann_msgs::msg::AckermannDrive::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
         
}


