// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "carla/geom/Math.h"
#include "carla/geom/GeoLocation.h"
#include <vector>
#include "carla/sensor/data/LibITS.h"
#include <chrono>

class CaService
{
public:
  CaService(URandomEngine *random_engine);
  void SetActor(UWorld *world, AActor *Owner);

  void SetParams(const float GenCamMin, const float GenCamMax, const bool FixedRate);
  void SetVelDeviation(const float noise_vel_stddev_x);
  void SetYawrateDeviation(const float noise_yawrate_stddev, const float noise_yawrate_bias);
  void SetAccelerationStandardDeviation(const FVector &Vec);
  void SetGNSSDeviation(const float noise_lat_stddev,
                        const float noise_lon_stddev,
                        const float noise_alt_stddev,
                        const float noise_head_stddev,
                        const float noise_lat_bias,
                        const float noise_lon_bias,
                        const float noise_alt_bias,
                        const float noise_head_bias);
  bool Trigger(float DeltaSeconds);
  CAM_t GetCamMessage()const;
  bool CarlaActorInitialized()const { return mCarlaActor!=nullptr; }
private:
  AActor *mActorOwner{nullptr};
  FCarlaActor *mCarlaActor{nullptr};
  UCarlaEpisode *mCarlaEpisode{nullptr};
  UWorld *mWorld{nullptr};
  ACarlaWheeledVehicle *mVehicle{nullptr};
  float mLastCamTimeStamp;
  float mLastLowCamTimeStamp;
  float mGenCamMin;
  float mGenCamMax;
  float mGenCam;
  double mElapsedTime;
  float mGenDeltaTimeMod;
  bool mDccRestriction;
  bool mFixedRate;
  unsigned int mGenCamLowDynamicsCounter;
  unsigned int mGenCamLowDynamicsLimit;
  float mGenerationInterval;

  float VehicleSpeed;
  FVector VehiclePosition;
  FVector VehicleHeading;

  FVector mLastCamPosition;
  float mLastCamSpeed;
  FVector mLastCamHeading;
  std::chrono::milliseconds mGenerationDelta0;

  bool CheckTriggeringConditions(float DeltaSeconds);
  bool CheckHeadingDelta(float DeltaSeconds);
  bool CheckPositionDelta(float DeltaSeconds);
  bool CheckSpeedDelta(float DeltaSeconds);
  double GetFVectorAngle(const FVector &a, const FVector &b)const;
  void GenerateCamMessage(float DeltaTime);
  ITSContainer::StationType_t GetStationType()const;

  float GetHeading()const;
  long GetVehicleRole()const;

  /* Constant values for message*/
  const long mProtocolVersion = 2;
  const long mMessageId = ITSContainer::messageID_cam;
  long mStationId;
  long mStationType;

  carla::geom::Vector3D ComputeAccelerometer(const float DeltaTime);
  const carla::geom::Vector3D ComputeAccelerometerNoise(const FVector &Accelerometer)const;
  /// Standard deviation for acceleration settings.
  FVector StdDevAccel;

  /// Used to compute the acceleration
  std::array<FVector, 2> PrevLocation;

  /// Used to compute the acceleration
  float PrevDeltaTime;

  // GNSS reference position and heading
  FVector GetReferencePosition()const;
  carla::geom::GeoProjection CurrentGeoProjection;
  float LatitudeDeviation;
  float LongitudeDeviation;
  float AltitudeDeviation;
  float HeadingDeviation;

  float LatitudeBias;
  float LongitudeBias;
  float AltitudeBias;
  float HeadingBias;

  // Velocity
  float ComputeSpeed()const;
  float VelocityDeviation;

  // Yaw rate
  const float ComputeYawNoise(const FVector &Gyroscope)const;
  float ComputeYawRate()const;
  float YawrateDeviation;
  float YawrateBias;

  CAM_t CreateCooperativeAwarenessMessage(float DeltaTime);
  void CreateITSPduHeader(CAM_t &message);
  void AddCooperativeAwarenessMessage(CAMContainer::CoopAwareness_t &CoopAwarenessMessage, float DeltaTime);
  void AddBasicContainer(CAMContainer::BasicContainer_t &BasicContainer);
  void AddBasicVehicleContainerHighFrequency(CAMContainer::HighFrequencyContainer_t &hfc, float DeltaTime);
  void AddRSUContainerHighFrequency(CAMContainer::HighFrequencyContainer_t &hfc);
  void AddLowFrequencyContainer(CAMContainer::LowFrequencyContainer_t &lfc);
  CAM_t mCAMMessage;

  // random for noise
  URandomEngine *mRandomEngine;
  ITSContainer::SpeedValue_t BuildSpeedValue(const float vel)const;
};
