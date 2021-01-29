/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    fxexdefs.h
/// @author  Mathew Robertson
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2004-03-19
///
//
/****************************************************************************/

#ifndef FXEXDEFS_H
#define FXEXDEFS_H

#include <config.h>

#include <fx.h>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#endif

// loads the FOX defs
#ifndef FXDEFS_H
#include <fxdefs.h>
/**
 * The FOX namespace is defined with the symbol 'FX'.  Refer to the FOX documentation for
 * more information.
 */
using namespace FX;
#endif

// Disable warnings on extern before template instantiation for the Microsoft compiler.
// see "HOWTO: Exporting STL Components Inside & Outside of a Class" on the microsoft website
#if defined(WIN32) && defined(_MSC_VER)
#pragma warning (disable : 4231)
#endif


// implement CALLBACK for unix
#ifndef CALLBACK
#define CALLBACK
#endif

/**
 * FXEX is the namespace encapsulating the FOXEX library, in a similar vein to the FOX
 * namespace.  This should not affect you too much, since if you dont explicately #define
 * FX_NO_GLOBAL_NAMESPACE, the FXEX namespace automatically imported when you #include
 * <fxex/fxex.h>.
 */
namespace FXEX {

// provide an extern mechanism
#ifdef WIN32
#ifdef FOXDLL
#ifndef FOXDLL_EXPORTS
#define FXEXTERN extern
#endif
#endif
#endif

#ifndef FXEXTERN
#define FXEXTERN
#endif


// Valid compression factors for Bzip and Gzip compression libraries
#define COMPRESS_MAX 9
#define COMPRESS_NORMAL 6
#define COMPRESS_FAST 1
#define COMPRESS_NONE 0


// Win32 defines INFINITE to be -1, we might as well do it too.
#ifndef WIN32
# define INFINITE (-1)
#endif


// determine the newline charater(s)
#ifdef WIN32
#define FXNEWLINE "\r\n"
#endif
#ifdef MAC
#define FXNEWLINE "\r"
#endif
#ifndef FXNEWLINE
#define FXNEWLINE "\n"
#endif


// make a short from two chars
#define MKUSHORT(l,h)  ((((FX::FXuchar)(l))&0xff) | (((FX::FXuchar)(h))<<8))

/* !!!!
// Make a long from two ints - provided longs are twice the size of an int
#ifdef FX_LONG
#  define MKULONG(l,h) ((((FX::FXuint)(l))&0xffffffff) | (((FX::FXuint)(h))<<32))
#else
#  if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64
#    error "You wanted 64bit file sizes (actually 63), but I couldn't make it so..."
#  endif
#endif
!!!! */

/**
 * Quite a while ago (18/08/2000), these macros (or something similar) were
 * posted to the FOX users list, by Dirk Zoller. I cleaned them up a little.
 */
#define FXDeclare(Class) \
    FXDECLARE(Class)
#define FXDeclareAbstract(Class) \
    FXDECLARE_ABSTRACT(Class)
#define FXDefMap(Class) \
    FXDEFMAP(Class) Class##Map[]
#define FXImplementAbstract(Class,Parent) \
    FXIMPLEMENT_ABSTRACT(Class,Parent,Class##Map,ARRAYNUMBER(Class##Map))
#define FXImplement(Class,Parent) \
    FXIMPLEMENT(Class,Parent,Class##Map,ARRAYNUMBER(Class##Map))
#define FXFuncDecl(Func) \
    long on##Func (FXObject*,FXSelector,void*)
#define FXFuncImpl(Class,Func,tgt,sel,ptr) \
    long Class::on##Func (FXOject *tgt,FXSelector sel, void *ptr)
#define FXMapTypes(Class,FromType,ToType,Func) \
    FXMAPTYPES(SEL_##FromType,SEL_##ToType,Class::on##Func)
#define FXMapType(Class,SelType,Func) \
    FXMAPTYPE(SEL_##SelType,Class::on##Func)
#define FXMapFuncs(Class,SelType,FromId,ToId,Func) \
    FXMAPFUNCS(SEL_##SelType,Class::ID_##FromId,Class::ID_##ToId,Class::on#Func)
#define FXMapFunc(Class,SelType,Id,Func) \
    FXMAPFUNC(SEL_##SelType,Class::ID_##Id,Class::on##Func)


/// Zed A Shaw posted these (09/09/02), or a variation of them
#define FXSEND(tgt,sel,msg,ptr) \
    (tgt->handle(this,FXSEL(sel,msg),ptr)
#define FXROUTE(src,tgt,sel,msg,ptr) \
    (tgt->handle(src,FXSEL(sel,msg),ptr)


// debugging macros
#ifndef NDEBUG
#  define FXCTOR() fxtrace (100,"%s::%s %p\n",getClassName(),getClassName(),this)
#  define FXMETHOD(methodname) fxtrace (100,"%s::%s %p\n",getClassName(),#methodname,this)
#  define FXDTOR() fxtrace (100,"%s::~%s %p\n",getClassName(),getClassName(),this)
#else
#  define FXCTOR()
#  define FXMETHOD(methodname)
#  define FXDTOR()
#endif


// New selector types
enum {
    SEL_DATA = FX::SEL_LAST,    // form of data as an event
    SEL_THREAD,                 // thread/runnable events
    SEL_WAITABLE_WAIT,          // waitable event such as a Condition variable, semaphore, etc
    SEL_WAITABLE_ACTIVATE,      // waitable event such as a Condition variable, semaphore, etc
    SEL_INTERLOCK,              // interlocked event; object went into lock
    SEL_BARRIER_LOCK,           // barrier event; thread waiting in barrier lock
    SEL_BARRIER_UNLOCK,         // barrier event; barrier object reset, threads released
    SEL_INPUT,                  // some type of input event
    SEL_OUTPUT,                 // some type of output event
    SEL_ERROR,                  // some type of error event
    SEL_IO,                     // Some form of IO
    SEL_IO_CONNECT,             // Connection event
    SEL_EVENT,                  // a generic event
    SEL_BEGIN,                  // en event defining some begining
    SEL_END,                    // an event defining some ending
    SEL_TAG,                    // tag event
    SEL_CONTENT,                // content event
    SEL_REGISTRY,               // a registry event (TODO I have a plan for this...)
    SEL_LASTEX,// Last message
    SEL_THREAD_EVENT
};


/// IO status definitions
typedef FXint FXIOStatus;
enum {
    FXIOStatusUnknown = -2,
    FXIOStatusError = -1,
    FXIOStatusOk = 0,
    FXIOStatusOK = 0,
    FXIOStatusLast
};
#ifndef INVALID_HANDLE
#  ifdef WIN32
#    define INVALID_HANDLE INVALID_HANDLE_VALUE
#  else
#    define INVALID_HANDLE -1
#  endif
#endif

#ifndef VALID_RESULT
#  define VALID_RESULT 0
#endif


/// IO state definitions
typedef FXint FXIOState;
enum {
    FXIOStateUnknown = -1,
    FXIOStateNone = -1,
    FXIOStateOk = 0,
    FXIOStateOK = 0,
    FXIOStateUnconnected,
    FXIOStateConnected,
    FXIOStateOpen = FXIOStateConnected,
    FXIOStateListener,
    FXIOStateAccepted,
    FXIOStateDuplicated,
    FXIOStateLast
};


/**
 * Socket definitions, for types and families
 */
/// Socket types
enum FXSocketType {
    FXSocketTypeNone = 0,                  // unknown socket type
    FXSocketTypeStream,                    // TCP socket
    FXSocketTypeTCP = FXSocketTypeStream,  // TCP socket
    FXSocketTypeDatagram,                  // UDP socket
    FXSocketTypeUDP = FXSocketTypeDatagram // UDP socket
};

/// Socket families
enum FXSocketFamily {
    FXSocketFamilyNone = 0,  // unknown socket family
    FXSocketFamilyLocal,     // Local domain socket (on nearly every system, same as Unix domain)
    FXSocketFamilyInet,      // Internet domain socket, version 4 (ie the default internet family)
    FXSocketFamilyInet6,     // Internet domain socket, version 6
    FXSocketFamilyAutomatic, // automatically choose UNIX domain (local) socket, when connecting
    // to localhost, internet domain for internet sockets
    FXSocketFamilyUnix = FXSocketFamilyLocal
};


/**
 * File permissions:
 *
 * Use these with fxfilemode to set platform specific mode flags. For example:
 *
 *   FXFileIO fileio(getApp(),"somefile.txt");
 *   ...
 *   fileio->mode(FXUtils::fxfilemode(FILEPERM_DEFAULT_IO));
 *
 * or:
 *
 *   FXFile::mode("file.txt",FXUtils::fxfilemode(FILEPERM_DEFAULT_IO));
 *
 * or:
 *
 *   FXuint mode=FXFile::mode("file.txt");
 *   if (mode & FILEPERM_READ) {...}
 *
 */
enum FXFilePermission {
    FILEPERM_NONE         = 0,            // file has no permissions
    FILEPERM_USER_READ    = 0x00000001,   // user can read from file
    FILEPERM_USER_WRITE   = 0x00000002,   // user can write to file
    FILEPERM_USER_EXEC    = 0X00000004,   // user can execute file
    FILEPERM_GROUP_READ   = 0x00000010,   // group can read from file
    FILEPERM_GROUP_WRITE  = 0x00000020,   // group can write to file
    FILEPERM_GROUP_EXEC   = 0x00000040,   // group can execute the file
    FILEPERM_OTHER_READ   = 0x00000100,   // everybody can read from file
    FILEPERM_OTHER_WRITE  = 0x00000200,   // everybody can write to file
    FILEPERM_OTHER_EXEC   = 0x00000400,   // everybody can execute the file
    FILEPERM_READ         = 0x00000111,   // file read mask; set all read permissions
    FILEPERM_WRITE        = 0x00000222,   // file write mask; set all write permissions
    FILEPERM_EXEC         = 0x00000444,   // file execute mask; set all execute permissions
    FILEPERM_ALL          = 0x00000777,   // permissions mask; set all permissions
    FILEPERM_SET_UID      = 0x00001000,   // set the UID permission
    FILEPERM_SET_GID      = 0x00002000,   // set the GID permisssion
    FILEPERM_STICKY       = 0x00004000,   // set the STICKY permission
    FILEPERM_SECURE_IO    = FILEPERM_USER_READ | FILEPERM_USER_WRITE, // permissions suitable for single user IO access
    FILEPERM_DEFAULT_IO   = FILEPERM_READ | FILEPERM_USER_WRITE | FILEPERM_GROUP_WRITE, // permissions suitable for group IO access
    FILEPERM_DEFAULT_EXEC = FILEPERM_READ | FILEPERM_USER_WRITE | FILEPERM_GROUP_WRITE | FILEPERM_EXEC // permissions suitable for all users to execute a file
};


// thread stuff
#ifndef WIN32
typedef void* FXThreadHandle;       // handle to a thread
typedef void* FXThreadMutex;        // handle to a mutex
typedef void* FXThreadCondition;    // handle to a condition variable
typedef void* FXThreadSemaphore;    // handle to a semaphore
typedef FXInputHandle* FXThreadEventHandle;  // handle to a thread event object
#else
typedef HANDLE FXThreadHandle;       // handle to a thread
typedef HANDLE FXThreadMutex;        // handle to a mutex
typedef HANDLE FXThreadCondition;    // handle to a condition variable
typedef HANDLE FXThreadSemaphore;    // handle to a semaphore
typedef FXInputHandle FXThreadEventHandle;  // handle to a thread event object
#endif


// dynamic library loading
#ifndef WIN32
typedef void*      FXDLLHandle;  // handle to a dynamically loaded file
#else
typedef HMODULE      FXDLLHandle;  // handle to a dynamically loaded file
#endif


// database interface handle
//typedef void*                   FXDatabaseHandle;   // handle to a database connection


namespace FXUtils {

////////////// global variables ////////////////////

/// Version number that the library version is compiled with
extern FXAPI const FXuchar fxexversion[3];


////////////// global functions ////////////////////

/// time conversion routines
#ifdef WIN32
static void get_time_now(unsigned long* abs_sec, unsigned long* abs_nsec);
static DWORD get_timeout(unsigned long secs, unsigned long nsecs, DWORD default_to);
#else
void convert_timeval(struct timeval* tv, FXuint ms);
void convert_timespec(struct timespec* ts, FXuint ms);
#endif

} // namespace FXUtils
} // namespace FXEX

#endif // FXEXDEFS_H

