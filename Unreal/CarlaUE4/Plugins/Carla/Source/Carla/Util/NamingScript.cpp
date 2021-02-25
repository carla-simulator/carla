// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "NamingScript.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture.h"
#include "Engine/TextureCube.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialParameterCollection.h"
#include "Particles/ParticleSystem.h"

UNamingScript::UNamingScript(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {

	InitializeContainers();

	TexSuffMsg = "----------------------------------\n"
		"Texture suffixes you should use\n"
		"----------------------------------\n"
		"Diffuse / Albedo / Base Color = _D\n"
		"Normal = _N\n"
		"Roughness = _R\n"
		"Alpha / Opacity = _A\n"
		"Ambient Occlusion = _O\n"
		"Bump = _B\n"
		"Emissive = _E\n"
		"Mask = _M\n"
		"Specular = _S\n"
		"Metallic = _M\n"
		"Occlusion, roughness, metallic = _ORM\n"
		"SubSurface Scattering = _SSS\n";
}

void UNamingScript::InitializeContainers() {

	TextureSuffixes.Add("_D");
	TextureSuffixes.Add("_N");
	TextureSuffixes.Add("_R");
	TextureSuffixes.Add("_A");
	TextureSuffixes.Add("_O");
	TextureSuffixes.Add("_B");
	TextureSuffixes.Add("_E");
	TextureSuffixes.Add("_M");
	TextureSuffixes.Add("_S");
	TextureSuffixes.Add("_M");
	TextureSuffixes.Add("_ORM");
	TextureSuffixes.Add("_SSS");
}

void UNamingScript::FlushData() {

	AssetFilenames.Empty();
	AssetData.Empty();
	MessageLog.Empty();
}

bool UNamingScript::LoadDataFromPath(const FString &Path) {

	//Load the modules
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IFileManager &FileManager = IFileManager::Get();

	//Scan directories recursively from disk in order to access assets
	FString FullPath = FileManager.ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	FullPath /= Path;

	//If directory exists, then convert the absolute path
	//to in-game path (this is used later for the asset registry module in order to read the assets)
	if (FPaths::DirectoryExists(FullPath)) {

		FileManager.FindFilesRecursive(AssetFilenames, *FullPath, TEXT("*"), false, true);
		if (AssetFilenames.Num() != 0) {

			for (int i = 0; i < AssetFilenames.Num(); ++i) {

				//Convert from "Content/Carla..." to "Game/Carla..." directory.
				//This is needed for the asset registry module
				AssetFilenames[i] = AssetFilenames[i].RightChop(AssetFilenames[i].Find("Content", ESearchCase::Type::CaseSensitive, ESearchDir::Type::FromStart) + 7);
				FString FinalString = "/Game";
				FinalString += AssetFilenames[i];
				AssetFilenames[i] = FinalString;
			}
		}
	}
	else {

		FlushData();
		return false;
	}

	AssetFilenames.Add("/Game/" + Path);

	//Scan the obtained filenames for assets
	AssetRegistryModule.Get().ScanPathsSynchronous(AssetFilenames);

	//Array that will hold all assets found
	for (int i = 0; i < AssetFilenames.Num(); ++i) {

		AssetRegistryModule.Get().GetAssetsByPath(*AssetFilenames[i], AssetData);
	}

	return true;
}

void UNamingScript::ValidateAssetName(const FString &AssetPath, const FString &AssetName, const AssetTypes &AssetType, const FString &ValidAssetName, const FString &ValidPrefix, bool bUsesSuffix) {

	if (AssetName.IsEmpty() == true) {

		//Do something else?
		return;
	}

	MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("Validating name : " + AssetName), EMessageSeverity::Type::Info));
	MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString(AssetPath), EMessageSeverity::Type::CriticalError));

	bool IsValid = true;

	int CharOcurrences = 0;
	for (int i = 0; i < AssetName.GetCharArray().Num(); ++i) {

		if (AssetName.GetCharArray()[i] == '_') {

			++CharOcurrences;
		}
	}

	if (CharOcurrences < 1 || CharOcurrences > 3) {

		MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("The name of the asset should only have between 1 or 3 '_' characters"), EMessageSeverity::Type::Error));
		MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("The name of the asset is invalid. You should use : " + ValidAssetName), EMessageSeverity::Type::Error));
		return;
	}

	bool FoundCharacter = false;
	FString CurrentAssetPrefix;
	for (int i = 0; i < AssetName.Len() && FoundCharacter == false; ++i) {

		if (AssetName[i] != '_') {

			CurrentAssetPrefix += AssetName[i];
		}
		else {

			CurrentAssetPrefix += AssetName[i];
			FoundCharacter = true;
		}
	}

	//Validate the prefix of the asset
	if (ValidPrefix.Equals(CurrentAssetPrefix, ESearchCase::CaseSensitive) == false) {

		if (ValidPrefix.Equals(CurrentAssetPrefix.ToUpper(), ESearchCase::CaseSensitive) == true) {

			MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("The prefix of the asset is correct but must in uppercase " + ValidPrefix.ToUpper()), EMessageSeverity::Type::Error));
		}
		else {

			MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("The name of the asset must have prefix " + ValidPrefix), EMessageSeverity::Type::Error));
		}

		IsValid = false;
	}

	//Validate the suffix of the asset if it uses any
	if (bUsesSuffix == true) {

		FString CurrentAssetSuffix = AssetName.RightChop(AssetName.Find("_", ESearchCase::CaseSensitive, ESearchDir::FromEnd));
		FString *SearchResult = NULL;

		if (AssetType == AssetTypes::Texture) {

			SearchResult = TextureSuffixes.Find(CurrentAssetSuffix);
		}
		else {
		
			UE_LOG(LogTemp, Error, TEXT("Asset uses suffix but it's not recognized"));
			return;
		}

		//Keys in a set are NOT case sensitive. That's why "_n" might be found in the set, which in reality is "_N"  
		if (SearchResult != NULL && SearchResult->Equals(CurrentAssetSuffix, ESearchCase::CaseSensitive) == false) {

			if (SearchResult->Equals(CurrentAssetSuffix.ToUpper()) == true) {

				MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("The suffix of the asset is correct but must be in uppercase " + CurrentAssetSuffix.ToUpper()), EMessageSeverity::Type::Error));
			}
			else {

				MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("The name of the asset has invalid suffix " + CurrentAssetSuffix), EMessageSeverity::Type::Error));
			}

			IsValid = false;
		}
		else if (SearchResult == NULL) {

			MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("The name of the asset has invalid suffix " + CurrentAssetSuffix), EMessageSeverity::Type::Error));
			IsValid = false;
		}
	}

	if (IsValid == false) {

		MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("The name of the asset is invalid. You should use : " + ValidAssetName), EMessageSeverity::Type::Error));
	}
	else {

		MessageLog.Add(TPair<FText, EMessageSeverity::Type>(FText::FromString("The name of the asset seems to be valid but you should check that base name of the asset and the variant use capital letters : " + ValidAssetName), EMessageSeverity::Type::Warning));
	}
}

