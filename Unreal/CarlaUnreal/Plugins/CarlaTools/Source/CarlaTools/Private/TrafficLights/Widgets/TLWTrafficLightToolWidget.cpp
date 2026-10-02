#include "TrafficLights/Widgets/TLWTrafficLightToolWidget.h"

#include "CarlaTools.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "CoreGlobals.h"
#include "Delegates/Delegate.h"
#include "DesktopPlatformModule.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMeshSocket.h"
#include "EngineUtils.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "IDesktopPlatform.h"
#include "Logging/LogMacros.h"
#include "Logging/LogVerbosity.h"
#include "Math/MathFwd.h"
#include "Misc/AssertionMacros.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "ScopedTransaction.h"
#include "Selection.h"
#include "TrafficLights/TLHead.h"
#include "TrafficLights/TLLightTypeDataTable.h"
#include "TrafficLights/TLMeshFactory.h"
#include "TrafficLights/TLModule.h"
#include "TrafficLights/TLPole.h"
#include "UObject/Object.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"

void STrafficLightToolWidget::Construct(const FArguments& InArgs)
{
	if (StyleOptions.IsEmpty())
	{
		if (UEnum* EnumPtr = StaticEnum<ETLStyle>())
		{
			for (int32 i{0}; i < EnumPtr->NumEnums() - 1; ++i)
			{
				StyleOptions.Add(MakeShared<FString>(EnumPtr->GetDisplayNameTextByIndex(i).ToString()));
			}
		}
	}
	if (AttachmentOptions.IsEmpty())
	{
		if (UEnum* EnumPtr = StaticEnum<ETLHeadAttachment>())
		{
			for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i)
			{
				AttachmentOptions.Add(MakeShared<FString>(EnumPtr->GetDisplayNameTextByIndex(i).ToString()));
			}
		}
	}
	if (OrientationOptions.IsEmpty())
	{
		if (UEnum* EnumPtr = StaticEnum<ETLOrientation>())
		{
			for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i)
			{
				OrientationOptions.Add(MakeShared<FString>(EnumPtr->GetDisplayNameTextByIndex(i).ToString()));
			}
		}
	}
	if (LightTypeOptions.IsEmpty())
	{
		if (UEnum* EnumPtr = StaticEnum<ETLLightType>())
		{
			for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i)
			{
				LightTypeOptions.Add(MakeShared<FString>(EnumPtr->GetDisplayNameTextByIndex(i).ToString()));
			}
		}
	}

#if WITH_EDITOR
	if (GEditor)
	{
		USelection* Sel{GEditor->GetSelectedActors()};
		for (FSelectionIterator It(*Sel); It; ++It)
		{
			if (AActor * Actor{Cast<AActor>(*It)})
			{
				const FString Folder{Actor->GetFolderPath().ToString()};
				if (Folder.StartsWith(TEXT("TrafficLightActor")))
				{
					UE_LOG(LogCarlaTools, Log, TEXT("Traffic Light bake folder detected: %s"), *Folder);
					break;
				}
			}
		}
	}
#endif

	ChildSlot[SNew(SSplitter)

			  + SSplitter::Slot().Value(0.4f)[BuildPoleSection()]

			  + SSplitter::Slot().Value(0.6f)[SAssignNew(PreviewViewport, STrafficLightPreviewViewport)]];
}

// ----------------------------------------------------------------------------------------------------------------
// ******************************   REBUILD
// ----------------------------------------------------------------------------------------------------------------
void STrafficLightToolWidget::UpdatePreviewActor()
{
	check(PreviewViewport.IsValid());
	ATrafficLightActor* TL{PreviewViewport->GetPreviewTrafficLight()};
	check(IsValid(TL));
	TL->Poles = Poles;
	TL->RerunConstructionScripts();
}

void STrafficLightToolWidget::Rebuild()
{
	check(PreviewViewport.IsValid());

	for (int32 PoleIndex{0}; PoleIndex < EditorPoles.Num(); ++PoleIndex)
	{
		FEditorPole& EditorPole{EditorPoles[PoleIndex]};
		for (int32 HeadIndex{0}; HeadIndex < EditorPole.Heads.Num(); ++HeadIndex)
		{
			for (int32 ModuleIndex{0}; ModuleIndex < EditorPole.Heads[HeadIndex].Modules.Num(); ++ModuleIndex)
			{
				RefreshModuleMeshOptions(PoleIndex, HeadIndex, ModuleIndex);
			}
		}
	}

	UpdatePreviewActor();
	PreviewViewport->Reload();

	RefreshPoleList();
}

void STrafficLightToolWidget::RebuildModuleChain(FTLHead& Head)
{
	if (Head.Modules.IsEmpty())
	{
		UE_LOG(LogCarlaTools, Warning, TEXT("RebuildModuleChain: Modules.Num()"));
		return;
	}

	static const FName EndSocketName{FName("Socket2")};
	static const FName BeginSocketName{FName("Socket1")};

	{
		FTLModule& Module{Head.Modules[0]};
		Module.Transform = FTransform::Identity;
		if (Module.ModuleMeshComponent)
		{
			Module.ModuleMeshComponent->SetRelativeTransform(Module.Transform * Head.Transform * Module.Offset);
		}
	}

	for (int32 i{1}; i < Head.Modules.Num(); ++i)
	{
		const FTLModule& Prev{Head.Modules[i - 1]};
		FTLModule& Curr{Head.Modules[i]};

		if (!IsValid(Prev.ModuleMeshComponent))
		{
			UE_LOG(LogCarlaTools, Warning, TEXT("Previous module mesh component is invalid at module %d"), i);
			continue;
		}
		if (!IsValid(Curr.ModuleMeshComponent))
		{
			UE_LOG(LogCarlaTools, Warning, TEXT("Missing component at module %d"), i);
			continue;
		}

		const UStaticMeshSocket* PrevSocket{Prev.ModuleMesh->FindSocket(EndSocketName)};
		const UStaticMeshSocket* CurrSocket{Curr.ModuleMesh->FindSocket(BeginSocketName)};
		if (!IsValid(PrevSocket))
		{
			UE_LOG(LogCarlaTools, Warning, TEXT("Previous socket not found at module %d"), i);
			continue;
		}
		if (!IsValid(CurrSocket))
		{
			UE_LOG(LogCarlaTools, Warning, TEXT("Current socket not found at module %d"), i);
			continue;
		}

		const FTransform PrevBase{Prev.Transform * Prev.Offset};
		const FTransform PrevLocal(FQuat::Identity, PrevSocket->RelativeLocation, FVector::OneVector);
		const FTransform CurrLocal(FQuat::Identity, CurrSocket->RelativeLocation, FVector::OneVector);
		const FTransform SnapDelta{PrevBase * PrevLocal * CurrLocal.Inverse()};
		Curr.Transform = SnapDelta;
		Curr.ModuleMeshComponent->SetRelativeTransform(Curr.Transform * Head.Transform * Curr.Offset);
	}
}

void STrafficLightToolWidget::RefreshPoleList()
{
	check(RootContainer.IsValid());
	RootContainer->ClearChildren();

	for (int32 PoleIndex{0}; PoleIndex < EditorPoles.Num(); ++PoleIndex)
	{
		RootContainer->AddSlot().AutoHeight().Padding(2, 2)[BuildPoleEntry(PoleIndex)];
	}
}

void STrafficLightToolWidget::RefreshModuleMeshOptions(int32 PoleIndex, int32 HeadIndex, int32 ModuleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};
	FTLHead& Head{Pole.Heads[HeadIndex]};

	check(EditorHead.Modules.IsValidIndex(ModuleIndex));
	FEditorModule& EditorModule{EditorHead.Modules[ModuleIndex]};
	FTLModule& Module{Head.Modules[ModuleIndex]};

	TArray<UStaticMesh*> Meshes{FTLMeshFactory::GetAllMeshesForModule(Head, Module)};

	EditorModule.MeshNameOptions.Empty();
	if (IsValid(Module.ModuleMesh))
	{
		if (!Meshes.Contains(Module.ModuleMesh))
		{
			Module.ModuleMesh = nullptr;
		}
	}
}

