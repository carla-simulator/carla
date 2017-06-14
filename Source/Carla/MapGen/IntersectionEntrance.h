// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/Actor.h"
#include "IntersectionEntrance.generated.h"

USTRUCT(BlueprintType)
struct FRoute {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category=TrafficRoutes, EditAnywhere)
	TArray < AActor *> points;
	
	UPROPERTY(BlueprintReadWrite, Category=TrafficRoutes, EditAnywhere)
	float probability = 0.0f;
};


UCLASS(BlueprintType)
class CARLA_API AIntersectionEntrance : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIntersectionEntrance(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

UFUNCTION(BlueprintCallable, Category="Trigger")
    TArray<FVector> GetRoute(int route);

UFUNCTION(BlueprintCallable, Category="Trigger")
    float GetProbability(int route);
/*
#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
*/
public:

	UPROPERTY(Category = "Routes", EditAnywhere)
  	bool bCreateRoutes = false;


	UPROPERTY(BlueprintReadWrite, Category=TrafficRoutes, EditAnywhere)
	TArray< FRoute > Routes;
	
};
