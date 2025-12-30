// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyFileHandle.h"

FSimReadyFileHandle::FSimReadyFileHandle(TArray64<uint8> InContent)
    : Content(InContent)
    , CurrentPosition(0)
{
}

FSimReadyFileHandle::~FSimReadyFileHandle()
{}

int64 FSimReadyFileHandle::Tell()
{
    return 0;
}

bool FSimReadyFileHandle::Seek(int64 NewPosition)
{
    if (NewPosition >= 0 && NewPosition < Size())
    {
        CurrentPosition = NewPosition;
        return true;
    }
    return false;
}

bool FSimReadyFileHandle::SeekFromEnd(int64 NewPositionRelativeToEnd)
{
    if (NewPositionRelativeToEnd <= 0 && NewPositionRelativeToEnd > -Size())
    {
        CurrentPosition = Size() - 1 + NewPositionRelativeToEnd;
        return true;
    }
    return false;
}

bool FSimReadyFileHandle::Read(uint8* Destination, int64 BytesToRead)
{
    if (Destination == nullptr || BytesToRead <= 0)
    {
        return false;
    }

    if (BytesToRead > Size() - CurrentPosition)
    {
        BytesToRead = Size() - CurrentPosition;
    }

    FMemory::Memcpy(Destination, Content.GetData() + CurrentPosition, BytesToRead);
    return true;
}

bool FSimReadyFileHandle::Write(const uint8* Source, int64 BytesToWrite)
{
    return false;
}

bool FSimReadyFileHandle::Flush(const bool bFullFlush)
{
    return false;
}

bool FSimReadyFileHandle::Truncate(int64 NewSize)
{
    return false;
}

int64 FSimReadyFileHandle::Size()
{
    return Content.Num();
}