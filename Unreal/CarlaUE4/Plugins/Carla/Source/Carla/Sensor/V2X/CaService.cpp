// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CaService.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Sensor/InertialMeasurementUnit.h"
#include <boost/algorithm/clamp.hpp>
#include "carla/rpc/String.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include <chrono>
static const float scLowFrequencyContainerInterval = 0.5;

ITSContainer::SpeedValue_t CaService::BuildSpeedValue(const float vel)
{
    static const float lower = 0.0;    // meter_per_second
    static const float upper = 163.82; // meter_per_second

    ITSContainer::SpeedValue_t speed = ITSContainer::SpeedValue_unavailable;
    if (vel >= upper)
    {
        speed = 16382; // see CDD A.74 (TS 102 894 v1.2.1)
    }
    else if (vel >= lower)
    {
        // to cm per second
        speed = std::round(vel * 100.0) * ITSContainer::SpeedValue_oneCentimeterPerSec;
    }
    return speed;
}

CaService::CaService(URandomEngine *random_engine)
{
    mRandomEngine = random_engine;
    // The starting point now
    std::chrono::system_clock::time_point start_Point = std::chrono::system_clock::now();
    // the reference point for ETSI (2004-01-01T00:00:00:000Z)
    std::chrono::system_clock::time_point ref_point = std::chrono::system_clock::from_time_t(1072915200); // Unix time for 2004-01-01

    mGenerationDelta0 = std::chrono::duration_cast<std::chrono::milliseconds>(start_Point - ref_point);
}

void CaService::SetOwner(UWorld *world, AActor *Owner)
{
    UE_LOG(LogCarla, Warning, TEXT("CaService:SetOwner function called"));
    mWorld = world;
    mCarlaEpisode = UCarlaStatics::GetCurrentEpisode(world);

    CurrentGeoProjection = mCarlaEpisode->GetGeoProjection();

    mActorOwner = Owner;
    mCarlaEpisode = UCarlaStatics::GetCurrentEpisode(mWorld);
    mCarlaActor = mCarlaEpisode->FindCarlaActor(Owner);

    if (mCarlaActor != nullptr)
    {
        mVehicle = Cast<ACarlaWheeledVehicle>(Owner);

        if (mCarlaActor->GetActorType() == FCarlaActor::ActorType::Vehicle)
        {
            UE_LOG(LogCarla, Warning, TEXT("CaService:Initialize Vehicle type"));

            mLastCamTimeStamp = mCarlaEpisode->GetElapsedGameTime() - mGenCamMax;
            mLastLowCamTimeStamp = mCarlaEpisode->GetElapsedGameTime() - scLowFrequencyContainerInterval;
            // Can add logic for this later
            mDccRestriction = false;

            mElapsedTime = 0;
            VehicleSpeed = 0;
            VehiclePosition = {0, 0, 0};
            VehicleHeading = {0, 0, 0};
            mLastCamSpeed = 0;
            mLastCamPosition = {0, 0, 0};
            mLastCamHeading = {0, 0, 0};
        }
        else if ((mCarlaActor->GetActorType() == FCarlaActor::ActorType::TrafficLight) ||
                 (mCarlaActor->GetActorType() == FCarlaActor::ActorType::TrafficSign))
        {
            mGenerationInterval = 0.5;
            mLastCamTimeStamp = -mGenerationInterval;
            UE_LOG(LogCarla, Warning, TEXT("CaService:Initialize RSU type"));
        }

        mStationId = static_cast<long>(mCarlaActor->GetActorId());
        mStationType = GetStationType();
    }
}

void CaService::SetParams(const float GenCamMin, const float GenCamMax, const bool FixedRate)
{
    UE_LOG(LogCarla, Warning, TEXT("CaService:SetParams function called"));
    // Max and Min for generation rate

    mGenCamMin = GenCamMin; // in second
    mGenCamMax = GenCamMax; // in second
    mGenCam = mGenCamMax;
    // If we want set a fix interval make this as true
    mFixedRate = FixedRate;
}

