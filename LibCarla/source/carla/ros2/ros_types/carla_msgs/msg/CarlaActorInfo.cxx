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
 * @file CarlaActorInfo.cpp
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

#include "CarlaActorInfo.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

carla_msgs::msg::CarlaActorInfo::CarlaActorInfo()
{
    // m_id com.eprosima.idl.parser.typecode.PrimitiveTypeCode@11e21d0e
    m_id = 0;
    // m_parent_id com.eprosima.idl.parser.typecode.PrimitiveTypeCode@1dd02175
    m_parent_id = 0;
    // m_type com.eprosima.idl.parser.typecode.StringTypeCode@31206beb
    m_type ="";
    // m_rosname com.eprosima.idl.parser.typecode.StringTypeCode@3e77a1ed
    m_rosname ="";
    // m_rolename com.eprosima.idl.parser.typecode.StringTypeCode@3ffcd140
    m_rolename ="";
    // m_object_type com.eprosima.idl.parser.typecode.StringTypeCode@23bb8443
    m_object_type ="";
    // m_base_type com.eprosima.idl.parser.typecode.StringTypeCode@1176dcec
    m_base_type ="";
    // m_topic_prefix com.eprosima.idl.parser.typecode.StringTypeCode@120d6fe6
    m_topic_prefix ="";

}

carla_msgs::msg::CarlaActorInfo::~CarlaActorInfo()
{







}

carla_msgs::msg::CarlaActorInfo::CarlaActorInfo(
        const CarlaActorInfo& x)
{
    m_id = x.m_id;
    m_parent_id = x.m_parent_id;
    m_type = x.m_type;
    m_rosname = x.m_rosname;
    m_rolename = x.m_rolename;
    m_object_type = x.m_object_type;
    m_base_type = x.m_base_type;
    m_topic_prefix = x.m_topic_prefix;
}

carla_msgs::msg::CarlaActorInfo::CarlaActorInfo(
        CarlaActorInfo&& x)
{
    m_id = x.m_id;
    m_parent_id = x.m_parent_id;
    m_type = std::move(x.m_type);
    m_rosname = std::move(x.m_rosname);
    m_rolename = std::move(x.m_rolename);
    m_object_type = std::move(x.m_object_type);
    m_base_type = std::move(x.m_base_type);
    m_topic_prefix = std::move(x.m_topic_prefix);
}

carla_msgs::msg::CarlaActorInfo& carla_msgs::msg::CarlaActorInfo::operator =(
        const CarlaActorInfo& x)
{

    m_id = x.m_id;
    m_parent_id = x.m_parent_id;
    m_type = x.m_type;
    m_rosname = x.m_rosname;
    m_rolename = x.m_rolename;
    m_object_type = x.m_object_type;
    m_base_type = x.m_base_type;
    m_topic_prefix = x.m_topic_prefix;

    return *this;
}

carla_msgs::msg::CarlaActorInfo& carla_msgs::msg::CarlaActorInfo::operator =(
        CarlaActorInfo&& x)
{

    m_id = x.m_id;
    m_parent_id = x.m_parent_id;
    m_type = std::move(x.m_type);
    m_rosname = std::move(x.m_rosname);
    m_rolename = std::move(x.m_rolename);
    m_object_type = std::move(x.m_object_type);
    m_base_type = std::move(x.m_base_type);
    m_topic_prefix = std::move(x.m_topic_prefix);

    return *this;
}

bool carla_msgs::msg::CarlaActorInfo::operator ==(
        const CarlaActorInfo& x) const
{

    return (m_id == x.m_id && m_parent_id == x.m_parent_id && m_type == x.m_type && m_rosname == x.m_rosname && m_rolename == x.m_rolename && m_object_type == x.m_object_type && m_base_type == x.m_base_type && m_topic_prefix == x.m_topic_prefix);
}

bool carla_msgs::msg::CarlaActorInfo::operator !=(
        const CarlaActorInfo& x) const
{
    return !(*this == x);
}

size_t carla_msgs::msg::CarlaActorInfo::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255 + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255 + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255 + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255 + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255 + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255 + 1;


    return current_alignment - initial_alignment;
}

