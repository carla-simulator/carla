#include "Carla/Util/GeometryImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Modules/ModuleManager.h"
#include "BlueprintLibary/MapGenFunctionLibrary.h"
#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "AutomatedAssetImportData.h"
#include "Factories/FbxFactory.h"
#include "Editor.h"
#endif

DEFINE_LOG_CATEGORY(LogGeometryImporter);

TArray<FVector2D> UGeometryImporter::ReadCSVCoordinates(FString Path, FVector2D OriginGeoCoordinates)
{
    UE_LOG(LogTemp, Warning, TEXT("Reading latlon coordinates"));

    TArray<FVector2D> Coordinates;

    FString FileContent;

    if (FFileHelper::LoadFileToString(FileContent, *Path))
    {
        TArray<FString> Lines;
        FileContent.ParseIntoArrayLines(Lines);

        for (int32 i = 0; i < Lines.Num(); ++i)
        {
            FString Line = Lines[i];
            TArray<FString> Columns;
            Line.ParseIntoArray(Columns, TEXT(","), true);

            if (Columns.Num() >= 2)
            {
                float X = FCString::Atof(*Columns[0]);
                float Y = FCString::Atof(*Columns[1]);
                FVector2D Pos = UMapGenFunctionLibrary::GetTransversemercProjection(Y, X, OriginGeoCoordinates.X, OriginGeoCoordinates.Y);
                Coordinates.Add(Pos);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to read file at: %s"), *Path);
    }

    return Coordinates;
}

USplineComponent *UGeometryImporter::CreateSpline(UWorld *World, const TArray<FVector> &Points, const FString SplineName)
{

    if (!World || Points.Num() < 2)
    {
        UE_LOG(LogTemp, Log, TEXT("Invalid world pointer"));
        return nullptr;
    }

    AActor *SplineActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!SplineActor)
    {
        UE_LOG(LogGeometryImporter, Log, TEXT("Spline actor not created"));
        return nullptr;
    }
#if WITH_EDITOR
    SplineActor->SetActorLabel(SplineName);
#endif
    SplineActor->Tags.Add(FName("curb"));

    USplineComponent *Spline = NewObject<USplineComponent>(SplineActor);
    Spline->ClearSplinePoints();
    Spline->RegisterComponent();
    Spline->SetMobility(EComponentMobility::Static);
    SplineActor->SetRootComponent(Spline);

    for (int32 i = 0; i < Points.Num(); ++i)
    {
        Spline->AddSplinePoint(Points[i], ESplineCoordinateSpace::World);
    }

    for (int32 i = 0; i < Spline->GetNumberOfSplinePoints(); ++i)
    {
        Spline->SetSplinePointType(i, ESplinePointType::Curve, false);
    }

    Spline->UpdateSpline();

    const float TangentScale = 0.25f;  // 0 = no curve, 1 = full handle length = distance to neighbor
    int32 Num = Spline->GetNumberOfSplinePoints();

    for (int32 i = 0; i < Num; ++i)
    {
        FVector Current = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
        FVector Prev = Spline->GetLocationAtSplinePoint(FMath::Max(i - 1, 0), ESplineCoordinateSpace::Local);
        FVector Next = Spline->GetLocationAtSplinePoint(FMath::Min(i + 1, Num - 1), ESplineCoordinateSpace::Local);
        auto D1 = FVector::Distance(Current, Prev);
        auto D2 = FVector::Distance(Current, Next);
        auto Clamped = FMath::Min(D1, D2);

        // Direction toward next point
        FVector Dir = (Next - Prev) * 0.5f;
        float DesiredLength = FMath::Min(Dir.Size() * TangentScale, Clamped);
        FVector Tangent = Dir.GetSafeNormal() * DesiredLength;

        // Arrive tangent points backward toward Prev, leave tangent toward Next
        Spline->SetTangentAtSplinePoint(i, Tangent, ESplineCoordinateSpace::Local, false);
    }

    Spline->UpdateSpline();

    Spline->SetClosedLoop(true);
    Spline->UpdateSpline();

    return Spline;
}

TArray<USplineComponent *> UGeometryImporter::ImportGeoJsonPolygonsToSplines(UWorld *World, const FString &GeoJsonFilePath, const FVector2D OriginGeoCoordinates)
{
    UE_LOG(LogTemp, Log, TEXT("Importing geojson and creating splines from file: %s"), *GeoJsonFilePath);

    TArray<USplineComponent *> CreatedSplines;

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *GeoJsonFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load GeoJSON file: %s"), *GeoJsonFilePath);
        return CreatedSplines;
    }

    TSharedPtr<FJsonObject> JsonParsed;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonParsed) || !JsonParsed.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse GeoJSON file."));
        return CreatedSplines;
    }

    const TArray<TSharedPtr<FJsonValue>> *Features;
    if (!JsonParsed->TryGetArrayField(TEXT("features"), Features))
    {
        UE_LOG(LogTemp, Error, TEXT("No 'features' array found in GeoJSON."));
        return CreatedSplines;
    }

    int i = 0;
    for (const TSharedPtr<FJsonValue> &FeatureValue : *Features)
    {
        const TSharedPtr<FJsonObject> FeatureObj = FeatureValue->AsObject();
        const TSharedPtr<FJsonObject> Geometry = FeatureObj->GetObjectField(TEXT("geometry"));

        FString GeometryType = Geometry->GetStringField(TEXT("type"));
        if (GeometryType != "Polygon")
            continue;

        const TArray<TSharedPtr<FJsonValue>> *Rings;
        if (!Geometry->TryGetArrayField(TEXT("coordinates"), Rings) || Rings->Num() == 0)
            continue;

        const TArray<TSharedPtr<FJsonValue>> &OuterRing = (*Rings)[0]->AsArray();
        TArray<FVector> Points;

        for (const auto &Coord : OuterRing)
        {
            const TArray<TSharedPtr<FJsonValue>> &CoordArray = Coord->AsArray();
            double Lon = CoordArray[0]->AsNumber();
            double Lat = CoordArray[1]->AsNumber();
            FVector2D Pos2D = UMapGenFunctionLibrary::GetTransversemercProjection(Lat, Lon, OriginGeoCoordinates.X, OriginGeoCoordinates.Y);
            FVector Pos = FVector(Pos2D.X, Pos2D.Y, 0.0f); // Initialize height as 0
            Points.Add(Pos);
        }

        // Remove last point if it's a duplicate of the first
        if (Points.Num() > 1 && Points[0].Equals(Points.Last(), 0.01f))
        {
            Points.Pop();
        }

        FString SplineName = "Spline_" + FString::FromInt(i);
        USplineComponent *Spline = CreateSpline(World, Points, SplineName);
        if (Spline)
        {
            CreatedSplines.Add(Spline);
        }

        i++;
    }

    return CreatedSplines;
}

