// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>


using ActorPowerMap = std::map<AActor *, float>;
using ActorPowerPair = std::pair<AActor *, float>;

enum EPathState
{
    LOS,
    NLOSb,
    NLOSv
};

enum EPathLossModel
{
    Winner,
    Geometric,
};

enum EScenario
{
    Highway,
    Rural,
    Urban
};

class PathLossModel
{
public:
    PathLossModel(URandomEngine *random_engine, AActor *Owner);
    void SetScenario(EScenario scenario);
    void Simulate(const std::vector<ActorPowerPair> ActorList, UCarlaEpisode *CarlaEpisode, UWorld *World);
    ActorPowerMap GetReceiveActorPowerList();
    void SetParams(const float TransmitPower,
                   const float ReceiverSensitivity,
                   const float Frequency,
                   const float combined_antenna_gain,
                   const float path_loss_exponent,
                   const float reference_distance_fspl,
                   const float filter_distance,
                   const bool use_etsi_fading,
                   const float custom_fading_stddev);
    float GetTransmitPower() { return TransmitPower; }
    void SetPathLossModel(const EPathLossModel path_loss_model);

private:
    // diffraction for NLOSv
    double CalcVehicleLoss(const double d1, const double d2, const double h);
    // powers
    float CalculateReceivedPower(AActor *OtherActor,
                                 const float OtherTransmitPower,
                                 const FVector Source,
                                 const FVector Destination,
                                 const double Distance3d,
                                 const double ht,
                                 const double hr,
                                 const double reference_z);
    void EstimatePathStateAndVehicleObstacles(AActor *OtherActor, FVector Source, double TxHeight, double RxHeight, double reference_z, EPathState &state, std::vector<FVector> &vehicle_obstacles);
    double MakeVehicleBlockageLoss(double TxHeight, double RxHeight, double obj_height, double obj_distance);
    // variables
    AActor *mActorOwner;
    UCarlaEpisode *mCarlaEpisode;
    UWorld *mWorld;
    URandomEngine *mRandomEngine;

    ActorPowerMap mReceiveActorPowerList;
    FVector CurrentActorLocation;

    // constants
    constexpr static float c_speedoflight = 299792458.0; // m/s

    // full two ray path loss
    const double epsilon_r = 1.02;

    // params
    static double Frequency_GHz;   // 5.9f;//5.9 GHz
    static double Frequency;       // Frequency_GHz * std::pow(10,9);
    static double lambda;          // c_speedoflight/Frequency;
    float reference_distance_fspl; // m
    float TransmitPower;           // dBm
    float ReceiverSensitivity;     // dBm
    EScenario scenario;
    float path_loss_exponent; // no unit, default 2.7;
    float filter_distance;    // in meters  default 500.0
    EPathLossModel model;
    bool use_etsi_fading;
    float custom_fading_stddev;
    float combined_antenna_gain; // 10.0 dBi

    // dependent params that are precalculated on setting of params
    float m_fspl_d0;

protected:
    /// Method that allow to preprocess if the rays will be traced.

    float ComputeLoss(AActor *OtherActor, FVector Source, FVector Destination, double Distance3d, double TxHeight, double RxHeight, double reference_z);
    bool IsVehicle(const FHitResult &HitInfo);
    bool GetLocationIfVehicle(const FVector CurrentActorLocation, const FHitResult &HitInfo, const double reference_z, FVector &location);
    bool HitIsSelfOrOther(const FHitResult &HitInfo, AActor *OtherActor);
    float CalculatePathLoss_WINNER(EPathState state, double Distance);
    double CalculateNLOSvLoss(const FVector Source, const FVector Destination, const double TxHeight, const double RxHeight, const double RxDistance3d, std::vector<FVector> &vehicle_obstacles);

    float CalculateShadowFading(EPathState state);

    // full two ray model
    double CalculateTwoRayPathLoss(double Distance3d, double TxHeight, double RxHeight);
    // simplified two ray model
    float CalculateTwoRayPathLossSimple(double Distance3d, double TxHeight, double RxHeight);

    // functions for precalculation
    void CalculateFSPL_d0();
    TArray<FHitResult> HitResult;
};
