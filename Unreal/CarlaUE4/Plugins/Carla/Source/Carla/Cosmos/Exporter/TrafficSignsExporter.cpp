// TrafficPolesExporter.cpp

#include "Carla/Cosmos/Exporter/TrafficSignsExporter.h"
#include "Carla/Traffic/TrafficSignBase.h"

// UE
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
// CARLA


FString UTrafficSignsExporter::MakeStableLabelId(const FString& Uuid, const FVector& P0M, const FVector& P1M)
{
	struct { float x0,y0,z0,x1,y1,z1; } S{ P0M.X, P0M.Y, P0M.Z, P1M.X, P1M.Y, P1M.Z };
	uint32 Crc = FCrc::MemCrc32(&S, sizeof(S));
	FString Hex = FString::Printf(TEXT("%08x-%04x-%04x-%04x-%012x"),
		(Crc) & 0xFFFFFFFFu,
		(Crc >> 16) & 0xFFFFu,
		(Crc ^ 0xA5A5) & 0xFFFFu,
		(Crc ^ 0x5A5A) & 0xFFFFu,
		(Crc * 2654435761u) & 0xFFFFFFFFFFFFu);
	return FString::Printf(TEXT("mads:000:%s:000000"), *Hex);
}

// --------------------- Poles (unchanged) ---------------------

void UTrafficSignsExporter::BuildPolePolylineMeters(const UStaticMeshComponent* C, FVector& OutTopM, FVector& OutBaseM)
{
	FVector Origin, Extent;
	C->GetLocalBounds(Origin, Extent);

	const FTransform& T = C->GetComponentTransform();
	const FVector UpWS = T.GetUnitAxis(EAxis::Z);
	const FVector CenterWS = T.TransformPosition(Origin);
	const float HalfHeightWS = T.GetScale3D().Z * Extent.Z;

	const FVector TopWS  = CenterWS + UpWS * HalfHeightWS;
	const FVector BaseWS = CenterWS - UpWS * HalfHeightWS;

	OutTopM  = TopWS  / 100.0f;
	OutBaseM = BaseWS / 100.0f;
}

FString UTrafficSignsExporter::ClassifyPoleType(const UStaticMeshComponent* C)
{
	const TArray<FName>& Tags = C->ComponentTags;
	if (Tags.Contains("sign"))  return TEXT("SIGN");
	if (Tags.Contains("light")) return TEXT("SENTRY");
	if (Tags.Contains("tree"))  return TEXT("TREE");
	return TEXT("SENTRY");
}

