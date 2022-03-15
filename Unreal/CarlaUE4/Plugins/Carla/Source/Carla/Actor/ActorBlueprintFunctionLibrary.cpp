// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/LidarDescription.h"
#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Carla/Sensor/ShaderBasedSensor.h"
#include "Carla/Util/ScopedStack.h"

#include <algorithm>
#include <limits>
#include <stack>

/// Checks validity of FActorDefinition.
class FActorDefinitionValidator
{
public:

  /// Iterate all actor definitions and their properties and display messages on
  /// error.
  bool AreValid(const TArray<FActorDefinition> &ActorDefinitions)
  {
    return AreValid(TEXT("Actor Definition"), ActorDefinitions);
  }

  /// Validate @a ActorDefinition and display messages on error.
  bool SingleIsValid(const FActorDefinition &Definition)
  {
    auto ScopeText = FString::Printf(TEXT("[Actor Definition : %s]"), *Definition.Id);
    auto Scope = Stack.PushScope(ScopeText);
    return IsValid(Definition);
  }

private:

  /// If @a Predicate is false, print an error message. If possible the message
  /// is printed to the editor window.
  template <typename T, typename ... ARGS>
  bool OnScreenAssert(bool Predicate, const T &Format, ARGS && ... Args) const
  {
    if (!Predicate)
    {
      FString Message;
      for (auto &String : Stack)
      {
        Message += String;
      }
      Message += TEXT(" ");
      Message += FString::Printf(Format, std::forward<ARGS>(Args) ...);

      UE_LOG(LogCarla, Error, TEXT("%s"), *Message);
#if WITH_EDITOR
      if (GEngine)
      {
        GEngine->AddOnScreenDebugMessage(42, 15.0f, FColor::Red, Message);
      }
#endif // WITH_EDITOR
    }
    return Predicate;
  }

  template <typename T>
  FString GetDisplayId(const FString &Type, size_t Index, const T &Item)
  {
    return FString::Printf(TEXT("[%s %d : %s]"), *Type, Index, *Item.Id);
  }

  FString GetDisplayId(const FString &Type, size_t Index, const FString &Item)
  {
    return FString::Printf(TEXT("[%s %d : %s]"), *Type, Index, *Item);
  }

  /// Applies @a Validator to each item in @a Array. Pushes a new context to the
  /// stack for each item.
  template <typename T, typename F>
  bool ForEach(const FString &Type, const TArray<T> &Array, F Validator)
  {
    bool Result = true;
    auto Counter = 0u;
    for (const auto &Item : Array)
    {
      auto Scope = Stack.PushScope(GetDisplayId(Type, Counter, Item));
      Result &= Validator(Item);
      ++Counter;
    }
    return Result;
  }

  /// Applies @a IsValid to each item in @a Array. Pushes a new context to the
  /// stack for each item.
  template <typename T>
  bool AreValid(const FString &Type, const TArray<T> &Array)
  {
    return ForEach(Type, Array, [this](const auto &Item) { return IsValid(Item); });
  }

  bool IsIdValid(const FString &Id)
  {
    /// @todo Do more checks.
    return OnScreenAssert((!Id.IsEmpty() && Id != TEXT(".")), TEXT("Id cannot be empty"));
  }

  bool AreTagsValid(const FString &Tags)
  {
    /// @todo Do more checks.
    return OnScreenAssert(!Tags.IsEmpty(), TEXT("Tags cannot be empty"));
  }

  bool IsValid(const EActorAttributeType Type)
  {
    /// @todo Do more checks.
    return OnScreenAssert(Type < EActorAttributeType::SIZE, TEXT("Invalid type"));
  }

  bool ValueIsValid(const EActorAttributeType Type, const FString &Value)
  {
    /// @todo Do more checks.
    return true;
  }

  bool IsValid(const FActorVariation &Variation)
  {
    return
      IsIdValid(Variation.Id) &&
      IsValid(Variation.Type) &&
      OnScreenAssert(Variation.RecommendedValues.Num() > 0, TEXT("Recommended values cannot be empty")) &&
      ForEach(TEXT("Recommended Value"), Variation.RecommendedValues, [&](auto &Value) {
      return ValueIsValid(Variation.Type, Value);
    });
  }

  bool IsValid(const FActorAttribute &Attribute)
  {
    return
      IsIdValid(Attribute.Id) &&
      IsValid(Attribute.Type) &&
      ValueIsValid(Attribute.Type, Attribute.Value);
  }

  bool IsValid(const FActorDefinition &ActorDefinition)
  {
    /// @todo Validate Class and make sure IDs are not repeated.
    return
      IsIdValid(ActorDefinition.Id) &&
      AreTagsValid(ActorDefinition.Tags) &&
      AreValid(TEXT("Variation"), ActorDefinition.Variations) &&
      AreValid(TEXT("Attribute"), ActorDefinition.Attributes);
  }

  FScopedStack<FString> Stack;
};

template <typename ... ARGS>
static FString JoinStrings(const FString &Separator, ARGS && ... Args)
{
  return FString::Join(TArray<FString>{std::forward<ARGS>(Args) ...}, *Separator);
}

static FString ColorToFString(const FColor &Color)
{
  return JoinStrings(
      TEXT(","),
      FString::FromInt(Color.R),
      FString::FromInt(Color.G),
      FString::FromInt(Color.B));
}

/// ============================================================================
/// -- Actor definition validators ---------------------------------------------
/// ============================================================================

bool UActorBlueprintFunctionLibrary::CheckActorDefinition(const FActorDefinition &ActorDefinition)
{
  FActorDefinitionValidator Validator;
  return Validator.SingleIsValid(ActorDefinition);
}

bool UActorBlueprintFunctionLibrary::CheckActorDefinitions(const TArray<FActorDefinition> &ActorDefinitions)
{
  FActorDefinitionValidator Validator;
  return Validator.AreValid(ActorDefinitions);
}

/// ============================================================================
/// -- Helpers to create actor definitions -------------------------------------
/// ============================================================================

template <typename ... TStrs>
static void FillIdAndTags(FActorDefinition &Def, TStrs && ... Strings)
{
  Def.Id = JoinStrings(TEXT("."), std::forward<TStrs>(Strings) ...).ToLower();
  Def.Tags = JoinStrings(TEXT(","), std::forward<TStrs>(Strings) ...).ToLower();
  // each actor gets an actor role name attribute (empty by default)
  FActorVariation ActorRole;
  ActorRole.Id = TEXT("role_name");
  ActorRole.Type = EActorAttributeType::String;
  ActorRole.RecommendedValues = { TEXT("default") };
  ActorRole.bRestrictToRecommended = false;
  Def.Variations.Emplace(ActorRole);
}

static void AddRecommendedValuesForActorRoleName(
    FActorDefinition &Definition,
    TArray<FString> &&RecommendedValues)
{
  for (auto &&ActorVariation: Definition.Variations)
  {
    if (ActorVariation.Id == "role_name")
    {
      ActorVariation.RecommendedValues = RecommendedValues;
      return;
    }
  }
}

static void AddRecommendedValuesForSensorRoleNames(FActorDefinition &Definition)
{
  AddRecommendedValuesForActorRoleName(Definition, {TEXT("front"), TEXT("back"), TEXT("left"), TEXT(
      "right"), TEXT("front_left"), TEXT("front_right"), TEXT("back_left"), TEXT("back_right")});
}

static void AddVariationsForSensor(FActorDefinition &Def)
{
  FActorVariation Tick;

  Tick.Id = TEXT("sensor_tick");
  Tick.Type = EActorAttributeType::Float;
  Tick.RecommendedValues = { TEXT("0.0") };
  Tick.bRestrictToRecommended = false;

  Def.Variations.Emplace(Tick);
}