/*
 * Function to check trigger condition for RSU
 */
bool CaService::Trigger(float DeltaSeconds)
{

    mElapsedTime = mCarlaEpisode->GetElapsedGameTime();
    bool Trigger = false;
    if (mStationType == ITSContainer::StationType_roadSideUnit)
    {
        if (mElapsedTime - mLastCamTimeStamp >= mGenerationInterval)
        {
            Trigger = true;
            mCAMMessage = CreateCooperativeAwarenessMessage(DeltaSeconds);
            mLastCamTimeStamp = mElapsedTime;
        }
    }
    else
    {
        Trigger = CheckTriggeringConditions(DeltaSeconds);
    }
    return Trigger;
}

/*
 * Function to provide CAM message to other objects if necessary
 */
CAM_t CaService::GetCamMessage()
{
    return mCAMMessage;
}

/*
 * Check the trigger condition in case of vehicles and if trigger is true request
 * to generate CAM message
 */
bool CaService::CheckTriggeringConditions(float DeltaSeconds)
{
    float &T_GenCam = mGenCam;
    const float T_GenCamMin = mGenCamMin;
    const float T_GenCamMax = mGenCamMax;
    const float T_GenCamDcc = mDccRestriction ? 0 : T_GenCamMin;
    const float T_elapsed = mElapsedTime - mLastCamTimeStamp;
    if (T_elapsed >= T_GenCamDcc)
    {
        // If message need to be generated every sim tick then set this to true.
        if (mFixedRate)
        {
            GenerateCamMessage(DeltaSeconds);
            return true;
        }

        else if (CheckHeadingDelta(DeltaSeconds) || CheckPositionDelta(DeltaSeconds) || CheckSpeedDelta(DeltaSeconds))
        {
            GenerateCamMessage(DeltaSeconds);
            T_GenCam = std::min(T_elapsed, T_GenCamMax);
            mGenCamLowDynamicsCounter = 0;
            return true;
        }
        else if (T_elapsed >= T_GenCam)
        {
            GenerateCamMessage(DeltaSeconds);
            if (++mGenCamLowDynamicsCounter >= mGenCamLowDynamicsLimit)
            {
                T_GenCam = T_GenCamMax;
            }
            return true;
        }
    }
    return false;
}

bool CaService::CheckPositionDelta(float DeltaSeconds)
{
    // If position change is more the 4m
    VehiclePosition = mVehicle->GetActorLocation();
    double Distance = FVector::Distance(VehiclePosition, mLastCamPosition) / 100.0f; // From cm to m
    if (Distance > 4.0f)
    {
        return true;
    }
    return false;
}

bool CaService::CheckSpeedDelta(float DeltaSeconds)
{
    VehicleSpeed = mVehicle->GetVehicleForwardSpeed() / 100.0f; // From cm/s to m/s
    float DeltaSpeed = std::abs(VehicleSpeed - mLastCamSpeed);

    // Speed differance is greater than 0.5m/s
    if (DeltaSpeed > 0.5)
    {
        return true;
    }

    return false;
}

double CaService::GetFVectorAngle(const FVector &a, const FVector &b)
{
    double Dot = a.X * b.X + a.Y * b.Y + a.Z * b.Z;
    return std::acos(Dot / (a.Size() * b.Size()));
}

void CaService::GenerateCamMessage(float DeltaTime)
{
    mCAMMessage = CreateCooperativeAwarenessMessage(DeltaTime);
    mLastCamPosition = VehiclePosition;
    mLastCamSpeed = VehicleSpeed;
    mLastCamHeading = VehicleHeading;
    mLastCamTimeStamp = mElapsedTime;
}

// Function to get the station type
ITSContainer::StationType_t CaService::GetStationType()
{
    check(mActorOwner != nullptr);
    mCarlaEpisode = UCarlaStatics::GetCurrentEpisode(mWorld);
    mCarlaActor = mCarlaEpisode->FindCarlaActor(mActorOwner);
    ITSContainer::StationType_t stationType = ITSContainer::StationType_unknown;
    // return unknown if carla actor is gone
    if (mCarlaActor == nullptr)
    {
        return static_cast<long>(stationType);
    }
    auto Tag = ATagger::GetTagOfTaggedComponent(*mVehicle->GetMesh());

    switch (Tag)
    {
    case crp::CityObjectLabel::None:
        stationType = ITSContainer::StationType_unknown;
        break;
    case crp::CityObjectLabel::Pedestrians:
        stationType = ITSContainer::StationType_pedestrian;
        break;
    case crp::CityObjectLabel::Bicycle:
        stationType = ITSContainer::StationType_cyclist;
        break;
    case crp::CityObjectLabel::Motorcycle:
        stationType = ITSContainer::StationType_motorcycle;
        break;
    case crp::CityObjectLabel::Car:
        stationType = ITSContainer::StationType_passengerCar;
        break;
    case crp::CityObjectLabel::Bus:
        stationType = ITSContainer::StationType_bus;
        break;
    // TODO Modify this in future is CARLA adds difference truck
    case crp::CityObjectLabel::Truck:
        stationType = ITSContainer::StationType_lightTruck;
        break;
    case crp::CityObjectLabel::Buildings:
    case crp::CityObjectLabel::Walls:
    case crp::CityObjectLabel::Fences:
    case crp::CityObjectLabel::Poles:
    case crp::CityObjectLabel::TrafficLight:
    case crp::CityObjectLabel::TrafficSigns:
        stationType = ITSContainer::StationType_roadSideUnit;
        break;
    case crp::CityObjectLabel::Train:
        stationType = ITSContainer::StationType_tram;
        break;
    default:
        stationType = ITSContainer::StationType_unknown;
    }

    // Can improve this later for different special vehicles once carla implements it
    FCarlaActor::ActorType Type = mCarlaActor->GetActorType();
    if (Type == FCarlaActor::ActorType::Vehicle)
    {
        if (mCarlaActor->GetActorInfo()->Description.Variations.Contains("special_type"))
        {
            std::string special_type = carla::rpc::FromFString(*mCarlaActor->GetActorInfo()->Description.Variations["special_type"].Value);
            if (special_type.compare("emergency") == 0)
            {
                stationType = ITSContainer::StationType_specialVehicles;
            }
        }
    }
    return static_cast<long>(stationType);
}

FVector CaService::GetReferencePosition()
{
    FVector RefPos;
    carla::geom::Location ActorLocation = mActorOwner->GetActorLocation();
    ALargeMapManager *LargeMap = UCarlaStatics::GetLargeMapManager(mWorld);
    if (LargeMap)
    {
        ActorLocation = LargeMap->LocalToGlobalLocation(ActorLocation);
    }
    carla::geom::Location Location = ActorLocation;
    carla::geom::GeoLocation CurrentLocation = CurrentGeoProjection.TransformToGeoLocation(Location);

    // Compute the noise for the sensor
    const float LatError = mRandomEngine->GetNormalDistribution(0.0f, LatitudeDeviation);
    const float LonError = mRandomEngine->GetNormalDistribution(0.0f, LongitudeDeviation);
    const float AltError = mRandomEngine->GetNormalDistribution(0.0f, AltitudeDeviation);

    // Apply the noise to the sensor
    double Latitude = CurrentLocation.latitude + LatitudeBias + LatError;
    double Longitude = CurrentLocation.longitude + LongitudeBias + LonError;
    double Altitude = CurrentLocation.altitude + AltitudeBias + AltError;

    RefPos.X = Latitude;
    RefPos.Y = Longitude;
    RefPos.Z = Altitude;
    return RefPos;
}

float CaService::GetHeading()
{
    // Magnetometer: orientation with respect to the North in rad
    const FVector CarlaNorthVector = FVector(0.0f, -1.0f, 0.0f);
    const FVector ForwVect = mActorOwner->GetActorForwardVector().GetSafeNormal2D();
    const float DotProd = FVector::DotProduct(CarlaNorthVector, ForwVect);

    // We check if the dot product is higher than 1.0 due to numerical error
    if (DotProd >= 1.00f)
        return 0.0f;

    float Heading = std::acos(DotProd);
    // Keep the angle between [0, 2pi)
    if (FVector::CrossProduct(CarlaNorthVector, ForwVect).Z < 0.0f)
        Heading = carla::geom::Math::Pi2<float>() - Heading;

    const double HeadingDegree = carla::geom::Math::ToDegrees(Heading);

    // Compute the noise for the sensor
    const float HeadingError = mRandomEngine->GetNormalDistribution(0.0f, HeadingDeviation);

    // add errors
    return HeadingDegree + HeadingBias + HeadingError;
}

// Function to get the vehicle role
long CaService::GetVehicleRole()
{
    long VehicleRole = ITSContainer::VehicleRole_default;
    long StationType = GetStationType();
    switch (StationType)
    {
    case ITSContainer::StationType_cyclist:
    case ITSContainer::StationType_moped:
    case ITSContainer::StationType_motorcycle:
        VehicleRole = ITSContainer::VehicleRole_default;
        break;
    case ITSContainer::StationType_bus:
    case ITSContainer::StationType_tram:
        VehicleRole = ITSContainer::VehicleRole_publicTransport;
        break;
    case ITSContainer::StationType_specialVehicles:
        VehicleRole = ITSContainer::VehicleRole_emergency;
        break;
    default:
        VehicleRole = ITSContainer::VehicleRole_default;
        break;
    }
    return VehicleRole;
}

CAM_t CaService::CreateCooperativeAwarenessMessage(float DeltaTime)
{
    CAM_t message = CAM_t();

    CreateITSPduHeader(message);
    AddCooperativeAwarenessMessage(message.cam, DeltaTime);

    return message;
}

void CaService::CreateITSPduHeader(CAM_t &message)
{
    ITSContainer::ItsPduHeader_t &header = message.header;
    header.protocolVersion = mProtocolVersion;
    header.messageID = mMessageId;
    header.stationID = mStationId;
}

void CaService::AddCooperativeAwarenessMessage(CAMContainer::CoopAwareness_t &CoopAwarenessMessage, float DeltaTime)
{

    /* GenerationDeltaTime */
    auto genDeltaTime = mGenerationDelta0 + std::chrono::milliseconds(static_cast<long long>(mCarlaEpisode->GetElapsedGameTime() * 1000));
    CoopAwarenessMessage.generationDeltaTime = genDeltaTime.count() % 65536 * CAMContainer::GenerationDeltaTime_oneMilliSec; // TODOCheck this logic
    AddBasicContainer(CoopAwarenessMessage.camParameters.basicContainer);
    if (CoopAwarenessMessage.camParameters.basicContainer.stationType == ITSContainer::StationType_roadSideUnit)
    {
        // TODO Future Implementation
        AddRSUContainerHighFrequency(CoopAwarenessMessage.camParameters.highFrequencyContainer);
    }
    else if (CoopAwarenessMessage.camParameters.basicContainer.stationType == ITSContainer::StationType_pedestrian)
    {
        // TODO no container available for Pedestrains
    }
    else
    {
        // BasicVehicleContainer
        AddBasicVehicleContainerHighFrequency(CoopAwarenessMessage.camParameters.highFrequencyContainer, DeltaTime);

        if (mElapsedTime - mLastLowCamTimeStamp >= scLowFrequencyContainerInterval)
        {
            AddLowFrequencyContainer(CoopAwarenessMessage.camParameters.lowFrequencyContainer);
            mLastLowCamTimeStamp = mElapsedTime;
        }
        else
        {
            // Store nothing if not used
            CoopAwarenessMessage.camParameters.lowFrequencyContainer.present = CAMContainer::LowFrequencyContainer_PR_NOTHING;
        }
        /*
         *TODO Add Special container if it a special vehicle
         */
    }
}

void CaService::AddBasicContainer(CAMContainer::BasicContainer_t &BasicContainer)
{
    BasicContainer.stationType = mStationType;

    /* CamParameters ReferencePosition */
    FVector RefPos = GetReferencePosition();
    BasicContainer.referencePosition.latitude = std::round(RefPos.X * 1e6) * ITSContainer::Latitude_oneMicroDegreeNorth;
    BasicContainer.referencePosition.longitude = std::round(RefPos.Y * 1e6) * ITSContainer::Longitude_oneMicroDegreeEast;
    BasicContainer.referencePosition.positionConfidenceEllipse.semiMajorConfidence = ITSContainer::SemiAxisLength_unavailable;
    BasicContainer.referencePosition.positionConfidenceEllipse.semiMinorConfidence = ITSContainer::SemiAxisLength_unavailable;
    BasicContainer.referencePosition.positionConfidenceEllipse.semiMajorOrientation = ITSContainer::HeadingValue_unavailable;
    BasicContainer.referencePosition.altitude.altitudeValue = std::round(RefPos.Z * 100.0) * ITSContainer::AltitudeValue_oneCentimeter;
    BasicContainer.referencePosition.altitude.altitudeConfidence = ITSContainer::AltitudeConfidence_unavailable;
}

void CaService::SetAccelerationStandardDeviation(const FVector &Vec)
{
    StdDevAccel = Vec;
}

void CaService::SetGNSSDeviation(const float noise_lat_stddev,
                                 const float noise_lon_stddev,
                                 const float noise_alt_stddev,
                                 const float noise_head_stddev,
                                 const float noise_lat_bias,
                                 const float noise_lon_bias,
                                 const float noise_alt_bias,
                                 const float noise_head_bias)
{
    LatitudeDeviation = noise_lat_stddev;
    LongitudeDeviation = noise_lon_stddev;
    AltitudeDeviation = noise_alt_stddev;
    HeadingDeviation = noise_head_stddev;
    LatitudeBias = noise_lat_bias;
    LongitudeBias = noise_lon_bias;
    AltitudeBias = noise_alt_bias;
    HeadingBias = noise_head_bias;
}

void CaService::SetVelDeviation(const float noise_vel_stddev_x)
{
    VelocityDeviation = noise_vel_stddev_x;
}

void CaService::SetYawrateDeviation(const float noise_yawrate_stddev, const float noise_yawrate_bias)
{
    YawrateDeviation = noise_yawrate_stddev;
    YawrateBias = noise_yawrate_bias;
}

void CaService::AddBasicVehicleContainerHighFrequency(CAMContainer::HighFrequencyContainer_t &hfc, float DeltaTime)
{
    hfc.present = CAMContainer::HighFrequencyContainer_PR_basicVehicleContainerHighFrequency;
    CAMContainer::BasicVehicleContainerHighFrequency_t &bvc = hfc.basicVehicleContainerHighFrequency;
    // heading
    bvc.heading.headingValue = std::round(GetHeading() * 10.0);
    bvc.heading.headingConfidence = ITSContainer::HeadingConfidence_equalOrWithinOneDegree; // TODO
    // speed
    // speed with noise
    bvc.speed.speedValue = BuildSpeedValue(ComputeSpeed());
    bvc.speed.speedConfidence = ITSContainer::SpeedConfidence_equalOrWithInOneCentimerterPerSec * 3; // TODO
    // direction
    bvc.driveDirection = (mVehicle->GetVehicleForwardSpeed() / 100.0f) >= 0.0 ? ITSContainer::DriveDirection_forward : ITSContainer::DriveDirection_backward;
    // length and width
    auto bb = UBoundingBoxCalculator::GetActorBoundingBox(mActorOwner); // cm
    float length = bb.Extent.X * 2.0;                                   // half box
    float width = bb.Extent.Y * 2.0;                                    // half box

    bvc.vehicleLength.vehicleLengthValue = std::round(length * 10.0); // 0.1 meter
    bvc.vehicleLength.vehicleLengthConfidenceIndication = ITSContainer::VehicleLengthConfidenceIndication_unavailable;
    bvc.vehicleWidth = std::round(width * 10.0); // 0.1 meter

    // acceleration
    carla::geom::Vector3D Accel = ComputeAccelerometer(DeltaTime);

    const double lonAccelValue = Accel.x * 10.0; // m/s to 0.1 m/s
    // limit changes
    if (lonAccelValue >= -160.0 && lonAccelValue <= 161.0)
    {
        bvc.longitudinalAcceleration.longitudinalAccelerationValue = std::round(lonAccelValue) * ITSContainer::LongitudinalAccelerationValue_pointOneMeterPerSecSquaredForward;
    }
    else
    {
        bvc.longitudinalAcceleration.longitudinalAccelerationValue = ITSContainer::LongitudinalAccelerationValue_unavailable;
    }
    bvc.longitudinalAcceleration.longitudinalAccelerationConfidence = ITSContainer::AccelerationConfidence_unavailable; // TODO

    // curvature TODO
    bvc.curvature.curvatureValue = ITSContainer::CurvatureValue_unavailable;
    bvc.curvature.curvatureConfidence = ITSContainer::CurvatureConfidence_unavailable;
    bvc.curvatureCalculationMode = ITSContainer::CurvatureCalculationMode_yarRateUsed;

    // yaw rate is in rad/s --> to centidegree per second
    bvc.yawRate.yawRateValue = std::round(carla::geom::Math::ToDegrees(ComputeYawRate()) * 100.0) * ITSContainer::YawRateValue_degSec_000_01ToLeft;
    if (bvc.yawRate.yawRateValue < -32766 || bvc.yawRate.yawRateValue > 32766)
    {
        bvc.yawRate.yawRateValue = ITSContainer::YawRateValue_unavailable;
    }
    bvc.yawRate.yawRateConfidence = ITSContainer::YawRateConfidence_unavailable; // TODO

    // optional lat and vertical accelerations
    bvc.lateralAccelerationAvailable = true;
    const double latAccelValue = Accel.y * 10.0; // m/s to 0.1 m/s
    if (latAccelValue >= -160.0 && latAccelValue <= 161.0)
    {
        bvc.lateralAcceleration.lateralAccelerationValue = std::round(latAccelValue) * ITSContainer::LateralAccelerationValue_pointOneMeterPerSecSquaredToLeft;
    }
    else
    {
        bvc.lateralAcceleration.lateralAccelerationValue = ITSContainer::LateralAccelerationValue_unavailable;
    }
    bvc.lateralAcceleration.lateralAccelerationConfidence = ITSContainer::AccelerationConfidence_unavailable; // TODO

    bvc.verticalAccelerationAvailable = true;
    const double vertAccelValue = Accel.z * 10.0; // m/s to 0.1 m/s
    if (vertAccelValue >= -160.0 && vertAccelValue <= 161.0)
    {
        bvc.verticalAcceleration.verticalAccelerationValue = std::round(vertAccelValue) * ITSContainer::VerticalAccelerationValue_pointOneMeterPerSecSquaredUp;
    }
    else
    {
        bvc.verticalAcceleration.verticalAccelerationValue = ITSContainer::VerticalAccelerationValue_unavailable;
    }
    bvc.verticalAcceleration.verticalAccelerationConfidence = ITSContainer::AccelerationConfidence_unavailable; // TODO

    // TODO
    bvc.accelerationControlAvailable = false;
    bvc.lanePositionAvailable = false;
    bvc.steeringWheelAngleAvailable = false;
    bvc.performanceClassAvailable = false;
    bvc.cenDsrcTollingZoneAvailable = false;
}

const carla::geom::Vector3D CaService::ComputeAccelerometerNoise(
    const FVector &Accelerometer)
{
    // Normal (or Gaussian or Gauss) distribution will be used as noise function.
    // A mean of 0.0 is used as a first parameter, the standard deviation is
    // determined by the client
    constexpr float Mean = 0.0f;
    return carla::geom::Vector3D{
        Accelerometer.X + mRandomEngine->GetNormalDistribution(Mean, StdDevAccel.X),
        Accelerometer.Y + mRandomEngine->GetNormalDistribution(Mean, StdDevAccel.Y),
        Accelerometer.Z + mRandomEngine->GetNormalDistribution(Mean, StdDevAccel.Z)};
}

