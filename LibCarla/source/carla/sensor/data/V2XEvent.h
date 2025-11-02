// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/V2XSerializer.h"
#include "carla/sensor/data/V2XData.h"

namespace carla
{
    namespace sensor
    {
        namespace data
        {

            class CAMEvent : public Array<data::CAMData>
            {
                using Super = Array<data::CAMData>;

            protected:
                using Serializer = s11n::CAMDataSerializer;

                friend Serializer;

                explicit CAMEvent(RawData &&data)
                    : Super(0u, std::move(data)) {}

            public:
                Super::size_type GetMessageCount() const
                {
                    return Super::size();
                }
            };

            class CustomV2XEvent : public Array<data::CustomV2XData>
            {
                using Super = Array<data::CustomV2XData>;

            protected:
                using Serializer = s11n::CustomV2XDataSerializer;

                friend Serializer;

                explicit CustomV2XEvent(RawData &&data)
                    : Super(0u, std::move(data)) {}

            public:
                Super::size_type GetMessageCount() const
                {
                    return Super::size();
                }
            };

        } // namespace data
    }     // namespace sensor
} // namespace carla