static void AddVariationsForTrigger(FActorDefinition &Def)
{
  // Friction
  FActorVariation Friction;
  Friction.Id = FString("friction");
  Friction.Type = EActorAttributeType::Float;
  Friction.RecommendedValues = { TEXT("3.5f") };
  Friction.bRestrictToRecommended = false;
  Def.Variations.Emplace(Friction);

  // Extent
  FString Extent("extent");
  FString Coordinates[3] = {FString("x"), FString("y"), FString("z")};

  for (auto Coordinate : Coordinates)
  {
    FActorVariation ExtentCoordinate;

    ExtentCoordinate.Id = JoinStrings(TEXT("_"), Extent, Coordinate);
    ExtentCoordinate.Type = EActorAttributeType::Float;
    ExtentCoordinate.RecommendedValues = { TEXT("1.0f") };
    ExtentCoordinate.bRestrictToRecommended = false;

    Def.Variations.Emplace(ExtentCoordinate);
  }
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeGenericDefinition(
    const FString &Category,
    const FString &Type,
    const FString &Id)
{
  FActorDefinition Definition;
  FillIdAndTags(Definition, Category, Type, Id);
  return Definition;
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(
    const FString &Type,
    const FString &Id)
{
  auto Definition = MakeGenericDefinition(TEXT("sensor"), Type, Id);
  AddRecommendedValuesForSensorRoleNames(Definition);
  return Definition;
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeCameraDefinition(
    const FString &Id,
    const bool bEnableModifyingPostProcessEffects)
{
  FActorDefinition Definition;
  bool Success;
  MakeCameraDefinition(Id, bEnableModifyingPostProcessEffects, Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeCameraDefinition(
    const FString &Id,
    const bool bEnableModifyingPostProcessEffects,
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("camera"), Id);
  AddRecommendedValuesForSensorRoleNames(Definition);
  AddVariationsForSensor(Definition);

  // FOV
  FActorVariation FOV;
  FOV.Id = TEXT("fov");
  FOV.Type = EActorAttributeType::Float;
  FOV.RecommendedValues = { TEXT("90.0") };
  FOV.bRestrictToRecommended = false;

  // Resolution
  FActorVariation ResX;
  ResX.Id = TEXT("image_size_x");
  ResX.Type = EActorAttributeType::Int;
  ResX.RecommendedValues = { TEXT("800") };
  ResX.bRestrictToRecommended = false;

  FActorVariation ResY;
  ResY.Id = TEXT("image_size_y");
  ResY.Type = EActorAttributeType::Int;
  ResY.RecommendedValues = { TEXT("600") };
  ResY.bRestrictToRecommended = false;

  // Lens parameters
  FActorVariation LensCircleFalloff;
  LensCircleFalloff.Id = TEXT("lens_circle_falloff");
  LensCircleFalloff.Type = EActorAttributeType::Float;
  LensCircleFalloff.RecommendedValues = { TEXT("5.0") };
  LensCircleFalloff.bRestrictToRecommended = false;

  FActorVariation LensCircleMultiplier;
  LensCircleMultiplier.Id = TEXT("lens_circle_multiplier");
  LensCircleMultiplier.Type = EActorAttributeType::Float;
  LensCircleMultiplier.RecommendedValues = { TEXT("0.0") };
  LensCircleMultiplier.bRestrictToRecommended = false;

  FActorVariation LensK;
  LensK.Id = TEXT("lens_k");
  LensK.Type = EActorAttributeType::Float;
  LensK.RecommendedValues = { TEXT("-1.0") };
  LensK.bRestrictToRecommended = false;

  FActorVariation LensKcube;
  LensKcube.Id = TEXT("lens_kcube");
  LensKcube.Type = EActorAttributeType::Float;
  LensKcube.RecommendedValues = { TEXT("0.0") };
  LensKcube.bRestrictToRecommended = false;

  FActorVariation LensXSize;
  LensXSize.Id = TEXT("lens_x_size");
  LensXSize.Type = EActorAttributeType::Float;
  LensXSize.RecommendedValues = { TEXT("0.08") };
  LensXSize.bRestrictToRecommended = false;

  FActorVariation LensYSize;
  LensYSize.Id = TEXT("lens_y_size");
  LensYSize.Type = EActorAttributeType::Float;
  LensYSize.RecommendedValues = { TEXT("0.08") };
  LensYSize.bRestrictToRecommended = false;

  Definition.Variations.Append({
      ResX,
      ResY,
      FOV,
      LensCircleFalloff,
      LensCircleMultiplier,
      LensK,
      LensKcube,
      LensXSize,
      LensYSize});

  if (bEnableModifyingPostProcessEffects)
  {
    FActorVariation PostProccess;
    PostProccess.Id = TEXT("enable_postprocess_effects");
    PostProccess.Type = EActorAttributeType::Bool;
    PostProccess.RecommendedValues = { TEXT("true") };
    PostProccess.bRestrictToRecommended = false;

    // Gamma
    FActorVariation Gamma;
    Gamma.Id = TEXT("gamma");
    Gamma.Type = EActorAttributeType::Float;
    Gamma.RecommendedValues = { TEXT("2.4") };
    Gamma.bRestrictToRecommended = false;

    // Motion Blur
    FActorVariation MBIntesity;
    MBIntesity.Id = TEXT("motion_blur_intensity");
    MBIntesity.Type = EActorAttributeType::Float;
    MBIntesity.RecommendedValues = { TEXT("0.45") };
    MBIntesity.bRestrictToRecommended = false;

    FActorVariation MBMaxDistortion;
    MBMaxDistortion.Id = TEXT("motion_blur_max_distortion");
    MBMaxDistortion.Type = EActorAttributeType::Float;
    MBMaxDistortion.RecommendedValues = { TEXT("0.35") };
    MBMaxDistortion.bRestrictToRecommended = false;

    FActorVariation MBMinObjectScreenSize;
    MBMinObjectScreenSize.Id = TEXT("motion_blur_min_object_screen_size");
    MBMinObjectScreenSize.Type = EActorAttributeType::Float;
    MBMinObjectScreenSize.RecommendedValues = { TEXT("0.1") };
    MBMinObjectScreenSize.bRestrictToRecommended = false;

    // Lens Flare
    FActorVariation LensFlareIntensity;
    LensFlareIntensity.Id = TEXT("lens_flare_intensity");
    LensFlareIntensity.Type = EActorAttributeType::Float;
    LensFlareIntensity.RecommendedValues = { TEXT("0.1") };
    LensFlareIntensity.bRestrictToRecommended = false;

    // Bloom
    FActorVariation BloomIntensity;
    BloomIntensity.Id = TEXT("bloom_intensity");
    BloomIntensity.Type = EActorAttributeType::Float;
    BloomIntensity.RecommendedValues = { TEXT("0.675") };
    BloomIntensity.bRestrictToRecommended = false;

    // More info at:
    // https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/AutomaticExposure/index.html
    // https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/DepthOfField/CinematicDOFMethods/index.html
    // https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/ColorGrading/index.html

    // Exposure
    FActorVariation ExposureMode;
    ExposureMode.Id = TEXT("exposure_mode");
    ExposureMode.Type = EActorAttributeType::String;
    ExposureMode.RecommendedValues = { TEXT("histogram"), TEXT("manual") };
    ExposureMode.bRestrictToRecommended = true;

    // Logarithmic adjustment for the exposure. Only used if a tonemapper is
    // specified.
    //  0 : no adjustment
    // -1 : 2x darker
    // -2 : 4x darker
    //  1 : 2x brighter
    //  2 : 4x brighter.
    FActorVariation ExposureCompensation;
    ExposureCompensation.Id = TEXT("exposure_compensation");
    ExposureCompensation.Type = EActorAttributeType::Float;
    ExposureCompensation.RecommendedValues = { TEXT("0.0") };
    ExposureCompensation.bRestrictToRecommended = false;

    // - Manual ------------------------------------------------

    // The formula used to compute the camera exposure scale is:
    // Exposure = 1 / (1.2 * 2^(log2( N²/t * 100/S )))

    // The camera shutter speed in seconds.
    FActorVariation ShutterSpeed; // (1/t)
    ShutterSpeed.Id = TEXT("shutter_speed");
    ShutterSpeed.Type = EActorAttributeType::Float;
    ShutterSpeed.RecommendedValues = { TEXT("200.0") };
    ShutterSpeed.bRestrictToRecommended = false;

    // The camera sensor sensitivity.
    FActorVariation ISO; // S
    ISO.Id = TEXT("iso");
    ISO.Type = EActorAttributeType::Float;
    ISO.RecommendedValues = { TEXT("100.0") };
    ISO.bRestrictToRecommended = false;

    // Defines the size of the opening for the camera lens.
    // Using larger numbers will reduce the DOF effect.
    FActorVariation Aperture; // N
    Aperture.Id = TEXT("fstop");
    Aperture.Type = EActorAttributeType::Float;
    Aperture.RecommendedValues = { TEXT("1.4") };
    Aperture.bRestrictToRecommended = false;

    // - Histogram ---------------------------------------------

    // The minimum brightness for auto exposure that limits the lower
    // brightness the eye can adapt within
    FActorVariation ExposureMinBright;
    ExposureMinBright.Id = TEXT("exposure_min_bright");
    ExposureMinBright.Type = EActorAttributeType::Float;
    ExposureMinBright.RecommendedValues = { TEXT("10.0") };
    ExposureMinBright.bRestrictToRecommended = false;

    // The maximum brightness for auto exposure that limits the upper
    // brightness the eye can adapt within
    FActorVariation ExposureMaxBright;
    ExposureMaxBright.Id = TEXT("exposure_max_bright");
    ExposureMaxBright.Type = EActorAttributeType::Float;
    ExposureMaxBright.RecommendedValues = { TEXT("12.0") };
    ExposureMaxBright.bRestrictToRecommended = false;

    // The speed at which the adaptation occurs from a dark environment
    // to a bright environment.
    FActorVariation ExposureSpeedUp;
    ExposureSpeedUp.Id = TEXT("exposure_speed_up");
    ExposureSpeedUp.Type = EActorAttributeType::Float;
    ExposureSpeedUp.RecommendedValues = { TEXT("3.0") };
    ExposureSpeedUp.bRestrictToRecommended = false;

    // The speed at which the adaptation occurs from a bright environment
    // to a dark environment.
    FActorVariation ExposureSpeedDown;
    ExposureSpeedDown.Id = TEXT("exposure_speed_down");
    ExposureSpeedDown.Type = EActorAttributeType::Float;
    ExposureSpeedDown.RecommendedValues = { TEXT("1.0") };
    ExposureSpeedDown.bRestrictToRecommended = false;

    // Calibration constant for 18% Albedo.
    FActorVariation CalibrationConstant;
    CalibrationConstant.Id = TEXT("calibration_constant");
    CalibrationConstant.Type = EActorAttributeType::Float;
    CalibrationConstant.RecommendedValues = { TEXT("16.0") };
    CalibrationConstant.bRestrictToRecommended = false;

    // Distance in which the Depth of Field effect should be sharp,
    // in unreal units (cm)
    FActorVariation FocalDistance;
    FocalDistance.Id = TEXT("focal_distance");
    FocalDistance.Type = EActorAttributeType::Float;
    FocalDistance.RecommendedValues = { TEXT("1000.0") };
    FocalDistance.bRestrictToRecommended = false;

    // Depth blur km for 50%
    FActorVariation DepthBlurAmount;
    DepthBlurAmount.Id = TEXT("blur_amount");
    DepthBlurAmount.Type = EActorAttributeType::Float;
    DepthBlurAmount.RecommendedValues = { TEXT("1.0") };
    DepthBlurAmount.bRestrictToRecommended = false;

    // Depth blur radius in pixels at 1920x
    FActorVariation DepthBlurRadius;
    DepthBlurRadius.Id = TEXT("blur_radius");
    DepthBlurRadius.Type = EActorAttributeType::Float;
    DepthBlurRadius.RecommendedValues = { TEXT("0.0") };
    DepthBlurRadius.bRestrictToRecommended = false;

    // Defines the opening of the camera lens, Aperture is 1.0/fstop,
    // typical lens go down to f/1.2 (large opening),
    // larger numbers reduce the DOF effect
    FActorVariation MaxAperture;
    MaxAperture.Id = TEXT("min_fstop");
    MaxAperture.Type = EActorAttributeType::Float;
    MaxAperture.RecommendedValues = { TEXT("1.2") };
    MaxAperture.bRestrictToRecommended = false;

    // Defines the number of blades of the diaphragm within the
    // lens (between 4 and 16)
    FActorVariation BladeCount;
    BladeCount.Id = TEXT("blade_count");
    BladeCount.Type = EActorAttributeType::Int;
    BladeCount.RecommendedValues = { TEXT("5") };
    BladeCount.bRestrictToRecommended = false;

    // - Tonemapper Settings -----------------------------------
    // You can adjust these tonemapper controls to emulate other
    // types of film stock for your project
    FActorVariation FilmSlope;
    FilmSlope.Id = TEXT("slope");
    FilmSlope.Type = EActorAttributeType::Float;
    FilmSlope.RecommendedValues = { TEXT("0.88") };
    FilmSlope.bRestrictToRecommended = false;

    FActorVariation FilmToe;
    FilmToe.Id = TEXT("toe");
    FilmToe.Type = EActorAttributeType::Float;
    FilmToe.RecommendedValues = { TEXT("0.55") };
    FilmToe.bRestrictToRecommended = false;

    FActorVariation FilmShoulder;
    FilmShoulder.Id = TEXT("shoulder");
    FilmShoulder.Type = EActorAttributeType::Float;
    FilmShoulder.RecommendedValues = { TEXT("0.26") };
    FilmShoulder.bRestrictToRecommended = false;

    FActorVariation FilmBlackClip;
    FilmBlackClip.Id = TEXT("black_clip");
    FilmBlackClip.Type = EActorAttributeType::Float;
    FilmBlackClip.RecommendedValues = { TEXT("0.0") };
    FilmBlackClip.bRestrictToRecommended = false;

    FActorVariation FilmWhiteClip;
    FilmWhiteClip.Id = TEXT("white_clip");
    FilmWhiteClip.Type = EActorAttributeType::Float;
    FilmWhiteClip.RecommendedValues = { TEXT("0.04") };
    FilmWhiteClip.bRestrictToRecommended = false;

    // Color
    FActorVariation Temperature;
    Temperature.Id = TEXT("temp");
    Temperature.Type = EActorAttributeType::Float;
    Temperature.RecommendedValues = { TEXT("6500.0") };
    Temperature.bRestrictToRecommended = false;

    FActorVariation Tint;
    Tint.Id = TEXT("tint");
    Tint.Type = EActorAttributeType::Float;
    Tint.RecommendedValues = { TEXT("0.0") };
    Tint.bRestrictToRecommended = false;

    FActorVariation ChromaticIntensity;
    ChromaticIntensity.Id = TEXT("chromatic_aberration_intensity");
    ChromaticIntensity.Type = EActorAttributeType::Float;
    ChromaticIntensity.RecommendedValues = { TEXT("0.0") };
    ChromaticIntensity.bRestrictToRecommended = false;

    FActorVariation ChromaticOffset;
    ChromaticOffset.Id = TEXT("chromatic_aberration_offset");
    ChromaticOffset.Type = EActorAttributeType::Float;
    ChromaticOffset.RecommendedValues = { TEXT("0.0") };
    ChromaticOffset.bRestrictToRecommended = false;

    Definition.Variations.Append({
      ExposureMode,
      ExposureCompensation,
      ShutterSpeed,
      ISO,
      Aperture,
      PostProccess,
      Gamma,
      MBIntesity,
      MBMaxDistortion,
      LensFlareIntensity,
      BloomIntensity,
      MBMinObjectScreenSize,
      ExposureMinBright,
      ExposureMaxBright,
      ExposureSpeedUp,
      ExposureSpeedDown,
      CalibrationConstant,
      FocalDistance,
      MaxAperture,
      BladeCount,
      DepthBlurAmount,
      DepthBlurRadius,
      FilmSlope,
      FilmToe,
      FilmShoulder,
      FilmBlackClip,
      FilmWhiteClip,
      Temperature,
      Tint,
      ChromaticIntensity,
      ChromaticOffset});
  }

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeNormalsCameraDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeNormalsCameraDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeNormalsCameraDefinition(bool &Success, FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("camera"), TEXT("normals"));
  AddRecommendedValuesForSensorRoleNames(Definition);
  AddVariationsForSensor(Definition);

  // FOV
  FActorVariation FOV;
  FOV.Id = TEXT("fov");
  FOV.Type = EActorAttributeType::Float;
  FOV.RecommendedValues = { TEXT("90.0") };
  FOV.bRestrictToRecommended = false;

  // Resolution
  FActorVariation ResX;
  ResX.Id = TEXT("image_size_x");
  ResX.Type = EActorAttributeType::Int;
  ResX.RecommendedValues = { TEXT("800") };
  ResX.bRestrictToRecommended = false;

  FActorVariation ResY;
  ResY.Id = TEXT("image_size_y");
  ResY.Type = EActorAttributeType::Int;
  ResY.RecommendedValues = { TEXT("600") };
  ResY.bRestrictToRecommended = false;

  // Lens parameters
  FActorVariation LensCircleFalloff;
  LensCircleFalloff.Id = TEXT("lens_circle_falloff");
  LensCircleFalloff.Type = EActorAttributeType::Float;
  LensCircleFalloff.RecommendedValues = { TEXT("5.0") };
  LensCircleFalloff.bRestrictToRecommended = false;

  FActorVariation LensCircleMultiplier;
  LensCircleMultiplier.Id = TEXT("lens_circle_multiplier");
  LensCircleMultiplier.Type = EActorAttributeType::Float;
  LensCircleMultiplier.RecommendedValues = { TEXT("0.0") };
  LensCircleMultiplier.bRestrictToRecommended = false;

  FActorVariation LensK;
  LensK.Id = TEXT("lens_k");
  LensK.Type = EActorAttributeType::Float;
  LensK.RecommendedValues = { TEXT("-1.0") };
  LensK.bRestrictToRecommended = false;

  FActorVariation LensKcube;
  LensKcube.Id = TEXT("lens_kcube");
  LensKcube.Type = EActorAttributeType::Float;
  LensKcube.RecommendedValues = { TEXT("0.0") };
  LensKcube.bRestrictToRecommended = false;

  FActorVariation LensXSize;
  LensXSize.Id = TEXT("lens_x_size");
  LensXSize.Type = EActorAttributeType::Float;
  LensXSize.RecommendedValues = { TEXT("0.08") };
  LensXSize.bRestrictToRecommended = false;

  FActorVariation LensYSize;
  LensYSize.Id = TEXT("lens_y_size");
  LensYSize.Type = EActorAttributeType::Float;
  LensYSize.RecommendedValues = { TEXT("0.08") };
  LensYSize.bRestrictToRecommended = false;

  Definition.Variations.Append({
      ResX,
      ResY,
      FOV,
      LensCircleFalloff,
      LensCircleMultiplier,
      LensK,
      LensKcube,
      LensXSize,
      LensYSize});

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeIMUDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeIMUDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeIMUDefinition(
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("imu"));
  AddVariationsForSensor(Definition);

  // - Noise seed --------------------------------
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;

  // - Accelerometer Standard Deviation ----------
  // X Component
  FActorVariation StdDevAccelX;
  StdDevAccelX.Id = TEXT("noise_accel_stddev_x");
  StdDevAccelX.Type = EActorAttributeType::Float;
  StdDevAccelX.RecommendedValues = { TEXT("0.0") };
  StdDevAccelX.bRestrictToRecommended = false;
  // Y Component
  FActorVariation StdDevAccelY;
  StdDevAccelY.Id = TEXT("noise_accel_stddev_y");
  StdDevAccelY.Type = EActorAttributeType::Float;
  StdDevAccelY.RecommendedValues = { TEXT("0.0") };
  StdDevAccelY.bRestrictToRecommended = false;
  // Z Component
  FActorVariation StdDevAccelZ;
  StdDevAccelZ.Id = TEXT("noise_accel_stddev_z");
  StdDevAccelZ.Type = EActorAttributeType::Float;
  StdDevAccelZ.RecommendedValues = { TEXT("0.0") };
  StdDevAccelZ.bRestrictToRecommended = false;

  // - Gyroscope Standard Deviation --------------
  // X Component
  FActorVariation StdDevGyroX;
  StdDevGyroX.Id = TEXT("noise_gyro_stddev_x");
  StdDevGyroX.Type = EActorAttributeType::Float;
  StdDevGyroX.RecommendedValues = { TEXT("0.0") };
  StdDevGyroX.bRestrictToRecommended = false;
  // Y Component
  FActorVariation StdDevGyroY;
  StdDevGyroY.Id = TEXT("noise_gyro_stddev_y");
  StdDevGyroY.Type = EActorAttributeType::Float;
  StdDevGyroY.RecommendedValues = { TEXT("0.0") };
  StdDevGyroY.bRestrictToRecommended = false;
  // Z Component
  FActorVariation StdDevGyroZ;
  StdDevGyroZ.Id = TEXT("noise_gyro_stddev_z");
  StdDevGyroZ.Type = EActorAttributeType::Float;
  StdDevGyroZ.RecommendedValues = { TEXT("0.0") };
  StdDevGyroZ.bRestrictToRecommended = false;

  // - Gyroscope Bias ----------------------------
  // X Component
  FActorVariation BiasGyroX;
  BiasGyroX.Id = TEXT("noise_gyro_bias_x");
  BiasGyroX.Type = EActorAttributeType::Float;
  BiasGyroX.RecommendedValues = { TEXT("0.0") };
  BiasGyroX.bRestrictToRecommended = false;
  // Y Component
  FActorVariation BiasGyroY;
  BiasGyroY.Id = TEXT("noise_gyro_bias_y");
  BiasGyroY.Type = EActorAttributeType::Float;
  BiasGyroY.RecommendedValues = { TEXT("0.0") };
  BiasGyroY.bRestrictToRecommended = false;
  // Z Component
  FActorVariation BiasGyroZ;
  BiasGyroZ.Id = TEXT("noise_gyro_bias_z");
  BiasGyroZ.Type = EActorAttributeType::Float;
  BiasGyroZ.RecommendedValues = { TEXT("0.0") };
  BiasGyroZ.bRestrictToRecommended = false;

  Definition.Variations.Append({
    NoiseSeed,
    StdDevAccelX,
    StdDevAccelY,
    StdDevAccelZ,
    StdDevGyroX,
    StdDevGyroY,
    StdDevGyroZ,
    BiasGyroX,
    BiasGyroY,
    BiasGyroZ});

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeRadarDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeRadarDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeRadarDefinition(
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("radar"));
  AddVariationsForSensor(Definition);

  FActorVariation HorizontalFOV;
  HorizontalFOV.Id = TEXT("horizontal_fov");
  HorizontalFOV.Type = EActorAttributeType::Float;
  HorizontalFOV.RecommendedValues = { TEXT("30") };
  HorizontalFOV.bRestrictToRecommended = false;

  FActorVariation VerticalFOV;
  VerticalFOV.Id = TEXT("vertical_fov");
  VerticalFOV.Type = EActorAttributeType::Float;
  VerticalFOV.RecommendedValues = { TEXT("30") };
  VerticalFOV.bRestrictToRecommended = false;

  FActorVariation Range;
  Range.Id = TEXT("range");
  Range.Type = EActorAttributeType::Float;
  Range.RecommendedValues = { TEXT("100") };
  Range.bRestrictToRecommended = false;

  FActorVariation PointsPerSecond;
  PointsPerSecond.Id = TEXT("points_per_second");
  PointsPerSecond.Type = EActorAttributeType::Int;
  PointsPerSecond.RecommendedValues = { TEXT("1500") };
  PointsPerSecond.bRestrictToRecommended = false;

  // Noise seed
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;

  Definition.Variations.Append({
    HorizontalFOV,
    VerticalFOV,
    Range,
    PointsPerSecond,
    NoiseSeed});

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeLidarDefinition(
    const FString &Id)
{
  FActorDefinition Definition;
  bool Success;
  MakeLidarDefinition(Id, Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeLidarDefinition(
    const FString &Id,
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("lidar"), Id);
  AddRecommendedValuesForSensorRoleNames(Definition);
  AddVariationsForSensor(Definition);
  // Number of channels.
  FActorVariation Channels;
  Channels.Id = TEXT("channels");
  Channels.Type = EActorAttributeType::Int;
  Channels.RecommendedValues = { TEXT("32") };
  // Range.
  FActorVariation Range;
  Range.Id = TEXT("range");
  Range.Type = EActorAttributeType::Float;
  Range.RecommendedValues = { TEXT("10.0") }; // 10 meters
  // Points per second.
  FActorVariation PointsPerSecond;
  PointsPerSecond.Id = TEXT("points_per_second");
  PointsPerSecond.Type = EActorAttributeType::Int;
  PointsPerSecond.RecommendedValues = { TEXT("56000") };
  // Frequency.
  FActorVariation Frequency;
  Frequency.Id = TEXT("rotation_frequency");
  Frequency.Type = EActorAttributeType::Float;
  Frequency.RecommendedValues = { TEXT("10.0") };
  // Upper FOV limit.
  FActorVariation UpperFOV;
  UpperFOV.Id = TEXT("upper_fov");
  UpperFOV.Type = EActorAttributeType::Float;
  UpperFOV.RecommendedValues = { TEXT("10.0") };
  // Lower FOV limit.
  FActorVariation LowerFOV;
  LowerFOV.Id = TEXT("lower_fov");
  LowerFOV.Type = EActorAttributeType::Float;
  LowerFOV.RecommendedValues = { TEXT("-30.0") };
  // Horizontal FOV.
  FActorVariation HorizontalFOV;
  HorizontalFOV.Id = TEXT("horizontal_fov");
  HorizontalFOV.Type = EActorAttributeType::Float;
  HorizontalFOV.RecommendedValues = { TEXT("360.0") };
  // Atmospheric Attenuation Rate.
  FActorVariation AtmospAttenRate;
  AtmospAttenRate.Id = TEXT("atmosphere_attenuation_rate");
  AtmospAttenRate.Type = EActorAttributeType::Float;
  AtmospAttenRate.RecommendedValues = { TEXT("0.004") };
  // Noise seed
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;
  // Dropoff General Rate
  FActorVariation DropOffGenRate;
  DropOffGenRate.Id = TEXT("dropoff_general_rate");
  DropOffGenRate.Type = EActorAttributeType::Float;
  DropOffGenRate.RecommendedValues = { TEXT("0.45") };
  // Dropoff intensity limit.
  FActorVariation DropOffIntensityLimit;
  DropOffIntensityLimit.Id = TEXT("dropoff_intensity_limit");
  DropOffIntensityLimit.Type = EActorAttributeType::Float;
  DropOffIntensityLimit.RecommendedValues = { TEXT("0.8") };
  // Dropoff at zero intensity.
  FActorVariation DropOffAtZeroIntensity;
  DropOffAtZeroIntensity.Id = TEXT("dropoff_zero_intensity");
  DropOffAtZeroIntensity.Type = EActorAttributeType::Float;
  DropOffAtZeroIntensity.RecommendedValues = { TEXT("0.4") };
  // Noise in lidar cloud points.
  FActorVariation StdDevLidar;
  StdDevLidar.Id = TEXT("noise_stddev");
  StdDevLidar.Type = EActorAttributeType::Float;
  StdDevLidar.RecommendedValues = { TEXT("0.0") };

  if (Id == "ray_cast") {
    Definition.Variations.Append({
      Channels,
      Range,
      PointsPerSecond,
      Frequency,
      UpperFOV,
      LowerFOV,
      AtmospAttenRate,
      NoiseSeed,
      DropOffGenRate,
      DropOffIntensityLimit,
      DropOffAtZeroIntensity,
      StdDevLidar,
      HorizontalFOV});
  }
  else if (Id == "ray_cast_semantic") {
    Definition.Variations.Append({
      Channels,
      Range,
      PointsPerSecond,
      Frequency,
      UpperFOV,
      LowerFOV,
      HorizontalFOV});
  }
  else {
    DEBUG_ASSERT(false);
  }

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeGnssDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeGnssDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeGnssDefinition(
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("gnss"));
  AddVariationsForSensor(Definition);

  // - Noise seed --------------------------------
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;

  // - Latitude ----------------------------------
  FActorVariation StdDevLat;
  StdDevLat.Id = TEXT("noise_lat_stddev");
  StdDevLat.Type = EActorAttributeType::Float;
  StdDevLat.RecommendedValues = { TEXT("0.0") };
  StdDevLat.bRestrictToRecommended = false;
  FActorVariation BiasLat;
  BiasLat.Id = TEXT("noise_lat_bias");
  BiasLat.Type = EActorAttributeType::Float;
  BiasLat.RecommendedValues = { TEXT("0.0") };
  BiasLat.bRestrictToRecommended = false;

  // - Longitude ---------------------------------
  FActorVariation StdDevLong;
  StdDevLong.Id = TEXT("noise_lon_stddev");
  StdDevLong.Type = EActorAttributeType::Float;
  StdDevLong.RecommendedValues = { TEXT("0.0") };
  StdDevLong.bRestrictToRecommended = false;
  FActorVariation BiasLong;
  BiasLong.Id = TEXT("noise_lon_bias");
  BiasLong.Type = EActorAttributeType::Float;
  BiasLong.RecommendedValues = { TEXT("0.0") };
  BiasLong.bRestrictToRecommended = false;

  // - Altitude ----------------------------------
  FActorVariation StdDevAlt;
  StdDevAlt.Id = TEXT("noise_alt_stddev");
  StdDevAlt.Type = EActorAttributeType::Float;
  StdDevAlt.RecommendedValues = { TEXT("0.0") };
  StdDevAlt.bRestrictToRecommended = false;
  FActorVariation BiasAlt;
  BiasAlt.Id = TEXT("noise_alt_bias");
  BiasAlt.Type = EActorAttributeType::Float;
  BiasAlt.RecommendedValues = { TEXT("0.0") };
  BiasAlt.bRestrictToRecommended = false;

  Definition.Variations.Append({
    NoiseSeed,
    StdDevLat,
    BiasLat,
    StdDevLong,
    BiasLong,
    StdDevAlt,
    BiasAlt});

  Success = CheckActorDefinition(Definition);
}

void UActorBlueprintFunctionLibrary::MakeVehicleDefinition(
    const FVehicleParameters &Parameters,
    bool &Success,
    FActorDefinition &Definition)
{
  /// @todo We need to validate here the params.
  FillIdAndTags(Definition, TEXT("vehicle"), Parameters.Make, Parameters.Model);
  AddRecommendedValuesForActorRoleName(Definition,
      {TEXT("autopilot"), TEXT("scenario"), TEXT("ego_vehicle")});
  Definition.Class = Parameters.Class;

  if (Parameters.RecommendedColors.Num() > 0)
  {
    FActorVariation Colors;
    Colors.Id = TEXT("color");
    Colors.Type = EActorAttributeType::RGBColor;
    Colors.bRestrictToRecommended = false;
    for (auto &Color : Parameters.RecommendedColors)
    {
      Colors.RecommendedValues.Emplace(ColorToFString(Color));
    }
    Definition.Variations.Emplace(Colors);
  }

  if (Parameters.SupportedDrivers.Num() > 0)
  {
    FActorVariation Drivers;
    Drivers.Id = TEXT("driver_id");
    Drivers.Type = EActorAttributeType::Int;
    Drivers.bRestrictToRecommended = true;
    for (auto &Id : Parameters.SupportedDrivers)
    {
      Drivers.RecommendedValues.Emplace(FString::FromInt(Id));
    }
    Definition.Variations.Emplace(Drivers);
  }

  FActorVariation StickyControl;
  StickyControl.Id = TEXT("sticky_control");
  StickyControl.Type = EActorAttributeType::Bool;
  StickyControl.bRestrictToRecommended = false;
  StickyControl.RecommendedValues.Emplace(TEXT("true"));
  Definition.Variations.Emplace(StickyControl);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("object_type"),
    EActorAttributeType::String,
    Parameters.ObjectType});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("base_type"),
    EActorAttributeType::String,
    Parameters.BaseType});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("special_type"),
    EActorAttributeType::String,
    Parameters.SpecialType});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("number_of_wheels"),
    EActorAttributeType::Int,
    FString::FromInt(Parameters.NumberOfWheels)});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("generation"),
    EActorAttributeType::Int,
    FString::FromInt(Parameters.Generation)});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("has_dynamic_doors"),
    EActorAttributeType::Bool,
    Parameters.HasDynamicDoors ? TEXT("true") : TEXT("false")});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("has_lights"),
    EActorAttributeType::Bool,
    Parameters.HasLights ? TEXT("true") : TEXT("false")});
  Success = CheckActorDefinition(Definition);
}

template <typename T, typename Functor>
static void FillActorDefinitionArray(
    const TArray<T> &ParameterArray,
    TArray<FActorDefinition> &Definitions,
    Functor Maker)
{
  for (auto &Item : ParameterArray)
  {
    FActorDefinition Definition;
    bool Success = false;
    Maker(Item, Success, Definition);
    if (Success)
    {
      Definitions.Emplace(std::move(Definition));
    }
  }
}

void UActorBlueprintFunctionLibrary::MakeVehicleDefinitions(
    const TArray<FVehicleParameters> &ParameterArray,
    TArray<FActorDefinition> &Definitions)
{
  FillActorDefinitionArray(ParameterArray, Definitions, &MakeVehicleDefinition);
}

void UActorBlueprintFunctionLibrary::MakePedestrianDefinition(
    const FPedestrianParameters &Parameters,
    bool &Success,
    FActorDefinition &Definition)
{
  /// @todo We need to validate here the params.
  FillIdAndTags(Definition, TEXT("walker"),  TEXT("pedestrian"), Parameters.Id);
  AddRecommendedValuesForActorRoleName(Definition, {TEXT("pedestrian")});
  Definition.Class = Parameters.Class;

  auto GetGender = [](EPedestrianGender Value) {
    switch (Value)
    {
      case EPedestrianGender::Female: return TEXT("female");
      case EPedestrianGender::Male:   return TEXT("male");
      default:                        return TEXT("other");
    }
  };

  auto GetAge = [](EPedestrianAge Value) {
    switch (Value)
    {
      case EPedestrianAge::Child:     return TEXT("child");
      case EPedestrianAge::Teenager:  return TEXT("teenager");
      case EPedestrianAge::Elderly:   return TEXT("elderly");
      default:                        return TEXT("adult");
    }
  };

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("gender"),
    EActorAttributeType::String,
    GetGender(Parameters.Gender)});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("generation"),
    EActorAttributeType::Int,
    FString::FromInt(Parameters.Generation)});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("age"),
    EActorAttributeType::String,
    GetAge(Parameters.Age)});

  if (Parameters.Speed.Num() > 0)
  {
    FActorVariation Speed;
    Speed.Id = TEXT("speed");
    Speed.Type = EActorAttributeType::Float;
    for (auto &Value : Parameters.Speed)
    {
      Speed.RecommendedValues.Emplace(FString::SanitizeFloat(Value));
    }
    Speed.bRestrictToRecommended = false;
    Definition.Variations.Emplace(Speed);
  }

  FActorVariation IsInvincible;
  IsInvincible.Id = TEXT("is_invincible");
  IsInvincible.Type = EActorAttributeType::Bool;
  IsInvincible.RecommendedValues = { TEXT("true") };
  IsInvincible.bRestrictToRecommended = false;
  Definition.Variations.Emplace(IsInvincible);

  Success = CheckActorDefinition(Definition);
}

void UActorBlueprintFunctionLibrary::MakePedestrianDefinitions(
    const TArray<FPedestrianParameters> &ParameterArray,
    TArray<FActorDefinition> &Definitions)
{
  FillActorDefinitionArray(ParameterArray, Definitions, &MakePedestrianDefinition);
}

void UActorBlueprintFunctionLibrary::MakeTriggerDefinitions(
    const TArray<FString> &ParameterArray,
    TArray<FActorDefinition> &Definitions)
{
  FillActorDefinitionArray(ParameterArray, Definitions, &MakeTriggerDefinition);
}

void UActorBlueprintFunctionLibrary::MakeTriggerDefinition(
    const FString &Id,
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("static"), TEXT("trigger"), Id);
  AddVariationsForTrigger(Definition);
  Success = CheckActorDefinition(Definition);
  check(Success);
}

void UActorBlueprintFunctionLibrary::MakePropDefinition(
    const FPropParameters &Parameters,
    bool &Success,
    FActorDefinition &Definition)
{
  /// @todo We need to validate here the params.
  FillIdAndTags(Definition, TEXT("static"),  TEXT("prop"), Parameters.Name);
  AddRecommendedValuesForActorRoleName(Definition, {TEXT("prop")});

  auto GetSize = [](EPropSize Value) {
    switch (Value)
    {
      case EPropSize::Tiny:    return TEXT("tiny");
      case EPropSize::Small:   return TEXT("small");
      case EPropSize::Medium:  return TEXT("medium");
      case EPropSize::Big:     return TEXT("big");
      case EPropSize::Huge:    return TEXT("huge");
      default:                 return TEXT("unknown");
    }
  };

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("size"),
    EActorAttributeType::String,
    GetSize(Parameters.Size)});

  Success = CheckActorDefinition(Definition);
}

