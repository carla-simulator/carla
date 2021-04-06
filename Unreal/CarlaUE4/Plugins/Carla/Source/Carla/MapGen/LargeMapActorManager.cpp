// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "MapGen/LargeMapActorManager.h"
#include "LargeMapUtils.h"

ULargeMapActorManager::ULargeMapActorManager()
{
  PrimaryComponentTick.bCanEverTick = false;

}

void ULargeMapActorManager::BeginPlay()
{
  Super::BeginPlay();

}

void ULargeMapActorManager::ChangeWorldOrigin(FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
  CurrentOriginInt = InDstOrigin;
  CurrentOriginD = FDVector(InDstOrigin);
}

bool ULargeMapActorManager::OnActorSpawned(const FActorView& ActorView)
{
  const AActor* Actor = ActorView.GetActor();

  if (IsValid(Actor) && Cast<APawn>(Actor))
  {
    LM_LOG(Warning, "ALargeMapManager::OnActorSpawned %s", Actor->GetName());

    // Check if is hero vehicle
    UWorld* World = GetWorld();
    const FActorInfo* ActorInfo = ActorView.GetActorInfo();

    if(!ActorInfo)
    {
      return false;
    }

    const FActorDescription& Description = ActorInfo->Description;
    const FActorAttribute* Attribute = Description.Variations.Find("role_name");
    // If is the hero vehicle
    if(Attribute && Attribute->Value.Contains("hero"))
    {
      AddActorToConsider(const_cast<AActor*>(Actor));
      return true;
    }
  }
  return false;
}

void ULargeMapActorManager::AddActor(const FActorView& ActorView, const FTransform& Transform)
{
  GhostActors.Add(ActorView.GetActorId(), {Transform, ActorView});
}

void ULargeMapActorManager::AddActorToConsider(AActor* InActor)
{
  ActorsToConsider.Add({ InActor });
}

void ULargeMapActorManager::RemoveActorToConsider(AActor* InActor)
{
  int Index = 0;
  for (int i = 0; i < ActorsToConsider.Num(); i++)
  {
    const FActorToConsider& ActorToConsider = ActorsToConsider[i];
    if (ActorToConsider.Actor == InActor)
    {
      ActorsToConsider.Remove(ActorToConsider);
      Index = i;
      break;
    }
  }
}

void ULargeMapActorManager::Reset()
{
  ActorsToConsider.Reset();
}

void ULargeMapActorManager::UpdateAndGetActorsToConsiderPosition(TArray<FDVector>& OutActorsLocation)
{
  OutActorsLocation.Reset();

  if (!ActorsToConsider.Num())
  {
    return;
  }

  TArray<FActorToConsider> ActorsToRemove;
  for (FActorToConsider& ActorToConsider : ActorsToConsider)
  {
    AActor* Actor = ActorToConsider.Actor;
    if (IsValid(Actor))
    {
      // Relative location to the current origin
      FDVector ActorLocation(Actor->GetActorLocation());
      // Absolute location of the actor
      ActorToConsider.Location = CurrentOriginD + ActorLocation;

      OutActorsLocation.Add(ActorToConsider.Location);
    }
    else
    {
      ActorsToRemove.Add(ActorToConsider);
    }
  }

  for (const FActorToConsider& ActorToRemove : ActorsToRemove)
  {
    ActorsToConsider.Remove(ActorToRemove);
  }
}

void ULargeMapActorManager::GetActorsToConsiderPosition(TArray<FDVector>& OutActorsLocation) const
{
  OutActorsLocation.Reset();

  for (const FActorToConsider& ActorToConsider : ActorsToConsider)
  {
    OutActorsLocation.Add(ActorToConsider.Location);
  }
}

#if WITH_EDITOR

void ULargeMapActorManager::PrintMapInfo(float MsgTime, int32 FirstMsgIndex, int32 MsgLimit)
{
  UWorld* World = GetWorld();

  const FColor PositonMsgColor = FColor::Purple;

  int CurrentMsgIndex = FirstMsgIndex;
  int LastMsgIndex = FirstMsgIndex + MsgLimit;
  GEngine->AddOnScreenDebugMessage(CurrentMsgIndex++, MsgTime, FColor::White,
    FString::Printf(TEXT("Origin: %s km"), *(FDVector(CurrentOriginInt) / (1000.0 * 100.0)).ToString()) );
  GEngine->AddOnScreenDebugMessage(CurrentMsgIndex++, MsgTime, FColor::White, TEXT("Closest tiles - Distance:"));

  GEngine->AddOnScreenDebugMessage(CurrentMsgIndex++, MsgTime, FColor::White,
    FString::Printf(TEXT("Actors To Consider (%d)"), ActorsToConsider.Num()) );
  for (const FActorToConsider& ActorToConsider : ActorsToConsider)
  {
    FString Output = "";
    float ToKm = 1000.0f * 100.0f;
    FVector TileActorLocation = ActorToConsider.Actor->GetActorLocation();
    FDVector ClientActorLocation = CurrentOriginD + FDVector(TileActorLocation);

    Output += FString::Printf(TEXT("Local Loc: %s meters\n"), *(TileActorLocation / ToKm).ToString());
    Output += FString::Printf(TEXT("Client Loc: %s km\n"), *(ClientActorLocation / ToKm).ToString());
    Output += "---------------";
    GEngine->AddOnScreenDebugMessage(CurrentMsgIndex++, MsgTime, PositonMsgColor, Output);

    if (CurrentMsgIndex > LastMsgIndex) break;
  }
}

#endif // WITH_EDITOR