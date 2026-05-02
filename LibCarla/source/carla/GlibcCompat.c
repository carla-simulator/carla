// Compatibility shim for glibc 2.38+ on systems where UE5's linker
// uses an older sysroot that lacks __isoc23_strtol / __isoc23_strtoll.
//
// When compiled with gnu++20, clang implicitly defines _GNU_SOURCE,
// which causes glibc 2.38+ headers to redirect strtol() calls to
// __isoc23_strtol(). UE5's sysroot (Rocky Linux 8, glibc 2.17) does
// not provide this symbol, resulting in an undefined symbol at link time.
//
// This file is compiled with -std=c11 (no _GNU_SOURCE, no redirect)
// so that strtol/strtoll resolve to the standard glibc symbols.

#include <stdlib.h>

long int __isoc23_strtol(const char *nptr, char **endptr, int base) {
    return strtol(nptr, endptr, base);
}

long long int __isoc23_strtoll(const char *nptr, char **endptr, int base) {
    return strtoll(nptr, endptr, base);
}

unsigned long int __isoc23_strtoul(const char *nptr, char **endptr, int base) {
    return strtoul(nptr, endptr, base);
}

unsigned long long int __isoc23_strtoull(const char *nptr, char **endptr, int base) {
    return strtoull(nptr, endptr, base);
}