void UActorBlueprintFunctionLibrary::MakePropDefinitions(
    const TArray<FPropParameters> &ParameterArray,
    TArray<FActorDefinition> &Definitions)
{
  FillActorDefinitionArray(ParameterArray, Definitions, &MakePropDefinition);
}

void UActorBlueprintFunctionLibrary::MakeObstacleDetectorDefinitions(
    const FString &Type,
    const FString &Id,
    FActorDefinition &Definition)
{
  Definition = MakeGenericSensorDefinition(TEXT("other"), TEXT("obstacle"));
  AddVariationsForSensor(Definition);
  // Distance.
  FActorVariation distance;
  distance.Id = TEXT("distance");
  distance.Type = EActorAttributeType::Float;
  distance.RecommendedValues = { TEXT("5.0") };
  distance.bRestrictToRecommended = false;
  // HitRadius.
  FActorVariation hitradius;
  hitradius.Id = TEXT("hit_radius");
  hitradius.Type = EActorAttributeType::Float;
  hitradius.RecommendedValues = { TEXT("0.5") };
  hitradius.bRestrictToRecommended = false;
  // Only Dynamics
  FActorVariation onlydynamics;
  onlydynamics.Id = TEXT("only_dynamics");
  onlydynamics.Type = EActorAttributeType::Bool;
  onlydynamics.RecommendedValues = { TEXT("false") };
  onlydynamics.bRestrictToRecommended = false;
  // Debug Line Trace
  FActorVariation debuglinetrace;
  debuglinetrace.Id = TEXT("debug_linetrace");
  debuglinetrace.Type = EActorAttributeType::Bool;
  debuglinetrace.RecommendedValues = { TEXT("false") };
  debuglinetrace.bRestrictToRecommended = false;

  Definition.Variations.Append({
    distance,
    hitradius,
    onlydynamics,
    debuglinetrace
  });

}
/// ============================================================================
/// -- Helpers to retrieve attribute values ------------------------------------
/// ============================================================================

