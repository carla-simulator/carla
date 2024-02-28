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
 * @file ObjectArray.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_DERIVED_OBJECT_MSGS_MSG_OBJECTARRAY_H_
#define _FAST_DDS_GENERATED_DERIVED_OBJECT_MSGS_MSG_OBJECTARRAY_H_

#include "Object.h"

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
#if defined(ObjectArray_SOURCE)
#define ObjectArray_DllAPI __declspec( dllexport )
#else
#define ObjectArray_DllAPI __declspec( dllimport )
#endif // ObjectArray_SOURCE
#else
#define ObjectArray_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define ObjectArray_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima


namespace derived_object_msgs {
    namespace msg {
        /*!
         * @brief This class represents the structure ObjectArray defined by the user in the IDL file.
         * @ingroup OBJECTARRAY
         */
        class ObjectArray
        {
        public:

            /*!
             * @brief Default constructor.
             */
            eProsima_user_DllExport ObjectArray();

            /*!
             * @brief Default destructor.
             */
            eProsima_user_DllExport ~ObjectArray();

            /*!
             * @brief Copy constructor.
             * @param x Reference to the object derived_object_msgs::msg::ObjectArray that will be copied.
             */
            eProsima_user_DllExport ObjectArray(
                    const ObjectArray& x);

            /*!
             * @brief Move constructor.
             * @param x Reference to the object derived_object_msgs::msg::ObjectArray that will be copied.
             */
            eProsima_user_DllExport ObjectArray(
                    ObjectArray&& x);

            /*!
             * @brief Copy assignment.
             * @param x Reference to the object derived_object_msgs::msg::ObjectArray that will be copied.
             */
            eProsima_user_DllExport ObjectArray& operator =(
                    const ObjectArray& x);

            /*!
             * @brief Move assignment.
             * @param x Reference to the object derived_object_msgs::msg::ObjectArray that will be copied.
             */
            eProsima_user_DllExport ObjectArray& operator =(
                    ObjectArray&& x);

            /*!
             * @brief Comparison operator.
             * @param x derived_object_msgs::msg::ObjectArray object to compare.
             */
            eProsima_user_DllExport bool operator ==(
                    const ObjectArray& x) const;

            /*!
             * @brief Comparison operator.
             * @param x derived_object_msgs::msg::ObjectArray object to compare.
             */
            eProsima_user_DllExport bool operator !=(
                    const ObjectArray& x) const;

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
             * @brief This function copies the value in member objects
             * @param _objects New value to be copied in member objects
             */
            eProsima_user_DllExport void objects(
                    const std::vector<derived_object_msgs::msg::Object>& _objects);

            /*!
             * @brief This function moves the value in member objects
             * @param _objects New value to be moved in member objects
             */
            eProsima_user_DllExport void objects(
                    std::vector<derived_object_msgs::msg::Object>&& _objects);

            /*!
             * @brief This function returns a constant reference to member objects
             * @return Constant reference to member objects
             */
            eProsima_user_DllExport const std::vector<derived_object_msgs::msg::Object>& objects() const;

            /*!
             * @brief This function returns a reference to member objects
             * @return Reference to member objects
             */
            eProsima_user_DllExport std::vector<derived_object_msgs::msg::Object>& objects();

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
                    const derived_object_msgs::msg::ObjectArray& data,
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
            std::vector<derived_object_msgs::msg::Object> m_objects;
        };
    } // namespace msg
} // namespace derived_object_msgs

#endif // _FAST_DDS_GENERATED_DERIVED_OBJECT_MSGS_MSG_OBJECTARRAY_H_