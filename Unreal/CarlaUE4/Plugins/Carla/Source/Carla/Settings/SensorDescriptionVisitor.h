// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

class UCameraDescription;
class ULidarDescription;

class ISensorDescriptionVisitor
{
public:

  virtual void Visit(const UCameraDescription &) = 0;

  virtual void Visit(const ULidarDescription &) = 0;
};
