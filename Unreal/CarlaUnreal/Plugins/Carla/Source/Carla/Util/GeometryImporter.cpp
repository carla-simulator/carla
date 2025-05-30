#include "Carla/Util/GeometryImporter.h"
#include "Misc/FileHelper.h"
#include "Engine/Engine.h"
#include "BlueprintLibary/MapGenFunctionLibrary.h"

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
        UE_LOG(LogTemp, Log, TEXT("Spline actor not created"));
        return nullptr;
    }
    // SplineActor->Rename(*SplineName);

    USplineComponent *Spline = NewObject<USplineComponent>(SplineActor);
    Spline->ClearSplinePoints();
    Spline->RegisterComponent();
    Spline->SetMobility(EComponentMobility::Movable);
    SplineActor->SetRootComponent(Spline);

    for (int32 i = 0; i < Points.Num(); ++i)
    {
        Spline->AddSplinePoint(Points[i], ESplineCoordinateSpace::World);
    }

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
    if (!JsonParsed->TryGetArrayField("features", Features))
    {
        UE_LOG(LogTemp, Error, TEXT("No 'features' array found in GeoJSON."));
        return CreatedSplines;
    }

    int i = 0;
    for (const TSharedPtr<FJsonValue> &FeatureValue : *Features)
    {
        const TSharedPtr<FJsonObject> FeatureObj = FeatureValue->AsObject();
        const TSharedPtr<FJsonObject> Geometry = FeatureObj->GetObjectField("geometry");

        FString GeometryType = Geometry->GetStringField("type");
        if (GeometryType != "Polygon")
            continue;

        const TArray<TSharedPtr<FJsonValue>> *Rings;
        if (!Geometry->TryGetArrayField("coordinates", Rings) || Rings->Num() == 0)
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