TArray<USplineComponent *> UGeometryImporter::CreateSplinesFromJson(
    UWorld *World,
    const FString &JsonFilePath,
    FVector2D Center,
    FVector2D Extent,
    FIntPoint RTExtent)
{
    TArray<USplineComponent *> CreatedSplines;

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
    {
        UE_LOG(LogGeometryImporter, Error, TEXT("Failed to load JSON from: %s"), *JsonFilePath);
        return CreatedSplines;
    }

    TSharedPtr<FJsonValue> RootValue;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    if (!FJsonSerializer::Deserialize(Reader, RootValue) || !RootValue.IsValid())
    {
        UE_LOG(LogGeometryImporter, Error, TEXT("Failed to parse JSON"));
        return CreatedSplines;
    }

    const TArray<TSharedPtr<FJsonValue>> *ContourArray;
    if (!RootValue->TryGetArray(ContourArray))
        return CreatedSplines;

    float ZOffset = 0.0f;

    for (int32 ContourIdx = 0; ContourIdx < ContourArray->Num(); ++ContourIdx)
    {
        const TArray<TSharedPtr<FJsonValue>> *PointList;
        if (!(*ContourArray)[ContourIdx]->TryGetArray(PointList))
            continue;

        TArray<FVector> Points;

        for (const TSharedPtr<FJsonValue> &PointVal : *PointList)
        {
            const TArray<TSharedPtr<FJsonValue>> *XY;
            if (!(PointVal->TryGetArray(XY) && XY->Num() == 2))
                continue;
            double X = 0.0, Y = 0.0;
            if (!((*XY)[0]->TryGetNumber(X) && (*XY)[1]->TryGetNumber(Y)))
                continue;
            auto UV = FVector2D(X, Y);
            UV /= FVector2D(RTExtent);
            UV.Y = 1.0F - UV.Y;
            UV -= FVector2D(0.5F, 0.5F);
            UV.X = -UV.X;
            FVector2D P = Center + UV * Extent;
            Points.Add(FVector(P.X, P.Y, ZOffset));
        }

        FString Name = FString::Printf(TEXT("Spline_%d"), ContourIdx);
        USplineComponent *Spline = CreateSpline(World, Points, Name);
        if (Spline)
        {
            CreatedSplines.Add(Spline);
        }
    }
    return CreatedSplines;
}