void STrafficLightToolWidget::RefreshPoleMeshes(int32 PoleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FTLPole& Pole{Poles[PoleIndex]};
	TArray<UStaticMesh*> BaseMeshes{FTLMeshFactory::GetAllBaseMeshesForPole(Pole)};
	TArray<UStaticMesh*> ExtensibleMeshes{FTLMeshFactory::GetAllExtensibleMeshesForPole(Pole)};
	TArray<UStaticMesh*> CapMeshes{FTLMeshFactory::GetAllCapMeshesForPole(Pole)};

	if (IsValid(Pole.BasePoleMesh))
	{
		if (!BaseMeshes.Contains(Pole.BasePoleMesh))
		{
			Pole.BasePoleMesh = nullptr;
		}
	}
	if (IsValid(Pole.ExtensiblePoleMesh))
	{
		if (!BaseMeshes.Contains(Pole.ExtensiblePoleMesh))
		{
			Pole.ExtensiblePoleMesh = nullptr;
		}
	}
	if (IsValid(Pole.CapPoleMesh))
	{
		if (!BaseMeshes.Contains(Pole.CapPoleMesh))
		{
			Pole.CapPoleMesh = nullptr;
		}
	}
}

// ----------------------------------------------------------------------------------------------------------------
// ******************************   EVENTS
// ----------------------------------------------------------------------------------------------------------------
FReply STrafficLightToolWidget::OnExportJSONClicked()
{
	if (!PreviewViewport.IsValid())
	{
		return FReply::Unhandled();
	}
	ATrafficLightActor* TL{PreviewViewport->GetPreviewTrafficLight()};
	if (!IsValid(TL))
	{
		return FReply::Handled();
	}
	bool bUseTransform{false};
#if WITH_EDITOR
	if (GEditor)
	{
		USelection* Sel{GEditor->GetSelectedActors()};
		for (FSelectionIterator It(*Sel); It; ++It)
		{
			if (AActor * Actor{Cast<AActor>(*It)})
			{
				const FString Folder{Actor->GetFolderPath().ToString()};
				if (Folder.StartsWith(TEXT("TrafficLightActor")))
				{
					UE_LOG(LogCarlaTools, Log, TEXT("Traffic Light bake folder detected: %s"), *Folder);
					bUseTransform = true;
					break;
				}
			}
		}
	}
#endif

	const FString JsonString{TL->ExportToJSON(bUseTransform)};
	IDesktopPlatform* Desktop{FDesktopPlatformModule::Get()};
	if (Desktop)
	{
		const void* ParentWindowHandle{FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr)};
		TArray<FString> OutFiles;
		const bool bSaved = Desktop->SaveFileDialog(ParentWindowHandle, TEXT("Save Traffic Light JSON"), FPaths::ProjectDir(),
			TEXT("TrafficLight.json"), TEXT("JSON files (*.json)|*.json"), EFileDialogFlags::None, OutFiles);
		if (bSaved && OutFiles.Num() > 0)
		{
			FFileHelper::SaveStringToFile(JsonString, *OutFiles[0]);
		}
	}
	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnImportJSONClicked()
{
	IDesktopPlatform* Desktop{FDesktopPlatformModule::Get()};
	if (!Desktop)
	{
		return FReply::Handled();
	}
	const void* ParentWindowHandle{FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr)};
	TArray<FString> Filenames;
	const bool bFileChosen{Desktop->OpenFileDialog(ParentWindowHandle, TEXT("Import Traffic Light JSON"),
		FPaths::ProjectContentDir(), TEXT(""), TEXT("JSON files (*.json)|*.json"), EFileDialogFlags::None, Filenames)};
	if (!bFileChosen || Filenames.Num() == 0)
	{
		return FReply::Handled();
	}

	const FString& FilePath{Filenames[0]};
	FString JSONString;
	if (!FFileHelper::LoadFileToString(JSONString, *FilePath))
	{
		UE_LOG(LogCarlaTools, Error, TEXT("Failed to load JSON: %s"), *FilePath);
		return FReply::Handled();
	}

	if (PreviewViewport.IsValid())
	{
		ATrafficLightActor* TL{PreviewViewport->GetPreviewTrafficLight()};
		if (IsValid(TL))
		{
			TL->BuildFromJSONString(JSONString);
		}
		else
		{
			UE_LOG(LogCarlaTools, Error, TEXT("No preview actor available"));
		}
	}

	if (ATrafficLightActor * TL{PreviewViewport->GetPreviewTrafficLight()})
	{
		Poles = TL->Poles;
		EditorPoles.Reset();
		for (const FTLPole& P : Poles)
		{
			FEditorPole EPol;
			EPol.Heads.SetNum(P.Heads.Num());
			for (int32 h = 0; h < P.Heads.Num(); ++h)
			{
				const FTLHead& H{P.Heads[h]};
				EPol.Heads[h].Modules.SetNum(H.Modules.Num());
				for (int32 m{0}; m < H.Modules.Num(); ++m)
				{
					const FTLModule& M{H.Modules[m]};
					EPol.Heads[h].Modules[m].Lights.SetNum(M.Lights.Num());
				}
			}
			EditorPoles.Add(MoveTemp(EPol));
		}
	}

	Rebuild();
	RefreshPoleList();

	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnModifyClicked()
{
	if (!PreviewViewport.IsValid())
	{
		return FReply::Handled();
	}

	ATrafficLightActor* const PreviewActor{PreviewViewport->GetPreviewTrafficLight()};
	if (!IsValid(PreviewActor))
	{
		return FReply::Handled();
	}

	if (!GEditor)
	{
		return FReply::Handled();
	}

	UWorld* const EditorWorld{GEditor->GetEditorWorldContext().World()};
	if (!EditorWorld)
	{
		return FReply::Handled();
	}

	int32 SelectedCount{0};
	AActor* SelectedActor{nullptr};
	for (FSelectionIterator It{GEditor->GetSelectedActorIterator()}; It; ++It)
	{
		AActor* const Actor{Cast<AActor>(*It)};
		if (IsValid(Actor))
		{
			SelectedActor = Actor;
			++SelectedCount;
		}
	}

	if (SelectedCount != 1)
	{
		return FReply::Handled();
	}

	USceneComponent* const SelectedRoot{SelectedActor->GetRootComponent()};
	if (!(IsValid(SelectedRoot) && SelectedRoot->GetName() == TEXT("BakedRoot")))
	{
		return FReply::Handled();
	}

	const FTransform SelectedTransform{SelectedActor->GetActorTransform()};
	const FString DesiredLabel{SelectedActor->GetActorLabel()};

	FString MapName{};
	{
		FString CurrentMapName{};
		CurrentMapName = EditorWorld->GetMapName();
		CurrentMapName.RemoveFromStart(EditorWorld->StreamingLevelsPrefix);
		MapName = CurrentMapName;
	}

	const FString BpPath{TEXT("/CarlaDigitalTwinsTool/Blueprints/TrafficLight/BP_TrafficLightActor.BP_TrafficLightActor_C")};
	UClass* const TrafficLightBpClass{LoadObject<UClass>(nullptr, *BpPath)};
	if (!IsValid(TrafficLightBpClass))
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction{NSLOCTEXT("TLW", "ModifyTrafficLight", "Modify Traffic Light")};

	AActor* const ActorToDestroy{SelectedActor};
	SelectedActor = nullptr;
	if (IsValid(ActorToDestroy))
	{
		ActorToDestroy->Modify();
		ActorToDestroy->Destroy();
	}

	ATrafficLightActor* const NewTrafficLightActor{EditorWorld->SpawnActorDeferred<ATrafficLightActor>(
		TrafficLightBpClass, SelectedTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn)};
	if (!IsValid(NewTrafficLightActor))
	{
		return FReply::Handled();
	}

	NewTrafficLightActor->Poles = PreviewActor->Poles;
	NewTrafficLightActor->FinishSpawning(SelectedTransform);
	NewTrafficLightActor->SetActorTransform(SelectedTransform, false, nullptr, ETeleportType::TeleportPhysics);
	NewTrafficLightActor->Build();
	NewTrafficLightActor->Bake(MapName, DesiredLabel);
	NewTrafficLightActor->Destroy();

	AActor* BakedActorFound{nullptr};
	for (TActorIterator<AActor> It{EditorWorld}; It; ++It)
	{
		AActor* const Actor{*It};
		if (!IsValid(Actor))
		{
			continue;
		}
		if (Actor->GetActorLabel() != DesiredLabel)
		{
			continue;
		}
		USceneComponent* const Root{Actor->GetRootComponent()};
		if (!(IsValid(Root) && Root->GetName() == TEXT("BakedRoot")))
		{
			continue;
		}
		BakedActorFound = Actor;
		break;
	}

	GEditor->SelectNone(false, true);
	if (IsValid(BakedActorFound))
	{
		GEditor->SelectActor(BakedActorFound, true, true);
	}
	GEditor->NoteSelectionChange();

	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnAddPoleClicked()
{
	const int32 PoleIndex{EditorPoles.Emplace()};
	Poles.Emplace();

	FTLPole& Pole{Poles[PoleIndex]};
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};

	if (PoleIndex == 0)
	{
		PreviewViewport->ResetFrame();
	}

	Rebuild();
	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnDeletePoleClicked(int32 PoleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	EditorPoles.RemoveAt(PoleIndex);
	Poles.RemoveAt(PoleIndex);
	RefreshPoleList();
	Rebuild();

	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnAddHeadClicked(int32 PoleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	const int32 HeadIndex{EditorPole.Heads.Emplace()};
	Pole.Heads.Emplace();
	FEditorHead& NewEditorHead{EditorPole.Heads[HeadIndex]};
	FTLHead& NewHead{Pole.Heads[HeadIndex]};

	OnAddModuleClicked(PoleIndex, HeadIndex);
	Rebuild();

	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnDeleteHeadClicked(int32 PoleIndex, int32 HeadIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	EditorPole.Heads.RemoveAt(HeadIndex);
	Pole.Heads.RemoveAt(HeadIndex);
	Rebuild();

	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnAddModuleClicked(int32 PoleIndex, int32 HeadIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};
	FTLHead& Head{Pole.Heads[HeadIndex]};

	FEditorModule NewEditorModule;
	FTLModule NewModule;

	NewModule.ModuleMesh = FTLMeshFactory::GetMeshForModule(Head, NewModule);
	if (!IsValid(NewModule.ModuleMesh))
	{
		UE_LOG(LogCarlaTools, Error, TEXT("OnAddModuleClicked: failed to load the module."));
		return FReply::Handled();
	}

	const int32 NumLights{FTLMeshFactory::CountLedMaterials(NewModule.ModuleMesh)};
	NewEditorModule.Lights.SetNum(NumLights);
	NewModule.Lights.SetNum(NumLights);

	Head.Modules.Add(NewModule);
	EditorHead.Modules.Add(NewEditorModule);
	Rebuild();

	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnDeleteModuleClicked(int32 PoleIndex, int32 HeadIndex, int32 ModuleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	check(Pole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};
	FTLHead& Head{Pole.Heads[HeadIndex]};

	check(EditorHead.Modules.IsValidIndex(ModuleIndex));
	EditorHead.Modules.RemoveAt(ModuleIndex);
	Head.Modules.RemoveAt(ModuleIndex);
	Rebuild();

	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnMoveModuleUpClicked(int32 PoleIndex, int32 HeadIndex, int32 ModuleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};
	FTLHead& Head{Pole.Heads[HeadIndex]};

	if (ModuleIndex > 0 && ModuleIndex < EditorHead.Modules.Num())
	{
		EditorHead.Modules.Swap(ModuleIndex, ModuleIndex - 1);
		Head.Modules.Swap(ModuleIndex, ModuleIndex - 1);
		Rebuild();
	}

	return FReply::Handled();
}

FReply STrafficLightToolWidget::OnMoveModuleDownClicked(int32 PoleIndex, int32 HeadIndex, int32 ModuleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};
	FTLHead& Head{Pole.Heads[HeadIndex]};

	if (ModuleIndex >= 0 && ModuleIndex < EditorHead.Modules.Num() - 1)
	{
		EditorHead.Modules.Swap(ModuleIndex, ModuleIndex + 1);
		Head.Modules.Swap(ModuleIndex, ModuleIndex + 1);
		Rebuild();
	}

	return FReply::Handled();
}

void STrafficLightToolWidget::OnHeadOrientationChanged(ETLOrientation NewOrientation, int32 PoleIndex, int32 HeadIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FTLPole& Pole{Poles[PoleIndex]};
	check(Pole.Heads.IsValidIndex(HeadIndex));
	FTLHead& Head{Pole.Heads[HeadIndex]};
	Head.Modules.Empty();
	EditorPoles[PoleIndex].Heads[HeadIndex].Modules.Empty();
	Head.Orientation = NewOrientation;
	for (int32 ModuleIndex{0}; ModuleIndex < Head.Modules.Num(); ++ModuleIndex)
	{
		FTLModule& Module{Head.Modules[ModuleIndex]};
		RefreshModuleMeshOptions(PoleIndex, HeadIndex, ModuleIndex);
	}
	Rebuild();
}

void STrafficLightToolWidget::OnHeadStyleChanged(ETLStyle NewStyle, int32 PoleIndex, int32 HeadIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FTLPole& Pole{Poles[PoleIndex]};
	check(Pole.Heads.IsValidIndex(HeadIndex));
	FTLHead& Head{Pole.Heads[HeadIndex]};

	Head.Modules.Empty();
	EditorPoles[PoleIndex].Heads[HeadIndex].Modules.Empty();
	Head.Style = NewStyle;
	for (int32 ModuleIndex{0}; ModuleIndex < Head.Modules.Num(); ++ModuleIndex)
	{
		FTLModule& Module{Head.Modules[ModuleIndex]};
		RefreshModuleMeshOptions(PoleIndex, HeadIndex, ModuleIndex);
	}
	Rebuild();
}

void STrafficLightToolWidget::OnPoleOrientationChanged(ETLOrientation NewOrientation, int32 PoleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FTLPole& Pole{Poles[PoleIndex]};
	Pole.Orientation = NewOrientation;
	RefreshPoleMeshes(PoleIndex);
	Rebuild();
}

void STrafficLightToolWidget::OnPoleStyleChanged(ETLStyle NewStyle, int32 PoleIndex)
{
	check(Poles.IsValidIndex(PoleIndex));
	FTLPole& Pole{Poles[PoleIndex]};
	Pole.Style = NewStyle;
	RefreshPoleMeshes(PoleIndex);
	Rebuild();
}

void STrafficLightToolWidget::OnModuleVisorChanged(ECheckBoxState NewState, int32 PoleIndex, int32 HeadIndex, int32 ModuleIndex)
{
	check(Poles.IsValidIndex(PoleIndex));
	FTLPole& Pole{Poles[PoleIndex]};
	check(Pole.Heads.IsValidIndex(HeadIndex));
	FTLHead& Head{Pole.Heads[HeadIndex]};
	check(Head.Modules.IsValidIndex(ModuleIndex));
	FTLModule& Module{Head.Modules[ModuleIndex]};

	Module.bHasVisor = (NewState == ECheckBoxState::Checked);
	UStaticMesh* NewStaticMesh{FTLMeshFactory::GetMeshForModule(Head, Module)};
	if (!IsValid(NewStaticMesh))
	{
		UE_LOG(LogCarlaTools, Error, TEXT("OnHeadStyleChanged: failed to get mesh"));
		return;
	}
	if (Module.ModuleMesh != NewStaticMesh)
	{
		Module.ModuleMesh = NewStaticMesh;
	}

	Rebuild();
}

// ----------------------------------------------------------------------------------------------------------------
// ******************************   BUILD SECTIONS
// ----------------------------------------------------------------------------------------------------------------
TSharedRef<SWidget> STrafficLightToolWidget::BuildPoleEntry(int32 PoleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	TArray<TSharedPtr<FString>>& BaseNames{EditorPole.BaseMeshNameOptions};
	TArray<UStaticMesh*> BasePoleMeshes{FTLMeshFactory::GetAllBaseMeshesForPole(Pole)};
	BaseNames.Empty();
	for (const UStaticMesh* Mesh : BasePoleMeshes)
	{
		BaseNames.Add(MakeShared<FString>(Mesh->GetName()));
	}

	TArray<TSharedPtr<FString>>& ExtensibleNames{EditorPole.ExtensibleMeshNameOptions};
	TArray<UStaticMesh*> ExtensiblePoleMeshes{FTLMeshFactory::GetAllExtensibleMeshesForPole(Pole)};
	ExtensibleNames.Empty();
	for (const UStaticMesh* Mesh : ExtensiblePoleMeshes)
	{
		ExtensibleNames.Add(MakeShared<FString>(Mesh->GetName()));
	}

	TArray<TSharedPtr<FString>>& CapNames{EditorPole.CapMeshNameOptions};
	TArray<UStaticMesh*> CapPoleMeshes{FTLMeshFactory::GetAllCapMeshesForPole(Pole)};
	CapNames.Empty();
	for (const UStaticMesh* Mesh : CapPoleMeshes)
	{
		CapNames.Add(MakeShared<FString>(Mesh->GetName()));
	}

	// clang-format off
	return SNew(SExpandableArea)
		.InitiallyCollapsed(!EditorPole.bExpanded)
		.AreaTitle(FText::FromString(FString::Printf(TEXT("Pole #%d"), PoleIndex)))
		.Padding(4)
		.OnAreaExpansionChanged_Lambda([&EditorPole](bool b) { EditorPole.bExpanded = b; })
		.BodyContent()
		[SNew(SVerticalBox)
			// — STYLE ComboBox —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 4)
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[SNew(STextBlock)
					.Text(FText::FromString("Style:"))
				] +
				SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8,0)
				[SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&StyleOptions)
					.InitiallySelectedItem(StyleOptions[(int32) Pole.Style])
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> In)
					{
						return SNew(STextBlock).Text(FText::FromString(*In));
					})
					.OnSelectionChanged_Lambda([this, PoleIndex](TSharedPtr<FString> NewStyle, ESelectInfo::Type)
					{
						const int32 Choice{StyleOptions.IndexOfByPredicate([&](auto& S) { return S == NewStyle; })};
						Poles[PoleIndex].Style = static_cast<ETLStyle>(Choice);
						OnPoleStyleChanged(Poles[PoleIndex].Style, PoleIndex);
						Rebuild();
					})
					[SNew(STextBlock)
						.Text_Lambda([this, PoleIndex]()
						{
							return FText::FromString(*StyleOptions[(int32) Poles[PoleIndex].Style]);
						})
					]
				]
			]
			// — ORIENTATION ComboBox —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 4)
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[SNew(STextBlock).
					Text(FText::FromString("Orientation:"))
				] +
				SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8, 0)
				[SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&OrientationOptions)
					.InitiallySelectedItem(OrientationOptions[(int32) Pole.Orientation])
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> In)
					{
						return SNew(STextBlock).Text(FText::FromString(*In));
					})
					.OnSelectionChanged_Lambda([this, PoleIndex](TSharedPtr<FString> NewOrientation, ESelectInfo::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						const int32 Choice{OrientationOptions.IndexOfByPredicate([&](auto& S) {return S == NewOrientation;})};
						OnPoleOrientationChanged(static_cast<ETLOrientation>(Choice), PoleIndex);
						Rebuild();
					})
					[SNew(STextBlock)
						.Text_Lambda([this, PoleIndex]()
						{
							FTLPole& Pole{Poles[PoleIndex]};
							return FText::FromString(*OrientationOptions[(int32) Pole.Orientation]);
						})
					]
				]
			]
			// — TRANSFORM (Location XYZ) —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 4)
			[SNew(STextBlock)
				.Text(FText::FromString("Transform Location"))
			] +
			SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 0)
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex]()
					{
						return Poles[PoleIndex].Transform.GetLocation().X;
					})
					.OnValueCommitted_Lambda([this, PoleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FVector L{Pole.Transform.GetLocation()};
						L.X = V;
						Pole.Transform.SetLocation(L);
						Rebuild();
					})
				] +
				SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex]()
					{
						return Poles[PoleIndex].Transform.GetLocation().Y;
					})
					.OnValueCommitted_Lambda([this, PoleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FVector L{Pole.Transform.GetLocation()};
						L.Y = V;
						Pole.Transform.SetLocation(L);
						Rebuild();
					})
				] +
				SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex]()
					{
						return Poles[PoleIndex].Transform.GetLocation().Z;
					})
					.OnValueCommitted_Lambda([this, PoleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FVector L{Pole.Transform.GetLocation()};
						L.Z = V;
						Pole.Transform.SetLocation(L);
						Rebuild();
					})
				]
			]
			// — TRANSFORM (Rotation Pitch/Yaw/Roll) —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 4)
			[SNew(STextBlock)
				.Text(FText::FromString("Transform Rotation"))
			] +
			SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 0)
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex]()
					{
						return Poles[PoleIndex].Transform.Rotator().Pitch;
					})
					.OnValueCommitted_Lambda([this, PoleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FRotator R{Pole.Transform.Rotator()};
						R.Pitch = V;
						Pole.Transform.SetRotation(FQuat(R));
						Rebuild();
					})
				] +
				SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex]()
					{
						const FTLPole& Pole{Poles[PoleIndex]};
						return Pole.Transform.Rotator().Yaw;
					})
					.OnValueCommitted_Lambda([this, PoleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FRotator R{Pole.Transform.Rotator()};
						R.Yaw = V;
						Pole.Transform.SetRotation(FQuat(R));
						Rebuild();
					})
				] +
				SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex]()
					{
						return Poles[PoleIndex].Transform.Rotator().Roll;
					})
					.OnValueCommitted_Lambda([this, PoleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FRotator R{Pole.Transform.Rotator()};
						R.Roll = V;
						Pole.Transform.SetRotation(FQuat(R));
						Rebuild();
					})
				]
			]

			// — TRANSFORM (Scale XYZ) —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 4)
			[SNew(STextBlock)
				.Text(FText::FromString("Transform Scale"))
			] +
			SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 0)
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex]()
					{
						return Poles[PoleIndex].Transform.GetScale3D().X;
					})
					.OnValueCommitted_Lambda([this, PoleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FVector S{Pole.Transform.GetScale3D()};
						S.X = V;
						Pole.Transform.SetScale3D(S);
						Rebuild();
					})
				] +
				SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex]()
					{
						return Poles[PoleIndex].Transform.GetScale3D().Y;
					})
					.OnValueCommitted_Lambda([this, PoleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FVector S{Pole.Transform.GetScale3D()};
						S.Y = V;
						Pole.Transform.SetScale3D(S);
						Rebuild();
					})
				] +
				SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda(
						[this, PoleIndex]()
						{
							return Poles[PoleIndex].Transform.GetScale3D().Z;
						})
					.OnValueCommitted_Lambda([this, PoleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FVector S{Pole.Transform.GetScale3D()};
						S.Z = V;
						Pole.Transform.SetScale3D(S);
						Rebuild();
					})
				]
			]

			// — Base Mesh Combo —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 4)
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[SNew(STextBlock)
					.Text(FText::FromString("Base Mesh:"))
				] +
				SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8, 0)
				[SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&EditorPole.BaseMeshNameOptions)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> InPtr)
					{
						return SNew(STextBlock).Text(FText::FromString(*InPtr));
					})
					.OnSelectionChanged_Lambda([this, PoleIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
					{
						if (!NewSelection)
						{
							return;
						}
						FTLPole& Pole{Poles[PoleIndex]};
						for (UStaticMesh* Mesh : FTLMeshFactory::GetAllBaseMeshesForPole(Pole))
						{
							if (IsValid(Mesh) && Mesh->GetName() == *NewSelection)
							{
								Pole.BasePoleMesh = Mesh;
								if (Pole.BasePoleMeshComponent)
								{
									Pole.BasePoleMeshComponent->SetStaticMesh(Mesh);
								}
								break;
							}
						}
						Rebuild();
					})
					[SNew(STextBlock)
						.Text_Lambda([this, PoleIndex]()
						{
							FTLPole& Pole{Poles[PoleIndex]};
							return FText::FromString(Pole.BasePoleMesh ? Pole.BasePoleMesh->GetName() : TEXT("Select..."));
						})
					]
				]
			]

			// — Extensible Mesh Combo —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 4)
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[SNew(STextBlock)
					.Text(FText::FromString("Extensible Mesh:"))
				] +
				SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8, 0)
				[SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&EditorPole.ExtensibleMeshNameOptions)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> InPtr)
					{
						return SNew(STextBlock).Text(FText::FromString(*InPtr));
					})
					.OnSelectionChanged_Lambda([this, PoleIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
					{
						if (!NewSelection)
						{
							return;
						}
						FTLPole& Pole{Poles[PoleIndex]};
						for (UStaticMesh* Mesh : FTLMeshFactory::GetAllExtensibleMeshesForPole(Pole))
						{
							if (IsValid(Mesh) && Mesh->GetName() == *NewSelection)
							{
								Pole.ExtensiblePoleMesh = Mesh;
								if (Pole.ExtensiblePoleMeshComponent)
								{
									Pole.ExtensiblePoleMeshComponent->SetStaticMesh(Mesh);
								}
								break;
							}
						}
						Rebuild();
					})
					[SNew(STextBlock)
						.Text_Lambda([this, PoleIndex]()
						{
							FTLPole& Pole{Poles[PoleIndex]};
							return FText::FromString(Pole.ExtensiblePoleMesh ? Pole.ExtensiblePoleMesh->GetName() : TEXT("Select..."));
						})
					]
				]
			]
			// — Cap Mesh Combo —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 4)
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[SNew(STextBlock)
					.Text(FText::FromString("Cap Mesh:"))
				] +
				SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8, 0)
				[SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&EditorPole.CapMeshNameOptions)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> InPtr)
					{
						return SNew(STextBlock).Text(FText::FromString(*InPtr));
					})
					.OnSelectionChanged_Lambda([this, PoleIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
					{
						if (!NewSelection)
						{
							return;
						}
						FTLPole& Pole{Poles[PoleIndex]};
						for (UStaticMesh* Mesh : FTLMeshFactory::GetAllCapMeshesForPole(Pole))
						{
							if (IsValid(Mesh) && Mesh->GetName() == *NewSelection)
							{
								Pole.CapPoleMesh = Mesh;
								if (Pole.CapPoleMeshComponent)
								{
									Pole.CapPoleMeshComponent->SetStaticMesh(Mesh);
								}
								break;
							}
						}
						Rebuild();
					})
					[SNew(STextBlock)
						.Text_Lambda([this, PoleIndex]()
						{
							FTLPole& Pole{Poles[PoleIndex]};
							return FText::FromString(Pole.CapPoleMesh ? Pole.CapPoleMesh->GetName() : TEXT("Select..."));
						})
					]
				]
			]
			// — Pole Height
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 4)
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[SNew(STextBlock)
					.Text(FText::FromString("Pole Height (m):"))
				] +
				SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.Padding(8, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex]()
					{
						const FTLPole& Pole{Poles[PoleIndex]};
						return Pole.PoleHeight;
					})
					.OnValueCommitted_Lambda([this, PoleIndex](float NewValue, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						Pole.PoleHeight = NewValue;
						Rebuild();
					})
				]
			]
			// — Heads Section —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2, 8)
			[SNew(SExpandableArea)
				.InitiallyCollapsed(!EditorPole.bHeadsExpanded)
				.AreaTitle(FText::FromString("Heads"))
				.Padding(4)
				.OnAreaExpansionChanged_Lambda([&EditorPole](bool b) { EditorPole.bHeadsExpanded = b; })
				.BodyContent()[BuildHeadSection(PoleIndex)]
			]
			// — Delete Pole button —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(2, 0)
			[SNew(SButton)
				.Text(FText::FromString("Delete Pole"))
				.OnClicked_Lambda([this, PoleIndex]() { return OnDeletePoleClicked(PoleIndex); })
			]
		];
	// clang-format on
}

