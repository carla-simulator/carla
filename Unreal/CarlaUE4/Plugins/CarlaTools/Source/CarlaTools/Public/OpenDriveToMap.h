// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProceduralMeshComponent.h"

#include <compiler/disable-ue4-macros.h>
#include <boost/optional.hpp>
#include <carla/road/Map.h>
#include <compiler/enable-ue4-macros.h>

#include "OpenDriveToMap.generated.h"


class UProceduralMeshComponent;
/**
 * 
 */
UCLASS()
class CARLATOOLS_API UOpenDriveToMap : public UUserWidget
{
  GENERATED_BODY()
  
public:

  UPROPERTY( meta = (BindWidget) )
  class UButton* ChooseFileButon;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="File")
  FString FilePath;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="File")
  FString MapName;

protected:
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;

private:

  UFUNCTION()
  void CreateMap();

  UFUNCTION()
  void OpenFileDialog();

  UFUNCTION()
  void LoadMap();

  void GenerateAll(const boost::optional<carla::road::Map>& CarlaMap);
  void GenerateRoadMesh(const boost::optional<carla::road::Map>& CarlaMap);
  void GenerateSpawnPoints(const boost::optional<carla::road::Map>& CarlaMap);

  const carla::rpc::OpendriveGenerationParameters opg_parameters;

  UStaticMesh* CreateStaticMeshAsset(UProceduralMeshComponent* ProcMeshComp, int32 MeshIndex );
};
