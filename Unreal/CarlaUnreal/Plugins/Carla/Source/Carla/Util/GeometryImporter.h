#pragma once

#include "Components/SplineComponent.h"
#include "JsonObjectConverter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeometryImporter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGeometryImporter, Log, All);

UCLASS(Blueprintable, BlueprintType)
class CARLA_API UGeometryImporter : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "GeometryImporter")
    TArray<FVector2D> ReadCSVCoordinates(FString Path, FVector2D OriginGeoCoordinates);

    UFUNCTION(BlueprintCallable, Category = "GeometryImporter")
    static USplineComponent *CreateSpline(UWorld *World, const TArray<FVector> &Points, const FString SplineName);

    UFUNCTION(BlueprintCallable, Category = "GeometryImporter")
    static TArray<USplineComponent *> ImportGeoJsonPolygonsToSplines(UWorld *World, const FString &GeoJsonFilePath, const FVector2D OriginGeoCoordinates);

    // carla-digitaltwins port: contour list (JSON array of arrays of [x, y] render
    // target pixels) to world-space closed splines.
    UFUNCTION(BlueprintCallable, Category = "GeometryImporter")
    static TArray<USplineComponent *> CreateSplinesFromJson(
        UWorld *World,
        const FString &JsonFilePath,
        FVector2D Center,
        FVector2D Extent,
        FIntPoint RTExtent);

    // carla-digitaltwins port: import an OBJ/FBX mesh and spawn it (editor only;
    // no-op in non-editor builds).
    UFUNCTION(BlueprintCallable, Category = "GeometryImporter")
    static void ImportObj(const FString &ObjFilePath, UWorld *World, UMaterialInterface *Material);
};
