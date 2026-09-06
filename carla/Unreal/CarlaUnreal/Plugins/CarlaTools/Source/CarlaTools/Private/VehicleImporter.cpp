// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "VehicleImporter.h"
#include "USDImporterWidget.h"
#include "CarlaTools.h"

#include <util/ue-header-guard-begin.h>
#include "HAL/PlatformProcess.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Common/TcpSocketBuilder.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Async/Async.h"
#include "Containers/Ticker.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Factories/FbxImportUI.h"
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "ChaosVehicleWheel.h"
#include "Factories/BlueprintFactory.h"
#include "Engine/Blueprint.h"
#include <util/ue-header-guard-end.h>

// Port CarlaStudio connects to.
static constexpr int32 GImporterPort = 18583;
// Max message size accepted (10 MB — large enough for any vehicle spec JSON).
static constexpr int32 GMaxMessageBytes = 10 * 1024 * 1024;

// ---------------------------------------------------------------------------
// Static members
// ---------------------------------------------------------------------------

FVehicleImporterServer* UVehicleImporter::ServerRunnable = nullptr;
FRunnableThread*        UVehicleImporter::ServerThread   = nullptr;

void UVehicleImporter::StartServer()
{
  if (ServerRunnable)
    return;

  ServerRunnable = new FVehicleImporterServer();
  ServerThread   = FRunnableThread::Create(ServerRunnable,
                     TEXT("CarlaVehicleImporter"),
                     0, TPri_BelowNormal);
  UE_LOG(LogCarlaTools, Log, TEXT("VehicleImporter: listening on port %d"), GImporterPort);
}

void UVehicleImporter::StopServer()
{
  if (ServerRunnable)
    ServerRunnable->Stop();

  if (ServerThread)
  {
    while (ServerRunnable && !ServerRunnable->bDone.Load())
    {
      FTSTicker::GetCoreTicker().Tick(0.0f);
      FPlatformProcess::Sleep(0.001f);
    }
    ServerThread->WaitForCompletion();
    delete ServerThread;
    ServerThread = nullptr;
  }
  delete ServerRunnable;
  ServerRunnable = nullptr;
}

// ---------------------------------------------------------------------------
// FVehicleImporterServer
// ---------------------------------------------------------------------------

FVehicleImporterServer::FVehicleImporterServer()  = default;
FVehicleImporterServer::~FVehicleImporterServer() = default;

bool FVehicleImporterServer::Init()
{
  ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
  if (!SS)
    return false;

  ListenSocket = FTcpSocketBuilder(TEXT("CarlaVehicleImporterListen"))
    .AsReusable()
    .BoundToEndpoint(FIPv4Endpoint(FIPv4Address::InternalLoopback, GImporterPort))
    .Listening(1)
    .Build();

  if (!ListenSocket)
  {
    UE_LOG(LogCarlaTools, Error,
           TEXT("VehicleImporter: failed to bind port %d"), GImporterPort);
    return false;
  }

  bRunning = true;
  return true;
}

uint32 FVehicleImporterServer::Run()
{
  FSocket* LocalSocket = ListenSocket;
  if (!LocalSocket) { bDone = true; return 0; }
  while (bRunning && LocalSocket)
  {
    bool bPending = false;
    if (LocalSocket->WaitForPendingConnection(bPending, FTimespan::FromSeconds(1.0)))
    {
      if (bPending)
      {
        FSocket* Client = LocalSocket->Accept(TEXT("CarlaStudio"));
        if (Client)
        {
          ServeClient(Client);
          Client->Close();
          ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Client);
        }
      }
    }
  }
  bDone = true;
  return 0;
}

void FVehicleImporterServer::Stop()
{
  bRunning = false;
  if (ListenSocket)
  {
    ListenSocket->Close();
    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
    ListenSocket = nullptr;
  }
}

// ---------------------------------------------------------------------------
// Protocol: 4-byte little-endian length prefix + UTF-8 JSON body
// ---------------------------------------------------------------------------

static bool RecvAll(FSocket* S, uint8* Buf, int32 Len)
{
  int32 Received = 0;
  while (Received < Len)
  {
    int32 Got = 0;
    if (!S->Recv(Buf + Received, Len - Received, Got) || Got <= 0)
      return false;
    Received += Got;
  }
  return true;
}

static bool SendAll(FSocket* S, const uint8* Buf, int32 Len)
{
  int32 Sent = 0;
  while (Sent < Len)
  {
    int32 Written = 0;
    if (!S->Send(Buf + Sent, Len - Sent, Written) || Written <= 0)
      return false;
    Sent += Written;
  }
  return true;
}

void FVehicleImporterServer::ServeClient(FSocket* Client)
{
  // Read 4-byte length header.
  uint8 LenBuf[4];
  if (!RecvAll(Client, LenBuf, 4))
    return;

  const int32 MsgLen = (int32)(LenBuf[0]
    | ((uint32)LenBuf[1] << 8)
    | ((uint32)LenBuf[2] << 16)
    | ((uint32)LenBuf[3] << 24));

  if (MsgLen <= 0 || MsgLen > GMaxMessageBytes)
  {
    UE_LOG(LogCarlaTools, Warning,
           TEXT("VehicleImporter: bad message length %d"), MsgLen);
    return;
  }

  TArray<uint8> Body;
  Body.SetNumUninitialized(MsgLen + 1);  // +1 for null terminator
  if (!RecvAll(Client, Body.GetData(), MsgLen))
    return;

  Body[MsgLen] = '\0';

  const FString JsonStr = FString(UTF8_TO_TCHAR(
    reinterpret_cast<const ANSICHAR*>(Body.GetData())));

  // Multiplex on the JSON `action` field: "spawn" → drop the named asset
  // into the editor world; anything else (or absent) → the original
  // import flow. Any new game-thread work goes through the same FTSTicker
  // dispatch as ProcessSpec — see the comment block in ProcessSpec for
  // why this dance is needed (TaskGraph RecursionGuard otherwise).
  FString Response;
  TSharedPtr<FJsonObject> Root;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
  if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
  {
    Response = MakeResponse(false, TEXT(""), TEXT("JSON parse error"));
  }
  else
  {
    FString Action;
    Root->TryGetStringField(TEXT("action"), Action);
    if (Action.Equals(TEXT("spawn"), ESearchCase::IgnoreCase))
    {
      FSpawnRequest Req;
      Root->TryGetStringField(TEXT("asset_path"), Req.AssetPath);
      double V = 0.0;
      if (Root->TryGetNumberField(TEXT("x"),   V)) Req.Loc.X   = (float)V;
      if (Root->TryGetNumberField(TEXT("y"),   V)) Req.Loc.Y   = (float)V;
      if (Root->TryGetNumberField(TEXT("z"),   V)) Req.Loc.Z   = (float)V;
      if (Root->TryGetNumberField(TEXT("yaw"), V)) Req.Yaw     = (float)V;

      auto P = MakeShared<TPromise<FString>>();
      TFuture<FString> Future = P->GetFuture();
      FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
        [this, Req, P](float) -> bool {
          P->SetValue(ProcessSpawn(Req));
          return false;
        }));
      Response = Future.Get();
    }
    else
    {
      FVehicleImportSpec Spec;
      if (!ParseSpec(JsonStr, Spec))
      {
        Response = MakeResponse(false, TEXT(""), TEXT("JSON parse error"));
      }
      else
      {
        auto P = MakeShared<TPromise<FString>>();
        TFuture<FString> Future = P->GetFuture();
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
          [this, Spec, P](float) -> bool {
            P->SetValue(ProcessSpec(Spec));
            return false;
          }));
        Response = Future.Get();
      }
    }
  }

  // Send length-prefixed response.
  FTCHARToUTF8 ResponseUTF8(*Response);
  const int32 RespLen = ResponseUTF8.Length();
  uint8 RespLenBuf[4] = {
    (uint8)(RespLen & 0xFF),
    (uint8)((RespLen >> 8)  & 0xFF),
    (uint8)((RespLen >> 16) & 0xFF),
    (uint8)((RespLen >> 24) & 0xFF)
  };
  SendAll(Client, RespLenBuf, 4);
  SendAll(Client, reinterpret_cast<const uint8*>(ResponseUTF8.Get()), RespLen);
}

// ---------------------------------------------------------------------------
// JSON helpers
// ---------------------------------------------------------------------------

static float JF(const TSharedPtr<FJsonObject>& O, const FString& K, float Def = 0.f)
{
  double V = Def;
  O->TryGetNumberField(K, V);
  return (float)V;
}

static TSharedPtr<FJsonObject> JO(const TSharedPtr<FJsonObject>& O, const FString& K)
{
  const TSharedPtr<FJsonObject>* Sub = nullptr;
  if (O->TryGetObjectField(K, Sub))
    return *Sub;
  return nullptr;
}

static void ParseWheel(const TSharedPtr<FJsonObject>& Root,
                       const FString& Key,
                       FWheelImportSpec& Out)
{
  TSharedPtr<FJsonObject> W = JO(Root, Key);
  if (!W) return;
  Out.X              = JF(W, "x");
  Out.Y              = JF(W, "y");
  Out.Z              = JF(W, "z");
  Out.Radius         = JF(W, "radius",           33.f);
  Out.MaxSteerAngle  = JF(W, "max_steer_angle",  70.f);
  Out.MaxBrakeTorque = JF(W, "max_brake_torque", 1500.f);
  Out.SuspMaxRaise   = JF(W, "susp_max_raise",   10.f);
  Out.SuspMaxDrop    = JF(W, "susp_max_drop",    10.f);
}

bool FVehicleImporterServer::ParseSpec(const FString& Json, FVehicleImportSpec& Out)
{
  TSharedPtr<FJsonObject> Root;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
  if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    return false;

  Root->TryGetStringField(TEXT("vehicle_name"),   Out.VehicleName);
  Root->TryGetStringField(TEXT("mesh_path"),       Out.MeshFilePath);
  Root->TryGetStringField(TEXT("content_path"),    Out.ContentPath);
  Root->TryGetStringField(TEXT("base_vehicle_bp"), Out.BaseVehicleBP);

  double V = 1500.0;
  Root->TryGetNumberField(TEXT("mass"),            V); Out.Mass = (float)V;
  Root->TryGetNumberField(TEXT("susp_damping"),    V); Out.SuspDamping = (float)V;

  V = 1.0;
  Root->TryGetNumberField(TEXT("source_scale_to_cm"), V); Out.SourceScaleToCm = (float)V;
  int32 IV = 2;
  Root->TryGetNumberField(TEXT("source_up_axis"),      IV); Out.SourceUpAxis      = IV;
  IV = 0;
  Root->TryGetNumberField(TEXT("source_forward_axis"), IV); Out.SourceForwardAxis = IV;

  ParseWheel(Root, TEXT("wheel_fl"), Out.WheelFL);
  ParseWheel(Root, TEXT("wheel_fr"), Out.WheelFR);
  ParseWheel(Root, TEXT("wheel_rl"), Out.WheelRL);
  ParseWheel(Root, TEXT("wheel_rr"), Out.WheelRR);

  return !Out.VehicleName.IsEmpty() && !Out.MeshFilePath.IsEmpty();
}

FString FVehicleImporterServer::MakeResponse(bool bOk, const FString& Path, const FString& Err)
{
  TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
  Obj->SetBoolField(TEXT("success"),      bOk);
  Obj->SetStringField(TEXT("asset_path"), Path);
  Obj->SetStringField(TEXT("error"),      Err);

  FString Out;
  TSharedRef<TJsonWriter<>> W = TJsonWriterFactory<>::Create(&Out);
  FJsonSerializer::Serialize(Obj.ToSharedRef(), W);
  return Out;
}

