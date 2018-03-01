// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "DynamicWeather.h"

#include "Util/IniFile.h"

#include "Components/ArrowComponent.h"

static FString GetIniFileName(const FString &MapName = TEXT(""))
{
  const FString BaseName = TEXT("CarlaWeather");
  constexpr auto Sep = TEXT(".");
  constexpr auto Ext = TEXT(".ini");
  return (MapName.IsEmpty() ? (BaseName + Ext) : (BaseName + Sep + MapName + Ext));
}

static bool GetWeatherIniFilePath(const FString &FileName, FString &FilePath)
{
  FilePath = FPaths::Combine(FPaths::ProjectConfigDir(), FileName);
  const bool bFileExists = FPaths::FileExists(FilePath);
  if (!bFileExists) {
    UE_LOG(LogCarla, Warning, TEXT("\"%s\" not found"), *FilePath);
  }
  return bFileExists;
}

static bool CheckWeatherValidity(const FWeatherDescription &Weather)
{
  if (Weather.Name.IsEmpty()) {
    UE_LOG(LogCarla, Error, TEXT("Weather doesn't have a name, please provide one"));
    return false;
  }
  return true;
}

void ADynamicWeather::LoadWeatherDescriptionsFromFile(
    const FString &MapName,
    TArray<FWeatherDescription> &Descriptions)
{
  // Try to load config file.
  FString DefaultFilePath;
  if (GetWeatherIniFilePath(GetIniFileName(), DefaultFilePath)) {
    UE_LOG(LogCarla, Log, TEXT("Loading weather description from %s"), *DefaultFilePath);
    FIniFile ConfigFile(DefaultFilePath);

    { // Override map specific presets.
      FString MapOverridesFilePath;
      if (GetWeatherIniFilePath(GetIniFileName(MapName), MapOverridesFilePath)) {
        UE_LOG(LogCarla, Log, TEXT("Loading weather description from %s"), *MapOverridesFilePath);
        ConfigFile.Combine(MapOverridesFilePath);
      }
    }

    // For every section in the config file add a weather description.
    for (auto &Item : ConfigFile.GetFConfigFile()) {
      Descriptions.AddDefaulted(1u);
      Descriptions.Last().ReadFromConfigFile(ConfigFile, Item.Key);
    }
  }

  // If no description was found, append a defaulted one.
  if (Descriptions.Num() == 0) {
    UE_LOG(LogCarla, Warning, TEXT("No weather description found"));
    Descriptions.AddDefaulted(1u);
    Descriptions.Last().Name = TEXT("Default");
  }
}

ADynamicWeather::ADynamicWeather(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
#if WITH_EDITORONLY_DATA
  , FileName(GetIniFileName())
#endif // WITH_EDITORONLY_DATA
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
#if WITH_EDITOR
  Update();
#endif // WITH_EDITOR
}

void ADynamicWeather::BeginPlay()
{
  Super::BeginPlay();
#if WITH_EDITOR
  Update();
#endif // WITH_EDITOR
}

#if WITH_EDITOR

void ADynamicWeather::PostEditChangeProperty(FPropertyChangedEvent &Event)
{
  Super::PostEditChangeProperty(Event);
  const FName PropertyName = (Event.Property != NULL ? Event.Property->GetFName() : NAME_None);
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ADynamicWeather, Weather)) {
    Update();
  } else if ((PropertyName == GET_MEMBER_NAME_CHECKED(ADynamicWeather, bSaveToConfigFile)) ||
             (PropertyName == GET_MEMBER_NAME_CHECKED(ADynamicWeather, bLoadFromConfigFile))) {
    // Do nothing.
  } else {
    AdjustSunPositionBasedOnActorRotation();
  }
  if (bSaveToConfigFile) {
    bSaveToConfigFile = false;
    if (SaveToConfigFile()) {
      UE_LOG(LogCarla, Log, TEXT("Weather \"%s\" saved to config file"), *Weather.Name);
    } else {
      UE_LOG(LogCarla, Error, TEXT("Error saving weather to config file"));
    }
  }
  if (bLoadFromConfigFile) {
    bLoadFromConfigFile = false;
    if (LoadFromConfigFile()) {
      UE_LOG(LogCarla, Log, TEXT("Weather \"%s\" loaded from config file"), *Weather.Name);
      Update();
    } else {
      UE_LOG(LogCarla, Error, TEXT("Error loading weather from config file"));
    }
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

void ADynamicWeather::AdjustSunPositionBasedOnActorRotation()
{
  const FVector Direction = - GetActorQuat().GetForwardVector();
  const FVector2D SphericalCoords = Direction.UnitCartesianToSpherical();
  Weather.SunPolarAngle = FMath::RadiansToDegrees(SphericalCoords.X);
  Weather.SunAzimuthAngle = FMath::RadiansToDegrees(SphericalCoords.Y);
}

#if WITH_EDITOR

void ADynamicWeather::Update()
{
  // Modify this actor's rotation according to Sun position.
  if (!SetActorRotation(FQuat(GetSunDirection().Rotation()), ETeleportType::None)) {
    UE_LOG(LogCarla, Warning, TEXT("Unable to rotate actor"));
  }

  if (bRefreshAutomatically) {
    RefreshWeather();
  }
}

bool ADynamicWeather::LoadFromConfigFile()
{
  FString FilePath;
  if (GetWeatherIniFilePath(FileName, FilePath) && CheckWeatherValidity(Weather)) {
    FIniFile ConfigFile(FilePath);
    if (!ConfigFile.HasSection(Weather.Name)) {
      UE_LOG(LogCarla, Error, TEXT("Weather \"%s\" is not present in config file"), *Weather.Name);
      return false;
    }
    Weather.ReadFromConfigFile(ConfigFile, Weather.Name);
    return true;
  } else {
    return false;
  }
}

bool ADynamicWeather::SaveToConfigFile() const
{
  FString FilePath;
  if (GetWeatherIniFilePath(FileName, FilePath) && CheckWeatherValidity(Weather)) {
    FIniFile ConfigFile(FilePath);
    Weather.WriteToConfigFile(ConfigFile);
    return ConfigFile.Write(FilePath);
  } else {
    return false;
  }
}

#endif // WITH_EDITOR