void UGeometryImporter::ImportObj(const FString &ObjFilePath, UWorld *World, UMaterialInterface *Material)
{
#if WITH_EDITOR
    if (!FPaths::FileExists(ObjFilePath))
    {
        UE_LOG(LogGeometryImporter, Error, TEXT("File does not exist: %s"), *ObjFilePath);
        return;
    }

    // Get AssetTools
    FAssetToolsModule &AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    // Setup import data
    UFbxFactory *ObjFactory = NewObject<UFbxFactory>();
    ObjFactory->AddToRoot(); // prevent GC

    UAutomatedAssetImportData *ImportData = NewObject<UAutomatedAssetImportData>();
    ImportData->Factory = ObjFactory;
    ImportData->Filenames.Add(ObjFilePath);
    ImportData->DestinationPath = TEXT("/Game/ImportedMeshes");
    ImportData->bReplaceExisting = true;

    // Perform the import
    TArray<UObject *> ImportedAssets = AssetToolsModule.Get().ImportAssetsAutomated(ImportData);

    if (ImportedAssets.Num() > 0)
    {
        if (UStaticMesh *ImportedMesh = Cast<UStaticMesh>(ImportedAssets[0]))
        {
            // Spawn the mesh in the world
            FActorSpawnParameters SpawnParams;
            FVector SpawnLocation(0, 0, 0);
            AStaticMeshActor *MeshActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator, SpawnParams);

            if (MeshActor)
            {
                MeshActor->GetStaticMeshComponent()->SetStaticMesh(ImportedMesh);
                MeshActor->SetActorLabel(TEXT("UpdatedRoad"));

                UStaticMeshComponent *MeshComp = MeshActor->GetStaticMeshComponent();
                MeshComp->SetStaticMesh(ImportedMesh);

                // Assign material
                if (Material)
                {
                    MeshComp->SetMaterial(0, Material);
                }
                else
                {
                    UE_LOG(LogGeometryImporter, Warning, TEXT("Material not found."));
                }

                UE_LOG(LogGeometryImporter, Log, TEXT("Imported and spawned mesh: %s"), *ImportedMesh->GetName());
            }
        }
        else
        {
            UE_LOG(LogGeometryImporter, Error, TEXT("Imported object is not a static mesh."));
        }
    }
    else
    {
        UE_LOG(LogGeometryImporter, Error, TEXT("No assets imported from file: %s"), *ObjFilePath);
    }
#else
    UE_LOG(LogGeometryImporter, Error, TEXT("ImportObj is editor-only; cannot import \"%s\"."), *ObjFilePath);
#endif
}