void UTrafficSignsExporter::AppendPoleLabelJson(
	TArray<TSharedPtr<FJsonValue>>& Labels,
	const FString& Uuid,
	const FString& StartTs,
	const FVector& TopM,
	const FVector& BaseM,
	const FString& PoleTypeText)
{
	TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();
	Label->SetStringField(TEXT("labelFamily"), TEXT("SHAPE3D"));

	// assetRef
	{
		TSharedRef<FJsonObject> AssetRef = MakeShared<FJsonObject>();
		AssetRef->SetStringField(TEXT("sessionId"), Uuid);
		AssetRef->SetStringField(TEXT("sensorName"), TEXT("lidar_gt_top_p128"));
		TSharedRef<FJsonObject> FramesObj = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> Frames; Frames.Add(MakeShared<FJsonValueNumber>(0));
		FramesObj->SetArrayField(TEXT("frames"), Frames);
		AssetRef->SetObjectField(TEXT("frames"), FramesObj);
		Label->SetObjectField(TEXT("assetRef"), AssetRef);
	}

	// labelClassKey
	{
		TSharedRef<FJsonObject> ClassKey = MakeShared<FJsonObject>();
		ClassKey->SetStringField(TEXT("labelClassNamespace"), TEXT("minimap"));
		ClassKey->SetStringField(TEXT("labelClassIdentifier"), TEXT("poles:autolabels"));
		ClassKey->SetStringField(TEXT("labelClassVersion"), TEXT("v0"));
		Label->SetObjectField(TEXT("labelClassKey"), ClassKey);
	}

	Label->SetStringField(TEXT("labelId"), MakeStableLabelId(Uuid, TopM, BaseM));

	// labelData.shape3d
	TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
	{
		TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();

		// attributes
		{
			TArray<TSharedPtr<FJsonValue>> Attrs;
			auto AddAttrText = [&](const TCHAR* Name, const FString& Text)
			{
				TSharedRef<FJsonObject> A = MakeShared<FJsonObject>();
				A->SetStringField(TEXT("name"), Name);
				A->SetStringField(TEXT("text"), Text);
				Attrs.Add(MakeShared<FJsonValueObject>(A));
			};

			AddAttrText(TEXT("coordinate_frame"), TEXT("rig"));
			AddAttrText(TEXT("timestamp"),        StartTs);
			AddAttrText(TEXT("asset_ref"),        TEXT(""));
			AddAttrText(TEXT("feature_id"),       TEXT(""));
			AddAttrText(TEXT("feature_version"),  TEXT(""));
			AddAttrText(TEXT("clip_version_id"),  TEXT(""));

			// label_name enum
			{
				TSharedRef<FJsonObject> A = MakeShared<FJsonObject>();
				A->SetStringField(TEXT("name"), TEXT("label_name"));
				A->SetStringField(TEXT("enum"), TEXT("pole"));
				Attrs.Add(MakeShared<FJsonValueObject>(A));
			}

			AddAttrText(TEXT("pole_type"), PoleTypeText);

			Shape3D->SetArrayField(TEXT("attributes"), Attrs);
		}

		Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

		// polyline3d.vertices : [Top, Base]
		{
			TSharedRef<FJsonObject> Polyline = MakeShared<FJsonObject>();
			TArray<TSharedPtr<FJsonValue>> Verts;

			auto AddTriple = [&](const FVector& V)
			{
				TArray<TSharedPtr<FJsonValue>> Tpl;
				Tpl.Add(MakeShared<FJsonValueNumber>(V.X));
				Tpl.Add(MakeShared<FJsonValueNumber>(V.Y));
				Tpl.Add(MakeShared<FJsonValueNumber>(V.Z));
				Verts.Add(MakeShared<FJsonValueArray>(Tpl));
			};

			AddTriple(TopM);
			AddTriple(BaseM);

			Polyline->SetArrayField(TEXT("vertices"), Verts);
			Shape3D->SetObjectField(TEXT("polyline3d"), Polyline);
		}

		LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
	}
	Label->SetObjectField(TEXT("labelData"), LabelData);
	Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

	Labels.Add(MakeShared<FJsonValueObject>(Label));
}

