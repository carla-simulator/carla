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

static FText GetHUDText(const ACarlaPlayerState &Vehicle)
{
  // Set number precision.
  FNumberFormattingOptions HighPrecision;
  HighPrecision.MinimumFractionalDigits = 2u;
  HighPrecision.MaximumFractionalDigits = 2u;

  FFormatNamedArguments Args;
  Args.Add("FPS", RoundedFloatAsText(Vehicle.GetFramesPerSecond()));
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
#ifdef CARLA_CINEMATIC_MODE
      LOCTEXT("HUDTextFormat",
          "Speed:         {Speed} km/h\n"
          "Gear:          {Gear}\n"
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
#else
      LOCTEXT("HUDTextFormat",
          "FPS: {FPS}\n"
          "\n"
          "Speed:         {Speed} km/h\n"
          "Gear:          {Gear}\n"
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
#endif // CARLA_CINEMATIC_MODE
      ,
      Args);
}

ACarlaHUD::ACarlaHUD() :
#ifdef CARLA_CINEMATIC_MODE
  bIsVisible(false)
#else
  bIsVisible(true)
#endif // CARLA_CINEMATIC_MODE
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

#undef LOCTEXT_NAMESPACE
