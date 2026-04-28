// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include <util/ue-header-guard-end.h>

#include "VehicleImporter.generated.h"

struct FWheelImportSpec
{
  float X = 0.f, Y = 0.f, Z = 0.f;
  float Radius          = 33.f;
  float MaxSteerAngle   = 70.f;
  float MaxBrakeTorque  = 1500.f;
  float SuspMaxRaise    = 10.f;
  float SuspMaxDrop     = 10.f;
};

struct FSpawnRequest
{
  FString  AssetPath;            // e.g. "/Game/Carla/Static/Vehicles/4Wheeled/.../BP_Foo"
  FVector  Loc = FVector(0, 0, 100);
  float    Yaw = 0.f;
};

struct FVehicleImportSpec
{
  FString VehicleName;
  FString MeshFilePath;
  FString ContentPath;
  FString BaseVehicleBP;

  FWheelImportSpec WheelFL, WheelFR, WheelRL, WheelRR;

  float Mass              = 1500.f;
  float SuspDamping       = 0.65f;

  // Source-mesh conventions detected by the client. Used to reorient/rescale
  // the imported StaticMesh so the result is always Z-up, cm regardless of
  // the source format (OBJ from Blender, glTF, DAE, FBX with custom axes, …).
  // Defaults match UE's native conventions (no transform applied).
  float SourceScaleToCm   = 1.f;
  int32 SourceUpAxis      = 2;   // 0=X, 1=Y, 2=Z
  int32 SourceForwardAxis = 0;
};

class FVehicleImporterServer final : public FRunnable
{
public:
  FVehicleImporterServer();
  ~FVehicleImporterServer();

  bool   Init() override;
  uint32 Run()  override;
  void   Stop() override;

private:
  class FSocket*   ListenSocket = nullptr;
  TAtomic<bool>    bRunning { false };

  void    ServeClient(FSocket* Client);
  FString ProcessSpec(const FVehicleImportSpec& Spec);
  FString ProcessSpawn(const FSpawnRequest& Req);

  static bool    ParseSpec(const FString& Json, FVehicleImportSpec& Out);
  static FString MakeResponse(bool bOk, const FString& Path, const FString& Err);
};

UCLASS()
class CARLATOOLS_API UVehicleImporter : public UObject
{
  GENERATED_BODY()
public:
  static void StartServer();
  static void StopServer();

private:
  static FVehicleImporterServer* ServerRunnable;
  static FRunnableThread*        ServerThread;
};
