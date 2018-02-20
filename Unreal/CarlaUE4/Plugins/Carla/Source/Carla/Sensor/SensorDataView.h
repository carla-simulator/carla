// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "ReadOnlyBufferView.h"

/// A view over a sensor's output data. Does not own the data.
class FSensorDataView {
public:

  FSensorDataView(
      FReadOnlyBufferView InHeader,
      FReadOnlyBufferView InData)
    : Header(InHeader),
      Data(InData) {}

  FReadOnlyBufferView GetHeader() const
  {
    return Header;
  }

  FReadOnlyBufferView GetData() const
  {
    return Data;
  }

private:

  FReadOnlyBufferView Header;

  FReadOnlyBufferView Data;
};
