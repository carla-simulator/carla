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
 * @file CarlaTrafficLightInfoList.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLATRAFFICLIGHTINFOLIST_H_
#define _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLATRAFFICLIGHTINFOLIST_H_

#include "CarlaTrafficLightInfo.h"

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
#if defined(CarlaTrafficLightInfoList_SOURCE)
#define CarlaTrafficLightInfoList_DllAPI __declspec( dllexport )
#else
#define CarlaTrafficLightInfoList_DllAPI __declspec( dllimport )
#endif // CarlaTrafficLightInfoList_SOURCE
#else
#define CarlaTrafficLightInfoList_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define CarlaTrafficLightInfoList_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima


namespace carla_msgs {
    namespace msg {
        /*!
         * @brief This class represents the structure CarlaTrafficLightInfoList defined by the user in the IDL file.
         * @ingroup CARLATRAFFICLIGHTINFOLIST
         */
        class CarlaTrafficLightInfoList
        {
        public:

            /*!
             * @brief Default constructor.
             */
            eProsima_user_DllExport CarlaTrafficLightInfoList();

            /*!
             * @brief Default destructor.
             */
            eProsima_user_DllExport ~CarlaTrafficLightInfoList();

            /*!
             * @brief Copy constructor.
             * @param x Reference to the object carla_msgs::msg::CarlaTrafficLightInfoList that will be copied.
             */
            eProsima_user_DllExport CarlaTrafficLightInfoList(
                    const CarlaTrafficLightInfoList& x);

            /*!
             * @brief Move constructor.
             * @param x Reference to the object carla_msgs::msg::CarlaTrafficLightInfoList that will be copied.
             */
            eProsima_user_DllExport CarlaTrafficLightInfoList(
                    CarlaTrafficLightInfoList&& x);

            /*!
             * @brief Copy assignment.
             * @param x Reference to the object carla_msgs::msg::CarlaTrafficLightInfoList that will be copied.
             */
            eProsima_user_DllExport CarlaTrafficLightInfoList& operator =(
                    const CarlaTrafficLightInfoList& x);

            /*!
             * @brief Move assignment.
             * @param x Reference to the object carla_msgs::msg::CarlaTrafficLightInfoList that will be copied.
             */
            eProsima_user_DllExport CarlaTrafficLightInfoList& operator =(
                    CarlaTrafficLightInfoList&& x);

            /*!
             * @brief Comparison operator.
             * @param x carla_msgs::msg::CarlaTrafficLightInfoList object to compare.
             */
            eProsima_user_DllExport bool operator ==(
                    const CarlaTrafficLightInfoList& x) const;

            /*!
             * @brief Comparison operator.
             * @param x carla_msgs::msg::CarlaTrafficLightInfoList object to compare.
             */
            eProsima_user_DllExport bool operator !=(
                    const CarlaTrafficLightInfoList& x) const;

            /*!
             * @brief This function copies the value in member traffic_lights
             * @param _traffic_lights New value to be copied in member traffic_lights
             */
            eProsima_user_DllExport void traffic_lights(
                    const std::vector<carla_msgs::msg::CarlaTrafficLightInfo>& _traffic_lights);

            /*!
             * @brief This function moves the value in member traffic_lights
             * @param _traffic_lights New value to be moved in member traffic_lights
             */
            eProsima_user_DllExport void traffic_lights(
                    std::vector<carla_msgs::msg::CarlaTrafficLightInfo>&& _traffic_lights);

            /*!
             * @brief This function returns a constant reference to member traffic_lights
             * @return Constant reference to member traffic_lights
             */
            eProsima_user_DllExport const std::vector<carla_msgs::msg::CarlaTrafficLightInfo>& traffic_lights() const;

            /*!
             * @brief This function returns a reference to member traffic_lights
             * @return Reference to member traffic_lights
             */
            eProsima_user_DllExport std::vector<carla_msgs::msg::CarlaTrafficLightInfo>& traffic_lights();

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
                    const carla_msgs::msg::CarlaTrafficLightInfoList& data,
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

            std::vector<carla_msgs::msg::CarlaTrafficLightInfo> m_traffic_lights;
        };
    } // namespace msg
} // namespace carla_msgs

#endif // _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLATRAFFICLIGHTINFOLIST_H_