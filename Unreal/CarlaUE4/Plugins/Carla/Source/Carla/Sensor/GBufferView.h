// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstddef>



namespace GBufferView
{
    struct FGBufferData;
}



namespace GBuffer
{
    class GBufferContents
    {
    public:

        static size_t GetTotalMemorySize(GBufferView::FGBufferData& Contents);
        static void SerializeGBuffer(GBufferView::FGBufferData& Contents);

    };
}