TSharedRef<SWidget> STrafficLightToolWidget::BuildPoleSection()
{
	SAssignNew(RootContainer, SVerticalBox);

	// clang-format off
	return SNew(SVerticalBox)
    // Load JSON Button
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10)
        [
            SNew(SButton)
            .Text(FText::FromString("Import JSON…"))
            .OnClicked(this, &STrafficLightToolWidget::OnImportJSONClicked)
        ]
    // Export JSON Button
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(10)
	[
		SNew(SButton)
		.Text(FText::FromString("Export JSON…"))
		.OnClicked(this, &STrafficLightToolWidget::OnExportJSONClicked)
	]
    // — Modify Button — (only visible when actor is selected)
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10)
        [
            SNew(SButton)
            .Visibility(this, &STrafficLightToolWidget::GetModifyVisibility)
            .Text(FText::FromString("Modify"))
            .OnClicked(this, &STrafficLightToolWidget::OnModifyClicked)
        ]
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[SNew(SButton)
			.Text(FText::FromString("Add Pole"))
			.OnClicked(this, &STrafficLightToolWidget::OnAddPoleClicked)
		] +
		SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(10)
		[SNew(SScrollBox) + SScrollBox::Slot()[RootContainer.ToSharedRef()]];
	// clang-format on
}

TSharedRef<SWidget> STrafficLightToolWidget::BuildHeadEntry(int32 PoleIndex, int32 HeadIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};
	FTLHead& Head{Pole.Heads[HeadIndex]};

	// clang-format off
	return SNew(SExpandableArea)
		.InitiallyCollapsed(!EditorHead.bExpanded)
		.AreaTitle(FText::FromString(FString::Printf(TEXT("Head #%d"), HeadIndex)))
		.Padding(4)
		.OnAreaExpansionChanged_Lambda([this, PoleIndex, HeadIndex](bool bExpanded)
		{
			FEditorHead& EditorHead{EditorPoles[PoleIndex].Heads[HeadIndex]};
			EditorHead.bExpanded = bExpanded;
		})
		.BodyContent()
		[SNew(SBorder)
			.BorderBackgroundColor(FLinearColor{0.15f, 0.15f, 0.15f})
			.Padding(8)
			[SNew(SVerticalBox)
				// — Head Style —
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(STextBlock)
					.Text(FText::FromString("Head Style"))
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&StyleOptions)
					.InitiallySelectedItem(StyleOptions[(int32) Head.Style])
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> In){ return SNew(STextBlock).Text(FText::FromString(*In)); })
					.OnSelectionChanged_Lambda([this, PoleIndex, HeadIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
					{
						FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
						const int32 Choice{StyleOptions.IndexOfByPredicate([&](auto& S) { return S == NewSelection; })};
						Head.Style = static_cast<ETLStyle>(Choice);
						OnHeadStyleChanged(Head.Style, PoleIndex, HeadIndex);
					})
					[SNew(STextBlock)
						.Text_Lambda([this, PoleIndex, HeadIndex]()
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return FText::FromString(*StyleOptions[(int32) Head.Style]);
						})
					]
				]
				// — Attachment Type —
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(STextBlock)
					.Text(FText::FromString("Attachment"))
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&AttachmentOptions)
					.InitiallySelectedItem(AttachmentOptions[(int32) Head.Attachment])
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> In) {return SNew(STextBlock).Text(FText::FromString(*In));})
					.OnSelectionChanged_Lambda([this, PoleIndex, HeadIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
					{
						FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
						const int32 Choice{AttachmentOptions.IndexOfByPredicate([&](auto& S) {return S == NewSelection;})};
						Head.Attachment = static_cast<ETLHeadAttachment>(Choice);
					})
					[SNew(STextBlock)
						.Text_Lambda([this, PoleIndex, HeadIndex]()
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return FText::FromString(*AttachmentOptions[(int32) Head.Attachment]);
						})
					]
				]
				// — Orientation —
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(STextBlock)
					.Text(FText::FromString("Orientation"))
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&OrientationOptions)
					.InitiallySelectedItem(OrientationOptions[(int32) Head.Orientation])
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> In){ return SNew(STextBlock).Text(FText::FromString(*In)); })
					.OnSelectionChanged_Lambda([this, PoleIndex, HeadIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
					{
						const int32 Choice{OrientationOptions.IndexOfByPredicate([&](auto& S) { return S == NewSelection; })};
						OnHeadOrientationChanged(static_cast<ETLOrientation>(Choice), PoleIndex, HeadIndex);
					})
					[SNew(STextBlock)
						.Text_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return FText::FromString(*OrientationOptions[(int32) Head.Orientation]);
						})
					]
				]
				// — Backplate —
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SCheckBox)
					.IsChecked_Lambda([this, PoleIndex, HeadIndex]()
					{
						const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
						return Head.bHasBackplate ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this, PoleIndex, HeadIndex](ECheckBoxState NewState)
					{
                        FTLPole& Pole{Poles[PoleIndex]};
						FTLHead& Head{Pole.Heads[HeadIndex]};
						bool bOn{(NewState == ECheckBoxState::Checked)};
						Head.bHasBackplate = bOn;
						Rebuild();
					})
					[SNew(STextBlock)
						.Text(FText::FromString("Has Backplate"))
					]
				]
				// Relative Location
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(STextBlock)
					.Text(FText::FromString("Relative Location"))
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Transform.GetLocation().X;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector L{Head.Transform.GetLocation()};
							L.X = V;
							Head.Transform.SetLocation(L);
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Transform.GetLocation().Y;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector L{Head.Transform.GetLocation()};
							L.Y = V;
							Head.Transform.SetLocation(L);
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Transform.GetLocation().Z;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector L{Head.Transform.GetLocation()};
							L.Z = V;
							Head.Transform.SetLocation(L);
							Rebuild();
						})
					]
				]
                // Relative Rotation
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(STextBlock)
					.Text(FText::FromString("Relative Rotation"))
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<double>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Transform.GetRotation().Rotator().Pitch;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](double V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FRotator3d R{Head.Transform.GetRotation().Rotator()};
							R.Pitch = V;
							Head.Transform.SetRotation(R.Quaternion());
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<double>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Transform.GetRotation().Rotator().Yaw;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](double V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FRotator3d R{Head.Transform.GetRotation().Rotator()};
							R.Yaw = V;
							Head.Transform.SetRotation(R.Quaternion());
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<double>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Transform.GetRotation().Rotator().Roll;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](double V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FRotator3d R{Head.Transform.GetRotation().Rotator()};
							R.Roll = V;
							Head.Transform.SetRotation(R.Quaternion());
							Rebuild();
						})
					]
				]
                // Relative Scale
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(STextBlock)
					.Text(FText::FromString("Relative Scale"))
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<double>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Transform.GetScale3D().X;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](double V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector L{Head.Transform.GetScale3D()};
							L.X = V;
							Head.Transform.SetScale3D(L);
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<double>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Transform.GetScale3D().Y;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](double V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector L{Head.Transform.GetScale3D()};
							L.Y = V;
							Head.Transform.SetScale3D(L);
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<double>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Transform.GetScale3D().Z;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](double V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector L{Head.Transform.GetScale3D()};
							L.Z = V;
							Head.Transform.SetScale3D(L);
							Rebuild();
						})
					]
				]
				// Offset Position
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(STextBlock)
					.Text(FText::FromString("Offset Position"))
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SHorizontalBox) +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Offset.GetLocation().X;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector L{Head.Offset.GetLocation()};
							L.X = V;
							Head.Offset.SetLocation(L);
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Offset.GetLocation().Y;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector L{Head.Offset.GetLocation()};
							L.Y = V;
							Head.Offset.SetLocation(L);
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Offset.GetLocation().Z;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector L{Head.Offset.GetLocation()};
							L.Z = V;
							Head.Offset.SetLocation(L);
							Rebuild();
						})
					]
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(STextBlock)
					.Text(FText::FromString("Offset Rotation"))
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SHorizontalBox) +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Offset.Rotator().Pitch;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FRotator R{Head.Offset.Rotator()};
							R.Pitch = V;
							Head.Offset.SetRotation(FQuat{R});
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Offset.Rotator().Yaw;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FRotator R{Head.Offset.Rotator()};
							R.Yaw = V;
							Head.Offset.SetRotation(FQuat{R});
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Offset.Rotator().Roll;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FRotator R{Head.Offset.Rotator()};
							R.Roll = V;
							Head.Offset.SetRotation(FQuat{R});
							Rebuild();
						})
					]
				] +
				// Offset Scale
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(STextBlock)
					.Text(FText::FromString("Offset Scale"))
				] +
				SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[SNew(SHorizontalBox) +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Offset.GetScale3D().X;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector S{Head.Offset.GetScale3D()};
							S.X = V;
							Head.Offset.SetScale3D(S);
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Offset.GetScale3D().Y;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector S{Head.Offset.GetScale3D()};
							S.Y = V;
							Head.Offset.SetScale3D(S);
							Rebuild();
						})
					] +
					SHorizontalBox::Slot()
					.FillWidth(1)
					[SNew(SNumericEntryBox<float>)
						.Value_Lambda([this, PoleIndex, HeadIndex]()
						{
							const FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							return Head.Offset.GetScale3D().Z;
						})
						.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex](float V, ETextCommit::Type)
						{
							FTLHead& Head{Poles[PoleIndex].Heads[HeadIndex]};
							FVector S{Head.Offset.GetScale3D()};
							S.Z = V;
							Head.Offset.SetScale3D(S);
							Rebuild();
						})
					]
				] +
				// — Add + Modules + Delete —
				SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.Padding(2, 4, 2, 0)
				[SNew(SVerticalBox)
					// Add Module
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.Padding(2, 4)
					[SNew(SButton)
						.Text(FText::FromString("Add Module"))
						.OnClicked(this, &STrafficLightToolWidget::OnAddModuleClicked, PoleIndex, HeadIndex)
					]
					// Expandable “Modules” section
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2, 8)
					[SNew(SExpandableArea)
						.InitiallyCollapsed(!EditorHead.bModulesExpanded)
						.AreaTitle(FText::FromString("Modules"))
						.Padding(4)
						.OnAreaExpansionChanged_Lambda([&EditorHead](bool bExp) { EditorHead.bModulesExpanded = bExp; })
						.BodyContent()[BuildModuleSection(PoleIndex, HeadIndex)]
					]
					// Delete Head
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2, 0)
					[SNew(SButton)
						.Text(FText::FromString("Delete Head"))
						.OnClicked(this, &STrafficLightToolWidget::OnDeleteHeadClicked, PoleIndex, HeadIndex)
					]
				]
			]
		];
	//clang-format on
}

TSharedRef<SWidget> STrafficLightToolWidget::BuildHeadSection(int32 PoleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};

	TSharedRef<SVerticalBox> Container{SNew(SVerticalBox)};

	//clang-format off
	// — Add Head button —
	Container->AddSlot()
	.AutoHeight()
	.Padding(2)
	[SNew(SHorizontalBox)+
		SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[SNew(SButton)
			.Text(FText::FromString("Add Head"))
			.OnClicked(this, &STrafficLightToolWidget::OnAddHeadClicked, PoleIndex)
		]
	];

	// — Scrollable list of head entries —
	Container->AddSlot()
	.FillHeight(1.0f)
	.Padding(2)
	[SNew(SScrollBox) +
		SScrollBox::Slot()
		[SAssignNew(EditorPole.Container, SVerticalBox)
		]
	];

	EditorPole.Container->ClearChildren();
	for (int32 HeadIndex{0}; HeadIndex < EditorPole.Heads.Num(); ++HeadIndex)
	{
		FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};
		const bool bInitiallyCollapsed{!EditorHead.bExpanded};

		EditorPole.Container->AddSlot().AutoHeight().Padding(4, 2)[BuildHeadEntry(PoleIndex, HeadIndex)];
	}

	return Container;
	// clang-format on
}

TSharedRef<SWidget> STrafficLightToolWidget::BuildModuleEntry(int32 PoleIndex, int32 HeadIndex, int32 ModuleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};
	FTLHead& Head{Pole.Heads[HeadIndex]};

	check(EditorHead.Modules.IsValidIndex(ModuleIndex));
	FEditorModule& EditorModule{EditorHead.Modules[ModuleIndex]};
	FTLModule& Module{Head.Modules[ModuleIndex]};

	TArray<TSharedPtr<FString>>& ModuleMeshNames{EditorModule.MeshNameOptions};
	const TArray<UStaticMesh*> ModuleMeshes{FTLMeshFactory::GetAllMeshesForModule(Head, Module)};
	ModuleMeshNames.Empty();
	for (const UStaticMesh* Mesh : ModuleMeshes)
	{
		ModuleMeshNames.Add(MakeShared<FString>(Mesh->GetName()));
	}

	// clang-format off
	return SNew(SBorder)
	.Padding(4)
	[SNew(SHorizontalBox) +
		SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[SNew(SVerticalBox) +
			SVerticalBox::Slot()
			.AutoHeight()
			[SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ContentPadding(2)
				.IsEnabled(ModuleIndex > 0)
				.OnClicked(this, &STrafficLightToolWidget::OnMoveModuleUpClicked, PoleIndex, HeadIndex, ModuleIndex)
				[SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("Icons.ArrowUp"))
				]
			] +
			SVerticalBox::Slot()
			.AutoHeight()
			[SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ContentPadding(2)
				.IsEnabled(ModuleIndex < Head.Modules.Num() - 1)
				.OnClicked(this, &STrafficLightToolWidget::OnMoveModuleDownClicked, PoleIndex, HeadIndex, ModuleIndex)
				[SNew(SImage)
					.Image(FAppStyle::Get()
					.GetBrush("Icons.ArrowDown"))
				]
			]
		]
		+SHorizontalBox::Slot()
		.FillWidth(1)
		.VAlign(VAlign_Center)
		[SNew(SVerticalBox)
			// — Visor checkbox —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2)
			[SNew(SCheckBox)
				.IsChecked_Lambda([this, PoleIndex, HeadIndex, ModuleIndex]()
				{
					const FTLPole& Pole{Poles[PoleIndex]};
					const FTLHead& Head{Pole.Heads[HeadIndex]};
					const FTLModule& Module{Head.Modules[ModuleIndex]};
					return Module.bHasVisor ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged(this, &STrafficLightToolWidget::OnModuleVisorChanged, PoleIndex, HeadIndex, ModuleIndex)
				[SNew(STextBlock).
					Text(FText::FromString("Has Visor"))
				]
			]
			// — Mesh Combo
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0,2)
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[SNew(STextBlock)
							.Text(FText::FromString("Mesh:"))
					]
					+ SHorizontalBox::Slot()
						.FillWidth(1)
						.Padding(8,0)
					[SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&ModuleMeshNames)
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> InPtr){return SNew(STextBlock).Text(FText::FromString(*InPtr));})
						.OnSelectionChanged_Lambda([this, PoleIndex, HeadIndex, ModuleIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
						{
							if (!NewSelection)
							{
								return;
							}

							FTLPole& Pole     {Poles[PoleIndex]};
							FTLHead& Head     {Pole.Heads[HeadIndex]};
							FTLModule& Module {Head.Modules[ModuleIndex]};
							FEditorModule& EditorModule{EditorPoles[PoleIndex].Heads[HeadIndex].Modules[ModuleIndex]};

							for (UStaticMesh* Mesh : FTLMeshFactory::GetAllMeshesForModule(Head, Module))
							{
								if (Mesh && Mesh->GetName() == *NewSelection)
								{
									Module.ModuleMesh = Mesh;
									break;
								}
							}

							Module.Lights.Empty();
							if (Module.ModuleMesh)
							{
								const TArray<FStaticMaterial>& StaticMats {Module.ModuleMesh->GetStaticMaterials()};
								for (int32 SlotIndex {0}; SlotIndex < StaticMats.Num(); ++SlotIndex)
								{
									if (StaticMats[SlotIndex].MaterialSlotName.ToString().StartsWith(TEXT("led_")))
									{
										Module.Lights.Add(FTLModuleLight{});
									}
								}
							}
							EditorModule.Lights.Empty();
							EditorModule.Lights.SetNum(Module.Lights.Num());
							Rebuild();
						})
						[
							SNew(STextBlock)
								.Text_Lambda([this, PoleIndex, HeadIndex, ModuleIndex]()
								{
									const FTLModule& Module {Poles[PoleIndex].Heads[HeadIndex].Modules[ModuleIndex]};
									return FText::FromString(Module.ModuleMesh ? Module.ModuleMesh->GetName() : TEXT("Select..."));
								})
						]
					]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0,2)
			[
				SNew(SExpandableArea)
				.InitiallyCollapsed(!EditorModule.bLightsExpanded)
				.AreaTitle(FText::FromString("Lights"))
				.Padding(4)
				.OnAreaExpansionChanged_Lambda([&EditorModule](bool b){ EditorModule.bLightsExpanded = b; })
				.BodyContent()
				[
					BuildModuleLightSection(PoleIndex, HeadIndex, ModuleIndex)
				]
			]

			// — Offset Location
			// per axis —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2)
			[SNew(STextBlock)
				.Text(FText::FromString("Offset Location"))
			]
			// X axis
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2)
			[SNew(SHorizontalBox)
				// Label X
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[SNew(STextBlock)
					.Text(FText::FromString("X"))
				]
				// Numeric Entry
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex, HeadIndex, ModuleIndex]()
					{
						const FTLPole& Pole{Poles[PoleIndex]};
						const FTLHead& Head{Pole.Heads[HeadIndex]};
						const FTLModule& Module{Head.Modules[ModuleIndex]};
						return Module.Offset.GetLocation().X;
					})
					.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex, ModuleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FTLHead& Head{Pole.Heads[HeadIndex]};
						FTLModule& Module{Head.Modules[ModuleIndex]};
						FVector L{Module.Offset.GetLocation()};
						L.X = V;
						Module.Offset.SetLocation(L);
						Rebuild();
					})
				]
			]
			// Y axis
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2)
			[SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[SNew(STextBlock)
					.Text(FText::FromString("Y"))
				] +
				SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex, HeadIndex, ModuleIndex]()
					{
						const FTLPole& Pole{Poles[PoleIndex]};
						const FTLHead& Head{Pole.Heads[HeadIndex]};
						const FTLModule& Module{Head.Modules[ModuleIndex]};
						return Module.Offset.GetLocation().Y;
					})
					.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex, ModuleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FTLHead& Head{Pole.Heads[HeadIndex]};
						FTLModule& Module{Head.Modules[ModuleIndex]};
						FVector L{Module.Offset.GetLocation()};
						L.Y = V;
						Module.Offset.SetLocation(L);
						Rebuild();
					})
				]
			]
			// Z axis
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2)
			[SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[SNew(STextBlock)
					.Text(FText::FromString("Z"))
				] +
				SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex, HeadIndex, ModuleIndex]()
					{
						const FTLPole& Pole{Poles[PoleIndex]};
						const FTLHead& Head{Pole.Heads[HeadIndex]};
						const FTLModule& Module{Head.Modules[ModuleIndex]};
						return Module.Offset.GetLocation().Z;
					})
					.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex, ModuleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FTLHead& Head{Pole.Heads[HeadIndex]};
						FTLModule& Module{Head.Modules[ModuleIndex]};
						FVector L{Module.Offset.GetLocation()};
						L.Z = V;
						Module.Offset.SetLocation(L);
						Rebuild();
					})
				]
			]
			//----------------------------------------------------------------
			// Offset Rotation (Pitch, Yaw, Roll)
			//----------------------------------------------------------------
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2)
			[SNew(STextBlock)
				.Text(FText::FromString("Offset Rotation"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2)
			[SNew(SHorizontalBox)
				// Pitch
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex, HeadIndex, ModuleIndex]()
					{
						const FTLPole& Pole{Poles[PoleIndex]};
						const FTLHead& Head{Pole.Heads[HeadIndex]};
						const FTLModule& Module{Head.Modules[ModuleIndex]};
						return Module.Offset.Rotator().Pitch;
					})
					.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex, ModuleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FTLHead& Head{Pole.Heads[HeadIndex]};
						FTLModule& Module{Head.Modules[ModuleIndex]};
						FRotator R{Module.Offset.Rotator()};
						R.Pitch = V;
						Module.Offset.SetRotation(FQuat(R));
						Rebuild();
					})
				]
				// Yaw
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex, HeadIndex, ModuleIndex]()
					{
						const FTLPole& Pole{Poles[PoleIndex]};
						const FTLHead& Head{Pole.Heads[HeadIndex]};
						const FTLModule& Module{Head.Modules[ModuleIndex]};
						return Module.Offset.Rotator().Yaw;
					})
					.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex, ModuleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FTLHead& Head{Pole.Heads[HeadIndex]};
						FTLModule& Module{Head.Modules[ModuleIndex]};
						FRotator R{Module.Offset.Rotator()};
						R.Yaw = V;
						Module.Offset.SetRotation(FQuat(R));
						Rebuild();
					})
				]
				// Roll
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(2, 0)
				[SNew(SNumericEntryBox<float>)
					.Value_Lambda([this, PoleIndex, HeadIndex, ModuleIndex]()
					{
						const FTLPole& Pole{Poles[PoleIndex]};
						const FTLHead& Head{Pole.Heads[HeadIndex]};
						const FTLModule& Module{Head.Modules[ModuleIndex]};
						return Module.Offset.Rotator().Roll;
					})
					.OnValueCommitted_Lambda([this, PoleIndex, HeadIndex, ModuleIndex](float V, ETextCommit::Type)
					{
						FTLPole& Pole{Poles[PoleIndex]};
						FTLHead& Head{Pole.Heads[HeadIndex]};
						FTLModule& Module{Head.Modules[ModuleIndex]};
						FRotator R{Module.Offset.Rotator()};
						R.Roll = V;
						Module.Offset.SetRotation(FQuat(R));
						Rebuild();
					})
				]
			]
			// — Delete Module button —
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2)
			[SNew(SButton)
				.Text(FText::FromString("Delete Module"))
				.OnClicked(this, &STrafficLightToolWidget::OnDeleteModuleClicked, PoleIndex, HeadIndex, ModuleIndex)
			]
		]
	];

	// clang-format on
}

TSharedRef<SWidget> STrafficLightToolWidget::BuildModuleSection(int32 PoleIndex, int32 HeadIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};

	TSharedRef<SVerticalBox> Container{SNew(SVerticalBox)};

	// clang-format off
	for (int32 ModuleIndex{0}; ModuleIndex < EditorHead.Modules.Num(); ++ModuleIndex)
	{
		FEditorModule& EditorModule{EditorHead.Modules[ModuleIndex]};

		Container->AddSlot()
		.AutoHeight()
		.Padding(4, 2)
		[SNew(SExpandableArea)
			.InitiallyCollapsed(!EditorModule.bExpanded)
			.AreaTitle(FText::FromString(FString::Printf(TEXT("Module %d"), ModuleIndex)))
			.Padding(4)
			.OnAreaExpansionChanged_Lambda([this, &EditorModule](bool bExpanded) { EditorModule.bExpanded = bExpanded; })
			.BodyContent()[BuildModuleEntry(PoleIndex, HeadIndex, ModuleIndex)]];
	}

	return Container;
	// clang-format on
}

TSharedRef<SWidget> STrafficLightToolWidget::BuildModuleLightEntry(
	int32 PoleIndex, int32 HeadIndex, int32 ModuleIndex, int32 LightIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};
	FTLPole& Pole{Poles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};
	FTLHead& Head{Pole.Heads[HeadIndex]};

	check(EditorHead.Modules.IsValidIndex(ModuleIndex));
	FEditorModule& EditorModule{EditorHead.Modules[ModuleIndex]};
	FTLModule& Module{Head.Modules[ModuleIndex]};

	check(Module.Lights.IsValidIndex(LightIndex));
	const FTLModuleLight& ModuleLight{Module.Lights[LightIndex]};

	// clang-format off
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2, 0)
		[
		SNew(SHorizontalBox)

		// — Light-Type Combo —
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0)
			[
			SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&LightTypeOptions)
				.InitiallySelectedItem(LightTypeOptions[static_cast<int32>(ModuleLight.LightType)])
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> In)
				{
					return SNew(STextBlock).Text(FText::FromString(*In));
				})
				.OnSelectionChanged_Lambda([this, PoleIndex, HeadIndex, ModuleIndex, LightIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
				{
					if (!NewSelection)
					{
						return;
					}
					FTLModuleLight& ModuleLight {Poles[PoleIndex].Heads[HeadIndex].Modules[ModuleIndex].Lights[LightIndex]};
					const int32 Choice {LightTypeOptions.IndexOfByPredicate(
					[&](auto& Ptr){ return *Ptr == *NewSelection; })};
					ModuleLight.LightType = static_cast<ETLLightType>(Choice);

					FVector2D AtlasUV {FTLMeshFactory::GetAtlasCoordsForLightType(ModuleLight.LightType)};
                    ModuleLight.U = AtlasUV.X;
                    ModuleLight.V = AtlasUV.Y;

                    if (UDataTable* DT {FTLMeshFactory::GetLightTypeMeshTable()})
					{
                        const UEnum* EnumPtr {StaticEnum<ETLLightType>()};
                        const FString Key {EnumPtr->GetNameStringByValue((int64)ModuleLight.LightType)};
                        if (const FTLLightTypeRow* Row {DT->FindRow<FTLLightTypeRow>(
                            FName(*Key), TEXT("BuildModuleLightEntry"))})
                        {
                            ModuleLight.EmissiveColor     = Row->Color;
                            ModuleLight.EmissiveIntensity = Row->EmissiveIntensity;
					}
					}

					Rebuild();
				})
			[
				SNew(STextBlock)
				.Text_Lambda([&]()
				{
					return FText::FromString(
					*LightTypeOptions[static_cast<int32>(ModuleLight.LightType)]);
				})
			]
			]

		// — Emissive-Intensity Slider —
		+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2, 0)
			[
			SNew(SSlider)
				.MinValue(0.0f)
				.MaxValue(1000.0f)
				.Value_Lambda([&]() { return ModuleLight.EmissiveIntensity; })
				.OnValueChanged_Lambda([this, PoleIndex, HeadIndex, ModuleIndex, LightIndex](float NewVal)
				{
				Poles[PoleIndex].Heads[HeadIndex].Modules[ModuleIndex].Lights[LightIndex].EmissiveIntensity = NewVal;
				Rebuild();
				})
			]
		]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(2, 4)
	[
		SNew(SColorPicker)
		.TargetColorAttribute_Lambda([&]() { return ModuleLight.EmissiveColor; })
		.UseAlpha(false)
		.OnlyRefreshOnMouseUp(false)
		.OnColorCommitted_Lambda([this, PoleIndex, HeadIndex, ModuleIndex, LightIndex](FLinearColor NewColor)
		{
			Poles[PoleIndex].Heads[HeadIndex].Modules[ModuleIndex].Lights[LightIndex].EmissiveColor = NewColor;
			Rebuild();
		})
	];
	// clang-format on
}

TSharedRef<SWidget> STrafficLightToolWidget::BuildModuleLightSection(int32 PoleIndex, int32 HeadIndex, int32 ModuleIndex)
{
	check(EditorPoles.IsValidIndex(PoleIndex));
	FEditorPole& EditorPole{EditorPoles[PoleIndex]};

	check(EditorPole.Heads.IsValidIndex(HeadIndex));
	FEditorHead& EditorHead{EditorPole.Heads[HeadIndex]};

	check(EditorHead.Modules.IsValidIndex(ModuleIndex));
	FEditorModule& EditorModule{EditorHead.Modules[ModuleIndex]};

	TArray<FEditorModuleLight>& EditorLightsArray{EditorModule.Lights};
	TSharedRef<SVerticalBox> Container{SNew(SVerticalBox)};

	// clang-format off
	for (int32 LightIndex{0}; LightIndex < EditorLightsArray.Num(); ++LightIndex)
	{
		FEditorModuleLight& EditorLight{EditorLightsArray[LightIndex]};
		Container->AddSlot()
		.AutoHeight()
		.Padding(4, 2)
		[SNew(SExpandableArea)
			.InitiallyCollapsed(!EditorLight.bExpanded)
			.AreaTitle(FText::FromString(FString::Printf(TEXT("Light %d"), LightIndex)))
			.Padding(4)
			.OnAreaExpansionChanged_Lambda([this, &EditorLight](bool bExpanded) { EditorLight.bExpanded = bExpanded; })
			.BodyContent()[BuildModuleLightEntry(PoleIndex, HeadIndex, ModuleIndex, LightIndex)]];
	}

	return Container;
	// clang-format on
}

