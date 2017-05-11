// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "DynamicWeather.h"

#include "Components/ArrowComponent.h"

ADynamicWeather::ADynamicWeather(const FObjectInitializer& ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent0"));

#if WITH_EDITORONLY_DATA
  ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent0"));
  if (ArrowComponent) {
    ArrowComponent->ArrowColor = FColor(150, 200, 255);
    ArrowComponent->bTreatAsASprite = true;
    ArrowComponent->SpriteInfo.Category = TEXT("Lighting");
    ArrowComponent->SpriteInfo.DisplayName = NSLOCTEXT( "SpriteCategory", "Lighting", "Lighting" );
    ArrowComponent->SetupAttachment(RootComponent);
    ArrowComponent->bLightAttachment = true;
    ArrowComponent->bIsScreenSizeScaled = true;
  }
#endif // WITH_EDITORONLY_DATA
}

void ADynamicWeather::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);
  Update();
}

#if WITH_EDITOR

void ADynamicWeather::PostEditChangeProperty(FPropertyChangedEvent &Event)
{
  Super::PostEditChangeProperty(Event);
  const FName PropertyName = (Event.Property != NULL ? Event.Property->GetFName() : NAME_None);
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ADynamicWeather, Weather)) {
    Update();
  } else {
    AdjustSunPositionBasedOnActorRotation();
  }
}

void ADynamicWeather::EditorApplyRotation(
    const FRotator &DeltaRotation,
    bool bAltDown,
    bool bShiftDown,
    bool bCtrlDown)
{
  Super::EditorApplyRotation(DeltaRotation, bAltDown, bShiftDown, bCtrlDown);
  AdjustSunPositionBasedOnActorRotation();
}

#endif // WITH_EDITOR

FVector ADynamicWeather::GetSunDirection() const
{
  const FVector2D SphericalCoords(
      FMath::DegreesToRadians(Weather.SunPolarAngle),
      FMath::DegreesToRadians(Weather.SunAzimuthAngle));
  return - SphericalCoords.SphericalToUnitCartesian();
}

void ADynamicWeather::Update()
{
  // Modify this actor's rotation according to Sun position.
  if (!SetActorRotation(FQuat(GetSunDirection().Rotation()), ETeleportType::None)) {
    UE_LOG(LogCarla, Warning, TEXT("Unable to rotate actor"));
  }

  RefreshWeather();
}

void ADynamicWeather::AdjustSunPositionBasedOnActorRotation()
{
  const FVector Direction = - GetActorQuat().GetForwardVector();
  const FVector2D SphericalCoords = Direction.UnitCartesianToSpherical();
  Weather.SunPolarAngle = FMath::RadiansToDegrees(SphericalCoords.X);
  Weather.SunAzimuthAngle = FMath::RadiansToDegrees(SphericalCoords.Y);
}