// ---------------------------------------------------------------------------
// Asset creation (runs on Game Thread)
// ---------------------------------------------------------------------------

// Sanitize an arbitrary string into a valid UE asset / package object name.
// UE only allows [A-Za-z0-9_]; anything else (spaces, '+', '-', '.', etc.)
// becomes '_'. Leading digits are also forbidden, so prefix if needed.
static FString SanitizeAssetName(const FString& In)
{
  FString Out;
  Out.Reserve(In.Len());
  for (TCHAR C : In)
  {
    const bool bOk = (C >= 'A' && C <= 'Z') || (C >= 'a' && C <= 'z')
                  || (C >= '0' && C <= '9') || C == '_';
    Out.AppendChar(bOk ? C : TEXT('_'));
  }
  if (Out.IsEmpty()) Out = TEXT("Vehicle");
  if (Out[0] >= '0' && Out[0] <= '9') Out = TEXT("V_") + Out;
  return Out;
}

static UStaticMesh* ImportStaticMesh(const FString& FilePath,
                                     const FString& ContentPath,
                                     const FString& AssetName,
                                     const FVehicleImportSpec& Spec)
{
  UAssetImportTask* Task = NewObject<UAssetImportTask>();
  Task->Filename        = FilePath;
  Task->DestinationPath = ContentPath;
  Task->DestinationName = SanitizeAssetName(AssetName);
  // Don't save during import. Saving while we're holding the game-thread
  // tick (we run ProcessSpec from FTSTicker) triggers source-control
  // modals + a save dialog cascade that hangs the editor. The asset is
  // saved separately at the very end of the import, after all the
  // wheel/vehicle BPs have been built.
  Task->bSave           = false;
  Task->bAutomated      = true;
  Task->bReplaceExisting = true;

  IAssetTools& AssetTools =
    FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
  AssetTools.ImportAssetTasks({ Task });

  TArray<UObject*> Imported = Task->GetObjects();
  if (Imported.Num() == 0)
    return nullptr;
  UStaticMesh* Mesh = Cast<UStaticMesh>(Imported[0]);
  if (!Mesh) return nullptr;

  // Unit scaling is done by CarlaStudio at the source (it pre-scales the
  // OBJ vertices before sending the file path) so MeshDescription and
  // RenderData bounds stay in sync. Force BuildScale3D back to (1,1,1)
  // explicitly — UE preserves the previous BuildSettings on re-import, so
  // an asset that was previously imported with a 100× scale will keep
  // multiplying. Resetting here makes re-imports idempotent.
  if (Mesh->GetNumSourceModels() > 0)
  {
    FStaticMeshSourceModel& SM = Mesh->GetSourceModel(0);
    if (!SM.BuildSettings.BuildScale3D.Equals(FVector(1.0)))
    {
      SM.BuildSettings.BuildScale3D = FVector(1.0);
      Mesh->Build(false);
      Mesh->MarkPackageDirty();
    }
  }
  return Mesh;
}