// ----------------------------------------------------------------------------------------------------------------
// ******************************   ENUM TO TEXT
// ----------------------------------------------------------------------------------------------------------------
FString STrafficLightToolWidget::GetHeadStyleText(ETLStyle Style)
{
	const UEnum* EnumPtr{StaticEnum<ETLStyle>()};
	if (!IsValid(EnumPtr))
	{
		return FString(TEXT("Unknown"));
	}
	return EnumPtr->GetDisplayNameTextByValue((int64) Style).ToString();
}

FString STrafficLightToolWidget::GetHeadAttachmentText(ETLHeadAttachment Attach)
{
	const UEnum* EnumPtr{StaticEnum<ETLHeadAttachment>()};
	if (!IsValid(EnumPtr))
	{
		return FString(TEXT("Unknown"));
	}
	return EnumPtr->GetDisplayNameTextByValue((int64) Attach).ToString();
}

FString STrafficLightToolWidget::GetHeadOrientationText(ETLOrientation Orient)
{
	const UEnum* EnumPtr{StaticEnum<ETLOrientation>()};
	if (!IsValid(EnumPtr))
	{
		return FString(TEXT("Unknown"));
	}
	return EnumPtr->GetDisplayNameTextByValue((int64) Orient).ToString();
}

FString STrafficLightToolWidget::GetLightTypeText(ETLLightType Type)
{
	const UEnum* EnumPtr{StaticEnum<ETLLightType>()};
	if (!IsValid(EnumPtr))
	{
		return FString(TEXT("Unknown"));
	}
	return EnumPtr->GetDisplayNameTextByValue((int64) Type).ToString();
}

EVisibility STrafficLightToolWidget::GetModifyVisibility() const
{
	if (!GEditor)
	{
		return EVisibility::Collapsed;
	}

	UWorld* const EditorWorld{GEditor->GetEditorWorldContext().World()};
	if (!EditorWorld)
	{
		return EVisibility::Collapsed;
	}

	int32 SelectedCount{0};
	AActor* SelectedActor{nullptr};
	for (FSelectionIterator It{GEditor->GetSelectedActorIterator()}; It; ++It)
	{
		AActor* const Actor{Cast<AActor>(*It)};
		if (IsValid(Actor))
		{
			SelectedActor = Actor;
			++SelectedCount;
		}
	}

	if (SelectedCount != 1)
	{
		return EVisibility::Collapsed;
	}

	USceneComponent* const Root{SelectedActor->GetRootComponent()};
	if (!(IsValid(Root) && Root->GetName() == TEXT("BakedRoot")))
	{
		return EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}
