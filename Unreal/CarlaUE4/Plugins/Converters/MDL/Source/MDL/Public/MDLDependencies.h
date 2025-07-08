// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif
// @third party code - BEGIN MDL SDK
#pragma warning(push)
#pragma warning(disable : 4456)
#include <mi/mdl_sdk.h>
#pragma warning(pop)
// @third party code - END MDL SDK
#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif

