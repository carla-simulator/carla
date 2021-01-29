set(utils_gui_events_STAT_SRCS
   GUIEvent.h
   GUIEvent_Message.h
   GUIEvent_SimulationStep.h
)

if(MSVC)
    add_custom_target(z_utils_gui_events SOURCES ${utils_gui_events_STAT_SRCS})
endif(MSVC)