void UNamingScript::IdentifyAndCheckAssets() {

	for (int i = 0; i < AssetData.Num(); ++i) {

		if (AssetFilters.bScanForTextures == true) {

			if (Cast<UTexture>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::Texture, "T_BaseAssetName_VariantOrVersionIfAny_TextureSuffix", "T_", true);
			}
			else if (Cast<UTexture2D>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::Texture, "T_BaseAssetName_VariantOrVersionIfAny_TextureSuffix", "T_", true);
			}
			else if (Cast<UTextureCube>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::TextureCube, "TC_BaseAssetName_VariantOrVersionIfAny_TextureSuffix", "TC_", false);
			}
		}

		if (AssetFilters.bScanForMaterials == true) {

			if (Cast<UMaterial>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::Material, "M_BaseAssetName_VariantOrVersionIfAny", "M_", false);
			}
			else if (Cast<UMaterialInstance>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::MaterialInstance, "MI_BaseAssetName_VariantOrVersionIfAny", "MI_", false);
			}
			else if (Cast<UMaterialParameterCollection>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::MaterialParameterCollection, "MPC_BaseAssetName_VariantOrVersionIfAny", "MPC_", false);
			}
			else if (Cast<UMaterialFunctionInterface>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::MaterialFunction, "MF_BaseAssetName_VariantOrVersionIfAny", "MF_", false);
			}
		}

		if (AssetFilters.bScanForParticles == true) {

			if (Cast<UParticleSystem>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::ParticleSystem, "PS_BaseAssetName_VariantOrVersionIfAny", "PS_", false);
			}
		}

		if (AssetFilters.bScanForAnimations == true) {

			if (Cast<UAnimBlueprint>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::AnimationBlueprint, "ABP_BaseAssetName_VariantOrVersionIfAny", "ABP_", false);
			}
			else if (Cast<UAnimSequence>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::AnimationSequence, "A_BaseAssetName_VariantOrVersionIfAny", "A_", false);
			}
		}

		if (AssetFilters.bScanForMeshes == true) {

			if (Cast<UStaticMesh>(AssetData[i].GetAsset())) {

				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::StaticMesh, "S_BaseAssetName_VariantOrVersionIfAny", "S_", false);
			}
			else if (Cast<USkeletalMesh>(AssetData[i].GetAsset())) {
			
				ValidateAssetName(AssetData[i].PackageName.ToString(), AssetData[i].AssetName.ToString(), AssetTypes::SkeletalMesh, "SK_BaseAssetName_VariantOrVersionIfAny", "SK_", false);
			}
		}
	}
}

void UNamingScript::CheckAssetsNaming(FString DirectoryToScan, FFilterAssetsScan AssetsToScan) {

	if (LoadDataFromPath(DirectoryToScan) == false) {

		return;
	}

	FMessageLog MessageLogWindow("Naming script");
	MessageLogWindow.Open(EMessageSeverity::Info, true);

	AssetFilters = AssetsToScan;

	if (AssetsToScan.bScanForTextures == true) MessageLogWindow.Message(EMessageSeverity::Info, FText::FromString(TexSuffMsg));

	//Function will check the name of the loaded assets from the given path
	IdentifyAndCheckAssets();

	//Display log errors ocurred during the validation process
	MessageLogWindow.Message(EMessageSeverity::Info, FText::FromString("-----------------------------------------"));
	MessageLogWindow.Message(EMessageSeverity::Info, FText::FromString("Displaying 'Naming Script' Logging errors"));
	MessageLogWindow.Message(EMessageSeverity::Info, FText::FromString("------------------------------------------"));

	for (int i = 0; i < MessageLog.Num(); ++i) {

		if (MessageLog[i].Value == EMessageSeverity::Type::CriticalError) {

			MessageLogWindow.Info()->AddToken(FAssetNameToken::Create(MessageLog[i].Key.ToString()));
		}
		else {

			MessageLogWindow.Message(MessageLog[i].Value, MessageLog[i].Key);
		}
	}

	FlushData();
}