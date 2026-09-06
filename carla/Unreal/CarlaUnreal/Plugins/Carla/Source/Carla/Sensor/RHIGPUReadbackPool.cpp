// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/RHIGPUReadbackPool.h"

FRHIGPUTextureReadback *FRHIGPUReadbackPool::Acquire(int32 &OutSlotIndex)
{
  FScopeLock ScopeLock(&Lock);

  for (int32 Index = 0; Index < Slots.Num(); ++Index)
  {
    if (!InUse[Index])
    {
      InUse[Index] = true;
      if (!Slots[Index].IsValid())
      {
        Slots[Index] = MakeUnique<FRHIGPUTextureReadback>(*DebugName);
      }
      OutSlotIndex = Index;
      return Slots[Index].Get();
    }
  }

  if (Slots.Num() < MaxSize)
  {
    const int32 NewIndex = Slots.Add(MakeUnique<FRHIGPUTextureReadback>(*DebugName));
    InUse.Add(true);
    OutSlotIndex = NewIndex;
    return Slots[NewIndex].Get();
  }

  OutSlotIndex = INDEX_NONE;
  return nullptr;
}

void FRHIGPUReadbackPool::Release(int32 SlotIndex)
{
  FScopeLock ScopeLock(&Lock);
  if (SlotIndex >= 0 && SlotIndex < InUse.Num())
  {
    InUse[SlotIndex] = false;
  }
}