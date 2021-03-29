#if WITH_EDITOR

#include "FileHelper.h"
#include "Paths.h"

#define LARGEMAP_LOGS 0

#if LARGEMAP_LOGS
#define LM_LOG(Level, Msg, ...) UE_LOG(LogCarla, Level, TEXT(Msg), __VA_ARGS__)
#else
#define LM_LOG(...)
#endif // LARGEMAP_LOGS

#endif // WITH_EDITOR