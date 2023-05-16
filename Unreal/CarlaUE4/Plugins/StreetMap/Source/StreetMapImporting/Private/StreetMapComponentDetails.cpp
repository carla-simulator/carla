// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "StreetMapComponentDetails.h"
#include "StreetMapImporting.h"



#include "SlateBasics.h"
#include "RawMesh.h"
#include "PropertyEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "IDetailsView.h"
#include "IDetailCustomization.h"
#include "AssetRegistryModule.h"
#include "Dialogs/DlgPickAssetPath.h"
#include "IDetailCustomization.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/AssertionMacros.h"


#include "StreetMapComponent.h"


#define LOCTEXT_NAMESPACE "StreetMapComponentDetails"


FStreetMapComponentDetails::FStreetMapComponentDetails() :
	SelectedStreetMapComponent(nullptr),
	LastDetailBuilderPtr(nullptr)
{

}

TSharedRef<IDetailCustomization> FStreetMapComponentDetails::MakeInstance()
{
	return MakeShareable(new FStreetMapComponentDetails());
}

void FStreetMapComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	LastDetailBuilderPtr = &DetailBuilder;

	TArray <TWeakObjectPtr<UObject>> SelectedObjects = DetailBuilder.GetDetailsView()->GetSelectedObjects();

	for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
	{
		UStreetMapComponent* TempStreetMapComp = Cast<UStreetMapComponent>(Object.Get());
		if (TempStreetMapComp != nullptr && !TempStreetMapComp->IsTemplate())
		{
			SelectedStreetMapComponent = TempStreetMapComp;
			break;
		}
	}


	if (SelectedStreetMapComponent == nullptr)
	{
		TArray<TWeakObjectPtr<AActor>> SelectedActors = DetailBuilder.GetDetailsView()->GetSelectedActors();

		for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
		{
			AActor* TempActor = Cast<AActor>(Object.Get());
			if (TempActor != nullptr && !TempActor->IsTemplate())
			{
				UStreetMapComponent* TempStreetMapComp = TempActor->FindComponentByClass<UStreetMapComponent>();
				if (TempStreetMapComp != nullptr && !TempStreetMapComp->IsTemplate())
				{
					SelectedStreetMapComponent = TempStreetMapComp;
					break;
				}
				break;
			}
		}
	}



	if (SelectedStreetMapComponent == nullptr)
	{
		return;
	}


	IDetailCategoryBuilder& StreetMapCategory = DetailBuilder.EditCategory("StreetMap", FText::GetEmpty(), ECategoryPriority::Important);
	StreetMapCategory.InitiallyCollapsed(false);


	const bool bCanRebuildMesh = HasValidMapObject();
	const bool bCanClearMesh = HasValidMeshData();
	const bool bCanCreateMeshAsset = HasValidMeshData();

	TSharedPtr< SHorizontalBox > TempHorizontalBox;

	StreetMapCategory.AddCustomRow(FText::GetEmpty(), false)
		[
			SAssignNew(TempHorizontalBox, SHorizontalBox)
			+ SHorizontalBox::Slot()
		[
			SNew(SButton)
			.ToolTipText(LOCTEXT("GenerateMesh_Tooltip", "Generate a cached mesh from raw street map data."))
		.OnClicked(this, &FStreetMapComponentDetails::OnBuildMeshClicked)
		.IsEnabled(bCanRebuildMesh)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(bCanClearMesh ? LOCTEXT("RebuildMesh", "Rebuild Mesh") : LOCTEXT("BuildMesh", "Build Mesh"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		]
		];

	TempHorizontalBox->AddSlot()
		[
			SNew(SButton)
			.ToolTipText(LOCTEXT("ClearMesh_Tooltip", "Clear current mesh data , in case we have a valid mesh "))
		.OnClicked(this, &FStreetMapComponentDetails::OnClearMeshClicked)
		.IsEnabled(bCanClearMesh)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ClearMesh", "Clear Mesh"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		];


	StreetMapCategory.AddCustomRow(FText::GetEmpty(), false)
		[
			SAssignNew(TempHorizontalBox, SHorizontalBox)
			+ SHorizontalBox::Slot()
		[
			SNew(SButton)
			.ToolTipText(LOCTEXT("CreateStaticMeshAsset_Tooltip", "Create a new Static Mesh Asset from selected StreetMapComponent."))
		.OnClicked(this, &FStreetMapComponentDetails::OnCreateStaticMeshAssetClicked)
		.IsEnabled(bCanCreateMeshAsset)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CreateStaticMeshAsset", "Create Static Mesh Asset"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		]
		];

	if (bCanCreateMeshAsset)
	{

		const int32 NumVertices = SelectedStreetMapComponent->GetRawMeshVertices().Num();
		const FString NumVerticesToString = TEXT("Vertex Count : ") + FString::FromInt(NumVertices);

		const int32 NumTriangles = SelectedStreetMapComponent->GetRawMeshIndices().Num() / 3;
		const FString NumTrianglesToString = TEXT("Triangle Count : ") + FString::FromInt(NumTriangles);

		const bool bCollisionEnabled = SelectedStreetMapComponent->IsCollisionEnabled();
		const FString CollisionStatusToString = bCollisionEnabled ? TEXT("Collision : ON") : TEXT("Collision : OFF");

		StreetMapCategory.AddCustomRow(FText::GetEmpty(), true)
			[
				SAssignNew(TempHorizontalBox, SHorizontalBox)
				+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(FSlateFontInfo("Verdana", 8))
			.Text(FText::FromString(NumVerticesToString))
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(FSlateFontInfo("Verdana", 8))
			.Text(FText::FromString(NumTrianglesToString))
			]
			+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(FSlateFontInfo("Verdana", 8))
				.Text(FText::FromString(CollisionStatusToString))
				]
			];
	}

}

bool FStreetMapComponentDetails::HasValidMeshData() const
{
	return SelectedStreetMapComponent != nullptr && SelectedStreetMapComponent->HasValidMesh();
}


bool FStreetMapComponentDetails::HasValidMapObject() const
{
	return SelectedStreetMapComponent != nullptr && SelectedStreetMapComponent->GetStreetMap() != nullptr;
}

FReply FStreetMapComponentDetails::OnCreateStaticMeshAssetClicked()
{
	if (SelectedStreetMapComponent != nullptr)
	{
		FString NewNameSuggestion = SelectedStreetMapComponent->GetStreetMapAssetName();
		FString PackageName = FString(TEXT("/Game/Meshes/")) + NewNameSuggestion;
		FString Name;
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().CreateUniqueAssetName(PackageName, TEXT(""), PackageName, Name);

		TSharedPtr<SDlgPickAssetPath> PickAssetPathWidget =
			SNew(SDlgPickAssetPath)
			.Title(LOCTEXT("ConvertToStaticMeshPickName", "Choose New StaticMesh Location"))
			.DefaultAssetPath(FText::FromString(PackageName));

		if (PickAssetPathWidget->ShowModal() == EAppReturnType::Ok)
		{
			// Get the full name of where we want to create the physics asset.
			FString UserPackageName = PickAssetPathWidget->GetFullAssetPath().ToString();
			FName MeshName(*FPackageName::GetLongPackageAssetName(UserPackageName));

			// Check if the user inputed a valid asset name, if they did not, give it the generated default name
			if (MeshName == NAME_None)
			{
				// Use the defaults that were already generated.
				UserPackageName = PackageName;
				MeshName = *Name;
			}

			// Raw mesh data we are filling in
			FRawMesh RawMesh;
			// Materials to apply to new mesh
			TArray<UMaterialInterface*> MeshMaterials = SelectedStreetMapComponent->GetMaterials();

			
			const TArray<FStreetMapVertex > RawMeshVertices = SelectedStreetMapComponent->GetRawMeshVertices();
			const TArray< uint32 > RawMeshIndices = SelectedStreetMapComponent->GetRawMeshIndices();


			// Copy verts
			for (int32 VertIndex = 0; VertIndex < RawMeshVertices.Num();VertIndex++)
			{
				RawMesh.VertexPositions.Add(RawMeshVertices[VertIndex].Position);
			}

			// Copy 'wedge' info
			int32 NumIndices = RawMeshIndices.Num();
			for (int32 IndexIdx = 0; IndexIdx < NumIndices; IndexIdx++)
			{
				int32 VertexIndex = RawMeshIndices[IndexIdx];

				RawMesh.WedgeIndices.Add(VertexIndex);

				const FStreetMapVertex& StreetMapVertex = RawMeshVertices[VertexIndex];

				FVector TangentX = StreetMapVertex.TangentX;
				FVector TangentZ = StreetMapVertex.TangentZ;
				FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal();

				RawMesh.WedgeTangentX.Add(TangentX);
				RawMesh.WedgeTangentY.Add(TangentY);
				RawMesh.WedgeTangentZ.Add(TangentZ);

				RawMesh.WedgeTexCoords[0].Add(StreetMapVertex.TextureCoordinate);
				RawMesh.WedgeColors.Add(StreetMapVertex.Color);
			}

			// copy face info
			int32 NumTris = NumIndices / 3;
			for (int32 TriIdx = 0; TriIdx < NumTris; TriIdx++)
			{
				RawMesh.FaceMaterialIndices.Add(0);
				RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
			}

			// If we got some valid data.
			if (RawMesh.VertexPositions.Num() > 3 && RawMesh.WedgeIndices.Num() > 3)
			{
				// Then find/create it.
				UPackage* Package = CreatePackage(NULL, *UserPackageName);
				check(Package);

				// Create StaticMesh object
				UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, MeshName, RF_Public | RF_Standalone);
				StaticMesh->InitResources();

				StaticMesh->LightingGuid = FGuid::NewGuid();

				// Add source to new StaticMesh
				FStaticMeshSourceModel* SrcModel = new (StaticMesh->SourceModels) FStaticMeshSourceModel();
				SrcModel->BuildSettings.bRecomputeNormals = false;
				SrcModel->BuildSettings.bRecomputeTangents = false;
				SrcModel->BuildSettings.bRemoveDegenerates = false;
				SrcModel->BuildSettings.bUseHighPrecisionTangentBasis = false;
				SrcModel->BuildSettings.bUseFullPrecisionUVs = false;
				SrcModel->BuildSettings.bGenerateLightmapUVs = true;
				SrcModel->BuildSettings.SrcLightmapIndex = 0;
				SrcModel->BuildSettings.DstLightmapIndex = 1;
				SrcModel->RawMeshBulkData->SaveRawMesh(RawMesh);

				// Copy materials to new mesh
				for (UMaterialInterface* Material : MeshMaterials)
				{
					StaticMesh->StaticMaterials.Add(FStaticMaterial(Material));
				}

				//Set the Imported version before calling the build
				StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;

				// Build mesh from source
				StaticMesh->Build(/** bSilent =*/ false);
				StaticMesh->PostEditChange();

				StaticMesh->MarkPackageDirty();

				// Notify asset registry of new asset
				FAssetRegistryModule::AssetCreated(StaticMesh);


				// Display notification so users can quickly access the mesh
				if (GIsEditor)
				{
					FNotificationInfo Info(FText::Format(LOCTEXT("StreetMapMeshConverted", "Successfully Converted Mesh"), FText::FromString(StaticMesh->GetName())));
					Info.ExpireDuration = 8.0f;
					Info.bUseLargeFont = false;
					Info.Hyperlink = FSimpleDelegate::CreateLambda([=]() { FAssetEditorManager::Get().OpenEditorForAssets(TArray<UObject*>({ StaticMesh })); });
					Info.HyperlinkText = FText::Format(LOCTEXT("OpenNewAnimationHyperlink", "Open {0}"), FText::FromString(StaticMesh->GetName()));
					TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
					if (Notification.IsValid())
					{
						Notification->SetCompletionState(SNotificationItem::CS_Success);
					}
				}
			}
		}
	}

	return FReply::Handled();
}

FReply FStreetMapComponentDetails::OnBuildMeshClicked()
{

	if(SelectedStreetMapComponent != nullptr)
	{
		//
		SelectedStreetMapComponent->BuildMesh();

		// regenerates details panel layouts , to take in consideration new changes.
		RefreshDetails();
	}

	return FReply::Handled();
}

FReply FStreetMapComponentDetails::OnClearMeshClicked()
{
	if (SelectedStreetMapComponent != nullptr)
	{
		//
		SelectedStreetMapComponent->InvalidateMesh();

		// regenerates details panel layouts , to take in consideration new changes.
		RefreshDetails();
	}

	return FReply::Handled();
}

void FStreetMapComponentDetails::RefreshDetails()
{
	if(LastDetailBuilderPtr != nullptr)
	{
		LastDetailBuilderPtr->ForceRefreshDetails();
	}
}

#undef LOCTEXT_NAMESPACE
