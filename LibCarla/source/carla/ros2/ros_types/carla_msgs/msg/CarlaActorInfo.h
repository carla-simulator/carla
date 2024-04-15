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
 * @file CarlaActorInfo.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAACTORINFO_H_
#define _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAACTORINFO_H_

#include <stdint.h>
#include <array>
#include <bitset>
#include <map>
#include <string>
#include <vector>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec(dllexport)
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(CarlaActorInfo_SOURCE)
#define CarlaActorInfo_DllAPI __declspec(dllexport)
#else
#define CarlaActorInfo_DllAPI __declspec(dllimport)
#endif  // CarlaActorInfo_SOURCE
#else
#define CarlaActorInfo_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define CarlaActorInfo_DllAPI
#endif  // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
}  // namespace fastcdr
}  // namespace eprosima

namespace carla_msgs {
namespace msg {
/*!
 * @brief This class represents the structure CarlaActorInfo defined by the user in the IDL file.
 * @ingroup CARLAACTORINFO
 */
class CarlaActorInfo {
public:
  /*!
   * @brief Default constructor.
   */
  eProsima_user_DllExport CarlaActorInfo();

  /*!
   * @brief Default destructor.
   */
  eProsima_user_DllExport ~CarlaActorInfo();

  /*!
   * @brief Copy constructor.
   * @param x Reference to the object carla_msgs::msg::CarlaActorInfo that will be copied.
   */
  eProsima_user_DllExport CarlaActorInfo(const CarlaActorInfo& x);

  /*!
   * @brief Move constructor.
   * @param x Reference to the object carla_msgs::msg::CarlaActorInfo that will be copied.
   */
  eProsima_user_DllExport CarlaActorInfo(CarlaActorInfo&& x);

  /*!
   * @brief Copy assignment.
   * @param x Reference to the object carla_msgs::msg::CarlaActorInfo that will be copied.
   */
  eProsima_user_DllExport CarlaActorInfo& operator=(const CarlaActorInfo& x);

  /*!
   * @brief Move assignment.
   * @param x Reference to the object carla_msgs::msg::CarlaActorInfo that will be copied.
   */
  eProsima_user_DllExport CarlaActorInfo& operator=(CarlaActorInfo&& x);

  /*!
   * @brief Comparison operator.
   * @param x carla_msgs::msg::CarlaActorInfo object to compare.
   */
  eProsima_user_DllExport bool operator==(const CarlaActorInfo& x) const;

  /*!
   * @brief Comparison operator.
   * @param x carla_msgs::msg::CarlaActorInfo object to compare.
   */
  eProsima_user_DllExport bool operator!=(const CarlaActorInfo& x) const;

  /*!
   * @brief This function sets a value in member id
   * @param _id New value for member id
   */
  eProsima_user_DllExport void id(uint32_t _id);

  /*!
   * @brief This function returns the value of member id
   * @return Value of member id
   */
  eProsima_user_DllExport uint32_t id() const;

  /*!
   * @brief This function returns a reference to member id
   * @return Reference to member id
   */
  eProsima_user_DllExport uint32_t& id();

  /*!
   * @brief This function sets a value in member parent_id
   * @param _parent_id New value for member parent_id
   */
  eProsima_user_DllExport void parent_id(uint32_t _parent_id);

  /*!
   * @brief This function returns the value of member parent_id
   * @return Value of member parent_id
   */
  eProsima_user_DllExport uint32_t parent_id() const;

  /*!
   * @brief This function returns a reference to member parent_id
   * @return Reference to member parent_id
   */
  eProsima_user_DllExport uint32_t& parent_id();

  /*!
   * @brief This function copies the value in member type
   * @param _type New value to be copied in member type
   */
  eProsima_user_DllExport void type(const std::string& _type);

  /*!
   * @brief This function moves the value in member type
   * @param _type New value to be moved in member type
   */
  eProsima_user_DllExport void type(std::string&& _type);

  /*!
   * @brief This function returns a constant reference to member type
   * @return Constant reference to member type
   */
  eProsima_user_DllExport const std::string& type() const;

  /*!
   * @brief This function returns a reference to member type
   * @return Reference to member type
   */
  eProsima_user_DllExport std::string& type();
  /*!
   * @brief This function copies the value in member rosname
   * @param _rosname New value to be copied in member rosname
   */
  eProsima_user_DllExport void rosname(const std::string& _rosname);

  /*!
   * @brief This function moves the value in member rosname
   * @param _rosname New value to be moved in member rosname
   */
  eProsima_user_DllExport void rosname(std::string&& _rosname);

  /*!
   * @brief This function returns a constant reference to member rosname
   * @return Constant reference to member rosname
   */
  eProsima_user_DllExport const std::string& rosname() const;