size_t carla_msgs::msg::CarlaActorInfo::getCdrSerializedSize(
        const carla_msgs::msg::CarlaActorInfo& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.type().size() + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.rosname().size() + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.rolename().size() + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.object_type().size() + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.base_type().size() + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.topic_prefix().size() + 1;


    return current_alignment - initial_alignment;
}

void carla_msgs::msg::CarlaActorInfo::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{

    scdr << m_id;
    scdr << m_parent_id;
    scdr << m_type;
    scdr << m_rosname;
    scdr << m_rolename;
    scdr << m_object_type;
    scdr << m_base_type;
    scdr << m_topic_prefix;

}

void carla_msgs::msg::CarlaActorInfo::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{

    dcdr >> m_id;
    dcdr >> m_parent_id;
    dcdr >> m_type;
    dcdr >> m_rosname;
    dcdr >> m_rolename;
    dcdr >> m_object_type;
    dcdr >> m_base_type;
    dcdr >> m_topic_prefix;
}

/*!
 * @brief This function sets a value in member id
 * @param _id New value for member id
 */
void carla_msgs::msg::CarlaActorInfo::id(
        uint32_t _id)
{
    m_id = _id;
}

/*!
 * @brief This function returns the value of member id
 * @return Value of member id
 */
uint32_t carla_msgs::msg::CarlaActorInfo::id() const
{
    return m_id;
}

/*!
 * @brief This function returns a reference to member id
 * @return Reference to member id
 */
uint32_t& carla_msgs::msg::CarlaActorInfo::id()
{
    return m_id;
}

/*!
 * @brief This function sets a value in member parent_id
 * @param _parent_id New value for member parent_id
 */
void carla_msgs::msg::CarlaActorInfo::parent_id(
        uint32_t _parent_id)
{
    m_parent_id = _parent_id;
}

/*!
 * @brief This function returns the value of member parent_id
 * @return Value of member parent_id
 */
uint32_t carla_msgs::msg::CarlaActorInfo::parent_id() const
{
    return m_parent_id;
}

/*!
 * @brief This function returns a reference to member parent_id
 * @return Reference to member parent_id
 */
uint32_t& carla_msgs::msg::CarlaActorInfo::parent_id()
{
    return m_parent_id;
}

/*!
 * @brief This function copies the value in member type
 * @param _type New value to be copied in member type
 */
void carla_msgs::msg::CarlaActorInfo::type(
        const std::string& _type)
{
    m_type = _type;
}

/*!
 * @brief This function moves the value in member type
 * @param _type New value to be moved in member type
 */
void carla_msgs::msg::CarlaActorInfo::type(
        std::string&& _type)
{
    m_type = std::move(_type);
}

/*!
 * @brief This function returns a constant reference to member type
 * @return Constant reference to member type
 */
const std::string& carla_msgs::msg::CarlaActorInfo::type() const
{
    return m_type;
}

/*!
 * @brief This function returns a reference to member type
 * @return Reference to member type
 */
std::string& carla_msgs::msg::CarlaActorInfo::type()
{
    return m_type;
}
/*!
 * @brief This function copies the value in member rosname
 * @param _rosname New value to be copied in member rosname
 */
void carla_msgs::msg::CarlaActorInfo::rosname(
        const std::string& _rosname)
{
    m_rosname = _rosname;
}

/*!
 * @brief This function moves the value in member rosname
 * @param _rosname New value to be moved in member rosname
 */
void carla_msgs::msg::CarlaActorInfo::rosname(
        std::string&& _rosname)
{
    m_rosname = std::move(_rosname);
}

/*!
 * @brief This function returns a constant reference to member rosname
 * @return Constant reference to member rosname
 */
const std::string& carla_msgs::msg::CarlaActorInfo::rosname() const
{
    return m_rosname;
}

/*!
 * @brief This function returns a reference to member rosname
 * @return Reference to member rosname
 */
std::string& carla_msgs::msg::CarlaActorInfo::rosname()
{
    return m_rosname;
}
/*!
 * @brief This function copies the value in member rolename
 * @param _rolename New value to be copied in member rolename
 */