static TSubclassOf<UChaosVehicleWheel> CreateWheelBlueprint(
    const FString& ContentPath,
    const FString& Name,
    const FWheelImportSpec& W)
{
  IAssetTools& AssetTools =
    FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

  UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
  Factory->ParentClass = UChaosVehicleWheel::StaticClass();

  UObject* Asset = AssetTools.CreateAsset(Name, ContentPath,
                                          UBlueprint::StaticClass(), Factory);
  UBlueprint* BP = Cast<UBlueprint>(Asset);
  if (!BP || !BP->GeneratedClass) return nullptr;

  // BP factory does an initial compile inside CreateAsset, so GeneratedClass
  // is already valid. Just patch the CDO with our wheel parameters and skip
  // the explicit CompileBlueprint recompile — that call needs other game-
  // thread tasks to pump (shader/asset registry/save callbacks) to make
  // progress, which we can't provide while we're holding the FTSTicker
  // callback open.
  UChaosVehicleWheel* Defaults =
    Cast<UChaosVehicleWheel>(BP->GeneratedClass->ClassDefaultObject);
  if (Defaults)
  {
    Defaults->WheelRadius        = W.Radius;
    Defaults->MaxSteerAngle      = W.MaxSteerAngle;
    Defaults->MaxBrakeTorque     = W.MaxBrakeTorque;
    Defaults->SuspensionMaxRaise = W.SuspMaxRaise;
    Defaults->SuspensionMaxDrop  = W.SuspMaxDrop;
  }
  return TSubclassOf<UChaosVehicleWheel>(BP->GeneratedClass);
}

