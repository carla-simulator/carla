// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CarlaHUD.h"

#include "Vehicle/CarlaVehicleController.h"

#include "CommandLine.h"
#include "ConstructorHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

#define LOCTEXT_NAMESPACE "CarlaHUD"

static bool GetDefaultHUDVisibility() {
  return !FParse::Param(FCommandLine::Get(), TEXT("carla-no-hud"));
}

static FText RoundedFloatAsText(float Value)
{
  return FText::AsNumber(FMath::RoundHalfFromZero(Value));
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

static FText GetTrafficLightAsText(ETrafficLightState State)
{
  switch (State) {
    case ETrafficLightState::Green:  return FText(LOCTEXT("GreenTrafficLight",   "Green"));
    case ETrafficLightState::Yellow: return FText(LOCTEXT("YellowTrafficLight",  "Yellow"));
    case ETrafficLightState::Red:    return FText(LOCTEXT("RedTrafficLight",     "Red"));
    default:                         return FText(LOCTEXT("InvalidTrafficLight", "INVALID"));
  }
}

static FText GetHUDText(const ACarlaPlayerState &Vehicle)
{
  // Set number precision.
  FNumberFormattingOptions HighPrecision;
  HighPrecision.MinimumFractionalDigits = 2u;
  HighPrecision.MaximumFractionalDigits = 2u;

  constexpr float TO_MILLISECONDS = 1e3;
  constexpr float TO_METERS = 1e-2;
  constexpr float TO_KMPH = 0.036f;

  FFormatNamedArguments Args;
  Args.Add("SimStep", RoundedFloatAsText(Vehicle.GetSimulationStepInSeconds() * TO_MILLISECONDS));
  Args.Add("Location", GetVectorAsText(Vehicle.GetLocation() * TO_METERS));
  Args.Add("Acceleration", GetVectorAsText(Vehicle.GetAcceleration() * TO_METERS, HighPrecision));
  Args.Add("Orientation", GetVectorAsText(Vehicle.GetOrientation(), HighPrecision));
  Args.Add("Speed", RoundedFloatAsText(Vehicle.GetForwardSpeed() * TO_KMPH));
  Args.Add("Gear", GetGearAsText(Vehicle.GetCurrentGear()));
  Args.Add("SpeedLimit", RoundedFloatAsText(Vehicle.GetSpeedLimit()));
  Args.Add("TrafficLightState", GetTrafficLightAsText(Vehicle.GetTrafficLightState()));
  Args.Add("CollisionCars", RoundedFloatAsText(Vehicle.GetCollisionIntensityCars() * TO_METERS));
  Args.Add("CollisionPedestrians", RoundedFloatAsText(Vehicle.GetCollisionIntensityPedestrians() * TO_METERS));
  Args.Add("CollisionOther", RoundedFloatAsText(Vehicle.GetCollisionIntensityOther() * TO_METERS));
  Args.Add("IntersectionOtherLane", RoundedFloatAsText(100.0f * Vehicle.GetOtherLaneIntersectionFactor()));
  Args.Add("IntersectionOffRoad", RoundedFloatAsText(100.0f * Vehicle.GetOffRoadIntersectionFactor()));
  return FText::Format(
      LOCTEXT("HUDTextFormat",
          "Simulation Step: {SimStep} ms\n"
          "\n"
          "Speed:         {Speed} km/h\n"
          "Gear:          {Gear}\n"
          "\n"
          "Speed Limit:   {SpeedLimit} km/h\n"
          "Traffic Light: {TrafficLightState}\n"
          "\n"
          "Location:      {Location}\n"
          "Orientation:   {Orientation}\n"
          "Acceleration:  {Acceleration}\n"
          "\n"
          "Collision (Cars):       {CollisionCars}\n"
          "Collision (Pedestrian): {CollisionPedestrians}\n"
          "Collision (Other):      {CollisionOther}\n"
          "\n"
          "Intersection (Lane):    {IntersectionOtherLane}%\n"
          "Intersection (OffRoad): {IntersectionOffRoad}%")
      ,
      Args);
}

ACarlaHUD::ACarlaHUD() :
  bIsVisible(GetDefaultHUDVisibility())
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
    FCanvasTextItem HUDTextItem(FVector2D(HUDXRatio * 50.0f, HUDYRatio * 330.0f), Text, HUDFont, FLinearColor::White);
    HUDTextItem.Scale = ScaleVec;
    Canvas->DrawItem(HUDTextItem);
  }
}

#undef LOCTEXT_NAMESPACE