carla::geom::Vector3D CaService::ComputeAccelerometer(
    const float DeltaTime)
{
    // Used to convert from UE4's cm to meters
    constexpr float TO_METERS = 1e-2;
    // Earth's gravitational acceleration is approximately 9.81 m/s^2
    constexpr float GRAVITY = 9.81f;

    // 2nd derivative of the polynomic (quadratic) interpolation
    // using the point in current time and two previous steps:
    // d2[i] = -2.0*(y1/(h1*h2)-y2/((h2+h1)*h2)-y0/(h1*(h2+h1)))
    const FVector CurrentLocation = mVehicle->GetActorLocation();

    const FVector Y2 = PrevLocation[0];
    const FVector Y1 = PrevLocation[1];
    const FVector Y0 = CurrentLocation;
    const float H1 = DeltaTime;
    const float H2 = PrevDeltaTime;

    const float H1AndH2 = H2 + H1;
    const FVector A = Y1 / (H1 * H2);
    const FVector B = Y2 / (H2 * (H1AndH2));
    const FVector C = Y0 / (H1 * (H1AndH2));
    FVector FVectorAccelerometer = TO_METERS * -2.0f * (A - B - C);

    // Update the previous locations
    PrevLocation[0] = PrevLocation[1];
    PrevLocation[1] = CurrentLocation;
    PrevDeltaTime = DeltaTime;

    // Add gravitational acceleration
    FVectorAccelerometer.Z += GRAVITY;

    FQuat ImuRotation = mActorOwner->GetRootComponent()->GetComponentTransform().GetRotation();
    FVectorAccelerometer = ImuRotation.UnrotateVector(FVectorAccelerometer);

    // Cast from FVector to our Vector3D to correctly send the data in m/s^2
    // and apply the desired noise function, in this case a normal distribution
    const carla::geom::Vector3D Accelerometer =
        ComputeAccelerometerNoise(FVectorAccelerometer);

    return Accelerometer;
}

float CaService::ComputeSpeed()
{

    const float speed = mVehicle->GetVehicleForwardSpeed() / 100.0f;

    // Normal (or Gaussian or Gauss) distribution and a bias will be used as
    // noise function.
    // A mean of 0.0 is used as a first parameter.The standard deviation and the
    // bias are determined by the client
    constexpr float Mean = 0.0f;
    return boost::algorithm::clamp(speed + mRandomEngine->GetNormalDistribution(Mean, VelocityDeviation), 0.0f, std::numeric_limits<float>::max());
}

float CaService::ComputeYawRate()
{
    check(mActorOwner != nullptr);
    const FVector AngularVelocity =
        AInertialMeasurementUnit::GetActorAngularVelocityInRadians(*mActorOwner);

    const FQuat SensorLocalRotation =
        mActorOwner->GetRootComponent()->GetRelativeTransform().GetRotation();

    const FVector FVectorGyroscope =
        SensorLocalRotation.RotateVector(AngularVelocity);

    // Cast from FVector to our Vector3D to correctly send the data in rad/s
    // and apply the desired noise function, in this case a normal distribution
    float yawrate =
        ComputeYawNoise(FVectorGyroscope);

    return yawrate; // rad/s
}

const float CaService::ComputeYawNoise(
    const FVector &Gyroscope)
{
    // Normal (or Gaussian or Gauss) distribution and a bias will be used as
    // noise function.
    // A mean of 0.0 is used as a first parameter.The standard deviation and the
    // bias are determined by the client
    constexpr float Mean = 0.0f;
    return Gyroscope.Z + YawrateBias + mRandomEngine->GetNormalDistribution(Mean, YawrateDeviation);
}

