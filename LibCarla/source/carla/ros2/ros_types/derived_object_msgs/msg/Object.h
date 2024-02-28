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
 * @file Object.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_DERIVED_OBJECT_MSGS_MSG_OBJECT_H_
#define _FAST_DDS_GENERATED_DERIVED_OBJECT_MSGS_MSG_OBJECT_H_

#include "geometry_msgs/msg/Polygon.h"
#include "geometry_msgs/msg/Twist.h"
#include "std_msgs/msg/Header.h"
#include "shape_msgs/msg/SolidPrimitive.h"
#include "geometry_msgs/msg/Accel.h"
#include "geometry_msgs/msg/Pose.h"

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
#if defined(Object_SOURCE)
#define Object_DllAPI __declspec( dllexport )
#else
#define Object_DllAPI __declspec( dllimport )
#endif // Object_SOURCE
#else
#define Object_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define Object_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima


namespace derived_object_msgs {
    namespace msg {
        namespace Object_Constants {
            const uint8_t OBJECT_DETECTED = 0;
            const uint8_t OBJECT_TRACKED = 1;
            const uint8_t CLASSIFICATION_UNKNOWN = 0;
            const uint8_t CLASSIFICATION_UNKNOWN_SMALL = 1;
            const uint8_t CLASSIFICATION_UNKNOWN_MEDIUM = 2;
            const uint8_t CLASSIFICATION_UNKNOWN_BIG = 3;
            const uint8_t CLASSIFICATION_PEDESTRIAN = 4;
            const uint8_t CLASSIFICATION_BIKE = 5;
            const uint8_t CLASSIFICATION_CAR = 6;
            const uint8_t CLASSIFICATION_TRUCK = 7;
            const uint8_t CLASSIFICATION_MOTORCYCLE = 8;
            const uint8_t CLASSIFICATION_OTHER_VEHICLE = 9;
            const uint8_t CLASSIFICATION_BARRIER = 10;
            const uint8_t CLASSIFICATION_SIGN = 11;
        } // namespace Object_Constants
        /*!
         * @brief This class represents the structure Object defined by the user in the IDL file.
         * @ingroup OBJECT
         */
        class Object
        {
        public:

            /*!
             * @brief Default constructor.
             */
            eProsima_user_DllExport Object();

            /*!
             * @brief Default destructor.
             */
            eProsima_user_DllExport ~Object();

            /*!
             * @brief Copy constructor.
             * @param x Reference to the object derived_object_msgs::msg::Object that will be copied.
             */
            eProsima_user_DllExport Object(
                    const Object& x);

            /*!
             * @brief Move constructor.
             * @param x Reference to the object derived_object_msgs::msg::Object that will be copied.
             */
            eProsima_user_DllExport Object(
                    Object&& x);

            /*!
             * @brief Copy assignment.
             * @param x Reference to the object derived_object_msgs::msg::Object that will be copied.
             */
            eProsima_user_DllExport Object& operator =(
                    const Object& x);

            /*!
             * @brief Move assignment.
             * @param x Reference to the object derived_object_msgs::msg::Object that will be copied.
             */
            eProsima_user_DllExport Object& operator =(
                    Object&& x);

            /*!
             * @brief Comparison operator.
             * @param x derived_object_msgs::msg::Object object to compare.
             */
            eProsima_user_DllExport bool operator ==(
                    const Object& x) const;

            /*!
             * @brief Comparison operator.
             * @param x derived_object_msgs::msg::Object object to compare.
             */
            eProsima_user_DllExport bool operator !=(
                    const Object& x) const;

            /*!
             * @brief This function copies the value in member header
             * @param _header New value to be copied in member header
             */
            eProsima_user_DllExport void header(
                    const std_msgs::msg::Header& _header);

            /*!
             * @brief This function moves the value in member header
             * @param _header New value to be moved in member header
             */
            eProsima_user_DllExport void header(
                    std_msgs::msg::Header&& _header);

            /*!
             * @brief This function returns a constant reference to member header
             * @return Constant reference to member header
             */
            eProsima_user_DllExport const std_msgs::msg::Header& header() const;

            /*!
             * @brief This function returns a reference to member header
             * @return Reference to member header
             */
            eProsima_user_DllExport std_msgs::msg::Header& header();
            /*!
             * @brief This function sets a value in member id
             * @param _id New value for member id
             */
            eProsima_user_DllExport void id(
                    uint32_t _id);

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
             * @brief This function sets a value in member detection_level
             * @param _detection_level New value for member detection_level
             */
            eProsima_user_DllExport void detection_level(
                    uint8_t _detection_level);

            /*!
             * @brief This function returns the value of member detection_level
             * @return Value of member detection_level
             */
            eProsima_user_DllExport uint8_t detection_level() const;

            /*!
             * @brief This function returns a reference to member detection_level
             * @return Reference to member detection_level
             */
            eProsima_user_DllExport uint8_t& detection_level();

            /*!
             * @brief This function sets a value in member object_classified
             * @param _object_classified New value for member object_classified
             */
            eProsima_user_DllExport void object_classified(
                    bool _object_classified);

            /*!
             * @brief This function returns the value of member object_classified
             * @return Value of member object_classified
             */
            eProsima_user_DllExport bool object_classified() const;

            /*!
             * @brief This function returns a reference to member object_classified
             * @return Reference to member object_classified
             */
            eProsima_user_DllExport bool& object_classified();

            /*!
             * @brief This function copies the value in member pose
             * @param _pose New value to be copied in member pose
             */
            eProsima_user_DllExport void pose(
                    const geometry_msgs::msg::Pose& _pose);

