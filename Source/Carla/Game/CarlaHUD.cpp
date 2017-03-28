// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaHUD.h"

#include "CarlaVehicleController.h"
#include "ConstructorHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

#define LOCTEXT_NAMESPACE "CarlaHUD"

static FText RoundedFloatAsText(float Value)
{
  return FText::AsNumber(FMath::FloorToInt(Value));
}

static FText GetVectorAsText(const FVector &Vector)
{
  return FText::Format(
      LOCTEXT("FVectorFormat", "({0}, {1}, {2})"),
      RoundedFloatAsText(Vector.X),
      RoundedFloatAsText(Vector.Y),
      RoundedFloatAsText(Vector.Z));
}

static FText GetVectorAsText(const FVector &Vector, const FNumberFormattingOptions &FormatOpt)
{
  return FText::Format(
      LOCTEXT("FVectorFormat", "({0}, {1}, {2})"),
      FText::AsNumber(Vector.X, &FormatOpt),
      FText::AsNumber(Vector.Y, &FormatOpt),
      FText::AsNumber(Vector.Z, &FormatOpt));
}

static FText GetGearAsText(int32 Gear)
{
  if (Gear < 0) {
    return FText(LOCTEXT("ReverseGear", "R"));
  } else {
    return (Gear == 0 ? LOCTEXT("NeutralGear", "N") : FText::AsNumber(Gear));
  }
}

static FText GetHUDText(const ACarlaPlayerState &Vehicle)
{
  // Set number precision.
  FNumberFormattingOptions HighPrecision;
  HighPrecision.MinimumFractionalDigits = 2u;
  HighPrecision.MaximumFractionalDigits = 2u;

  FFormatNamedArguments Args;
  Args.Add("Location", GetVectorAsText(Vehicle.GetLocation()));
  Args.Add("Acceleration", GetVectorAsText(Vehicle.GetAcceleration(), HighPrecision));
  Args.Add("Orientation", GetVectorAsText(Vehicle.GetOrientation(), HighPrecision));
  Args.Add("Speed", RoundedFloatAsText(Vehicle.GetForwardSpeed()));
  Args.Add("Gear", GetGearAsText(Vehicle.GetCurrentGear()));
  Args.Add("CollisionCars", RoundedFloatAsText(Vehicle.GetCollisionIntensityCars()));
  Args.Add("CollisionPedestrians", RoundedFloatAsText(Vehicle.GetCollisionIntensityPedestrians()));
  Args.Add("CollisionOther", RoundedFloatAsText(Vehicle.GetCollisionIntensityOther()));
  Args.Add("IntersectionOtherLane", RoundedFloatAsText(100.0f * Vehicle.GetOtherLaneIntersectionFactor()));
  Args.Add("IntersectionOffRoad", RoundedFloatAsText(100.0f * Vehicle.GetOffRoadIntersectionFactor()));
  return FText::Format(
      LOCTEXT("HUDTextFormat",
          "Speed:         {Speed} km/h\n"
          "Gear:          {Gear}\n"
          "\n"
          "Location:      {Location}\n"
          "Acceleration:  {Acceleration}\n"
          "Orientation:   {Orientation}\n"
          "\n"
          "Collision (Cars):       {CollisionCars}\n"
          "Collision (Pedestrian): {CollisionPedestrians}\n"
          "Collision (Other):      {CollisionOther}\n"
          "\n"
          "Intersection (Lane):    {IntersectionOtherLane}%\n"
          "Intersection (OffRoad): {IntersectionOffRoad}%"
      ),
      Args);
}

ACarlaHUD::ACarlaHUD()
{
  static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Engine/EngineFonts/DroidSansMono"));
  HUDFont = Font.Object;
}

void ACarlaHUD::DrawHUD()
{
  Super::DrawHUD();

  if (!bIsVisible)
    return;

  // Calculate ratio from 720p
  const float HUDXRatio = Canvas->SizeX / 1280.f;
  const float HUDYRatio = Canvas->SizeY / 720.f;

  ACarlaVehicleController *Vehicle = Cast<ACarlaVehicleController>(
      GetOwningPawn() == nullptr ? nullptr : GetOwningPawn()->GetController());

  if (Vehicle != nullptr)
  {
    FVector2D ScaleVec(HUDYRatio * 1.4f, HUDYRatio * 1.4f);

    auto Text = GetHUDText(Vehicle->GetPlayerState());
    FCanvasTextItem HUDTextItem(FVector2D(HUDXRatio * 100.0f, HUDYRatio * 400.0f), Text, HUDFont, FLinearColor::White);
    HUDTextItem.Scale = ScaleVec;
    Canvas->DrawItem(HUDTextItem);
  }
}

void ACarlaHUD::ToggleHUDView()
{
  SetVisible(!bIsVisible);
}

void ACarlaHUD::SetVisible(bool IsVisible)
{
  bIsVisible = IsVisible;
}

#undef LOCTEXT_NAMESPACE
