// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#pragma once

#include <cstdint>
#include <vector>
#include <cstdio>
#include "LibITS.h"

namespace carla
{
    namespace sensor
    {

        namespace s11n
        {
            class CAMDataSerializer;
            class CustomV2XDataSerializer;
        }

        namespace data
        {
            class CAMData
            {
            public:
                float Power;
                CAM_t Message;
            };

            class CustomV2XData
            {
            public:
                float Power;
                CustomV2XM Message;
            };

            class CAMDataS
            {

            public:
                explicit CAMDataS() = default;

                CAMDataS &operator=(CAMDataS &&) = default;

                // Returns the number of current received messages.
                size_t GetMessageCount() const
                {
                    return MessageList.size();
                }

                // Deletes the current messages.
                void Reset()
                {
                    MessageList.clear();
                }

                // Adds a new detection.
                void WriteMessage(CAMData message)
                {
                    MessageList.push_back(message);
                }

            private:
                std::vector<CAMData> MessageList;

                friend class s11n::CAMDataSerializer;
            };

            class CustomV2XDataS
            {

            public:
                explicit CustomV2XDataS() = default;

                CustomV2XDataS &operator=(CustomV2XDataS &&) = default;

                // Returns the number of current received messages.
                size_t GetMessageCount() const
                {
                    return MessageList.size();
                }

                // Deletes the current messages.
                void Reset()
                {
                    MessageList.clear();
                }

                // Adds a new detection.
                void WriteMessage(CustomV2XData message)
                {
                    MessageList.push_back(message);
                }

            private:
                std::vector<CustomV2XData> MessageList;

                friend class s11n::CustomV2XDataSerializer;
            };

        } // namespace s11n
    }     // namespace sensor
} // namespace carla