FString FVehicleImporterServer::ProcessSpec(const FVehicleImportSpec& Spec)
{
  UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: enter (vehicle=%s mesh=%s)"),
         *Spec.VehicleName, *Spec.MeshFilePath);

  // Resolve content path: ensure it ends without slash.
  FString ContentRoot = Spec.ContentPath;
  if (ContentRoot.EndsWith(TEXT("/")))
    ContentRoot.RemoveFromEnd(TEXT("/"));
  const FString VehicleContentPath = ContentRoot / Spec.VehicleName;

  // 1. Import body static mesh.
  UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: step 1/5 — importing body mesh"));
  UStaticMesh* BodyMesh = ImportStaticMesh(
    Spec.MeshFilePath, VehicleContentPath,
    Spec.VehicleName + TEXT("_body"), Spec);
  if (!BodyMesh)
    return MakeResponse(false, TEXT(""), TEXT("Failed to import mesh: ") + Spec.MeshFilePath);
  UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: body mesh imported OK"));

  // 2. Create wheel blueprints.
  UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: step 2/5 — creating 4 wheel blueprints"));
  auto MakeWheelBP = [&](const FString& Suffix, const FWheelImportSpec& W)
    -> TSubclassOf<UChaosVehicleWheel>
  {
    UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec:   wheel %s …"), *Suffix);
    auto R = CreateWheelBlueprint(
      VehicleContentPath,
      Spec.VehicleName + TEXT("_Wheel_") + Suffix,
      W);
    UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec:   wheel %s done (%s)"),
           *Suffix, R ? TEXT("ok") : TEXT("FAILED"));
    return R;
  };

  TSubclassOf<UChaosVehicleWheel> WheelFL = MakeWheelBP(TEXT("FLW"), Spec.WheelFL);
  TSubclassOf<UChaosVehicleWheel> WheelFR = MakeWheelBP(TEXT("FRW"), Spec.WheelFR);
  TSubclassOf<UChaosVehicleWheel> WheelRL = MakeWheelBP(TEXT("RLW"), Spec.WheelRL);
  TSubclassOf<UChaosVehicleWheel> WheelRR = MakeWheelBP(TEXT("RRW"), Spec.WheelRR);

  if (!WheelFL || !WheelFR || !WheelRL || !WheelRR)
    return MakeResponse(false, TEXT(""), TEXT("Failed to create wheel blueprints"));

  // 3. Resolve base vehicle class from provided BP path or fall back to the
  //    Carla C++ base so GenerateNewVehicleBlueprint can always spawn a template.
  UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: step 3/5 — loading base BP %s"),
         *Spec.BaseVehicleBP);
  UClass* BaseClass = nullptr;
  if (!Spec.BaseVehicleBP.IsEmpty())
  {
    UObject* BPObj = UEditorAssetLibrary::LoadAsset(Spec.BaseVehicleBP);
    UBlueprint* BPAsset = Cast<UBlueprint>(BPObj);
    if (BPAsset && BPAsset->GeneratedClass)
      BaseClass = BPAsset->GeneratedClass;
  }
  if (!BaseClass)
  {
    // The only base BP with a real USkeletalMesh + PhysicsAsset that
    // GenerateNewVehicleBlueprint can consume. Production vehicle BPs
    // (Jeep, Sprinter, …) are static-mesh rigs and won't work here.
    UObject* TplObj = UEditorAssetLibrary::LoadAsset(
      TEXT("/Game/Carla/Blueprints/USDImportTemplates/BaseUSDImportVehicle"));
    if (UBlueprint* TplBP = Cast<UBlueprint>(TplObj))
      BaseClass = TplBP->GeneratedClass;
  }
  if (!BaseClass)
    return MakeResponse(false, TEXT(""),
      TEXT("Could not resolve base vehicle class — set base_vehicle_bp in the spec"));

  // 4. Build the merged-parts struct that GenerateNewVehicleBlueprint expects.
  //    For this import, the body mesh covers the full vehicle. Wheel geometry
  //    is handled by the physics asset spheres — no separate wheel static meshes
  //    are required for a functional vehicle.
  FMergedVehicleMeshParts Parts;
  Parts.Body = BodyMesh;
  Parts.Anchors.WheelFL = FVector(Spec.WheelFL.X, Spec.WheelFL.Y, Spec.WheelFL.Z);
  Parts.Anchors.WheelFR = FVector(Spec.WheelFR.X, Spec.WheelFR.Y, Spec.WheelFR.Z);
  Parts.Anchors.WheelRL = FVector(Spec.WheelRL.X, Spec.WheelRL.Y, Spec.WheelRL.Z);
  Parts.Anchors.WheelRR = FVector(Spec.WheelRR.X, Spec.WheelRR.Y, Spec.WheelRR.Z);

  FWheelTemplates WheelTemplates;
  WheelTemplates.WheelFL = WheelFL;
  WheelTemplates.WheelFR = WheelFR;
  WheelTemplates.WheelRL = WheelRL;
  WheelTemplates.WheelRR = WheelRR;

  // 5. Find the skeletal mesh + physics asset on the base vehicle. We try
  //    three strategies, falling through if the prior one yields nothing:
  //      a) CDO components (works only if SkelMesh is C++-declared)
  //      b) SCS walk up the BP class hierarchy (works for BP-declared mesh)
  //      c) Spawn the actor in the editor world and inspect the instance
  //         (most reliable, but heavier — only used if a/b miss)
  //    Diagnostic log dumps every component class we see along the way so
  //    we can keep refining when a real-world BP doesn't fit a/b.
  AActor* TemplateDefault = BaseClass->GetDefaultObject<AActor>();
  USkeletalMeshComponent* SkelComp = nullptr;
  auto TakeIfBetter = [&](USkeletalMeshComponent* C) {
    if (!C) return;
    if (C->GetSkeletalMeshAsset()) { SkelComp = C; return; }
    if (!SkelComp) SkelComp = C;
  };

  // (a) CDO components.
  if (TemplateDefault)
  {
    TArray<UActorComponent*> Components;
    TemplateDefault->GetComponents(Components);
    UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: CDO has %d components on %s"),
           Components.Num(), *BaseClass->GetName());
    for (UActorComponent* C : Components)
    {
      if (!C) continue;
      UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec:   CDO comp: %s (%s)"),
             *C->GetName(), *C->GetClass()->GetName());
      TakeIfBetter(Cast<USkeletalMeshComponent>(C));
    }
  }

  // (b) Walk every BP in the inheritance chain and inspect its SCS nodes.
  if (!SkelComp || !SkelComp->GetSkeletalMeshAsset())
  {
    UClass* WalkClass = BaseClass;
    while (WalkClass)
    {
      UBlueprint* WalkBP = Cast<UBlueprint>(WalkClass->ClassGeneratedBy);
      UBlueprintGeneratedClass* OwnerGen = Cast<UBlueprintGeneratedClass>(WalkBP ? WalkBP->GeneratedClass : nullptr);
      if (WalkBP && WalkBP->SimpleConstructionScript && OwnerGen)
      {
        const auto& Nodes = WalkBP->SimpleConstructionScript->GetAllNodes();
        UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: SCS for %s has %d node(s)"),
               *WalkClass->GetName(), Nodes.Num());
        for (USCS_Node* Node : Nodes)
        {
          if (!Node) continue;
          UActorComponent* Tmpl = Node->GetActualComponentTemplate(OwnerGen);
          UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec:   SCS node: %s (template=%s)"),
                 *Node->GetVariableName().ToString(),
                 Tmpl ? *Tmpl->GetClass()->GetName() : TEXT("null"));
          TakeIfBetter(Cast<USkeletalMeshComponent>(Tmpl));
        }
        if (SkelComp && SkelComp->GetSkeletalMeshAsset()) break;
      }
      WalkClass = WalkClass->GetSuperClass();
    }
  }

  // (c) Last-resort: spawn the actor into the transient editor world and
  //     inspect its real components. SCS components only get materialized
  //     at spawn time — this catches BPs whose mesh slot is set on a
  //     base-class component overridden by the BP (common for CARLA's
  //     ACarlaWheeledVehicle, which declares Mesh in C++ and BPs override
  //     the asset slot). Destroyed immediately after.
  if (!SkelComp || !SkelComp->GetSkeletalMeshAsset())
  {
    UWorld* SpawnWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (SpawnWorld)
    {
      FActorSpawnParameters Params;
      Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      Params.ObjectFlags = RF_Transient;
      AActor* Tmp = SpawnWorld->SpawnActor<AActor>(BaseClass, FTransform::Identity, Params);
      UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: spawned %s temporarily for inspection (%s)"),
             *BaseClass->GetName(), Tmp ? TEXT("ok") : TEXT("FAILED"));
      if (Tmp)
      {
        TArray<USkeletalMeshComponent*> SMCs;
        Tmp->GetComponents<USkeletalMeshComponent>(SMCs);
        UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec:   spawned actor has %d SkeletalMeshComponent(s)"), SMCs.Num());
        for (USkeletalMeshComponent* C : SMCs) TakeIfBetter(C);
        Tmp->Destroy();
      }
    }
  }

  USkeletalMesh*  SkelMesh    = SkelComp ? SkelComp->GetSkeletalMeshAsset() : nullptr;
  UPhysicsAsset*  PhysAsset   = SkelMesh  ? SkelMesh->GetPhysicsAsset()      : nullptr;
  UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: step 4/5 — SkelMesh=%s PhysAsset=%s"),
         SkelMesh  ? *SkelMesh->GetName()  : TEXT("null"),
         PhysAsset ? *PhysAsset->GetName() : TEXT("null"));

  if (!SkelMesh || !PhysAsset)
    return MakeResponse(false, TEXT(""),
      FString::Printf(TEXT("Base vehicle blueprint has no skeletal mesh / physics asset (BP=%s SkelMesh=%s PhysAsset=%s)"),
        *Spec.BaseVehicleBP,
        SkelMesh ? *SkelMesh->GetName() : TEXT("null"),
        PhysAsset ? *PhysAsset->GetName() : TEXT("null")));

  // 6. Build the vehicle blueprint.
  UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpec: step 5/5 — generating vehicle blueprint"));
  UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
  if (!World)
    return MakeResponse(false, TEXT(""), TEXT("No editor world available"));

  const FString BPPath = VehicleContentPath / (TEXT("BP_") + Spec.VehicleName);

  UUSDImporterWidget::GenerateNewVehicleBlueprint(
    World, BaseClass, SkelMesh, PhysAsset, BPPath, Parts, WheelTemplates);

  return MakeResponse(true, BPPath, TEXT(""));
}

