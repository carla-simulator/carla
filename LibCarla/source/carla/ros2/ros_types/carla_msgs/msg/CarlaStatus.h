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
 * @file CarlaStatus.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLASTATUS_H_
#define _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLASTATUS_H_


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
#if defined(CarlaStatus_SOURCE)
#define CarlaStatus_DllAPI __declspec( dllexport )
#else
#define CarlaStatus_DllAPI __declspec( dllimport )
#endif // CarlaStatus_SOURCE
#else
#define CarlaStatus_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define CarlaStatus_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima


namespace carla_msgs {
    namespace msg {
        /*!
         * @brief This class represents the structure CarlaStatus defined by the user in the IDL file.
         * @ingroup CARLASTATUS
         */
        class CarlaStatus
        {
        public:

            /*!
             * @brief Default constructor.
             */
            eProsima_user_DllExport CarlaStatus();

            /*!
             * @brief Default destructor.
             */
            eProsima_user_DllExport ~CarlaStatus();

            /*!
             * @brief Copy constructor.
             * @param x Reference to the object carla_msgs::msg::CarlaStatus that will be copied.
             */
            eProsima_user_DllExport CarlaStatus(
                    const CarlaStatus& x);

            /*!
             * @brief Move constructor.
             * @param x Reference to the object carla_msgs::msg::CarlaStatus that will be copied.
             */
            eProsima_user_DllExport CarlaStatus(
                    CarlaStatus&& x);

            /*!
             * @brief Copy assignment.
             * @param x Reference to the object carla_msgs::msg::CarlaStatus that will be copied.
             */
            eProsima_user_DllExport CarlaStatus& operator =(
                    const CarlaStatus& x);

            /*!
             * @brief Move assignment.
             * @param x Reference to the object carla_msgs::msg::CarlaStatus that will be copied.
             */
            eProsima_user_DllExport CarlaStatus& operator =(
                    CarlaStatus&& x);

            /*!
             * @brief Comparison operator.
             * @param x carla_msgs::msg::CarlaStatus object to compare.
             */
            eProsima_user_DllExport bool operator ==(
                    const CarlaStatus& x) const;

            /*!
             * @brief Comparison operator.
             * @param x carla_msgs::msg::CarlaStatus object to compare.
             */
            eProsima_user_DllExport bool operator !=(
                    const CarlaStatus& x) const;

            /*!
             * @brief This function sets a value in member frame
             * @param _frame New value for member frame
             */
            eProsima_user_DllExport void frame(
                    uint64_t _frame);

            /*!
             * @brief This function returns the value of member frame
             * @return Value of member frame
             */
            eProsima_user_DllExport uint64_t frame() const;

            /*!
             * @brief This function returns a reference to member frame
             * @return Reference to member frame
             */
            eProsima_user_DllExport uint64_t& frame();

            /*!
             * @brief This function sets a value in member fixed_delta_seconds
             * @param _fixed_delta_seconds New value for member fixed_delta_seconds
             */
            eProsima_user_DllExport void fixed_delta_seconds(
                    float _fixed_delta_seconds);

            /*!
             * @brief This function returns the value of member fixed_delta_seconds
             * @return Value of member fixed_delta_seconds
             */
            eProsima_user_DllExport float fixed_delta_seconds() const;

            /*!
             * @brief This function returns a reference to member fixed_delta_seconds
             * @return Reference to member fixed_delta_seconds
             */
            eProsima_user_DllExport float& fixed_delta_seconds();

            /*!
             * @brief This function sets a value in member synchronous_mode
             * @param _synchronous_mode New value for member synchronous_mode
             */
            eProsima_user_DllExport void synchronous_mode(
                    bool _synchronous_mode);

            /*!
             * @brief This function returns the value of member synchronous_mode
             * @return Value of member synchronous_mode
             */
            eProsima_user_DllExport bool synchronous_mode() const;

            /*!
             * @brief This function returns a reference to member synchronous_mode
             * @return Reference to member synchronous_mode
             */
            eProsima_user_DllExport bool& synchronous_mode();

            /*!
             * @brief This function sets a value in member synchronous_mode_running
             * @param _synchronous_mode_running New value for member synchronous_mode_running
             */
            eProsima_user_DllExport void synchronous_mode_running(
                    bool _synchronous_mode_running);

            /*!
             * @brief This function returns the value of member synchronous_mode_running
             * @return Value of member synchronous_mode_running
             */
            eProsima_user_DllExport bool synchronous_mode_running() const;

            /*!
             * @brief This function returns a reference to member synchronous_mode_running
             * @return Reference to member synchronous_mode_running
             */
            eProsima_user_DllExport bool& synchronous_mode_running();


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
                    const carla_msgs::msg::CarlaStatus& data,
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

            uint64_t m_frame;
            float m_fixed_delta_seconds;
            bool m_synchronous_mode;
            bool m_synchronous_mode_running;
        };
    } // namespace msg
} // namespace carla_msgs

#endif // _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLASTATUS_H_