            /*!
             * @brief This function moves the value in member pose
             * @param _pose New value to be moved in member pose
             */
            eProsima_user_DllExport void pose(
                    geometry_msgs::msg::Pose&& _pose);

            /*!
             * @brief This function returns a constant reference to member pose
             * @return Constant reference to member pose
             */
            eProsima_user_DllExport const geometry_msgs::msg::Pose& pose() const;

            /*!
             * @brief This function returns a reference to member pose
             * @return Reference to member pose
             */
            eProsima_user_DllExport geometry_msgs::msg::Pose& pose();
            /*!
             * @brief This function copies the value in member twist
             * @param _twist New value to be copied in member twist
             */
            eProsima_user_DllExport void twist(
                    const geometry_msgs::msg::Twist& _twist);

            /*!
             * @brief This function moves the value in member twist
             * @param _twist New value to be moved in member twist
             */
            eProsima_user_DllExport void twist(
                    geometry_msgs::msg::Twist&& _twist);

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
             * @brief This function copies the value in member accel
             * @param _accel New value to be copied in member accel
             */
            eProsima_user_DllExport void accel(
                    const geometry_msgs::msg::Accel& _accel);

            /*!
             * @brief This function moves the value in member accel
             * @param _accel New value to be moved in member accel
             */
            eProsima_user_DllExport void accel(
                    geometry_msgs::msg::Accel&& _accel);

            /*!
             * @brief This function returns a constant reference to member accel
             * @return Constant reference to member accel
             */
            eProsima_user_DllExport const geometry_msgs::msg::Accel& accel() const;

            /*!
             * @brief This function returns a reference to member accel
             * @return Reference to member accel
             */
            eProsima_user_DllExport geometry_msgs::msg::Accel& accel();
            /*!
             * @brief This function copies the value in member polygon
             * @param _polygon New value to be copied in member polygon
             */
            eProsima_user_DllExport void polygon(
                    const geometry_msgs::msg::Polygon& _polygon);

            /*!
             * @brief This function moves the value in member polygon
             * @param _polygon New value to be moved in member polygon
             */
            eProsima_user_DllExport void polygon(
                    geometry_msgs::msg::Polygon&& _polygon);

            /*!
             * @brief This function returns a constant reference to member polygon
             * @return Constant reference to member polygon
             */
            eProsima_user_DllExport const geometry_msgs::msg::Polygon& polygon() const;

            /*!
             * @brief This function returns a reference to member polygon
             * @return Reference to member polygon
             */
            eProsima_user_DllExport geometry_msgs::msg::Polygon& polygon();
            /*!
             * @brief This function copies the value in member shape
             * @param _shape New value to be copied in member shape
             */
            eProsima_user_DllExport void shape(
                    const shape_msgs::msg::SolidPrimitive& _shape);

            /*!
             * @brief This function moves the value in member shape
             * @param _shape New value to be moved in member shape
             */
            eProsima_user_DllExport void shape(
                    shape_msgs::msg::SolidPrimitive&& _shape);

            /*!
             * @brief This function returns a constant reference to member shape
             * @return Constant reference to member shape
             */
            eProsima_user_DllExport const shape_msgs::msg::SolidPrimitive& shape() const;

            /*!
             * @brief This function returns a reference to member shape
             * @return Reference to member shape
             */
            eProsima_user_DllExport shape_msgs::msg::SolidPrimitive& shape();
            /*!
             * @brief This function sets a value in member classification
             * @param _classification New value for member classification
             */
            eProsima_user_DllExport void classification(
                    uint8_t _classification);

            /*!
             * @brief This function returns the value of member classification
             * @return Value of member classification
             */
            eProsima_user_DllExport uint8_t classification() const;

            /*!
             * @brief This function returns a reference to member classification
             * @return Reference to member classification
             */
            eProsima_user_DllExport uint8_t& classification();

            /*!
             * @brief This function sets a value in member classification_certainty
             * @param _classification_certainty New value for member classification_certainty
             */
            eProsima_user_DllExport void classification_certainty(
                    uint8_t _classification_certainty);

            /*!
             * @brief This function returns the value of member classification_certainty
             * @return Value of member classification_certainty
             */
            eProsima_user_DllExport uint8_t classification_certainty() const;

            /*!
             * @brief This function returns a reference to member classification_certainty
             * @return Reference to member classification_certainty
             */
            eProsima_user_DllExport uint8_t& classification_certainty();

            /*!
             * @brief This function sets a value in member classification_age
             * @param _classification_age New value for member classification_age
             */
            eProsima_user_DllExport void classification_age(
                    uint32_t _classification_age);

            /*!
             * @brief This function returns the value of member classification_age
             * @return Value of member classification_age
             */
            eProsima_user_DllExport uint32_t classification_age() const;

            /*!
             * @brief This function returns a reference to member classification_age
             * @return Reference to member classification_age
             */
            eProsima_user_DllExport uint32_t& classification_age();


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
                    const derived_object_msgs::msg::Object& data,
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

            std_msgs::msg::Header m_header;
            uint32_t m_id;
            uint8_t m_detection_level;
            bool m_object_classified;
            geometry_msgs::msg::Pose m_pose;
            geometry_msgs::msg::Twist m_twist;
            geometry_msgs::msg::Accel m_accel;
            geometry_msgs::msg::Polygon m_polygon;
            shape_msgs::msg::SolidPrimitive m_shape;
            uint8_t m_classification;
            uint8_t m_classification_certainty;
            uint32_t m_classification_age;
        };
    } // namespace msg
} // namespace derived_object_msgs

#endif // _FAST_DDS_GENERATED_DERIVED_OBJECT_MSGS_MSG_OBJECT_H_