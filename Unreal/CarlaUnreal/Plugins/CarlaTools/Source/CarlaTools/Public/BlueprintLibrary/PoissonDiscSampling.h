// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Engine headers
#include "PCGSettings.h"
#include "Metadata/PCGAttributePropertySelector.h"
#include "PoissonDiscSampling.generated.h"

/**
 * Various fractal noises that can be used to filter points
 */
UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGPoissonDiscSamplingSettings : public UPCGSettings
{
  GENERATED_BODY()

public:
  UPCGPoissonDiscSamplingSettings();

  //~Begin UPCGSettings interface
#if WITH_EDITOR
  virtual FName GetDefaultNodeName() const override { return FName(TEXT("Poisson Disc Sampling")); }
  virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGPoissonDiscSampling", "NodeTitle", "Poisson Disc Sampling"); }
  virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

protected:
  virtual TArray<FPCGPinProperties> InputPinProperties() const override;
  virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

  virtual FPCGElementPtr CreateElement() const override;
  //~End UPCGSettings interface

public:

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
  int32 SplineSampleCount = 10;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
  float MinDistance = 100.0F;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
  int32 MaxRetries = 32;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
  bool bFilterInsideSpline = true;

};

class FPCGPoissonDiscSampling : public IPCGElement
{
protected:

	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(
        const UPCGSettings* Settings) const override
    {
        return EPCGElementExecutionLoopMode::SinglePrimaryPin;
    }
};