bool UActorBlueprintFunctionLibrary::ActorAttributeToBool(
    const FActorAttribute &ActorAttribute,
    bool Default)
{
  if (ActorAttribute.Type != EActorAttributeType::Bool)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not a bool"), *ActorAttribute.Id);
    return Default;
  }
  return ActorAttribute.Value.ToBool();
}

int32 UActorBlueprintFunctionLibrary::ActorAttributeToInt(
    const FActorAttribute &ActorAttribute,
    int32 Default)
{
  if (ActorAttribute.Type != EActorAttributeType::Int)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not an int"), *ActorAttribute.Id);
    return Default;
  }
  return FCString::Atoi(*ActorAttribute.Value);
}

float UActorBlueprintFunctionLibrary::ActorAttributeToFloat(
    const FActorAttribute &ActorAttribute,
    float Default)
{
  if (ActorAttribute.Type != EActorAttributeType::Float)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not a float"), *ActorAttribute.Id);
    return Default;
  }
  return FCString::Atof(*ActorAttribute.Value);
}

FString UActorBlueprintFunctionLibrary::ActorAttributeToString(
    const FActorAttribute &ActorAttribute,
    const FString &Default)
{
  if (ActorAttribute.Type != EActorAttributeType::String)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not a string"), *ActorAttribute.Id);
    return Default;
  }
  return ActorAttribute.Value;
}

