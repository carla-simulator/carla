// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Misc/DateTime.h"

struct ScopedTimer
{

  ScopedTimer(FString msg)
  {
    Msg = msg;
    StartTime = FDateTime::UtcNow();
    StartTimestamp = StartTime.ToUnixTimestamp() * 1000 + StartTime.GetMillisecond();
    start = FPlatformTime::Seconds();
  }

  ~ScopedTimer()
  {
    FDateTime StopTime = FDateTime::UtcNow();
    int64 StopTimestamp = StopTime.ToUnixTimestamp() * 1000 + StopTime.GetMillisecond();
    double end = FPlatformTime::Seconds();
    UE_LOG(LogCarla, Error, TEXT("%s - Timer = %d ms Platform = %f ms"),
      *Msg, StopTimestamp-StartTimestamp, (end - start) * 1000.0f);
  }

  FString Msg;
  FDateTime StartTime;
  int64 StartTimestamp;
  double start;

};