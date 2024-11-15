// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/DVSCamera.h"
#include "Carla.h"
#include "Carla/Util/RandomEngine.h"

#include <util/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <carla/Buffer.h>
#include <carla/BufferView.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Actor/ActorBlueprintFunctionLibrary.h"
#include <util/ue-header-guard-end.h>

#include <random>
#include <cmath>
#include <algorithm>

static float FColorToGrayScaleFloat(FColor Color)
{
  return 0.2989 * Color.R + 0.587 * Color.G + 0.114 * Color.B;
}

ADVSCamera::ADVSCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  EnablePostProcessingEffects(true);
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));

  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
}

FActorDefinition ADVSCamera::GetSensorDefinition()
{
  constexpr bool bEnableModifyingPostProcessEffects = true;
  auto Definition = UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("dvs"), bEnableModifyingPostProcessEffects);

  FActorVariation Cp;
  Cp.Id = TEXT("positive_threshold");
  Cp.Type = EActorAttributeType::Float;
  Cp.RecommendedValues = { TEXT("0.3") };
  Cp.bRestrictToRecommended = false;

  FActorVariation Cm;
  Cm.Id = TEXT("negative_threshold");
  Cm.Type = EActorAttributeType::Float;
  Cm.RecommendedValues = { TEXT("0.3") };
  Cm.bRestrictToRecommended = false;

  FActorVariation Sigma_Cp;
  Sigma_Cp.Id = TEXT("sigma_positive_threshold");
  Sigma_Cp.Type = EActorAttributeType::Float;
  Sigma_Cp.RecommendedValues = { TEXT("0.0") };
  Sigma_Cp.bRestrictToRecommended = false;

  FActorVariation Sigma_Cm;
  Sigma_Cm.Id = TEXT("sigma_negative_threshold");
  Sigma_Cm.Type = EActorAttributeType::Float;
  Sigma_Cm.RecommendedValues = { TEXT("0.0") };
  Sigma_Cm.bRestrictToRecommended = false;

  FActorVariation Refractory_Period;
  Refractory_Period.Id = TEXT("refractory_period_ns");
  Refractory_Period.Type = EActorAttributeType::Int;
  Refractory_Period.RecommendedValues = { TEXT("0") };
  Refractory_Period.bRestrictToRecommended = false;

  FActorVariation Use_Log;
  Use_Log.Id = TEXT("use_log");
  Use_Log.Type = EActorAttributeType::Bool;
  Use_Log.RecommendedValues = { TEXT("True") };
  Use_Log.bRestrictToRecommended = false;

  FActorVariation Log_EPS;
  Log_EPS.Id = TEXT("log_eps");
  Log_EPS.Type = EActorAttributeType::Float;
  Log_EPS.RecommendedValues = { TEXT("0.001") };
  Log_EPS.bRestrictToRecommended = false;

  Definition.Variations.Append({ Cp, Cm, Sigma_Cp, Sigma_Cm, Refractory_Period, Use_Log, Log_EPS });

  return Definition;
}

void ADVSCamera::Set(const FActorDescription &Description)
{
  Super::Set(Description);

  this->config.Cp = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "positive_threshold",
      Description.Variations,
      0.5f);

  this->config.Cm = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "negative_threshold",
      Description.Variations,
      0.5f);

  this->config.sigma_Cp = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "sigma_positive_threshold",
      Description.Variations,
      0.0f);

  this->config.sigma_Cm = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "sigma_negative_threshold",
      Description.Variations,
      0.0f);

  this->config.refractory_period_ns = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
      "refractory_period_ns",
      Description.Variations,
      0.0);

  this->config.use_log = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      "use_log",
      Description.Variations,
      true);

  this->config.log_eps = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "log_eps",
      Description.Variations,
      1e-03);
}

void ADVSCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ADVSCamera::PostPhysTick);
  Super::PostPhysTick(World, TickType, DeltaTime);
  check(CaptureRenderTarget != nullptr);
  if (!HasActorBegunPlay() || IsValid(this))
  {
    return;
  }

  /// Immediate enqueues render commands of the scene at the current time.
  WaitForRenderThreadToFinish();

  //Super (ASceneCaptureSensor) Capture the Scene in a (UTextureRenderTarget2D) CaptureRenderTarge from the CaptureComponent2D
  /** Read the image **/
  TArray<FColor> RawImage;
  this->ReadPixels(RawImage);

  /** Convert image to gray scale **/
  if (this->config.use_log)
  {
    this->ImageToLogGray(RawImage);
  }
  else
  {
    this->ImageToGray(RawImage);
  }

  /** DVS Simulator **/
  ADVSCamera::DVSEventArray events = this->Simulation(DeltaTime);

  auto FrameIndex = FCarlaEngine::GetFrameCounter();
  auto DataStream = GetDataStream(*this);
  DataStream.SetFrameNumber(FrameIndex);
  auto Buffer = DataStream.PopBufferFromPool();

  // serialize data
  carla::Buffer BufferReady(carla::sensor::SensorRegistry::Serialize(*this, events, std::move(Buffer)));
  carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(BufferReady));

  // ROS2
  #if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send");
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    {
      // get resolution of camera
      int W = -1, H = -1;
      float Fov = -1.0f;
      auto WidthOpt = GetAttribute("image_size_x");
      if (WidthOpt.has_value())
        W = FCString::Atoi(*WidthOpt->Value);
      auto HeightOpt = GetAttribute("image_size_y");
      if (HeightOpt.has_value())
        H = FCString::Atoi(*HeightOpt->Value);
      auto FovOpt = GetAttribute("fov");
      if (FovOpt.has_value())
        Fov = FCString::Atof(*FovOpt->Value);
      AActor* ParentActor = GetAttachParentActor();
      if (ParentActor)
      {
        FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
        ROS2->ProcessDataFromDVS(DataStream.GetSensorType(), StreamId, LocalTransformRelativeToParent, BufView, W, H, Fov, this);
      }
      else
      {
        ROS2->ProcessDataFromDVS(DataStream.GetSensorType(), StreamId, DataStream.GetSensorTransform(), BufView, W, H, Fov, this);
      }
    }
  }
  #endif
  if (events.size() > 0)
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ADVSCamera Stream Send");
    /** Send the events **/
    DataStream.Send(*this, BufView);
  }
}

