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
 * @file CarlaVehicleStatusPubSubTypes.cpp
 * This header file contains the implementation of the serialization functions.
 *
 * This file was generated by the tool fastcdrgen.
 */


#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "CarlaVehicleStatusPubSubTypes.h"

using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace carla_msgs {
    namespace msg {
        namespace CarlaVehicleStatus_Constants {



        } //End of namespace CarlaVehicleStatus_Constants
        CarlaVehicleStatusPubSubType::CarlaVehicleStatusPubSubType()
        {
            setName("carla_msgs::msg::dds_::CarlaVehicleStatus_");
            auto type_size = CarlaVehicleStatus::getMaxCdrSerializedSize();
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* possible submessage alignment */
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*encapsulation*/
            m_isGetKeyDefined = CarlaVehicleStatus::isKeyDefined();
            size_t keyLength = CarlaVehicleStatus::getKeyMaxCdrSerializedSize() > 16 ?
                    CarlaVehicleStatus::getKeyMaxCdrSerializedSize() : 16;
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            memset(m_keyBuffer, 0, keyLength);
        }

        CarlaVehicleStatusPubSubType::~CarlaVehicleStatusPubSubType()
        {
            if (m_keyBuffer != nullptr)
            {
                free(m_keyBuffer);
            }
        }

        bool CarlaVehicleStatusPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            CarlaVehicleStatus* p_type = static_cast<CarlaVehicleStatus*>(data);

            // Object that manages the raw buffer.
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // Object that serializes the data.
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            // Serialize encapsulation
            ser.serialize_encapsulation();

            try
            {
                // Serialize the object.
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                return false;
            }

            // Get the serialized length
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;
        }

        bool CarlaVehicleStatusPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            //Convert DATA to pointer of your type
            CarlaVehicleStatus* p_type = static_cast<CarlaVehicleStatus*>(data);

            // Object that manages the raw buffer.
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

            // Object that deserializes the data.
            eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

            // Deserialize encapsulation.
            deser.read_encapsulation();
            payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

            try
            {
                // Deserialize the object.
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                return false;
            }

            return true;
        }

        std::function<uint32_t()> CarlaVehicleStatusPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<CarlaVehicleStatus*>(data))) +
                              4u /*encapsulation*/;
                   };
        }

        void* CarlaVehicleStatusPubSubType::createData()
        {
            return reinterpret_cast<void*>(new CarlaVehicleStatus());
        }

        void CarlaVehicleStatusPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<CarlaVehicleStatus*>(data));
        }

        bool CarlaVehicleStatusPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            CarlaVehicleStatus* p_type = static_cast<CarlaVehicleStatus*>(data);

            // Object that manages the raw buffer.
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    CarlaVehicleStatus::getKeyMaxCdrSerializedSize());

            // Object that serializes the data.
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            p_type->serializeKey(ser);
            if (force_md5 || CarlaVehicleStatus::getKeyMaxCdrSerializedSize() > 16)
            {
                m_md5.init();
                m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
                m_md5.finalize();
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_md5.digest[i];
                }
            }
            else
            {
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }


    } //End of namespace msg

} //End of namespace carla_msgs