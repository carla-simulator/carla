#include "CarlaTemplateExternalInterface.h"

#define LOCTEXT_NAMESPACE "FCarlaTemplateExternalInterface"

DEFINE_LOG_CATEGORY(LogCarlaTemplateExternalInterface)

void FCarlaTemplateExternalInterface::StartupModule()
{
}

void FCarlaTemplateExternalInterface::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCarlaTemplateExternalInterface, CarlaTemplateExternalInterface)

