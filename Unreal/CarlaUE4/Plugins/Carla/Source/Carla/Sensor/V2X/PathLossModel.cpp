// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Util/RandomEngine.h"
#include "Math/UnrealMathUtility.h"
#include "Carla/Util/RandomEngine.h"

#include "DrawDebugHelpers.h"

#include "PathLossModel.h"
#include <random>
#include <limits>

double PathLossModel::Frequency_GHz = 5.9f;
double PathLossModel::Frequency = 5.9f * std::pow(10, 9);
double PathLossModel::lambda = PathLossModel::c_speedoflight / (5.9f * std::pow(10, 9));

PathLossModel::PathLossModel(URandomEngine *random_engine, AActor *Owner)
{
    mRandomEngine = random_engine;
    mActorOwner = Owner;
}

void PathLossModel::SetParams(const float TransmitPower,
                              const float ReceiverSensitivity,
                              const float Frequency,
                              const float combined_antenna_gain,
                              const float path_loss_exponent,
                              const float reference_distance_fspl,
                              const float filter_distance,
                              const bool use_etsi_fading,
                              const float custom_fading_stddev)
{
    this->TransmitPower = TransmitPower;
    this->ReceiverSensitivity = ReceiverSensitivity;
    this->path_loss_exponent = path_loss_exponent;
    this->reference_distance_fspl = reference_distance_fspl;
    this->filter_distance = filter_distance;
    this->use_etsi_fading = use_etsi_fading;
    this->custom_fading_stddev = custom_fading_stddev;
    this->combined_antenna_gain = combined_antenna_gain;
    PathLossModel::Frequency_GHz = Frequency;
    PathLossModel::Frequency = PathLossModel::Frequency_GHz * std::pow(10, 9);
    PathLossModel::lambda = PathLossModel::c_speedoflight / PathLossModel::Frequency;
    // when reference distance is set, we prepare the FSPL for the reference distance to be used in LDPL
    CalculateFSPL_d0();
}

void PathLossModel::SetScenario(EScenario scenario)
{
    this->scenario = scenario;
}

std::map<AActor *, float> PathLossModel::GetReceiveActorPowerList()
{
    return mReceiveActorPowerList;
}

void PathLossModel::Simulate(const std::vector<ActorPowerPair> ActorList, UCarlaEpisode *CarlaEpisode, UWorld *World)
{
    // Set current world and episode
    mWorld = World;
    mCarlaEpisode = CarlaEpisode;

    CurrentActorLocation = mActorOwner->GetTransform().GetLocation();
    FVector OtherActorLocation;
    mReceiveActorPowerList.clear();
    float ReceivedPower = 0;

    const FActorRegistry &Registry = mCarlaEpisode->GetActorRegistry();

    for (auto &actor_power_pair : ActorList)
    {
        const FCarlaActor *view = Registry.FindCarlaActor(actor_power_pair.first);
        // ensure other actor is still alive
        if (!view)
        {
            continue;
        }
        OtherActorLocation = actor_power_pair.first->GetTransform().GetLocation();

        // calculate relative ht and hr respecting slope and elevation
        //  cm
        // if objects are on a slope, minimum Z height of both is the reference to calculate transmitter height
        double ref0 = std::min(CurrentActorLocation.Z, OtherActorLocation.Z);
        // cm
        double ht = CurrentActorLocation.Z - ref0;
        double hr = OtherActorLocation.Z - ref0;
        // localize to common ref0 as ground
        FVector source_rel = CurrentActorLocation;
        FVector dest_rel = OtherActorLocation;

        double Distance3d = FVector::Distance(source_rel, dest_rel) / 100.0f; // From cm to m
        // to meters
        ht = ht / 100.0f;
        hr = hr / 100.0f;

        if (Distance3d < filter_distance) // maybe change this for highway
        {
            float OtherTransmitPower = actor_power_pair.second;
            ReceivedPower = CalculateReceivedPower(actor_power_pair.first,
                                                   OtherTransmitPower,
                                                   CurrentActorLocation,
                                                   OtherActorLocation,
                                                   Distance3d,
                                                   ht,
                                                   hr,
                                                   ref0);
            if (ReceivedPower > -1.0 * std::numeric_limits<float>::max())
            {
                mReceiveActorPowerList.insert(std::make_pair(actor_power_pair.first, ReceivedPower));
            }
        }
    }
}

