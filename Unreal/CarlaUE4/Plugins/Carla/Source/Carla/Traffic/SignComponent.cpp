// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "SignComponent.h"

USignComponent::USignComponent()
{
  PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void USignComponent::BeginPlay()
{
  Super::BeginPlay();

}


// Called every frame
void USignComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

const FString& USignComponent::GetSignId() const
{
  return SignId;
}

void USignComponent::SetSignId(const FString &Id) {
  SignId = Id;
}
