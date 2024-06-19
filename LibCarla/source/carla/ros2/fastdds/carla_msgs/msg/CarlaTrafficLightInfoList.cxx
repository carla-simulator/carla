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
 * @file CarlaTrafficLightInfoList.cpp
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

#include "CarlaTrafficLightInfoList.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

carla_msgs::msg::CarlaTrafficLightInfoList::CarlaTrafficLightInfoList()
{
    // m_traffic_lights com.eprosima.idl.parser.typecode.SequenceTypeCode@485966cc


}

carla_msgs::msg::CarlaTrafficLightInfoList::~CarlaTrafficLightInfoList()
{
}

carla_msgs::msg::CarlaTrafficLightInfoList::CarlaTrafficLightInfoList(
        const CarlaTrafficLightInfoList& x)
{
    m_traffic_lights = x.m_traffic_lights;
}

carla_msgs::msg::CarlaTrafficLightInfoList::CarlaTrafficLightInfoList(
        CarlaTrafficLightInfoList&& x)
{
    m_traffic_lights = std::move(x.m_traffic_lights);
}

carla_msgs::msg::CarlaTrafficLightInfoList& carla_msgs::msg::CarlaTrafficLightInfoList::operator =(
        const CarlaTrafficLightInfoList& x)
{

    m_traffic_lights = x.m_traffic_lights;

    return *this;
}

carla_msgs::msg::CarlaTrafficLightInfoList& carla_msgs::msg::CarlaTrafficLightInfoList::operator =(
        CarlaTrafficLightInfoList&& x)
{

    m_traffic_lights = std::move(x.m_traffic_lights);

    return *this;
}

bool carla_msgs::msg::CarlaTrafficLightInfoList::operator ==(
        const CarlaTrafficLightInfoList& x) const
{

    return (m_traffic_lights == x.m_traffic_lights);
}

bool carla_msgs::msg::CarlaTrafficLightInfoList::operator !=(
        const CarlaTrafficLightInfoList& x) const
{
    return !(*this == x);
}

size_t carla_msgs::msg::CarlaTrafficLightInfoList::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += carla_msgs::msg::CarlaTrafficLightInfo::getMaxCdrSerializedSize(current_alignment);}

    return current_alignment - initial_alignment;
}

size_t carla_msgs::msg::CarlaTrafficLightInfoList::getCdrSerializedSize(
        const carla_msgs::msg::CarlaTrafficLightInfoList& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for(size_t a = 0; a < data.traffic_lights().size(); ++a)
    {
        current_alignment += carla_msgs::msg::CarlaTrafficLightInfo::getCdrSerializedSize(data.traffic_lights().at(a), current_alignment);}

    return current_alignment - initial_alignment;
}

void carla_msgs::msg::CarlaTrafficLightInfoList::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{

    scdr << m_traffic_lights;
}

void carla_msgs::msg::CarlaTrafficLightInfoList::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{

    dcdr >> m_traffic_lights;}

/*!
 * @brief This function copies the value in member traffic_lights
 * @param _traffic_lights New value to be copied in member traffic_lights
 */
void carla_msgs::msg::CarlaTrafficLightInfoList::traffic_lights(
        const std::vector<carla_msgs::msg::CarlaTrafficLightInfo>& _traffic_lights)
{
    m_traffic_lights = _traffic_lights;
}

/*!
 * @brief This function moves the value in member traffic_lights
 * @param _traffic_lights New value to be moved in member traffic_lights
 */
void carla_msgs::msg::CarlaTrafficLightInfoList::traffic_lights(
        std::vector<carla_msgs::msg::CarlaTrafficLightInfo>&& _traffic_lights)
{
    m_traffic_lights = std::move(_traffic_lights);
}

/*!
 * @brief This function returns a constant reference to member traffic_lights
 * @return Constant reference to member traffic_lights
 */
const std::vector<carla_msgs::msg::CarlaTrafficLightInfo>& carla_msgs::msg::CarlaTrafficLightInfoList::traffic_lights() const
{
    return m_traffic_lights;
}

/*!
 * @brief This function returns a reference to member traffic_lights
 * @return Reference to member traffic_lights
 */
std::vector<carla_msgs::msg::CarlaTrafficLightInfo>& carla_msgs::msg::CarlaTrafficLightInfoList::traffic_lights()
{
    return m_traffic_lights;
}

size_t carla_msgs::msg::CarlaTrafficLightInfoList::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t current_align = current_alignment;



    return current_align;
}

bool carla_msgs::msg::CarlaTrafficLightInfoList::isKeyDefined()
{
    return false;
}

void carla_msgs::msg::CarlaTrafficLightInfoList::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
     
}