// ---------------------------------------------------------------------------
// Drop-to-CARLA: spawn a previously-imported vehicle BP into the editor world
// ---------------------------------------------------------------------------

FString FVehicleImporterServer::ProcessSpawn(const FSpawnRequest& Req)
{
  UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpawn: enter (asset=%s loc=%s yaw=%.1f)"),
         *Req.AssetPath, *Req.Loc.ToString(), Req.Yaw);

  if (Req.AssetPath.IsEmpty())
    return MakeResponse(false, TEXT(""), TEXT("spawn: asset_path missing"));

  // Resolve the actor class from the asset path. We try in order:
  //   (1) LoadAsset → cast to UBlueprint, take GeneratedClass
  //   (2) LoadClass with explicit `_C` suffix on the package object
  //   (3) StaticLoadObject<UClass> with the full fixed-up path
  // Each of these covers a slightly different shape of input the user
  // (or our own importer) might have given us.
  auto fixupClassPath = [](const FString& In) -> FString {
    // "/Game/.../BP_X"          -> "/Game/.../BP_X.BP_X_C"
    // "/Game/.../BP_X.BP_X"     -> "/Game/.../BP_X.BP_X_C"
    // "/Game/.../BP_X.BP_X_C"   -> unchanged
    if (In.EndsWith(TEXT("_C"))) return In;
    int32 Dot = INDEX_NONE; In.FindLastChar(TEXT('.'), Dot);
    int32 Slash = INDEX_NONE; In.FindLastChar(TEXT('/'), Slash);
    if (Dot == INDEX_NONE || Dot < Slash)
    {
      const FString Leaf = (Slash == INDEX_NONE) ? In : In.RightChop(Slash + 1);
      return In + TEXT(".") + Leaf + TEXT("_C");
    }
    return In + TEXT("_C");
  };

  UClass* Cls = nullptr;
  FString AttemptedPaths;
  // (1) Direct LoadAsset → blueprint
  {
    AttemptedPaths += Req.AssetPath;
    UObject* Loaded = UEditorAssetLibrary::LoadAsset(Req.AssetPath);
    if (UBlueprint* BP = Cast<UBlueprint>(Loaded))
      Cls = BP->GeneratedClass;
    else if (UClass* DirectClass = Cast<UClass>(Loaded))
      Cls = DirectClass;
  }
  // (2) LoadClass with `_C`-fixed-up path
  if (!Cls)
  {
    const FString Fixed = fixupClassPath(Req.AssetPath);
    AttemptedPaths += TEXT(" | ") + Fixed;
    Cls = LoadClass<AActor>(nullptr, *Fixed);
  }
  // (3) StaticLoadObject as final fallback (handles edge cases the
  // first two miss, e.g. the asset registry not yet caching the BP).
  if (!Cls)
  {
    const FString Fixed = fixupClassPath(Req.AssetPath);
    Cls = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, *Fixed));
  }

  if (!Cls || !Cls->IsChildOf(AActor::StaticClass()))
    return MakeResponse(false, TEXT(""),
      FString::Printf(TEXT("spawn: could not resolve actor class. Tried: %s"),
                      *AttemptedPaths));

  UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
  if (!World)
    return MakeResponse(false, TEXT(""), TEXT("spawn: no editor world available"));

  FActorSpawnParameters Params;
  Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  AActor* Spawned = World->SpawnActor<AActor>(
      Cls, FTransform(FRotator(0.f, Req.Yaw, 0.f), Req.Loc), Params);
  if (!Spawned)
    return MakeResponse(false, TEXT(""),
      FString::Printf(TEXT("spawn: SpawnActor returned null for %s"), *Cls->GetName()));

  UE_LOG(LogCarlaTools, Display, TEXT("VI.ProcessSpawn: spawned %s (label=%s)"),
         *Cls->GetName(), *Spawned->GetActorLabel());
  return MakeResponse(true, Spawned->GetActorLabel(), TEXT(""));
}
