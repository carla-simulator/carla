set(netedit_dialogs_SRCS
   GNEVariableSpeedSignDialog.h
   GNEVariableSpeedSignDialog.cpp
   GNEAbout.h
   GNEAbout.cpp
   GNEAllowDisallow.h
   GNEAllowDisallow.cpp
   GNEParametersDialog.h
   GNEParametersDialog.cpp
   GNEFixAdditionalElements.h
   GNEFixAdditionalElements.cpp
   GNEFixDemandElements.h
   GNEFixDemandElements.cpp
   GNEAdditionalDialog.h
   GNEAdditionalDialog.cpp
   GNEDemandElementDialog.h
   GNEDemandElementDialog.cpp
   GNERerouterDialog.h
   GNERerouterDialog.cpp
   GNERerouterIntervalDialog.h
   GNERerouterIntervalDialog.cpp
   GNECalibratorDialog.h
   GNECalibratorDialog.cpp
   GNECalibratorFlowDialog.h
   GNECalibratorFlowDialog.cpp
   GNERouteDialog.h
   GNERouteDialog.cpp
   GNEVehicleTypeDialog.h
   GNEVehicleTypeDialog.cpp
   GNEDialogACChooser.h
   GNEDialogACChooser.cpp
   )

add_library(netedit_dialogs STATIC ${netedit_dialogs_SRCS})
set_property(TARGET netedit_dialogs PROPERTY PROJECT_LABEL "z_netedit_dialogs")
add_dependencies(netedit_dialogs generate-version-h)