FColor UActorBlueprintFunctionLibrary::ActorAttributeToColor(
    const FActorAttribute &ActorAttribute,
    const FColor &Default)
{
  if (ActorAttribute.Type != EActorAttributeType::RGBColor)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not a color"), *ActorAttribute.Id);
    return Default;
  }
  TArray<FString> Channels;
  ActorAttribute.Value.ParseIntoArray(Channels, TEXT(","), false);
  if (Channels.Num() != 3)
  {
    UE_LOG(LogCarla,
        Error,
        TEXT("ActorAttribute '%s': invalid color '%s'"),
        *ActorAttribute.Id,
        *ActorAttribute.Value);
    return Default;
  }
  TArray<uint8> Colors;
  for (auto &Str : Channels)
  {
    auto Val = FCString::Atoi(*Str);
    if ((Val < 0) || (Val > std::numeric_limits<uint8>::max()))
    {
      UE_LOG(LogCarla,
          Error,
          TEXT("ActorAttribute '%s': invalid color '%s'"),
          *ActorAttribute.Id,
          *ActorAttribute.Value);
      return Default;
    }
    Colors.Add(Val);
  }
  FColor Color;
  Color.R = Colors[0u];
  Color.G = Colors[1u];
  Color.B = Colors[2u];
  return Color;
}

bool UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    bool Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToBool(Attributes[Id], Default) :
         Default;
}

int32 UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    int32 Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToInt(Attributes[Id], Default) :
         Default;
}

float UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    float Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToFloat(Attributes[Id], Default) :
         Default;
}

FString UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    const FString &Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToString(Attributes[Id], Default) :
         Default;
}

FColor UActorBlueprintFunctionLibrary::RetrieveActorAttributeToColor(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    const FColor &Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToColor(Attributes[Id], Default) :
         Default;
}

/// ============================================================================
/// -- Helpers to set Actors ---------------------------------------------------
/// ============================================================================

// Here we do different checks when we are in editor because we don't want the
// editor crashing while people are testing new actor definitions.
#if WITH_EDITOR
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr)                    \
  if ((ActorPtr == nullptr) || ActorPtr->IsPendingKill())     \
  {                                                           \
    UE_LOG(LogCarla, Error, TEXT("Cannot set empty actor!")); \
    return;                                                   \
  }
#else
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr) \
  check((ActorPtr != nullptr) && !ActorPtr->IsPendingKill());
#endif // WITH_EDITOR

void UActorBlueprintFunctionLibrary::SetCamera(
    const FActorDescription &Description,
    ASceneCaptureSensor *Camera)
{
  CARLA_ABFL_CHECK_ACTOR(Camera);
  Camera->SetImageSize(
      RetrieveActorAttributeToInt("image_size_x", Description.Variations, 800),
      RetrieveActorAttributeToInt("image_size_y", Description.Variations, 600));
  Camera->SetFOVAngle(
      RetrieveActorAttributeToFloat("fov", Description.Variations, 90.0f));
  if (Description.Variations.Contains("enable_postprocess_effects"))
  {
    Camera->EnablePostProcessingEffects(
        ActorAttributeToBool(
        Description.Variations["enable_postprocess_effects"],
        true));
    Camera->SetTargetGamma(
        RetrieveActorAttributeToFloat("gamma", Description.Variations, 2.4f));
    Camera->SetMotionBlurIntensity(
        RetrieveActorAttributeToFloat("motion_blur_intensity", Description.Variations, 0.5f));
    Camera->SetMotionBlurMaxDistortion(
        RetrieveActorAttributeToFloat("motion_blur_max_distortion", Description.Variations, 5.0f));
    Camera->SetMotionBlurMinObjectScreenSize(
        RetrieveActorAttributeToFloat("motion_blur_min_object_screen_size", Description.Variations, 0.5f));
    Camera->SetLensFlareIntensity(
        RetrieveActorAttributeToFloat("lens_flare_intensity", Description.Variations, 0.1f));
    Camera->SetBloomIntensity(
        RetrieveActorAttributeToFloat("bloom_intensity", Description.Variations, 0.675f));
    // Exposure, histogram mode by default
    if (RetrieveActorAttributeToString("exposure_mode", Description.Variations, "histogram") == "histogram")
    {
      Camera->SetExposureMethod(EAutoExposureMethod::AEM_Histogram);
    }
    else
    {
      Camera->SetExposureMethod(EAutoExposureMethod::AEM_Manual);
    }
    Camera->SetExposureCompensation(
        RetrieveActorAttributeToFloat("exposure_compensation", Description.Variations, 0.0f));
    Camera->SetShutterSpeed(
        RetrieveActorAttributeToFloat("shutter_speed", Description.Variations, 200.0f));
    Camera->SetISO(
        RetrieveActorAttributeToFloat("iso", Description.Variations, 100.0f));
    Camera->SetAperture(
        RetrieveActorAttributeToFloat("fstop", Description.Variations, 1.4f));

    Camera->SetExposureMinBrightness(
        RetrieveActorAttributeToFloat("exposure_min_bright", Description.Variations, 7.0f));
    Camera->SetExposureMaxBrightness(
        RetrieveActorAttributeToFloat("exposure_max_bright", Description.Variations, 9.0f));
    Camera->SetExposureSpeedUp(
        RetrieveActorAttributeToFloat("exposure_speed_up", Description.Variations, 3.0f));
    Camera->SetExposureSpeedDown(
        RetrieveActorAttributeToFloat("exposure_speed_down", Description.Variations, 1.0f));
    Camera->SetExposureCalibrationConstant(
        RetrieveActorAttributeToFloat("calibration_constant", Description.Variations, 16.0f));

    Camera->SetFocalDistance(
        RetrieveActorAttributeToFloat("focal_distance", Description.Variations, 1000.0f));
    Camera->SetDepthBlurAmount(
        RetrieveActorAttributeToFloat("blur_amount", Description.Variations, 1.0f));
    Camera->SetDepthBlurRadius(
        RetrieveActorAttributeToFloat("blur_radius", Description.Variations, 0.0f));
    Camera->SetDepthOfFieldMinFstop(
        RetrieveActorAttributeToFloat("min_fstop", Description.Variations, 1.2f));
    Camera->SetBladeCount(
        RetrieveActorAttributeToInt("blade_count", Description.Variations, 5));

    Camera->SetFilmSlope(
        RetrieveActorAttributeToFloat("slope", Description.Variations, 0.88f));
    Camera->SetFilmToe(
        RetrieveActorAttributeToFloat("toe", Description.Variations, 0.55f));
    Camera->SetFilmShoulder(
        RetrieveActorAttributeToFloat("shoulder", Description.Variations, 0.26f));
    Camera->SetFilmBlackClip(
        RetrieveActorAttributeToFloat("black_clip", Description.Variations, 0.0f));
    Camera->SetFilmWhiteClip(
        RetrieveActorAttributeToFloat("white_clip", Description.Variations, 0.04f));

    Camera->SetWhiteTemp(
        RetrieveActorAttributeToFloat("temp", Description.Variations, 6500.0f));
    Camera->SetWhiteTint(
        RetrieveActorAttributeToFloat("tint", Description.Variations, 0.0f));

    Camera->SetChromAberrIntensity(
        RetrieveActorAttributeToFloat("chromatic_aberration_intensity", Description.Variations, 0.0f));
    Camera->SetChromAberrOffset(
        RetrieveActorAttributeToFloat("chromatic_aberration_offset", Description.Variations, 0.0f));
  }
}