  /*!
   * @brief This function returns a reference to member rosname
   * @return Reference to member rosname
   */
  eProsima_user_DllExport std::string& rosname();
  /*!
   * @brief This function copies the value in member rolename
   * @param _rolename New value to be copied in member rolename
   */
  eProsima_user_DllExport void rolename(const std::string& _rolename);

  /*!
   * @brief This function moves the value in member rolename
   * @param _rolename New value to be moved in member rolename
   */
  eProsima_user_DllExport void rolename(std::string&& _rolename);

  /*!
   * @brief This function returns a constant reference to member rolename
   * @return Constant reference to member rolename
   */
  eProsima_user_DllExport const std::string& rolename() const;

  /*!
   * @brief This function returns a reference to member rolename
   * @return Reference to member rolename
   */
  eProsima_user_DllExport std::string& rolename();
  /*!
   * @brief This function copies the value in member object_type
   * @param _object_type New value to be copied in member object_type
   */
  eProsima_user_DllExport void object_type(const std::string& _object_type);

  /*!
   * @brief This function moves the value in member object_type
   * @param _object_type New value to be moved in member object_type
   */
  eProsima_user_DllExport void object_type(std::string&& _object_type);

  /*!
   * @brief This function returns a constant reference to member object_type
   * @return Constant reference to member object_type
   */
  eProsima_user_DllExport const std::string& object_type() const;

  /*!
   * @brief This function returns a reference to member object_type
   * @return Reference to member object_type
   */
  eProsima_user_DllExport std::string& object_type();
  /*!
   * @brief This function copies the value in member base_type
   * @param _base_type New value to be copied in member base_type
   */
  eProsima_user_DllExport void base_type(const std::string& _base_type);

  /*!
   * @brief This function moves the value in member base_type
   * @param _base_type New value to be moved in member base_type
   */
  eProsima_user_DllExport void base_type(std::string&& _base_type);

  /*!
   * @brief This function returns a constant reference to member base_type
   * @return Constant reference to member base_type
   */
  eProsima_user_DllExport const std::string& base_type() const;

  /*!
   * @brief This function returns a reference to member base_type
   * @return Reference to member base_type
   */
  eProsima_user_DllExport std::string& base_type();
  /*!
   * @brief This function copies the value in member topic_prefix
   * @param _topic_prefix New value to be copied in member topic_prefix
   */
  eProsima_user_DllExport void topic_prefix(const std::string& _topic_prefix);

  /*!
   * @brief This function moves the value in member topic_prefix
   * @param _topic_prefix New value to be moved in member topic_prefix
   */
  eProsima_user_DllExport void topic_prefix(std::string&& _topic_prefix);

  /*!
   * @brief This function returns a constant reference to member topic_prefix
   * @return Constant reference to member topic_prefix
   */
  eProsima_user_DllExport const std::string& topic_prefix() const;

  /*!
   * @brief This function returns a reference to member topic_prefix
   * @return Reference to member topic_prefix
   */
  eProsima_user_DllExport std::string& topic_prefix();

  /*!
   * @brief This function returns the maximum serialized size of an object
   * depending on the buffer alignment.
   * @param current_alignment Buffer alignment.
   * @return Maximum serialized size.
   */
  eProsima_user_DllExport static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);

  /*!
   * @brief This function returns the serialized size of a data depending on the buffer alignment.
   * @param data Data which is calculated its serialized size.
   * @param current_alignment Buffer alignment.
   * @return Serialized size.
   */
  eProsima_user_DllExport static size_t getCdrSerializedSize(const carla_msgs::msg::CarlaActorInfo& data,
                                                             size_t current_alignment = 0);

  /*!
   * @brief This function serializes an object using CDR serialization.
   * @param cdr CDR serialization object.
   */
  eProsima_user_DllExport void serialize(eprosima::fastcdr::Cdr& cdr) const;

  /*!
   * @brief This function deserializes an object using CDR serialization.
   * @param cdr CDR serialization object.
   */
  eProsima_user_DllExport void deserialize(eprosima::fastcdr::Cdr& cdr);

  /*!
   * @brief This function returns the maximum serialized size of the Key of an object
   * depending on the buffer alignment.
   * @param current_alignment Buffer alignment.
   * @return Maximum serialized size.
   */
  eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);

  /*!
   * @brief This function tells you if the Key has been defined for this type
   */
  eProsima_user_DllExport static bool isKeyDefined();

  /*!
   * @brief This function serializes the key members of an object using CDR serialization.
   * @param cdr CDR serialization object.
   */
  eProsima_user_DllExport void serializeKey(eprosima::fastcdr::Cdr& cdr) const;

private:
  uint32_t m_id;
  uint32_t m_parent_id;
  std::string m_type;
  std::string m_rosname;
  std::string m_rolename;
  std::string m_object_type;
  std::string m_base_type;
  std::string m_topic_prefix;
};
}  // namespace msg
}  // namespace carla_msgs

#endif  // _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAACTORINFO_H_