float PathLossModel::CalculateReceivedPower(AActor *OtherActor,
                                            const float OtherTransmitPower,
                                            const FVector Source,
                                            const FVector Destination,
                                            const double Distance3d,
                                            const double ht,
                                            const double hr,
                                            const double reference_z)
{
    // hr in m
    // ht in m
    // reference_z in cm
    // distance3d in m
    bool ret = false;
    FCollisionObjectQueryParams ObjectParams;
    // Channels to check for collision with different object types
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Vehicle);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
    HitResult.Reset();

    FVector tx = Source;
    FVector rx = Destination;
    mWorld->LineTraceMultiByObjectType(HitResult, tx, rx, ObjectParams);

    // all losses
    float loss = ComputeLoss(OtherActor, Source, Destination, Distance3d, ht, hr, reference_z);

    // we incorporate the tx power of the sender (the other actor), not our own
    // NOTE: combined antenna gain is parametrized for each sensor. Better solution would be to parametrize individual antenna gain
    //       and combine at the receiver. This would allow to better account for antenna characteristics.
    float ReceivedPower = OtherTransmitPower + combined_antenna_gain - loss;

    if (ReceivedPower >= ReceiverSensitivity)
    {
        ret = true;
    }
    float deltaPercentage = loss / (OtherTransmitPower - ReceiverSensitivity);

    // Works only when run in debug mode
    DrawDebugLine(
        mWorld,
        tx,
        rx,
        FColor::Cyan,
        false, 0.05f, 0,
        30);
    if (deltaPercentage < 0.65)
    {
        DrawDebugLine(
            mWorld,
            tx,
            rx,
            FColor::Green,
            false, 0.1f, 0,
            30);
    }
    else if (deltaPercentage < 0.8)
    {
        DrawDebugLine(
            mWorld,
            tx,
            rx,
            FColor::Orange,
            false, 0.1f, 0,
            30);
    }
    else if (ReceivedPower >= ReceiverSensitivity)
    {
        DrawDebugLine(
            mWorld,
            tx,
            rx,
            FColor::Red,
            false, 0.1f, 0,
            30);
    }

    if (ret)
    {
        return ReceivedPower;
    }
    else
    {
        return -1.0 * std::numeric_limits<float>::max();
    }
}

void PathLossModel::EstimatePathStateAndVehicleObstacles(AActor *OtherActor,
                                                         FVector CurrentActorLocation,
                                                         double TxHeight,
                                                         double RxHeight,
                                                         double reference_z,
                                                         EPathState &state,
                                                         std::vector<FVector> &vehicle_obstacles)
{
    // CurrentActorLocation in cm original
    // TxHeight in m
    // RxHeight in m
    // reference_z in cm

    // init with LOS
    state = EPathState::LOS;
    if (HitResult.Num() == 0)
    {
        // no hits --> LOS
        return;
    }

    // check all hits
    for (const FHitResult &HitInfo : HitResult)
    {

        FVector location;
        if (HitIsSelfOrOther(HitInfo, OtherActor))
        {
            // the current hit is either Tx or Rx, so we can skip it, no obstacle
            continue;
        }

        // cal by ref
        if (GetLocationIfVehicle(CurrentActorLocation, HitInfo, reference_z, location))
        {
            // we found a vehicle
            // Note: we may set this several times if we have several vehicles in between
            state = EPathState::NLOSv;
            // z (height) is gonna be in m in relation to reference height
            // x,y also in meters
            //  call by reference
            vehicle_obstacles.emplace_back(location);

            // alternative (cf Etsi): statistical model
            //  vehicle_blockage_loss += MakeVehicleBlockageLoss(TxHeight, RxHeight, obj_height, obj_dist);
        }
        // but if we hit a building, we stop and switch to NLOSb
        else
        {
            state = EPathState::NLOSb;
            break;
        }
    }
}

double PathLossModel::CalcVehicleLoss(const double d1, const double d2, const double h)
{
    double V = h * sqrt(2.0 * (d1 + d2) / (lambda * d1 * d2));
    if (V >= -0.78) {
		double T = std::pow(V - 0.1, 2);
		return 6.9 + 20.0 * log10(sqrt(T + 1.0) + V - 0.1);
	}
    return 0.0;
}

double PathLossModel::CalculateNLOSvLoss(const FVector Source,
                                         const FVector Destination,
                                         const double TxHeight,
                                         const double RxHeight,
                                         const double RxDistance3d,
                                         std::vector<FVector> &vehicle_obstacles)
{

    // convert all positions to meters
    FVector pos_tx = Source;
    pos_tx.X /= 100.0f;
    pos_tx.Y /= 100.0f;
    pos_tx.Z = TxHeight;

    FVector pos_rx = Destination;
    pos_rx.X /= 100.0f;
    pos_rx.Y /= 100.0f;
    pos_rx.Z = RxHeight;

    double max_loss = 0.0;
    for(auto veh_it = vehicle_obstacles.begin(); veh_it != vehicle_obstacles.end(); veh_it++)
    {
        double dist_tx_veh = sqrt(std::pow(veh_it->X - pos_tx.X, 2) + std::pow(veh_it->Y - pos_tx.Y, 2));
        double dist_veh_rx = sqrt(std::pow(pos_rx.X - veh_it->X, 2) + std::pow(pos_rx.Y - veh_it->Y, 2));
        double cur_loss = CalcVehicleLoss(dist_tx_veh,dist_veh_rx,veh_it->Z);
        if(cur_loss >= max_loss)
        {
            max_loss = cur_loss;
        }
    }

    return max_loss;
}

