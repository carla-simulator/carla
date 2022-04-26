// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstddef>
#include "Carla/Sensor/AsyncDataStream.h"
#include <compiler/disable-ue4-macros.h>
#include <carla/Buffer.h>
#include <compiler/enable-ue4-macros.h>



namespace GBufferView
{
    struct FGBufferData;
}



namespace gbuffer
{
    namespace detail
    {
        class FGBufferStreamBase
        {
        public:

            static size_t GetTotalMemorySize(GBufferView::FGBufferData& Contents);
            static carla::Buffer SerializeGBuffer(GBufferView::FGBufferData& Contents);
        };
    }

    template <typename StreamType>
    class FGBufferStream :
        private detail::FGBufferStreamBase
    {
        using Base = detail::FGBufferStreamBase;

        StreamType Stream;
        GBufferView::FGBufferData Contents;

    public:

        void Send()
        {
            auto Buffer = Base::SerializeGBuffer(Contents);
            Stream.Send(Buffer);
        }

    };
}