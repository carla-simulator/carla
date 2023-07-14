// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TreeTableRow.generated.h"

/**
 *
 */

UENUM(BlueprintType)
enum class ELaneDescriptor : uint8 {
  None          = 0x0,
  Town          = 0x1 << 0,
  Motorway      = 0x1 << 1,
  Rural         = 0x1 << 2,
  Any           = 255 // 0xFE
};

USTRUCT(BlueprintType)
struct FTreeTableRow : public FTableRowBase {

  GENERATED_BODY()

public:

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Meshes")
  TArray<TSoftObjectPtr<UStaticMesh>> Meshes;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprints")
  TArray<TSoftClassPtr<AActor>> Blueprints;

};
