#pragma once

#include <util/ue-header-guard-begin.h>
#include "Kismet/BlueprintFunctionLibrary.h"
#include <util/ue-header-guard-end.h>

#include "CarlaTemplateExternalInterfaceBlueprintLibrary.generated.h"

class ACarlaTemplateExternalInterfaceActor;

UCLASS()
class CARLATEMPLATEEXTERNALINTERFACE_API UCarlaTemplateExternalInterfaceBlueprintLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

  public:
  UFUNCTION(BlueprintCallable)
  static FActorDefinition MakeCarlaTemplateExternalInterfaceDefinition(TSubclassOf<ACarlaTemplateExternalInterfaceActor> Class);

  static void MakeCarlaTemplateExternalInterfaceDefinition(bool &Success, FActorDefinition &Definition, TSubclassOf<ACarlaTemplateExternalInterfaceActor> Class);

  UFUNCTION(BlueprintCallable)
  static void SetCarlaTemplateExternalInterface(const FActorDescription &Description, ACarlaTemplateExternalInterfaceActor *CarlaTemplateExternalInterface);
};