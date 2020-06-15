set(utils_options_STAT_SRCS
   Option.cpp
   Option.h
   OptionsCont.cpp
   OptionsCont.h
   OptionsIO.cpp
   OptionsIO.h
   OptionsLoader.cpp
   OptionsLoader.h
   OptionsParser.cpp
   OptionsParser.h
)

add_library(utils_options STATIC ${utils_options_STAT_SRCS})
set_property(TARGET utils_options PROPERTY PROJECT_LABEL "z_utils_options")
