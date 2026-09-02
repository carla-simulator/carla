// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Navigation/CarlaNavModifierBox.h"

#include <util/ue-header-guard-begin.h>
#include "Components/BoxComponent.h"
#include "NavAreas/NavArea.h"
#include "NavModifierComponent.h"
#include <util/ue-header-guard-end.h>

ACarlaNavModifierBox::ACarlaNavModifierBox(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  SetCanBeDamaged(false);

  Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
  SetRootComponent(Box);
  // UNavModifierComponent only reads bounds from components that have
  // collision enabled and can affect navigation, but a component is only
  // exported as navmesh GEOMETRY when it blocks Pawn or Vehicle
  // (UPrimitiveComponent::IsNavigationRelevant) -- so query-only collision
  // that ignores every channel gives us modifier bounds without rasterizing
  // the box itself.
  Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  Box->SetCollisionResponseToAllChannels(ECR_Ignore);
  Box->SetGenerateOverlapEvents(false);
  Box->SetCanEverAffectNavigation(true);
  Box->SetMobility(EComponentMobility::Static);

  NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
}

void ACarlaNavModifierBox::Configure(TSubclassOf<UNavArea> InAreaClass, const FVector &BoxExtent)
{
  Box->SetBoxExtent(BoxExtent);
  NavModifier->SetAreaClass(InAreaClass);
}
