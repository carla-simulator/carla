// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Materials/MaterialExpression.h"

#if WITH_MDL_SDK

MDL_API void ArrangeNodes(UObject* Parent, const TArray<UMaterialExpression*>& OutputExpressions);

#endif