void PathLossModel::SetPathLossModel(const EPathLossModel path_loss_model)
{
    model = path_loss_model;
}

float PathLossModel::ComputeLoss(AActor *OtherActor, FVector Source, FVector Destination, double Distance3d, double TxHeight, double RxHeight, double reference_z)
{
    // TxHeight in m
    // RxHeight in m
    // reference_z in cm
    // distance3d in m
    EPathState state;

    float PathLoss = 0.0;
    double VehicleBlockageLoss = 0.0;
    float ShadowFadingLoss = 0.0;

    // state and vehicle obstacles are call-by-reference
    std::vector<FVector> vehicle_obstacles;
    EstimatePathStateAndVehicleObstacles(OtherActor, Source, TxHeight, RxHeight, reference_z, state, vehicle_obstacles);

    if (model == EPathLossModel::Winner)
    {
        // calculate pure path loss depending on state and scenario
        PathLoss = CalculatePathLoss_WINNER(state, Distance3d);
        // in nlosv case, add multi knife edge
        if (state == EPathState::NLOSv)
        {
            PathLoss = PathLoss + CalculateNLOSvLoss(Source, Destination, TxHeight, RxHeight, Distance3d, vehicle_obstacles);
        }
    }
    else
    {
        if (state == EPathState::LOS)
        {
            // full two ray path loss
            PathLoss = CalculateTwoRayPathLoss(Distance3d, TxHeight, RxHeight);
        }
        else if (state == EPathState::NLOSb)
        {
            // log distance path loss
            PathLoss = (m_fspl_d0 + 10.0 * path_loss_exponent * log10(Distance3d / reference_distance_fspl));
        }
        else
        {
            // fspl + knife edge
            // fspl
            double free_space_loss = 20.0 * log10(Distance3d) + 20.0 * log10(4.0 * PI / lambda);
            // add the knife edge vehicle blockage loss
            PathLoss = free_space_loss + CalculateNLOSvLoss(Source, Destination, TxHeight, RxHeight, Distance3d, vehicle_obstacles);
        }
    }

    // add random shadows
    ShadowFadingLoss = CalculateShadowFading(state);
    return PathLoss + ShadowFadingLoss;
}

bool PathLossModel::IsVehicle(const FHitResult &HitInfo)
{
    bool Vehicle = false;
    const FActorRegistry &Registry = mCarlaEpisode->GetActorRegistry();

    const AActor *actor = HitInfo.Actor.Get();

    if (actor != nullptr)
    {

        const FCarlaActor *view = Registry.FindCarlaActor(actor);
        if (view)
        {
            if (view->GetActorType() == FCarlaActor::ActorType::Vehicle)
            {
                Vehicle = true;
            }
        }
    }
    return Vehicle;
}

bool PathLossModel::HitIsSelfOrOther(const FHitResult &HitInfo, AActor *OtherActor)
{
    const AActor *actor = HitInfo.Actor.Get();
    if (actor != nullptr)
    {
        if (actor == mActorOwner)
        {
            return true;
        }
        else if (actor == OtherActor)
        {
            return true;
        }
    }
    return false;
}

bool PathLossModel::GetLocationIfVehicle(const FVector CurrentActorLocation, const FHitResult &HitInfo, const double reference_z, FVector &location)
{
    // reference_z in cm
    bool Vehicle = false;
    const FActorRegistry &Registry = mCarlaEpisode->GetActorRegistry();

    const AActor *actor = HitInfo.Actor.Get();

    if (actor != nullptr)
    {

        const FCarlaActor *view = Registry.FindCarlaActor(actor);
        if (view)
        {
            if (view->GetActorType() == FCarlaActor::ActorType::Vehicle)
            {
                Vehicle = true;
                location = actor->GetTransform().GetLocation();
                location.Z = location.Z - reference_z + (actor->GetSimpleCollisionHalfHeight() * 2.0) + 2.0;
                location.Z = location.Z / 100.0f;
                // cm to m
                location.X /= 100.0f;
                location.Y /= 100.0f;
            }
        }
    }
    return Vehicle;
}