bool UTrafficSignsExporter::ExportCosmosTrafficSigns(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
	OutError.Reset();

	FString Uuid, StartTs, EndTs;
	if (!UCosmosStaticExporter::ParseSessionIdParts(SessionId, Uuid, StartTs, EndTs))
	{
		OutError = FString::Printf(TEXT("SessionId '%s' is not 'uuid_start_end'"), *SessionId);
		return false;
	}
  bool bAllOk = true;

	TArray<UStaticMeshComponent*> PoleComps;
	TArray<UStaticMeshComponent*> LightComps;
	TArray<UStaticMeshComponent*> SignComps;

	TArray<AActor*> TrafficSignsActors;
	UGameplayStatics::GetAllActorsOfClass(World, ATrafficSignBase::StaticClass(), TrafficSignsActors);
	for (AActor* Actor : TrafficSignsActors)
	{
		TArray<UStaticMeshComponent*> Comps;
		Actor->GetComponents<UStaticMeshComponent>(Comps);
		for (UStaticMeshComponent* C : Comps)
		{
			if (C->ComponentTags.Contains("Pole"))
			{
				PoleComps.Add(C);
			}
			else if (C->ComponentTags.Contains("TrafficLight"))
			{
				LightComps.Add(C);
			}
			else if (C->ComponentTags.Contains("TrafficSign"))
			{
				SignComps.Add(C);
			}
		}
	}
  {
    // Poles
    FString OutPolesPath = OutFilePath + "3d_poles/" + SessionId + ".json";
    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> Labels;

    for (UStaticMeshComponent* C : PoleComps)
    {
      FVector TopM, BaseM;
      BuildPolePolylineMeters(C, TopM, BaseM);
      const FString PoleType = ClassifyPoleType(C);
      AppendPoleLabelJson(Labels, Uuid, StartTs, TopM, BaseM, PoleType);
    }

    Root->SetArrayField(TEXT("labels"), Labels);

    FString OutText;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutText);
    if (!FJsonSerializer::Serialize(Root, Writer))
    { 
      OutError += TEXT("[Poles] JSON serialization failed. "); 
      bAllOk = false; 
    }
    else
    {
      const FString Dir = FPaths::GetPath(OutPolesPath);
      IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);
      if (!FFileHelper::SaveStringToFile(OutText, *OutPolesPath, FFileHelper::EEncodingOptions::ForceUTF8))
      { 
        OutError += FString::Printf(TEXT("[Poles] Failed to write: %s. "), *OutPolesPath); 
        bAllOk = false; 
      }
    }
  }
  {
    // Traffic Lights
    FString OutTrafficSignsPath = OutFilePath + "3d_traffic_lights/" + SessionId + ".json";

    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> Labels;

    for (UStaticMeshComponent* C : SignComps)
    {
      TArray<FVector> V8;
      BuildComponentCuboidMeters(C, V8);
      if (V8.Num() != 8) continue;

      const FString SignClass = TEXT("TRAFFIC_SIGN"); 
      AppendTrafficSignLabelJson(Labels, Uuid, StartTs, V8, SignClass);
    }

    Root->SetArrayField(TEXT("labels"), Labels);

    FString OutText;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutText);
    if (!FJsonSerializer::Serialize(Root, Writer))
    { 
      OutError += TEXT("[TrafficSigns] JSON serialization failed. "); 
      bAllOk = false; 
    }
    else
    {
      const FString Dir = FPaths::GetPath(OutTrafficSignsPath);
      IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);
      if (!FFileHelper::SaveStringToFile(OutText, *OutTrafficSignsPath, FFileHelper::EEncodingOptions::ForceUTF8))
      { 
        OutError += FString::Printf(TEXT("[TrafficSigns] Failed to write: %s. "), *OutTrafficSignsPath); 
        bAllOk = false; 
      }
    }
  }
	return bAllOk;
}

// --------------------- Traffic Lights (new) ---------------------

void UTrafficSignsExporter::BuildComponentCuboidMeters(const UStaticMeshComponent* Comp, TArray<FVector>& Out8VertsMeters)
{
	Out8VertsMeters.Reset();
	Out8VertsMeters.Reserve(8);

	// Local bounds (center-origin, half-extents in local space)
	FVector Origin, Extent;
	Comp->GetLocalBounds(Origin, Extent);  // Origin is local center, Extent is half-sizes

	const FTransform& Xform = Comp->GetComponentTransform();

	// 8 local corners around Origin
	const FVector S[8] = {
		FVector(+Extent.X, +Extent.Y, +Extent.Z),
		FVector(+Extent.X, -Extent.Y, +Extent.Z),
		FVector(-Extent.X, -Extent.Y, +Extent.Z),
		FVector(-Extent.X, +Extent.Y, +Extent.Z),
		FVector(+Extent.X, +Extent.Y, -Extent.Z),
		FVector(+Extent.X, -Extent.Y, -Extent.Z),
		FVector(-Extent.X, -Extent.Y, -Extent.Z),
		FVector(-Extent.X, +Extent.Y, -Extent.Z),
	};

	for (int i=0;i<8;++i)
	{
		const FVector Local = Origin + S[i];
		const FVector World = Xform.TransformPosition(Local);
		Out8VertsMeters.Add(World / 100.0f); // cm -> m
	}
}

void UTrafficSignsExporter::AppendTrafficSignLabelJson(
	TArray<TSharedPtr<FJsonValue>>& Labels,
	const FString& Uuid,
	const FString& StartTs,
	const TArray<FVector>& V8Meters,
	const FString& LightClassText)
{
	// Matches the traffic-signs sample structure but with:
	//  - labelClassIdentifier = "traffic-lights:autolabels"
	//  - label_name enum      = "traffic_light"
	//  - cuboid3d.vertices    = 8 world points in meters
	//  - include an attribute "light_class" (text) analogous to "sign_class"
	// Schema reference: sample file you shared. :contentReference[oaicite:1]{index=1}

	TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();
	Label->SetStringField(TEXT("labelFamily"), TEXT("SHAPE3D"));

	// assetRef
	{
		TSharedRef<FJsonObject> AssetRef = MakeShared<FJsonObject>();
		AssetRef->SetStringField(TEXT("sessionId"), Uuid);
		AssetRef->SetStringField(TEXT("sensorName"), TEXT("lidar_gt_top_p128"));
		TSharedRef<FJsonObject> FramesObj = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> Frames; Frames.Add(MakeShared<FJsonValueNumber>(0));
		FramesObj->SetArrayField(TEXT("frames"), Frames);
		AssetRef->SetObjectField(TEXT("frames"), FramesObj);
		Label->SetObjectField(TEXT("assetRef"), AssetRef);
	}

	// labelClassKey
	{
		TSharedRef<FJsonObject> ClassKey = MakeShared<FJsonObject>();
		ClassKey->SetStringField(TEXT("labelClassNamespace"), TEXT("minimap"));
		ClassKey->SetStringField(TEXT("labelClassIdentifier"), TEXT("traffic-lights:autolabels"));
		ClassKey->SetStringField(TEXT("labelClassVersion"), TEXT("v0"));
		Label->SetObjectField(TEXT("labelClassKey"), ClassKey);
	}

	// A simple deterministic id from first & opposite vertices
	const FVector P0 = V8Meters.Num() > 0 ? V8Meters[0] : FVector::ZeroVector;
	const FVector P4 = V8Meters.Num() > 4 ? V8Meters[4] : FVector::ZeroVector;
	Label->SetStringField(TEXT("labelId"), MakeStableLabelId(Uuid, P0, P4));

	// labelData.shape3d (cuboid3d)
	TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
	{
		TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();

		// cuboid3d.vertices (8)
		{
			TSharedRef<FJsonObject> Cuboid = MakeShared<FJsonObject>();
			TArray<TSharedPtr<FJsonValue>> VertArray;
			VertArray.Reserve(8);
			for (const FVector& V : V8Meters)
			{
				TArray<TSharedPtr<FJsonValue>> Triple;
				Triple.Add(MakeShared<FJsonValueNumber>(V.X));
				Triple.Add(MakeShared<FJsonValueNumber>(V.Y));
				Triple.Add(MakeShared<FJsonValueNumber>(V.Z));
				VertArray.Add(MakeShared<FJsonValueArray>(Triple));
			}
			Cuboid->SetArrayField(TEXT("vertices"), VertArray);
			Shape3D->SetObjectField(TEXT("cuboid3d"), Cuboid);
		}

		// attributes (match the sample schema’s structure) :contentReference[oaicite:2]{index=2}
		{
			TArray<TSharedPtr<FJsonValue>> Attrs;

			auto AddAttrText = [&](const TCHAR* Name, const FString& Text)
			{
				if (Text.Len() == 0 && FCString::Strcmp(Name, TEXT("asset_ref")) != 0) // allow empty asset_ref
				{
					// skip truly empty optional fields except asset_ref which we keep for shape parity
				}
				TSharedRef<FJsonObject> A = MakeShared<FJsonObject>();
				A->SetStringField(TEXT("name"), Name);
				A->SetStringField(TEXT("text"), Text);
				Attrs.Add(MakeShared<FJsonValueObject>(A));
			};

			AddAttrText(TEXT("coordinate_frame"), TEXT("rig"));
			AddAttrText(TEXT("timestamp"),        StartTs);
			AddAttrText(TEXT("asset_ref"),        TEXT("")); // optionally fill
			AddAttrText(TEXT("feature_id"),       TEXT(""));
			AddAttrText(TEXT("feature_version"),  TEXT(""));
			AddAttrText(TEXT("clip_version_id"),  TEXT(""));

			// label_name enum = traffic_light
			{
				TSharedRef<FJsonObject> A = MakeShared<FJsonObject>();
				A->SetStringField(TEXT("name"), TEXT("label_name"));
				A->SetStringField(TEXT("enum"), TEXT("traffic_light"));
				Attrs.Add(MakeShared<FJsonValueObject>(A));
			}

			// light_class text (like sign_class in your sample)
			AddAttrText(TEXT("light_class"), LightClassText);

			Shape3D->SetArrayField(TEXT("attributes"), Attrs);
		}

		Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

		LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
	}
	Label->SetObjectField(TEXT("labelData"), LabelData);
	Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

	Labels.Add(MakeShared<FJsonValueObject>(Label));
}
