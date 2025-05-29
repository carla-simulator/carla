#pragma once

#include "Components/SplineComponent.h"
#include "JsonObjectConverter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeometryImporter.generated.h"

UCLASS(Blueprintable, BlueprintType)
class CARLADIGITALTWINSTOOL_API UGeometryImporter : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "GeometryImporter")
    TArray<FVector2D> ReadCSVCoordinates(FString Path, FVector2D OriginGeoCoordinates);

    UFUNCTION(BlueprintCallable, Category = "GeometryImporter")
    static USplineComponent* CreateSpline(UWorld* World, const TArray<FVector>& Points, const FString SplineName);

    UFUNCTION(BlueprintCallable, Category = "GeometryImporter")
    static TArray<USplineComponent*> ImportGeoJsonPolygonsToSplines(UWorld* World, const FString& GeoJsonFilePath, const FVector2D OriginGeoCoordinates);
};
