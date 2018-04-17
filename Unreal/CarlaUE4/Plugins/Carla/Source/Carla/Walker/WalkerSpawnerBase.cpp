// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "WalkerSpawnerBase.h"

#include "Util/RandomEngine.h"
#include "Walker/WalkerAIController.h"
#include "Walker/WalkerSpawnPoint.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static bool WalkerIsValid(const ACharacter *Walker)
{
  return ((Walker != nullptr) && !Walker->IsPendingKill());
}

static AWalkerAIController *GetController(ACharacter *Walker)
{
  return (WalkerIsValid(Walker) ? Cast<AWalkerAIController>(Walker->GetController()) : nullptr);
}

static float GetDistance(const FVector &Location0, const FVector &Location1)
{
  return FMath::Abs((Location0 - Location1).Size());
}

static float GetDistance(const AActor &Actor0, const AActor &Actor1)
{
  return GetDistance(Actor0.GetActorLocation(), Actor1.GetActorLocation());
}

static EWalkerStatus GetWalkerStatus(ACharacter *Walker)
{
  const auto *Controller = GetController(Walker);
  return (Controller == nullptr ? EWalkerStatus::Invalid : Controller->GetWalkerStatus());
}

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

AWalkerSpawnerBase::AWalkerSpawnerBase(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;
}

// =============================================================================
// -- Overriden from AActor ----------------------------------------------------
// =============================================================================

void AWalkerSpawnerBase::BeginPlay()
{
  Super::BeginPlay();

  NumberOfWalkers = FMath::Max(0, NumberOfWalkers);

  // Allocate space for walkers.
  Walkers.Reserve(NumberOfWalkers);

  // Find spawn points present in level.
  TArray<AWalkerSpawnPointBase *> BeginSpawnPoints;
  for (TActorIterator<AWalkerSpawnPointBase> It(GetWorld()); It; ++It) {
    BeginSpawnPoints.Add(*It);
    AWalkerSpawnPoint *SpawnPoint = Cast<AWalkerSpawnPoint>(*It);
    if (SpawnPoint != nullptr) {
      SpawnPoints.Add(SpawnPoint);
    }
  }
  #ifdef CARLA_AI_WALKERS_EXTRA_LOG
  UE_LOG(LogCarla, Log, TEXT("Found %d positions for spawning walkers at begin play."), BeginSpawnPoints.Num());
  UE_LOG(LogCarla, Log, TEXT("Found %d positions for spawning walkers during game play."), SpawnPoints.Num());
  #endif 
  if (SpawnPoints.Num() < 2) {
    bSpawnWalkers = false;
    #ifdef CARLA_AI_WALKERS_EXTRA_LOG
    UE_LOG(LogCarla, Error, TEXT("We don't have enough spawn points for walkers!"));
    #endif
  } else if (BeginSpawnPoints.Num() < NumberOfWalkers) {
    #ifdef CARLA_AI_WALKERS_EXTRA_LOG
    UE_LOG(LogCarla, Warning, TEXT("Requested %d walkers, but we only have %d spawn points. Some will fail to spawn."), NumberOfWalkers, BeginSpawnPoints.Num());
    #endif
  }

  GetRandomEngine()->Shuffle(BeginSpawnPoints);

  if (bSpawnWalkers && bSpawnWalkersAtBeginPlay) {
    uint32 Count = 0u;
    for (auto i = 0; i < NumberOfWalkers; ++i) {
      if (TryToSpawnWalkerAt(*BeginSpawnPoints[i % BeginSpawnPoints.Num()])) {
        ++Count;
      }
    }
    #ifdef CARLA_AI_WALKERS_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("Spawned %d walkers at begin play."), Count);
    #endif
  }
}

