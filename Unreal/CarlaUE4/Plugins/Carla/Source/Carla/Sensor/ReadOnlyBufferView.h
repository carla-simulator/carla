// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Containers/Array.h"

/// A view over a read-only buffer. Does not own the data.
class FReadOnlyBufferView {
public:

  FReadOnlyBufferView() = default;

  explicit FReadOnlyBufferView(const void *InData, uint32 InSize)
    : Data(InData),
      Size(InSize) {}

  template <typename T>
  FReadOnlyBufferView(const TArray<T> &Array)
    : FReadOnlyBufferView(Array.GetData(), sizeof(T) * Array.Num()) {}

  bool HasData() const
  {
    return (Data != nullptr) && (Size > 0u);
  }

  const void *GetData() const
  {
    return Data;
  }

  uint32 GetSize() const
  {
    return Size;
  }

private:

  const void *Data = nullptr;

  uint32 Size = 0u;
};
