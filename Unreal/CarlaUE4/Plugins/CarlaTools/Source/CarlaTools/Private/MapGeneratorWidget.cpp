// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #if WITH_EDITOR

#include "MapGeneratorWidget.h"

#define DEBUG_MSG(x, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT(x), __VA_ARGS__));}


// UMapGeneratorWidget::UMapGeneratorWidget()
// {

// }

// UMapGeneratorWidget::~UMapGeneratorWidget()
// {}

void UMapGeneratorWidget::HelloWorld()
{
    DEBUG_MSG("RT Changed")
}

// #endif