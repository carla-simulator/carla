// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "RHIGPUReadback.h"
#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"
#include <util/ue-header-guard-end.h>

// Pool of FRHIGPUTextureReadback objects shared by one ASceneCaptureSensor.
// The pool is owned via TSharedPtr<...,ESPMode::ThreadSafe>: the sensor holds
// one strong ref; every in-flight AsyncTask holds another strong ref so the
// pool outlives the sensor if a frame is still being consumed when the
// sensor ends play. Acquire/Release run from different threads (render
// thread for Acquire, AsyncTask for Release) and are guarded by Lock.
//
// Slots are lazy-allocated up to MaxSize; once allocated, an FRHIGPUTextureReadback
// instance and its underlying Vulkan staging buffer live for the rest of the
// pool's lifetime, recycled across frames. Eliminates the per-frame
// alloc/free of large staging buffers that was driving Vulkan suballocator
// fragmentation.
class FRHIGPUReadbackPool
{
public:

  explicit FRHIGPUReadbackPool(const TCHAR *InDebugName, int32 InMaxSize = 4)
    : DebugName(InDebugName), MaxSize(InMaxSize) {}

  // Returns a raw pointer (pool retains ownership) and writes the slot index
  // into OutSlotIndex. Returns nullptr only if every slot is in use AND we
  // already grew to MaxSize; the caller falls back to a per-call alloc.
  FRHIGPUTextureReadback *Acquire(int32 &OutSlotIndex);

  // Marks the slot free. The FRHIGPUTextureReadback object stays alive and
  // its Vulkan staging buffer is reused on the next Acquire.
  void Release(int32 SlotIndex);

private:

  FCriticalSection Lock;
  TArray<TUniquePtr<FRHIGPUTextureReadback>> Slots;
  TArray<bool> InUse;
  FString DebugName;
  int32 MaxSize;
};

using FRHIGPUReadbackPoolPtr = TSharedPtr<FRHIGPUReadbackPool, ESPMode::ThreadSafe>;