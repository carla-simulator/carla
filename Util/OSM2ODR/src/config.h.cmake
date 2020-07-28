/* Macros for Microsoft Visual Studio */
#ifdef _MSC_VER

    /* Disable "decorated name length exceeded, name was truncated" warnings. */
    #pragma warning(disable: 4503)
    /* Disable "identifier truncated in debug info" warnings. */
    #pragma warning(disable: 4786)
    /* Disable "C++ Exception Specification ignored" warnings */
    #pragma warning(disable: 4290)
    /* Disable DLL-Interface warnings */
    #pragma warning(disable: 4251)
    /* Disable integer overflow in arithmetics warnings */
    #pragma warning(disable: 26451)

    /* Disable "unsafe" warnings for crt functions in VC++ 2005. */
    #if _MSC_VER >= 1400
        #define _CRT_SECURE_NO_WARNINGS
    #endif

    /* define WIN32 */
    #ifndef WIN32
        #define WIN32
    #endif

    /* Define for dynamic Fox linkage */
    #define FOXDLL 1

    /* define default constructor for FOX moduls (Visual Studio) */
    #define FOX_CONSTRUCTOR(classname) __pragma(warning(suppress: 26495)) \
    classname() {}

/* Macros for GNU Compiler */
#else

    /* defined if we're using MINGW32 */
    #cmakedefine MINGW32

    /* Windows (MinGW32) */
    #ifdef MINGW32

        /* Define WIN32 */
        #ifndef WIN32
            #define WIN32
        #endif

        /* Define for dynamic Fox linkage */
        #define FOXDLL 1

        /* Define default constructor for FOX moduls (MinGW32) */
        #define FOX_CONSTRUCTOR(classname) classname() {}

    /* Linux and OS */
    #else

        /* Define default constructor for FOX moduls (Linux and OS) */
        #define FOX_CONSTRUCTOR(classname) classname() {}

    /* MinGW32 */
    #endif

/* Visual Studio */
#endif


/* Reporting string for enabled options */
#define HAVE_ENABLED "@ENABLED_FEATURES@"

/* defined if Eigen is available */
#cmakedefine HAVE_EIGEN

/* defined if ffmpeg is available */
#cmakedefine HAVE_FFMPEG

/* defined if FOX is available */
#cmakedefine HAVE_FOX

/* defined if GDAL is available */
#cmakedefine HAVE_GDAL

/* defined if GL2PS is available */
#cmakedefine HAVE_GL2PS

/* defined if osg is available */
#cmakedefine HAVE_OSG

/* defined if zlib is available */
#cmakedefine HAVE_ZLIB

/* set to proj.h, proj_api.h or empty depending on which proj is available */
#cmakedefine PROJ_API_FILE "@PROJ_API_FILE@"

/* defined if python is available */
#cmakedefine HAVE_PYTHON

/* Define if auto-generated version.h should be used. */
#define HAVE_VERSION_H
#ifndef HAVE_VERSION_H
    /* Define if auto-generated version.h is unavailable. */
    #define VERSION_STRING "1.6.0"
#endif

/* defines the epsilon to use on general floating point comparison */
#define NUMERICAL_EPS 0.001

/* defines the epsilon to use on position comparison */
#define POSITION_EPS 0.1

/* Define length for Xerces 3. */
#define XERCES3_SIZE_t XMLSize_t
