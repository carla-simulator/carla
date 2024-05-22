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
 * @file TwistWithCovariance.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWISTWITHCOVARIANCE_H_
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWISTWITHCOVARIANCE_H_

#include "Twist.h"

#include <fastrtps/utils/fixed_size_string.hpp>

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
#if defined(TwistWithCovariance_SOURCE)
#define TwistWithCovariance_DllAPI __declspec(dllexport)
#else
#define TwistWithCovariance_DllAPI __declspec(dllimport)
#endif  // TwistWithCovariance_SOURCE
#else
#define TwistWithCovariance_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define TwistWithCovariance_DllAPI
#endif  // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
}  // namespace fastcdr
}  // namespace eprosima

namespace geometry_msgs {
namespace msg {
typedef std::array<double, 36> geometry_msgs__TwistWithCovariance__double_array_36;
/*!
 * @brief This class represents the structure TwistWithCovariance defined by the user in the IDL file.
 * @ingroup TWISTWITHCOVARIANCE
 */
class TwistWithCovariance {
public:
  /*!
   * @brief Default constructor.
   */
  eProsima_user_DllExport TwistWithCovariance();

  /*!
   * @brief Default destructor.
   */
  eProsima_user_DllExport ~TwistWithCovariance();

  /*!
   * @brief Copy constructor.
   * @param x Reference to the object geometry_msgs::msg::TwistWithCovariance that will be copied.
   */
  eProsima_user_DllExport TwistWithCovariance(const TwistWithCovariance& x);

  /*!
   * @brief Move constructor.
   * @param x Reference to the object geometry_msgs::msg::TwistWithCovariance that will be copied.
   */
  eProsima_user_DllExport TwistWithCovariance(TwistWithCovariance&& x) noexcept;

  /*!
   * @brief Copy assignment.
   * @param x Reference to the object geometry_msgs::msg::TwistWithCovariance that will be copied.
   */
  eProsima_user_DllExport TwistWithCovariance& operator=(const TwistWithCovariance& x);

  /*!
   * @brief Move assignment.
   * @param x Reference to the object geometry_msgs::msg::TwistWithCovariance that will be copied.
   */
  eProsima_user_DllExport TwistWithCovariance& operator=(TwistWithCovariance&& x) noexcept;

  /*!
   * @brief Comparison operator.
   * @param x geometry_msgs::msg::TwistWithCovariance object to compare.
   */
  eProsima_user_DllExport bool operator==(const TwistWithCovariance& x) const;

  /*!
   * @brief Comparison operator.
   * @param x geometry_msgs::msg::TwistWithCovariance object to compare.
   */
  eProsima_user_DllExport bool operator!=(const TwistWithCovariance& x) const;

  /*!
   * @brief This function copies the value in member twist
   * @param _twist New value to be copied in member twist
   */
  eProsima_user_DllExport void twist(const geometry_msgs::msg::Twist& _twist);

  /*!
   * @brief This function moves the value in member twist
   * @param _twist New value to be moved in member twist
   */
  eProsima_user_DllExport void twist(geometry_msgs::msg::Twist&& _twist);

  /*!
   * @brief This function returns a constant reference to member twist
   * @return Constant reference to member twist
   */
  eProsima_user_DllExport const geometry_msgs::msg::Twist& twist() const;

  /*!
   * @brief This function returns a reference to member twist
   * @return Reference to member twist
   */
  eProsima_user_DllExport geometry_msgs::msg::Twist& twist();
  /*!
   * @brief This function copies the value in member covariance
   * @param _covariance New value to be copied in member covariance
   */
  eProsima_user_DllExport void covariance(
      const geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36& _covariance);

  /*!
   * @brief This function moves the value in member covariance
   * @param _covariance New value to be moved in member covariance
   */
  eProsima_user_DllExport void covariance(
      geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36&& _covariance);

  /*!
   * @brief This function returns a constant reference to member covariance
   * @return Constant reference to member covariance
   */
  eProsima_user_DllExport const geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36& covariance()
      const;

  /*!
   * @brief This function returns a reference to member covariance
   * @return Reference to member covariance
   */
  eProsima_user_DllExport geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36& covariance();

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
  eProsima_user_DllExport static size_t getCdrSerializedSize(const geometry_msgs::msg::TwistWithCovariance& data,
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
  geometry_msgs::msg::Twist m_twist;
  geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36 m_covariance;
};
}  // namespace msg
}  // namespace geometry_msgs

#endif  // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWISTWITHCOVARIANCE_H_