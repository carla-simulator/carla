// Copyright 2017 Mike Fricker. All Rights Reserved.

#pragma once

/** StreetMap Editor Style Helper Class. */
class FStreetMapStyle
{
public:

	static void Initialize();
	static void Shutdown();

	static TSharedPtr< class ISlateStyle > Get();

private:

	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

private:

	static TSharedPtr< class FSlateStyleSet > StyleSet;
};