long millisecondsSince2004()
{
    // Define the epoch time (2004-01-01T00:00:00.000Z)
    std::tm epoch_time = {};
    epoch_time = {0, 0, 0, 1, 0, 104}; // January 1, 2004

    // Convert epoch time to a std::chrono::time_point
    std::chrono::system_clock::time_point epoch = std::chrono::system_clock::from_time_t(std::mktime(&epoch_time));

    // Get the current time
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    // Calculate the duration since the epoch in milliseconds
    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - epoch);

    // Return the number of milliseconds as a long
    return duration.count();
}

void CaService::AddRSUContainerHighFrequency(CAMContainer::HighFrequencyContainer_t &hfc)
{
    hfc.present = CAMContainer::HighFrequencyContainer_PR_rsuContainerHighFrequency;
    CAMContainer::RSUContainerHighFrequency_t &rsu = hfc.rsuContainerHighFrequency;
    // TODO For future implementation ITSContainer::ProtectedCommunicationZonesRSU_t PCZR

    uint8_t ProtectedZoneDataLength = 16; // Maximum number of elements in path history

    for (uint8_t i = 0; i <= ProtectedZoneDataLength; ++i)
    {
        ITSContainer::ProtectedCommunicationZone_t PCZ;
        PCZ.protectedZoneType = ITSContainer::ProtectedZoneType_cenDsrcTolling;
        PCZ.expiryTimeAvailable = false;
        PCZ.protectedZoneLatitude = 50;
        PCZ.protectedZoneLongitude = 50;
        PCZ.protectedZoneRadiusAvailable = false;
        PCZ.protectedZoneIDAvailable = false;
        rsu.protectedCommunicationZonesRSU.list.push_back(PCZ);
        rsu.protectedCommunicationZonesRSU.ProtectedCommunicationZoneCount += 1;
    }
}

void CaService::AddLowFrequencyContainer(CAMContainer::LowFrequencyContainer_t &lfc)
{
    lfc.present = CAMContainer::LowFrequencyContainer_PR_basicVehicleContainerLowFrequency;
    CAMContainer::BasicVehicleContainerLowFrequency_t &bvc = lfc.basicVehicleContainerLowFrequency;

    /*Vehicle Role*/
    bvc.vehicleRole = GetVehicleRole();

    /*Exterior Lights*/
    uint8_t *buf = &bvc.exteriorLights;
    FVehicleLightState LightStateData = mVehicle->GetVehicleLightState();
    if (LightStateData.LowBeam)
    {
        buf[0] |= 1 << (7 - ITSContainer::ExteriorLights_lowBeamHeadlightsOn);
    }
    if (LightStateData.HighBeam)
    {
        buf[0] |= 1 << (7 - ITSContainer::ExteriorLights_highBeamHeadlightsOn);
    }
    if (LightStateData.LeftBlinker)
    {
        buf[0] |= 1 << (7 - ITSContainer::ExteriorLights_leftTurnSignalOn);
    }
    if (LightStateData.RightBlinker)
    {
        buf[0] |= 1 << (7 - ITSContainer::ExteriorLights_rightTurnSignalOn);
    }
    if (LightStateData.Reverse)
    {
        buf[0] |= 1 << (7 - ITSContainer::ExteriorLights_reverseLightOn);
    }
    if (LightStateData.Fog)
    {
        buf[0] |= 1 << (7 - ITSContainer::ExteriorLights_fogLightOn);
    }
    if (LightStateData.Position)
    {
        buf[0] |= 1 << (7 - ITSContainer::ExteriorLights_parkingLightsOn);
    }
    bvc.pathHistory.NumberOfPathPoint = 0;
}

bool CaService::CheckHeadingDelta(float DeltaSeconds)
{
    // if heading diff is more than 4degree
    VehicleHeading = mVehicle->GetVehicleOrientation();
    double HeadingDelta = carla::geom::Math::ToDegrees(GetFVectorAngle(mLastCamHeading, VehicleHeading));
    if (HeadingDelta > 4.0)
    {
        return true;
    }
    return false;
}
