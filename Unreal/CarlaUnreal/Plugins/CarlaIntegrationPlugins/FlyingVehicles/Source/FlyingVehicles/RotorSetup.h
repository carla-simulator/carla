#pragma once

#include "RotorSetup.generated.h"

USTRUCT(BlueprintType)
struct FLYINGVEHICLES_API FRotorSetup
{
    GENERATED_BODY()

    UPROPERTY(Category = "Rotor Setup", EditAnywhere, BlueprintReadWrite)
    float ThrustCoefficient = 0.109919F;

    UPROPERTY(Category = "Rotor Setup", EditAnywhere, BlueprintReadWrite)
    float TorqueCoefficient = 0.040164F;

    UPROPERTY(Category = "Rotor Setup", EditAnywhere, BlueprintReadWrite)
    float MaxRPM = 6396.667F; // revolutions per minute

    UPROPERTY(Category = "Rotor Setup", EditAnywhere, BlueprintReadWrite)
    float PropellerDiameter = 0.2286F; //diameter in meters, default is for DJI Phantom 2

    UPROPERTY(Category = "Rotor Setup", EditAnywhere, BlueprintReadWrite)
    float PropellerHeight = .01F; //height of cylindrical area when propeller rotates, 1 cm

    UPROPERTY(Category = "Rotor Setup", EditAnywhere, BlueprintReadWrite)
    bool Clockwise = true; // If false, spins counter-clockwise
};
