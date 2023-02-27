// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProceduralMeshComponent.h"
#include "Math/Vector2D.h"

#include <compiler/disable-ue4-macros.h>
#include <boost/optional.hpp>
#include <carla/road/Map.h>
#include <compiler/enable-ue4-macros.h>

#include "OpenDriveToMap.generated.h"


class UProceduralMeshComponent;
class UCustomFileDownloader;
/**
 * 
 */
UCLASS()
class CARLATOOLS_API UOpenDriveToMap : public UUserWidget
{
  GENERATED_BODY()

public:

  UFUNCTION()
  void ConvertOSMInOpenDrive(); 

  UPROPERTY( meta = (BindWidget) )
  class UButton* StartButton;

  UPROPERTY(meta = (BindWidget))
  class UButton* SaveButton;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="File")
  FString FilePath;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="File")
  FString MapName;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  FString Url;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  FVector2D OriginGeoCoordinates;

protected:
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;

  UFUNCTION( BlueprintCallable )
  void CreateMap();

  UFUNCTION( BlueprintCallable )
  void SaveMap();
private:

  UFUNCTION()
  void OpenFileDialog();

  UFUNCTION()
  void LoadMap();

  void GenerateAll(const boost::optional<carla::road::Map>& CarlaMap);
  void GenerateRoadMesh(const boost::optional<carla::road::Map>& CarlaMap);
  void GenerateSpawnPoints(const boost::optional<carla::road::Map>& CarlaMap);

  carla::rpc::OpendriveGenerationParameters opg_parameters;

  UStaticMesh* CreateStaticMeshAsset(UProceduralMeshComponent* ProcMeshComp, int32 MeshIndex, FString FolderName);
  TArray<UStaticMesh*> CreateStaticMeshAssets();
  
  UPROPERTY()
  UCustomFileDownloader* FileDownloader;
  UPROPERTY()
  TArray<AActor*> ActorMeshList;
  UPROPERTY()
  TArray<UStaticMesh*> MeshesToSpawn;
  UPROPERTY()
  TArray<FString> RoadType;
  UPROPERTY()
  TArray<UProceduralMeshComponent*> RoadMesh;
};
