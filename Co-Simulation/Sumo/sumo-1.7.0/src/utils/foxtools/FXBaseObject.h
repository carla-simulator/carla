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
/// @file    FXBaseObject.h
/// @author  Mathew Robertson
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2004-03-19
///
//
/****************************************************************************/

#ifndef FXBASEOBJECT_H
#define FXBASEOBJECT_H
#include <config.h>

#include <FXHash.h>
using namespace FX;
#ifndef FXOBJECT_H
#include <FXObject.h>
using namespace FX;
#endif
namespace FXEX {

/**
 * Define so that all types and all id's can go to a specific function.
 * This is particularily useful if you just want to forward the event to a target
 * but not specifically use the onDefault method.
 *
 * Note: if FXApp* is not specified, then FXApp::Instance() is used.
 */
#define FXMAPALL(func) {MKUINT(MINKEY,MINTYPE),MKUINT(MAXKEY,MAXTYPE),&func}

/**
 * Define so that we can use it to 'redirect' all unhandled events, of a specific key/ID
 */
#define FXMAPKEY(key,func)  {MKUINT(key,MINTYPE),MKUINT(key,MAXTYPE),&func}


/**
 * This is a base class for ojects which can send messages to the application
 */
class /* FXAPI // patch by Daniel Krajzewicz 24.02.2004 */
    FXBaseObject : public FXObject {
    FXDECLARE(FXBaseObject)

protected:
    /// flags defined are the same as those defined in FXWindow, etc.
    enum {
        FLAG_ENABLED  = 0x00000002,   // enabled
        FLAG_UPDATE   = 0x00000004,   // needs update
        FLAG_FOCUSED  = 0x00000010,   // has focus
        FLAG_DIRTY    = 0x00000020,   // dirty
        FLAG_RECALC   = 0x00000040,   // needs recalculation
        FLAG_DEFAULT  = 0x00000200,   // set to default
        FLAG_INITIAL  = 0x00000400,   // set to initial value
        FLAG_ACTIVE   = 0x00001000,   // active
        FLAG_CHANGED  = 0x00010000,   // changed
        FLAG_READONLY = 0x10000000    // read only
    };

private:
    FXApp*        app;             // application pointer

protected:
    FXObject*     target;          // application target
    FXSelector    message;         // application message
    void*         data;            // user data
    FXuint        datalen;         // length of user data
    FXuint        flags;           // state flags
    FXuint        options;         // option flags

public:
    enum {
        ID_NONE = 0,
        ID_DELETE = 6,
        ID_DISABLE,
        ID_ENABLE,
        ID_SETVALUE = 17,
        ID_SETINTVALUE,
        ID_SETREALVALUE,
        ID_SETSTRINGVALUE,
        ID_SETINTRANGE,
        ID_SETREALRANGE,
        ID_GETINTVALUE,
        ID_GETREALVALUE,
        ID_GETSTRINGVALUE,
        ID_XML,
        ID_META,
        ID_COMMENT,
        ID_DOCUMENT,
        ID_TAG,
        ID_CONTENT,
        ID_LAST
    };

public:
    long onCmdEnable(FXObject*, FXSelector, void*);
    long onCmdDisable(FXObject*, FXSelector, void*);
    long onUpdate(FXObject*, FXSelector, void*);

public:
    /// Just supply the target and selector (de-serialisation too)
    FXBaseObject(FXObject* tgt = NULL, FXSelector sel = 0);

    /// Alternnatively, supply the app object as well
    FXBaseObject(FXApp* a, FXObject* tgt = NULL, FXSelector sel = 0);

    /// application pointer
    FXApp* getApp();

    /// get the target
    FXObject* getTarget() {
        return target;
    }

    /// set the target
    void setTarget(FXObject* tgt) {
        target = tgt;
    }

    /// get the message
    FXSelector getSelector() {
        return message;
    }

    /// set the selector
    void setSelector(FXSelector sel) {
        message = sel;
    }

    /// get user data
    void* getUserData() {
        return data;
    }

    /// set user data
    void setUserData(void* d) {
        data = d;
    }

    /// get user daat length
    FXuint getUserDataLen() {
        return datalen;
    }

    /// set the user data length
    void setUserDataLen(FXuint len) {
        datalen = len;
    }

    /// are we enabled?
    FXbool isEnabled() {
        return (flags & FLAG_ENABLED) != 0;
    }

    /// enable us
    virtual void enable() {
        flags |= FLAG_ENABLED;
    }

    /// disable us
    virtual void disable() {
        flags &= ~FLAG_ENABLED;
    }

    /// are we modifiable
    virtual FXbool isReadonly() {
        return (flags & FLAG_READONLY) != 0;
    }

    /// set modifiable mode
    virtual void setReadonly(FXbool mode = TRUE);

    /// create resource
    virtual void create() {}

    /// detach resource
    virtual void detach() {}

    /// destroy resource
    virtual void destroy() {}

    /// save object to stream
    virtual void save(FXStream& store) const;

    /// load object from stream
    virtual void load(FXStream& store);

    /// dtor
    virtual ~FXBaseObject();
};

} // namespace FXEX
#endif // FXBASEOBJECT_H

