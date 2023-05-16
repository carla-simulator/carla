// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "StreetMapActor.h"
#include "StreetMapRuntime.h"
#include "StreetMapComponent.h"


AStreetMapActor::AStreetMapActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StreetMapComponent = CreateDefaultSubobject<UStreetMapComponent>(TEXT("StreetMapComp"));
	RootComponent = StreetMapComponent;
}