void carla_msgs::msg::CarlaActorInfo::rolename(
        const std::string& _rolename)
{
    m_rolename = _rolename;
}

/*!
 * @brief This function moves the value in member rolename
 * @param _rolename New value to be moved in member rolename
 */
void carla_msgs::msg::CarlaActorInfo::rolename(
        std::string&& _rolename)
{
    m_rolename = std::move(_rolename);
}

/*!
 * @brief This function returns a constant reference to member rolename
 * @return Constant reference to member rolename
 */
const std::string& carla_msgs::msg::CarlaActorInfo::rolename() const
{
    return m_rolename;
}

/*!
 * @brief This function returns a reference to member rolename
 * @return Reference to member rolename
 */
std::string& carla_msgs::msg::CarlaActorInfo::rolename()
{
    return m_rolename;
}
/*!
 * @brief This function copies the value in member object_type
 * @param _object_type New value to be copied in member object_type
 */
void carla_msgs::msg::CarlaActorInfo::object_type(
        const std::string& _object_type)
{
    m_object_type = _object_type;
}

/*!
 * @brief This function moves the value in member object_type
 * @param _object_type New value to be moved in member object_type
 */
void carla_msgs::msg::CarlaActorInfo::object_type(
        std::string&& _object_type)
{
    m_object_type = std::move(_object_type);
}

/*!
 * @brief This function returns a constant reference to member object_type
 * @return Constant reference to member object_type
 */
const std::string& carla_msgs::msg::CarlaActorInfo::object_type() const
{
    return m_object_type;
}

/*!
 * @brief This function returns a reference to member object_type
 * @return Reference to member object_type
 */
std::string& carla_msgs::msg::CarlaActorInfo::object_type()
{
    return m_object_type;
}
/*!
 * @brief This function copies the value in member base_type
 * @param _base_type New value to be copied in member base_type
 */
void carla_msgs::msg::CarlaActorInfo::base_type(
        const std::string& _base_type)
{
    m_base_type = _base_type;
}

/*!
 * @brief This function moves the value in member base_type
 * @param _base_type New value to be moved in member base_type
 */
void carla_msgs::msg::CarlaActorInfo::base_type(
        std::string&& _base_type)
{
    m_base_type = std::move(_base_type);
}

/*!
 * @brief This function returns a constant reference to member base_type
 * @return Constant reference to member base_type
 */
const std::string& carla_msgs::msg::CarlaActorInfo::base_type() const
{
    return m_base_type;
}

/*!
 * @brief This function returns a reference to member base_type
 * @return Reference to member base_type
 */
std::string& carla_msgs::msg::CarlaActorInfo::base_type()
{
    return m_base_type;
}
/*!
 * @brief This function copies the value in member topic_prefix
 * @param _topic_prefix New value to be copied in member topic_prefix
 */
void carla_msgs::msg::CarlaActorInfo::topic_prefix(
        const std::string& _topic_prefix)
{
    m_topic_prefix = _topic_prefix;
}

/*!
 * @brief This function moves the value in member topic_prefix
 * @param _topic_prefix New value to be moved in member topic_prefix
 */
void carla_msgs::msg::CarlaActorInfo::topic_prefix(
        std::string&& _topic_prefix)
{
    m_topic_prefix = std::move(_topic_prefix);
}

/*!
 * @brief This function returns a constant reference to member topic_prefix
 * @return Constant reference to member topic_prefix
 */
const std::string& carla_msgs::msg::CarlaActorInfo::topic_prefix() const
{
    return m_topic_prefix;
}

/*!
 * @brief This function returns a reference to member topic_prefix
 * @return Reference to member topic_prefix
 */
std::string& carla_msgs::msg::CarlaActorInfo::topic_prefix()
{
    return m_topic_prefix;
}

size_t carla_msgs::msg::CarlaActorInfo::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t current_align = current_alignment;



    return current_align;
}

bool carla_msgs::msg::CarlaActorInfo::isKeyDefined()
{
    return false;
}

void carla_msgs::msg::CarlaActorInfo::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
            
}


