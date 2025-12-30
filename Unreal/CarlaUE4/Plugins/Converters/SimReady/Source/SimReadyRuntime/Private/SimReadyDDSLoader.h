// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#if PLATFORM_WINDOWS

#include "DDSLoader.h"
THIRD_PARTY_INCLUDES_START
    #include "nvtt/nvtt.h"
THIRD_PARTY_INCLUDES_END


class FSimReadyDDSLoadHelper : public FDDSLoadHelper
{
public:
    /** @param Buffer must not be 0 */
    FSimReadyDDSLoadHelper(const uint8* Buffer, uint32 Length);

    uint32 ComputeMipMapSize(uint32 MipMap) const;
    uint32 GetDepth() const;
    uint32 GetBlockSize() const;
    nvtt::Format GetNVTTFormat() const;
    const uint8* GetData() const;
    bool LoadRawData(TArray64<uint8>& RawData);
};
#endif // PLATFORM_WINDOWS