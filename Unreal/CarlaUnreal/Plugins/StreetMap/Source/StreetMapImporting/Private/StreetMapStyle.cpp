// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "StreetMapStyle.h"
#include "StreetMapImporting.h"

#include "Styling/SlateStyle.h"
#include "Interfaces/IPluginManager.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( FStreetMapStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )

FString FStreetMapStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString IconsDir = IPluginManager::Get().FindPlugin(TEXT("StreetMap"))->GetContentDir() / TEXT("Icons");
	return (IconsDir / RelativePath) + Extension;
}

TSharedPtr< FSlateStyleSet > FStreetMapStyle::StyleSet = NULL;
TSharedPtr< class ISlateStyle > FStreetMapStyle::Get() { return StyleSet; }

void FStreetMapStyle::Initialize()
{
	// Const icon & thumbnail sizes
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon128x128(128.0f, 128.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet("StreetMapStyle"));
	FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("StreetMap"))->GetContentDir();
	StyleSet->SetContentRoot(ContentDir);

	StyleSet->Set("ClassIcon.StreetMap", new IMAGE_BRUSH("sm_asset_icon_32", Icon16x16));
	StyleSet->Set("ClassThumbnail.StreetMap", new IMAGE_BRUSH("sm_asset_icon_128", Icon128x128));

	StyleSet->Set("ClassIcon.StreetMapActor", new IMAGE_BRUSH("sm_actor_icon_32", Icon16x16));
	StyleSet->Set("ClassThumbnail.StreetMapActor", new IMAGE_BRUSH("sm_actor_icon_128", Icon128x128));

	StyleSet->Set("ClassIcon.StreetMapComponent", new IMAGE_BRUSH("sm_component_icon_32", Icon16x16));
	StyleSet->Set("ClassThumbnail.StreetMapComponent", new IMAGE_BRUSH("sm_component_icon_128", Icon128x128));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};

#undef IMAGE_BRUSH


void FStreetMapStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}