void ADVSCamera::ImageToGray(const TArray<FColor> &image)
{
  /** Sanity check **/
  if (image.Num() != (this->GetImageHeight() * this->GetImageWidth()))
    return;

  /** Reserve HxW elements **/
  last_image.SetNumUninitialized(image.Num());

  /** Convert image to gray raw image values **/
  for (size_t i = 0; i < image.Num(); ++i)
  {
    last_image[i] = FColorToGrayScaleFloat(image[i]);
  }
}

void ADVSCamera::ImageToLogGray(const TArray<FColor> &image)
{
  /** Sanity check **/
  if (image.Num() != (this->GetImageHeight() * this->GetImageWidth()))
    return;

  /** Reserve HxW elements **/
  last_image.SetNumUninitialized(image.Num());

  /** Convert image to gray raw image values **/
  for (size_t i = 0; i < image.Num(); ++i)
  {
    last_image[i] = std::log(this->config.log_eps + (FColorToGrayScaleFloat(image[i]) / 255.0));
  }
}

ADVSCamera::DVSEventArray ADVSCamera::Simulation (float DeltaTime)
{
  /** Array of events **/
  ADVSCamera::DVSEventArray events;

  /** Sanity check **/
  if (this->last_image.Num() != (this->GetImageHeight() * this->GetImageWidth()))
    return events;

  /** Check initialization **/
  if(this->prev_image.Num() == 0)
  {
    /** Set the first rendered image **/
    this->ref_values = this->last_image;
    this->prev_image = this->last_image;

    /** Resizes array to given number of elements. New elements will be zeroed.**/
    this->last_event_timestamp.SetNumZeroed(this->last_image.Num());

    /** Reset current time **/
    this->current_time = dvs::secToNanosec(this->GetEpisode().GetElapsedGameTime());

    return events;
  }

  static constexpr float tolerance = 1e-6;

  /** delta time in nanoseconds **/
  const std::uint64_t delta_t_ns = dvs::secToNanosec(
      this->GetEpisode().GetElapsedGameTime()) - this->current_time;

  /** Loop along the image size **/
  for (uint32 y = 0; y < this->GetImageHeight(); ++y)
  {
    for (uint32 x = 0; x < this->GetImageWidth(); ++x)
    {
      const uint32 i = (this->GetImageWidth() * y) + x;
      const float itdt = this->last_image[i];
      const float it = this->prev_image[i];
      const float prev_cross = this->ref_values[i];

      if (std::fabs (it - itdt) > tolerance)
      {
        const float pol = (itdt >= it) ? +1.0 : -1.0;
        float C = (pol > 0) ? this->config.Cp : this->config.Cm;
        const float sigma_C = (pol > 0) ? this->config.sigma_Cp : this->config.sigma_Cm;

        if(sigma_C > 0)
        {
          C += RandomEngine->GetNormalDistribution(0, sigma_C);
          constexpr float minimum_contrast_threshold = 0.01;
          C = std::max(minimum_contrast_threshold, C);
        }
        float curr_cross = prev_cross;
        bool all_crossings = false;

        do
        {
          curr_cross += pol * C;

          if ((pol > 0 && curr_cross > it && curr_cross <= itdt)
              || (pol < 0 && curr_cross < it && curr_cross >= itdt))
          {
            const std::uint64_t edt = (curr_cross - it) * delta_t_ns / (itdt - it);
            const std::int64_t t = this->current_time + edt;

            // check that pixel (x,y) is not currently in a "refractory" state
            // i.e. |t-that last_timestamp(x,y)| >= refractory_period
            const std::int64_t last_stamp_at_xy = dvs::secToNanosec(this->last_event_timestamp[i]);
            if (t >= last_stamp_at_xy)
            {
              const std::uint64_t dt = t - last_stamp_at_xy;
              if(this->last_event_timestamp[i] == 0 || dt >= this->config.refractory_period_ns)
              {
                events.push_back(::carla::sensor::data::DVSEvent(x, y, t, pol > 0));
                this->last_event_timestamp[i] = dvs::nanosecToSecTrunc(t);
              }
              else
              {
                /** Dropping event because time since last event < refractory_period_ns **/
              }
              this->ref_values[i] = curr_cross;
            }
          }
          else
          {
            all_crossings = true;
          }
        } while (!all_crossings);
      } // end tolerance
    } // end for each pixel
  }

  /** Update current time **/
  this->current_time = dvs::secToNanosec(this->GetEpisode().GetElapsedGameTime());

  this->prev_image = this->last_image;

  // Sort the events by increasing timestamps, since this is what
  // most event processing algorithms expect
  std::sort(events.begin(), events.end(), [](const ::carla::sensor::data::DVSEvent& it1, const ::carla::sensor::data::DVSEvent& it2){return it1.t < it2.t;});

  return events;
}
