set(utils_foxtools_STAT_SRCS
   FXBaseObject.cpp
   FXBaseObject.h
   FXConditionalLock.h
   fxexdefs.h
   MFXImageHelper.cpp
   MFXImageHelper.h
   FXLCDLabel.cpp
   FXLCDLabel.h
   FXLinkLabel.cpp
   FXLinkLabel.h
   FXSevenSegment.cpp
   FXSevenSegment.h
   FXSingleEventThread.cpp
   FXSingleEventThread.h
   FXSynchQue.h
   FXSynchSet.h
   FXThreadEvent.cpp
   FXThreadEvent.h
   FXWorkerThread.h
   MsgHandlerSynchronized.h
   MFXCheckableButton.cpp
   MFXCheckableButton.h
   MFXInterThreadEventClient.h
   MFXAddEditTypedTable.cpp
   MFXAddEditTypedTable.h
   MFXEditableTable.cpp
   MFXEditableTable.h
   MFXMenuHeader.cpp
   MFXMenuHeader.h
   MFXIconComboBox.cpp
   MFXIconComboBox.h
   MFXUtils.cpp
   MFXUtils.h
)

add_library(utils_foxtools STATIC ${utils_foxtools_STAT_SRCS})
set_property(TARGET utils_foxtools PROPERTY PROJECT_LABEL "z_utils_foxtools")
