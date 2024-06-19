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
 * @file CarlaV2XCustom.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAV2XCUSTOM_H_
#define _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAV2XCUSTOM_H_

#include "etsi_its_cam_msgs/msg/ItsPduHeader.h"

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
#if defined(CarlaV2XCustom_SOURCE)
#define CarlaV2XCustom_DllAPI __declspec( dllexport )
#else
#define CarlaV2XCustom_DllAPI __declspec( dllimport )
#endif // CarlaV2XCustom_SOURCE
#else
#define CarlaV2XCustom_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define CarlaV2XCustom_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima


namespace carla_msgs {
    namespace msg {
        /*!
         * @brief This class represents the structure CarlaV2XCustom defined by the user in the IDL file.
         * @ingroup CARLAV2XCUSTOM
         */
        class CarlaV2XCustom
        {
        public:

            /*!
             * @brief Default constructor.
             */
            eProsima_user_DllExport CarlaV2XCustom();

            /*!
             * @brief Default destructor.
             */
            eProsima_user_DllExport ~CarlaV2XCustom();

            /*!
             * @brief Copy constructor.
             * @param x Reference to the object carla_msgs::msg::CarlaV2XCustom that will be copied.
             */
            eProsima_user_DllExport CarlaV2XCustom(
                    const CarlaV2XCustom& x);

            /*!
             * @brief Move constructor.
             * @param x Reference to the object carla_msgs::msg::CarlaV2XCustom that will be copied.
             */
            eProsima_user_DllExport CarlaV2XCustom(
                    CarlaV2XCustom&& x);

            /*!
             * @brief Copy assignment.
             * @param x Reference to the object carla_msgs::msg::CarlaV2XCustom that will be copied.
             */
            eProsima_user_DllExport CarlaV2XCustom& operator =(
                    const CarlaV2XCustom& x);

            /*!
             * @brief Move assignment.
             * @param x Reference to the object carla_msgs::msg::CarlaV2XCustom that will be copied.
             */
            eProsima_user_DllExport CarlaV2XCustom& operator =(
                    CarlaV2XCustom&& x);

            /*!
             * @brief Comparison operator.
             * @param x carla_msgs::msg::CarlaV2XCustom object to compare.
             */
            eProsima_user_DllExport bool operator ==(
                    const CarlaV2XCustom& x) const;

            /*!
             * @brief Comparison operator.
             * @param x carla_msgs::msg::CarlaV2XCustom object to compare.
             */
            eProsima_user_DllExport bool operator !=(
                    const CarlaV2XCustom& x) const;

            /*!
             * @brief This function copies the value in member header
             * @param _header New value to be copied in member header
             */
            eProsima_user_DllExport void header(
                    const etsi_its_cam_msgs::msg::ItsPduHeader& _header);

            /*!
             * @brief This function moves the value in member header
             * @param _header New value to be moved in member header
             */
            eProsima_user_DllExport void header(
                    etsi_its_cam_msgs::msg::ItsPduHeader&& _header);

            /*!
             * @brief This function returns a constant reference to member header
             * @return Constant reference to member header
             */
            eProsima_user_DllExport const etsi_its_cam_msgs::msg::ItsPduHeader& header() const;

            /*!
             * @brief This function returns a reference to member header
             * @return Reference to member header
             */
            eProsima_user_DllExport etsi_its_cam_msgs::msg::ItsPduHeader& header();
            /*!
             * @brief This function copies the value in member message
             * @param _message New value to be copied in member message
             */
            eProsima_user_DllExport void message(
                    const std::string& _message);

            /*!
             * @brief This function moves the value in member message
             * @param _message New value to be moved in member message
             */
            eProsima_user_DllExport void message(
                    std::string&& _message);

            /*!
             * @brief This function returns a constant reference to member message
             * @return Constant reference to member message
             */
            eProsima_user_DllExport const std::string& message() const;

            /*!
             * @brief This function returns a reference to member message
             * @return Reference to member message
             */
            eProsima_user_DllExport std::string& message();

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
                    const carla_msgs::msg::CarlaV2XCustom& data,
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

            etsi_its_cam_msgs::msg::ItsPduHeader m_header;
            std::string m_message;
        };
    } // namespace msg
} // namespace carla_msgs

#endif // _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAV2XCUSTOM_H_