void AWalkerSpawnerBase::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (bSpawnWalkers && (NumberOfWalkers > GetCurrentNumberOfWalkers())) {
    // Try to spawn one walker.
    TryToSpawnWalkerAt(GetRandomSpawnPoint());
  }

  if (WalkersBlackList.Num() > 0) 
  {
	  CurrentBlackWalkerIndexToCheck = ++CurrentBlackWalkerIndexToCheck % WalkersBlackList.Num();
    ACharacter* BlackListedWalker = WalkersBlackList[CurrentBlackWalkerIndexToCheck];
	  AWalkerAIController* controller = BlackListedWalker!=nullptr?Cast<AWalkerAIController>(BlackListedWalker->GetController()):nullptr;
	  if(BlackListedWalker != nullptr && controller!=nullptr && IsValid(BlackListedWalker))
	  {
      const auto Status = GetWalkerStatus(BlackListedWalker);
      #ifdef CARLA_AI_WALKERS_EXTRA_LOG
	    UE_LOG(LogCarla, Log, TEXT("Watching walker %s with state %d"), *UKismetSystemLibrary::GetDisplayName(BlackListedWalker), (int)Status);
      #endif
	    switch(Status)
	    {
  	    case EWalkerStatus::RunOver:{
	  	  //remove from list and wait for auto-destroy
	        WalkersBlackList.RemoveAtSwap(CurrentBlackWalkerIndexToCheck);
	        break;
	      }
		    case EWalkerStatus::MoveCompleted:
		    {
          BlackListedWalker->Destroy();
		      break;	    
		    }
	      default: {
		      switch(controller->GetMoveStatus())
		      {
			      case EPathFollowingStatus::Idle: 
			        if(!TrySetDestination(*BlackListedWalker))
			        {
	  		        if(!SetRandomWalkerDestination(BlackListedWalker))
	  		        {
                  #ifdef CARLA_AI_WALKERS_EXTRA_LOG
		  	  	      UE_LOG(LogCarla,Error,TEXT("Could not set a random destination to walker %s"),*UKismetSystemLibrary::GetDisplayName(BlackListedWalker));
                  #endif
	  		        }
	  		      }
			      break;
			    case EPathFollowingStatus::Waiting: 
			      //incomplete path
		  	  break;
			   case EPathFollowingStatus::Paused: 
			    //waiting for blueprint code 
		  	  break;
			  case EPathFollowingStatus::Moving:			  
			    if(BlackListedWalker->GetVelocity().Size()>1.0f)
			    {
			      WalkersBlackList.RemoveAtSwap(CurrentBlackWalkerIndexToCheck);
		        Walkers.Add(BlackListedWalker);
			    }
		  	  break;
			   default: break;
		    }
	  	  break;
		    }
      }
      #ifdef CARLA_AI_WALKERS_EXTRA_LOG
	    UE_LOG(LogCarla, Log, TEXT("New state for walker %s : %d"), *UKismetSystemLibrary::GetDisplayName(BlackListedWalker), (int)GetWalkerStatus(BlackListedWalker));
      #endif
    } 
  	
  }

  if (Walkers.Num() > 0) 
  {
    // Check one walker, if fails black-list it or kill it.
	  CurrentWalkerIndexToCheck = ++CurrentWalkerIndexToCheck % Walkers.Num();
    auto Walker = Walkers[CurrentWalkerIndexToCheck];
	  if(Walker == nullptr || !IsValid(Walker))
	  {
  	  Walkers.RemoveAtSwap(CurrentWalkerIndexToCheck);
  	} else {
	    const auto Status = GetWalkerStatus(Walker);
  	  switch (Status)
	    {
  		  default: 
		    case EWalkerStatus::Paused:
		    case EWalkerStatus::Unknown:
  		    break;
	      case EWalkerStatus::RunOver: {
          Walkers.RemoveAtSwap(CurrentWalkerIndexToCheck);
	        break;
	      }
	      case EWalkerStatus::MoveCompleted:
    		  Walker->Destroy();
		      break;
  	    case EWalkerStatus::Invalid:
	      case EWalkerStatus::Stuck:
	      {
    		  SetRandomWalkerDestination(Walker);
		      // Black-list it and wait for this walker to move
		      WalkersBlackList.Add(Walker);
		      Walkers.RemoveAtSwap(CurrentWalkerIndexToCheck);
		      break;
        }
	    }
    }
  }
}

bool AWalkerSpawnerBase::SetRandomWalkerDestination(ACharacter *Walker)
{
	const auto &DestinationPoint = GetRandomSpawnPoint();
	auto Controller = GetController(Walker);
	if(!Controller) {
    #ifdef CARLA_AI_WALKERS_EXTRA_LOG
	  UE_LOG(LogCarla, Warning, TEXT("AWalkerSpawnerBase::SetRandomWalkerDestination: Walker %s has no controller"), 
	   *UKismetSystemLibrary::GetDisplayName(Walker)
      );
    #endif
	  return false;
	}
	const EPathFollowingRequestResult::Type request_result = Controller->MoveToLocation(DestinationPoint.GetActorLocation(),-1.0f,false,true,true,true,nullptr,true);
	switch(request_result)
	{
	  case EPathFollowingRequestResult::Type::Failed:
	  {
      #ifdef CARLA_AI_WALKERS_EXTRA_LOG
	    UE_LOG(LogCarla, Warning, TEXT("AWalkerSpawnerBase::SetRandomWalkerDestination: Bad destination point %s"), 
		  *UKismetSystemLibrary::GetDisplayName(&DestinationPoint)
		  );
      #endif
	    return false;
	  }
	  case EPathFollowingRequestResult::Type::AlreadyAtGoal:{
      #ifdef CARLA_AI_WALKERS_EXTRA_LOG
		  UE_LOG(LogCarla, Log, TEXT("AWalkerSpawnerBase::SetRandomWalkerDestination already in destination, generating new location"));
      #endif
	  	return SetRandomWalkerDestination(Walker);
	  }
	  default: case EPathFollowingRequestResult::Type::RequestSuccessful: return true;
	}
}

// =============================================================================
// -- Other member functions ---------------------------------------------------
// =============================================================================

void AWalkerSpawnerBase::SetNumberOfWalkers(const int32 Count)
{
  if (Count > 0) {
    bSpawnWalkers = true;
    NumberOfWalkers = Count;
  } else {
    bSpawnWalkers = false;
  }
}

const AWalkerSpawnPointBase &AWalkerSpawnerBase::GetRandomSpawnPoint()
{
  check(SpawnPoints.Num() > 0);
  const auto *SpawnPoint = GetRandomEngine()->PickOne(SpawnPoints);
  check(SpawnPoint != nullptr);
  return *SpawnPoint;
}

bool AWalkerSpawnerBase::TryGetValidDestination(const FVector &Origin, FVector &Destination)
{
  const auto &DestinationPoint = GetRandomSpawnPoint();
  Destination = DestinationPoint.GetActorLocation();
  return (GetDistance(Origin, Destination) >= MinimumWalkDistance);
}

bool AWalkerSpawnerBase::TryToSpawnWalkerAt(const AWalkerSpawnPointBase &SpawnPoint)
{
  // Try find destination.
  FVector Destination;
  if (!TryGetValidDestination(SpawnPoint.GetActorLocation(), Destination)) {
    return false;
  }

  // Spawn walker.
  ACharacter *Walker;
  SpawnWalker(SpawnPoint.GetActorTransform(), Walker);
  if (!WalkerIsValid(Walker)) {
    return false;
  }

  // Assign controller.
  Walker->AIControllerClass = AWalkerAIController::StaticClass();
  Walker->SpawnDefaultController();
  auto Controller = GetController(Walker);
  if (Controller == nullptr) { // Sometimes fails...
    UE_LOG(LogCarla, Error, TEXT("Something went wrong creating the controller for the new walker"));
    Walker->Destroy();
    return false;
  }

  // Add walker and set destination.
  Walkers.Add(Walker);
  if (Controller->MoveToLocation(Destination,-1.0f,false,true,true,true,nullptr,true)!=EPathFollowingRequestResult::Type::RequestSuccessful)
  {
    SetRandomWalkerDestination(Walker);
  }
  return true;
}

bool AWalkerSpawnerBase::TrySetDestination(ACharacter &Walker)
{
  // Try to retrieve controller.
  auto Controller = GetController(&Walker);
  if (Controller == nullptr) {
	  UE_LOG(LogCarla, Warning, TEXT("Could not get valid controller for walker: %s"), *Walker.GetName());
    return false;
  }

  // Try find destination.
  FVector Destination;
  if (!TryGetValidDestination(Walker.GetActorLocation(), Destination)) {
    #ifdef CARLA_AI_WALKERS_EXTRA_LOG
	  UE_LOG(
      LogCarla, Warning, 
		  TEXT("Could not get a new destiny: %s for walker: %s"), 
		  *Destination.ToString(), *Walker.GetName()
	  );
    #endif
    return false;
  }

  return Controller->MoveToLocation(Destination,-1.0f,false,true,true,true,nullptr,true)==EPathFollowingRequestResult::RequestSuccessful;
}
