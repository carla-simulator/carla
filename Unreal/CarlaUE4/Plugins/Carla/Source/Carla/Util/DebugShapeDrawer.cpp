// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/DebugShapeDrawer.h"
#include "Carla/Game/CarlaHUD.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"

#include "DrawDebugHelpers.h"
#include "Components/LineBatchComponent.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/DebugShape.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>



struct FShapeVisitor
{
  using Shape = carla::rpc::DebugShape;

  FShapeVisitor(UWorld &InWorld, FColor InColor, float InLifeTime, bool bInPersistentLines)
    : World(&InWorld),
      Color(FLinearColor(InColor) * BrightMultiplier),
      LifeTime(InLifeTime),
      bPersistentLines(bInPersistentLines)
  {
    World->PersistentLineBatcher->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  }

  ACarlaHUD * GetHUD() const {
    auto PlayerController = UGameplayStatics::GetPlayerController(World, 0);
    if (PlayerController == nullptr)
    {
      UE_LOG(LogCarla, Error, TEXT("Can't find player controller!"));
      return nullptr;
    }
    ACarlaHUD *Hud = Cast<ACarlaHUD>(PlayerController->GetHUD());
    return Hud;
  }

  void operator()(const Shape::Point &Point) const
  {
    FVector Location = FVector(Point.location);
    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Location = LargeMap->GlobalToLocalLocation(Location);
    }
    World->PersistentLineBatcher->DrawPoint(
        Location,
        Color,
        1e2f * Point.size,
        DepthPriority,
        LifeTime);
  }

  void operator()(const Shape::HUDPoint &Point) const
  {
    auto Hud = GetHUD();
    if (!Hud) return; // Don't draw if HUD is not available.

    FVector Location = FVector(Point.location);
    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Location = LargeMap->GlobalToLocalLocation(Location);
    }
    Hud->AddHUDLine(Location, Location, Point.size, Color.Quantize(), LifeTime);
  }

  void operator()(const Shape::Line &Line) const
  {
    FVector Begin = FVector(Line.begin);
    FVector End = FVector(Line.end);
    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Begin = LargeMap->GlobalToLocalLocation(Begin);
      End = LargeMap->GlobalToLocalLocation(End);
    }
    World->PersistentLineBatcher->DrawLine(
        Begin,
        End,
        Color,
        DepthPriority,
        1e2f * Line.thickness,
        LifeTime);
  }

  void operator()(const Shape::HUDLine &Line) const {
    auto Hud = GetHUD();
    if (!Hud) return; // Don't draw if HUD is not available.
    FVector Begin = FVector(Line.begin);
    FVector End = FVector(Line.end);
    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Begin = LargeMap->GlobalToLocalLocation(Begin);
      End = LargeMap->GlobalToLocalLocation(End);
    }
    Hud->AddHUDLine(Begin, End, Line.thickness, Color.Quantize(), LifeTime);
  }

  void operator()(const Shape::Arrow &Arrow) const
  {
    FVector Begin = FVector(Arrow.line.begin);
    FVector End = FVector(Arrow.line.end);
    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Begin = LargeMap->GlobalToLocalLocation(Begin);
      End = LargeMap->GlobalToLocalLocation(End);
    }
    const auto Diff = End - Begin;
    const FRotator LookAt = FRotationMatrix::MakeFromX(Diff).Rotator();
    const FTransform Transform = {LookAt, Begin};

    // Everything in centimeters
    const auto Dist = Diff.Size();
    const auto ArrowSize = 1e2f * Arrow.arrow_size;
    const auto ArrowTipDist = Dist - ArrowSize;
    const auto Thickness = 1e2f * Arrow.line.thickness;

    World->PersistentLineBatcher->DrawLines(TArray<FBatchedLine>({
        FBatchedLine(
            Begin, End, Color, LifeTime, Thickness, DepthPriority),
        FBatchedLine(
            Transform.TransformPosition(FVector(ArrowTipDist, +ArrowSize, +ArrowSize)),
            End, Color, LifeTime, Thickness, DepthPriority),
        FBatchedLine(
            Transform.TransformPosition(FVector(ArrowTipDist, +ArrowSize, -ArrowSize)),
            End, Color, LifeTime, Thickness, DepthPriority),
        FBatchedLine(
            Transform.TransformPosition(FVector(ArrowTipDist, -ArrowSize, +ArrowSize)),
            End, Color, LifeTime, Thickness, DepthPriority),
        FBatchedLine(
            Transform.TransformPosition(FVector(ArrowTipDist, -ArrowSize, -ArrowSize)),
            End, Color, LifeTime, Thickness, DepthPriority)}));
  }

  void operator()(const Shape::HUDArrow &Arrow) const {
    auto Hud = GetHUD();
    if (!Hud) return; // Don't draw if HUD is not available.
    FVector Begin = FVector(Arrow.line.begin);
    FVector End = FVector(Arrow.line.end);
    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Begin = LargeMap->GlobalToLocalLocation(Begin);
      End = LargeMap->GlobalToLocalLocation(End);
    }
    const auto Diff = End - Begin;
    const FRotator LookAt = FRotationMatrix::MakeFromX(Diff).Rotator();
    const FTransform Transform = {LookAt, Begin};

    // Everything in centimeters
    const auto Dist = Diff.Size();
    const auto ArrowSize = Arrow.arrow_size;
    const auto ArrowTipDist = Dist - ArrowSize;
    const auto Thickness = Arrow.line.thickness;

    Hud->AddHUDLine(Begin, End, Thickness, Color.Quantize(), LifeTime);
    Hud->AddHUDLine(
        Transform.TransformPosition(FVector(ArrowTipDist, +ArrowSize, +ArrowSize)),
        End, Thickness, Color.Quantize(), LifeTime);
    Hud->AddHUDLine(
        Transform.TransformPosition(FVector(ArrowTipDist, +ArrowSize, -ArrowSize)),
        End, Thickness, Color.Quantize(), LifeTime);
    Hud->AddHUDLine(
        Transform.TransformPosition(FVector(ArrowTipDist, -ArrowSize, +ArrowSize)),
        End, Thickness, Color.Quantize(), LifeTime);
    Hud->AddHUDLine(
        Transform.TransformPosition(FVector(ArrowTipDist, -ArrowSize, -ArrowSize)),
        End, Thickness, Color.Quantize(), LifeTime);
  }

  void operator()(const Shape::Box &Box) const
  {
    const FVector Extent = 1e2f * FVector{Box.box.extent.x, Box.box.extent.y, Box.box.extent.z};
    FTransform Transform = {FRotator(Box.rotation), Box.box.location};
    const auto Thickness = 1e2f * Box.thickness;

    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Transform = LargeMap->GlobalToLocalTransform(Transform);
    }

    FVector B[2], P, Q;
    B[0] = -Extent;
    B[1] =  Extent;

    for(int32 i = 0; i < 2; ++i)
    {
      for(int32 j = 0; j < 2; ++j)
      {
        P.X=B[i].X; Q.X=B[i].X; P.Y=B[j].Y;
        Q.Y=B[j].Y; P.Z=B[0].Z; Q.Z=B[1].Z;
        World->PersistentLineBatcher->DrawLine(
            Transform.TransformPosition(P), Transform.TransformPosition(Q),
            Color, DepthPriority, Thickness, LifeTime);

        P.Y=B[i].Y; Q.Y=B[i].Y; P.Z=B[j].Z;
        Q.Z=B[j].Z; P.X=B[0].X; Q.X=B[1].X;
        World->PersistentLineBatcher->DrawLine(
            Transform.TransformPosition(P), Transform.TransformPosition(Q),
            Color, DepthPriority, Thickness, LifeTime);

        P.Z=B[i].Z; Q.Z=B[i].Z; P.X=B[j].X;
        Q.X=B[j].X; P.Y=B[0].Y; Q.Y=B[1].Y;
        World->PersistentLineBatcher->DrawLine(
            Transform.TransformPosition(P), Transform.TransformPosition(Q),
            Color, DepthPriority, Thickness, LifeTime);
      }
    }
  }

  void operator()(const Shape::HUDBox &Box) const {
    auto Hud = GetHUD();
    if (!Hud) return; // Don't draw if HUD is not available.
    const FVector Extent = 1e2f * FVector{Box.box.extent.x, Box.box.extent.y, Box.box.extent.z};
    FTransform Transform = {FRotator(Box.rotation), Box.box.location};
    const auto Thickness = Box.thickness;

    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Transform = LargeMap->GlobalToLocalTransform(Transform);
    }

    FVector B[2], P, Q;
    B[0] = -Extent;
    B[1] =  Extent;

    for(int32 i = 0; i < 2; ++i)
    {
      for(int32 j = 0; j < 2; ++j)
      {
        P.X=B[i].X; Q.X=B[i].X; P.Y=B[j].Y;
        Q.Y=B[j].Y; P.Z=B[0].Z; Q.Z=B[1].Z;
        Hud->AddHUDLine(
            Transform.TransformPosition(P), Transform.TransformPosition(Q),
            Thickness, Color.Quantize(), LifeTime);

        P.Y=B[i].Y; Q.Y=B[i].Y; P.Z=B[j].Z;
        Q.Z=B[j].Z; P.X=B[0].X; Q.X=B[1].X;
        Hud->AddHUDLine(
            Transform.TransformPosition(P), Transform.TransformPosition(Q),
            Thickness, Color.Quantize(), LifeTime);

        P.Z=B[i].Z; Q.Z=B[i].Z; P.X=B[j].X;
        Q.X=B[j].X; P.Y=B[0].Y; Q.Y=B[1].Y;
        Hud->AddHUDLine(
            Transform.TransformPosition(P), Transform.TransformPosition(Q),
            Thickness, Color.Quantize(), LifeTime);
      }
    }
  }

  void operator()(const Shape::String &Str) const
  {
    auto Hud = GetHUD();
    if (!Hud) return;
    FVector Location = FVector(Str.location);
    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Location = LargeMap->GlobalToLocalLocation(Location);
    }
    Hud->AddHUDString(carla::rpc::ToFString(Str.text), Location, Color.Quantize(), LifeTime);
  }

private:

  UWorld *World;

  FLinearColor Color;

  float LifeTime;

  bool bPersistentLines;

  uint8 DepthPriority = SDPG_World;

  // Debug lines are way more dark in the package, that's why this
  // multiplier is needed.
#if UE_BUILD_SHIPPING
  static constexpr double BrightMultiplier = 1000.0;
#else
  // @TODO: Use UKismetSystemLibrary::IsStandalone to support colors
  // in Editor's standalone mode.
  static constexpr double BrightMultiplier = 1.0;
#endif

};

void FDebugShapeDrawer::Draw(const carla::rpc::DebugShape &Shape)
{
  auto Visitor = FShapeVisitor(World, Shape.color, Shape.life_time, Shape.persistent_lines);
  boost::variant2::visit(Visitor, Shape.primitive);
}

void FDebugShapeDrawer::Clear()
{
  World.PersistentLineBatcher->Flush();
}