void UActorBlueprintFunctionLibrary::SetCamera(
    const FActorDescription &Description,
    AShaderBasedSensor *Camera)
{
  CARLA_ABFL_CHECK_ACTOR(Camera);
  Camera->SetFloatShaderParameter(0, TEXT("CircleFalloff_NState"),
      RetrieveActorAttributeToFloat("lens_circle_falloff", Description.Variations, 5.0f));
  Camera->SetFloatShaderParameter(0, TEXT("CircleMultiplier_NState"),
      RetrieveActorAttributeToFloat("lens_circle_multiplier", Description.Variations, 0.0f));
  Camera->SetFloatShaderParameter(0, TEXT("K_NState"),
      RetrieveActorAttributeToFloat("lens_k", Description.Variations, -1.0f));
  Camera->SetFloatShaderParameter(0, TEXT("kcube"),
      RetrieveActorAttributeToFloat("lens_kcube", Description.Variations, 0.0f));
  Camera->SetFloatShaderParameter(0, TEXT("XSize_NState"),
      RetrieveActorAttributeToFloat("lens_x_size", Description.Variations, 0.08f));
  Camera->SetFloatShaderParameter(0, TEXT("YSize_NState"),
      RetrieveActorAttributeToFloat("lens_y_size", Description.Variations, 0.08f));
}

void UActorBlueprintFunctionLibrary::SetLidar(
    const FActorDescription &Description,
    FLidarDescription &Lidar)
{
  constexpr float TO_CENTIMETERS = 1e2;
  Lidar.Channels =
      RetrieveActorAttributeToInt("channels", Description.Variations, Lidar.Channels);
  Lidar.Range =
      RetrieveActorAttributeToFloat("range", Description.Variations, 10.0f) * TO_CENTIMETERS;
  Lidar.PointsPerSecond =
      RetrieveActorAttributeToInt("points_per_second", Description.Variations, Lidar.PointsPerSecond);
  Lidar.RotationFrequency =
      RetrieveActorAttributeToFloat("rotation_frequency", Description.Variations, Lidar.RotationFrequency);
  Lidar.UpperFovLimit =
      RetrieveActorAttributeToFloat("upper_fov", Description.Variations, Lidar.UpperFovLimit);
  Lidar.LowerFovLimit =
      RetrieveActorAttributeToFloat("lower_fov", Description.Variations, Lidar.LowerFovLimit);
  Lidar.HorizontalFov =
      RetrieveActorAttributeToFloat("horizontal_fov", Description.Variations, Lidar.HorizontalFov);
  Lidar.AtmospAttenRate =
      RetrieveActorAttributeToFloat("atmosphere_attenuation_rate", Description.Variations, Lidar.AtmospAttenRate);
  Lidar.RandomSeed =
      RetrieveActorAttributeToInt("noise_seed", Description.Variations, Lidar.RandomSeed);
  Lidar.DropOffGenRate =
      RetrieveActorAttributeToFloat("dropoff_general_rate", Description.Variations, Lidar.DropOffGenRate);
  Lidar.DropOffIntensityLimit =
      RetrieveActorAttributeToFloat("dropoff_intensity_limit", Description.Variations, Lidar.DropOffIntensityLimit);
  Lidar.DropOffAtZeroIntensity =
      RetrieveActorAttributeToFloat("dropoff_zero_intensity", Description.Variations, Lidar.DropOffAtZeroIntensity);
  Lidar.NoiseStdDev =
      RetrieveActorAttributeToFloat("noise_stddev", Description.Variations, Lidar.NoiseStdDev);
}

void UActorBlueprintFunctionLibrary::SetGnss(
    const FActorDescription &Description,
    AGnssSensor *Gnss)
{
  CARLA_ABFL_CHECK_ACTOR(Gnss);
  if (Description.Variations.Contains("noise_seed"))
  {
    Gnss->SetSeed(
      RetrieveActorAttributeToInt("noise_seed", Description.Variations, 0));
  }
  else
  {
    Gnss->SetSeed(Gnss->GetRandomEngine()->GenerateRandomSeed());
  }

  Gnss->SetLatitudeDeviation(
      RetrieveActorAttributeToFloat("noise_lat_stddev", Description.Variations, 0.0f));
  Gnss->SetLongitudeDeviation(
      RetrieveActorAttributeToFloat("noise_lon_stddev", Description.Variations, 0.0f));
  Gnss->SetAltitudeDeviation(
      RetrieveActorAttributeToFloat("noise_alt_stddev", Description.Variations, 0.0f));
  Gnss->SetLatitudeBias(
      RetrieveActorAttributeToFloat("noise_lat_bias", Description.Variations, 0.0f));
  Gnss->SetLongitudeBias(
      RetrieveActorAttributeToFloat("noise_lon_bias", Description.Variations, 0.0f));
  Gnss->SetAltitudeBias(
      RetrieveActorAttributeToFloat("noise_alt_bias", Description.Variations, 0.0f));
}

void UActorBlueprintFunctionLibrary::SetIMU(
    const FActorDescription &Description,
    AInertialMeasurementUnit *IMU)
{
  CARLA_ABFL_CHECK_ACTOR(IMU);
  if (Description.Variations.Contains("noise_seed"))
  {
    IMU->SetSeed(
        RetrieveActorAttributeToInt("noise_seed", Description.Variations, 0));
  }
  else
  {
    IMU->SetSeed(IMU->GetRandomEngine()->GenerateRandomSeed());
  }

  IMU->SetAccelerationStandardDeviation({
      RetrieveActorAttributeToFloat("noise_accel_stddev_x", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_accel_stddev_y", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_accel_stddev_z", Description.Variations, 0.0f)});

  IMU->SetGyroscopeStandardDeviation({
      RetrieveActorAttributeToFloat("noise_gyro_stddev_x", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_gyro_stddev_y", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_gyro_stddev_z", Description.Variations, 0.0f)});

  IMU->SetGyroscopeBias({
      RetrieveActorAttributeToFloat("noise_gyro_bias_x", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_gyro_bias_y", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_gyro_bias_z", Description.Variations, 0.0f)});
}

void UActorBlueprintFunctionLibrary::SetRadar(
    const FActorDescription &Description,
    ARadar *Radar)
{
  CARLA_ABFL_CHECK_ACTOR(Radar);
  constexpr float TO_CENTIMETERS = 1e2;

  if (Description.Variations.Contains("noise_seed"))
  {
    Radar->SetSeed(
      RetrieveActorAttributeToInt("noise_seed", Description.Variations, 0));
  }
  else
  {
    Radar->SetSeed(Radar->GetRandomEngine()->GenerateRandomSeed());
  }

  Radar->SetHorizontalFOV(
      RetrieveActorAttributeToFloat("horizontal_fov", Description.Variations, 30.0f));
  Radar->SetVerticalFOV(
      RetrieveActorAttributeToFloat("vertical_fov", Description.Variations, 30.0f));
  Radar->SetRange(
      RetrieveActorAttributeToFloat("range", Description.Variations, 100.0f) * TO_CENTIMETERS);
  Radar->SetPointsPerSecond(
      RetrieveActorAttributeToInt("points_per_second", Description.Variations, 1500));
}

#undef CARLA_ABFL_CHECK_ACTOR