void PathLossModel::CalculateFSPL_d0()
{
    m_fspl_d0 = 20.0 * log10(reference_distance_fspl) + 20.0 * log10(Frequency) + 20.0 * log10(4.0 * PI / c_speedoflight);
}

// Following ETSI TR 103 257-1 V1.1.1 (2019-05: from WINNER Project Board: "D5.3 - WINNER+ Final Channel Models", 30 06 2010.
float PathLossModel::CalculatePathLoss_WINNER(EPathState state, double Distance)
{

    if (state == EPathState::NLOSb)
    {
        // Distance should be euclidean here.
        return 36.85f + 30.0f * log10(Distance) + 18.9f * log10(Frequency_GHz);
    }
    else // LOS, NLOSv
    {
        if (scenario == EScenario::Highway)
        {
            return 32.4f + 20.0f * log10(Distance) + 20.0f * log10(Frequency_GHz);
        }
        else // Rural or Urban
        {
            return 38.77f + 16.7f * log10(Distance) + 18.2f * log10(Frequency_GHz);
        }
    }
}

// Following ETSI TR 103 257-1 V1.1.1 Table 6: Shadow-fading parameter σ for V2V
float PathLossModel::CalculateShadowFading(EPathState state)
{
    const float Mean = 0.0f;
    float std_dev_dB; // = 5.2;
    if (use_etsi_fading)
    {
        switch (state)
        {
        case EPathState::LOS:
            switch (scenario)
            {
            case EScenario::Highway:
                std_dev_dB = 3.3f;
                break;
            case EScenario::Urban:
                std_dev_dB = 5.2f;
                break;
            case EScenario::Rural:
                // no known values in ETSI Standard, take middle of Highway and Urban
                std_dev_dB = 4.25f;
                break;
            }
            break;
        case EPathState::NLOSb:
            switch (scenario)
            {
            // Note: according to ETSI, NLOSb is not applicable in Highway scenario, only Urban
            //  we use the same std_dev for all three scenarios if in NLOSb
            case EScenario::Highway:
                std_dev_dB = 6.8f;
                break;
            case EScenario::Urban:
                std_dev_dB = 6.8f;
                break;
            case EScenario::Rural:
                std_dev_dB = 6.8f;
                break;
            }
            break;
        case EPathState::NLOSv:
            switch (scenario)
            {
            case EScenario::Highway:
                std_dev_dB = 3.8f;
                break;
            case EScenario::Urban:
                std_dev_dB = 5.3f;
                break;
            case EScenario::Rural:
                // no known values in ETSI Standard, take middle of Highway and Urban
                std_dev_dB = 4.55f;
                break;
            }
            break;
        }
    }
    else
    {
        // custom fading param
        std_dev_dB = custom_fading_stddev;
    }

    // in dB
    return mRandomEngine->GetNormalDistribution(Mean, std_dev_dB);
}

float PathLossModel::CalculateTwoRayPathLossSimple(double Distance3d, double TxHeight, double RxHeight)
{
    // simplified Two Ray Path Loss (https://en.wikipedia.org/wiki/Two-ray_ground-reflection_model)
    //  is only a valid assumption, if distance >> 4 pi TxHeight * RxHeight / lambda
    //  with f=5.9 Ghz ->  lambda = c/ f = 0.05m
    //  and with average antenna height 2m: distance >> 1000m
    return 40 * log10(Distance3d) - 10 * log10(TxHeight * TxHeight * RxHeight * RxHeight);
}

double PathLossModel::CalculateTwoRayPathLoss(double Distance3d, double TxHeight, double RxHeight)
{
    // tx and rx height in m
    //  distance 3d is LOS in m
    double d_ground = sqrt(std::pow(Distance3d, 2) - std::pow(TxHeight - RxHeight, 2));

    // reflected path
    // d_reflection = sqrt(d_ground^2+(ht+hr)^2) -> with d_ground = sqrt(d_los^2 - (ht-hr)^2):
    double d_refl = sqrt(std::pow(Distance3d, 2) + 4.0 * TxHeight * RxHeight);

    // Sine and cosine of incident angle
    double sin_theta = (TxHeight + RxHeight) / d_refl;
    double cos_theta = d_ground / d_refl;

    double gamma = (sin_theta - sqrt(epsilon_r - std::pow(cos_theta, 2))) / (sin_theta + sqrt(epsilon_r - std::pow(cos_theta, 2)));

    double phi = (2.0 * PI / lambda * (Distance3d - d_refl));

    return 20 * log10(4.0 * PI * d_ground / lambda * 1.0 / sqrt(std::pow(1 + gamma * cos(phi), 2) + std::pow(gamma, 2) * std::pow(sin(phi), 2)));
}
