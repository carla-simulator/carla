set(microsim_logging_STAT_SRCS
    CastingFunctionBinding.h
    FuncBinding_IntParam.h
    FuncBinding_StringParam.h
    FunctionBinding.h
)

if(MSVC)
    add_custom_target(z_microsim_logging SOURCES ${microsim_logging_STAT_SRCS})
endif(MSVC)
