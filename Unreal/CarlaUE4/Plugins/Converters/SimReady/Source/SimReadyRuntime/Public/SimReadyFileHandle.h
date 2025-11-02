// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "Templates/SharedPointer.h"
#include "GenericPlatform/GenericPlatformFile.h"

class SIMREADYRUNTIME_API FSimReadyFileHandle : public IFileHandle
{
public:
    FSimReadyFileHandle(TArray64<uint8> Content);
    ~FSimReadyFileHandle();

    virtual int64		Tell();

    virtual bool		Seek(int64 NewPosition);

    virtual bool		SeekFromEnd(int64 NewPositionRelativeToEnd = 0);

    virtual bool		Read(uint8* Destination, int64 BytesToRead);

    virtual bool		Write(const uint8* Source, int64 BytesToWrite);

    virtual bool		Flush(const bool bFullFlush = false);

    virtual bool		Truncate(int64 NewSize);

    virtual int64		Size();

private:
    TArray64<uint8> Content;
    int64 CurrentPosition;
};

