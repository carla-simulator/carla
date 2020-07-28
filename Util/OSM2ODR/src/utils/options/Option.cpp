/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    Option.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 17 Dec 2001
///
// A class representing a single program option
/****************************************************************************/
#include <config.h>

#include <string>
#include <exception>
#include <sstream>
#include "Option.h"
#include <utils/common/StringUtils.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * Option - methods
 * ----------------------------------------------------------------------- */
Option::Option(bool set)
    : myAmSet(set), myHaveTheDefaultValue(true), myAmWritable(true) {}


Option::Option(const Option& s)
    : myAmSet(s.myAmSet), myHaveTheDefaultValue(s.myHaveTheDefaultValue),
      myAmWritable(s.myAmWritable) {}


Option::~Option() {}


Option&
Option::operator=(const Option& s) {
    if (this == &s) {
        return *this;
    }
    myAmSet = s.myAmSet;
    myHaveTheDefaultValue = s.myHaveTheDefaultValue;
    myAmWritable = s.myAmWritable;
    return *this;
}


bool
Option::isSet() const {
    return myAmSet;
}


double
Option::getFloat() const {
    throw InvalidArgument("This is not a double-option");
}


int
Option::getInt() const {
    throw InvalidArgument("This is not an int-option");
}


std::string
Option::getString() const {
    throw InvalidArgument("This is not a string-option");
}


bool
Option::getBool() const {
    throw InvalidArgument("This is not a bool-option");
}


const IntVector&
Option::getIntVector() const {
    throw InvalidArgument("This is not an int vector-option");
}

const StringVector&
Option::getStringVector() const {
    throw InvalidArgument("This is not a string vector-option");
}

bool
Option::markSet() {
    bool ret = myAmWritable;
    myHaveTheDefaultValue = false;
    myAmSet = true;
    myAmWritable = false;
    return ret;
}


void
Option::unSet() {
    myAmSet = false;
    myAmWritable = true;
}


bool
Option::isBool() const {
    return false;
}


bool
Option::isDefault() const {
    return myHaveTheDefaultValue;
}


bool
Option::isFileName() const {
    return false;
}


bool
Option::isWriteable() const {
    return myAmWritable;
}


void
Option::resetWritable() {
    myAmWritable = true;
}


void
Option::resetDefault() {
    myHaveTheDefaultValue = true;
}


const std::string&
Option::getDescription() const {
    return myDescription;
}


void
Option::setDescription(const std::string& desc) {
    myDescription = desc;
}


const std::string&
Option::getTypeName() const {
    return myTypeName;
}




/* -------------------------------------------------------------------------
 * Option_Integer - methods
 * ----------------------------------------------------------------------- */
Option_Integer::Option_Integer(int value)
    : Option(true), myValue(value) {
    myTypeName = "INT";
}


Option_Integer::~Option_Integer() {}


Option_Integer::Option_Integer(const Option_Integer& s)
    : Option(s) {
    myValue = s.myValue;
}


Option_Integer&
Option_Integer::operator=(const Option_Integer& s) {
    if (this == &s) {
        return *this;
    }
    Option::operator=(s);
    myValue = s.myValue;
    return *this;
}


int
Option_Integer::getInt() const {
    return myValue;
}


bool
Option_Integer::set(const std::string& v) {
    try {
        myValue = StringUtils::toInt(v);
        return markSet();
    } catch (...) {
        std::string s = "'" + v + "' is not a valid integer.";
        throw ProcessError(s);
    }
}


std::string
Option_Integer::getValueString() const {
    std::ostringstream s;
    s << myValue;
    return s.str();
}



/* -------------------------------------------------------------------------
 * Option_String - methods
 * ----------------------------------------------------------------------- */
Option_String::Option_String()
    : Option() {
    myTypeName = "STR";
}


Option_String::Option_String(const std::string& value, std::string typeName)
    : Option(true), myValue(value) {
    myTypeName = typeName;
}


Option_String::~Option_String() {}


Option_String::Option_String(const Option_String& s)
    : Option(s) {
    myValue = s.myValue;
}


Option_String&
Option_String::operator=(const Option_String& s) {
    if (this == &s) {
        return *this;
    }
    Option::operator=(s);
    myValue = s.myValue;
    return *this;
}


std::string
Option_String::getString() const {
    return myValue;
}


bool
Option_String::set(const std::string& v) {
    myValue = v;
    return markSet();
}


std::string
Option_String::getValueString() const {
    return myValue;
}



/* -------------------------------------------------------------------------
 * Option_Float - methods
 * ----------------------------------------------------------------------- */
Option_Float::Option_Float(double value)
    : Option(true), myValue(value) {
    myTypeName = "FLOAT";
}


Option_Float::~Option_Float() {}


Option_Float::Option_Float(const Option_Float& s)
    : Option(s) {
    myValue = s.myValue;
}


Option_Float&
Option_Float::operator=(const Option_Float& s) {
    if (this == &s) {
        return *this;
    }
    Option::operator=(s);
    myValue = s.myValue;
    return *this;
}


double
Option_Float::getFloat() const {
    return myValue;
}


bool
Option_Float::set(const std::string& v) {
    try {
        myValue = StringUtils::toDouble(v);
        return markSet();
    } catch (...) {
        throw ProcessError("'" + v + "' is not a valid float.");
    }
}


std::string
Option_Float::getValueString() const {
    std::ostringstream s;
    s << myValue;
    return s.str();
}



/* -------------------------------------------------------------------------
 * Option_Bool - methods
 * ----------------------------------------------------------------------- */
Option_Bool::Option_Bool(bool value)
    : Option(true), myValue(value) {
    myTypeName = "BOOL";
}


Option_Bool::~Option_Bool() {}


Option_Bool::Option_Bool(const Option_Bool& s)
    : Option(s) {
    myValue = s.myValue;
}


Option_Bool&
Option_Bool::operator=(const Option_Bool& s) {
    if (this == &s) {
        return *this;
    }
    Option::operator=(s);
    myValue = s.myValue;
    return *this;
}


bool
Option_Bool::getBool() const {
    return myValue;
}


bool
Option_Bool::set(const std::string& v) {
    try {
        myValue = StringUtils::toBool(v);
        return markSet();
    } catch (...) {
        throw ProcessError("'" + v + "' is not a valid bool.");
    }
}


std::string
Option_Bool::getValueString() const {
    if (myValue) {
        return "true";
    }
    return "false";
}


bool
Option_Bool::isBool() const {
    return true;
}



/* -------------------------------------------------------------------------
 * Option_BoolExtended - methods
 * ----------------------------------------------------------------------- */
Option_BoolExtended::Option_BoolExtended(bool value)
    : Option_Bool(value), myValueString(value ? "true" : "false") {
}


Option_BoolExtended::~Option_BoolExtended() {}


Option_BoolExtended::Option_BoolExtended(const Option_BoolExtended& s)
    : Option_Bool(s.myValue) {
    myValueString = s.myValueString;
}


Option_BoolExtended&
Option_BoolExtended::operator=(const Option_BoolExtended& s) {
    if (this == &s) {
        return *this;
    }
    Option::operator=(s);
    myValue = s.myValue;
    myValueString = s.myValueString;
    return *this;
}


bool
Option_BoolExtended::set(const std::string& v) {
    try {
        myValue = StringUtils::toBool(v);
        myValueString = "";
    } catch (...) {
        myValue = true;
        myValueString = v;
    }
    return markSet();
}


std::string
Option_BoolExtended::getValueString() const {
    return myValueString;
}


/* -------------------------------------------------------------------------
 * Option_UIntVector - methods
 * ----------------------------------------------------------------------- */
Option_IntVector::Option_IntVector()
    : Option() {
    myTypeName = "INT[]";
}


Option_IntVector::Option_IntVector(const IntVector& value)
    : Option(true), myValue(value) {
    myTypeName = "INT[]";
}


Option_IntVector::Option_IntVector(const Option_IntVector& s)
    : Option(s), myValue(s.myValue) {}


Option_IntVector::~Option_IntVector() {}


Option_IntVector&
Option_IntVector::operator=(const Option_IntVector& s) {
    Option::operator=(s);
    myValue = s.myValue;
    return (*this);
}


const IntVector&
Option_IntVector::getIntVector() const {
    return myValue;
}


bool
Option_IntVector::set(const std::string& v) {
    myValue.clear();
    try {
        if (v.find(';') != std::string::npos) {
            WRITE_WARNING("Please note that using ';' as list separator is deprecated and not accepted anymore.");
        }
        StringTokenizer st(v, ",", true);
        while (st.hasNext()) {
            myValue.push_back(StringUtils::toInt(st.next()));
        }
        return markSet();
    } catch (EmptyData&) {
        throw ProcessError("Empty element occurred in " + v);
    } catch (...) {
        throw ProcessError("'" + v + "' is not a valid integer vector.");
    }
}


std::string
Option_IntVector::getValueString() const {
    return joinToString(myValue, ',');
}


/* -------------------------------------------------------------------------
 * Option_StringVector - methods
 * ----------------------------------------------------------------------- */
Option_StringVector::Option_StringVector() : Option() {
    myTypeName = "STR[]";
}

Option_StringVector::Option_StringVector(const StringVector& value)
    : Option(true), myValue(value) {
    myTypeName = "STR[]";
}

Option_StringVector::Option_StringVector(const Option_StringVector& s)
    : Option(s), myValue(s.myValue) {}

Option_StringVector::~Option_StringVector() {}

Option_StringVector&
Option_StringVector::operator=(const Option_StringVector& s) {
    Option::operator=(s);
    myValue = s.myValue;
    return (*this);
}

const StringVector&
Option_StringVector::getStringVector() const {
    return myValue;
}

bool
Option_StringVector::set(const std::string& v) {
    myValue.clear();
    try {
        if (v.find(';') != std::string::npos) {
            WRITE_WARNING("Please note that using ';' as list separator is deprecated and not accepted anymore.");
        }
        StringTokenizer st(v, ",", true);
        while (st.hasNext()) {
            myValue.push_back(StringUtils::prune(st.next()));
        }
        return markSet();
    } catch (EmptyData&) {
        throw ProcessError("Empty element occurred in " + v);
    } catch (...) {
        throw ProcessError("'" + v + "' is not a valid string vector.");
    }
}

std::string
Option_StringVector::getValueString() const {
    return joinToString(myValue, ',');
}


/* -------------------------------------------------------------------------
 * Option_FileName - methods
 * ----------------------------------------------------------------------- */
Option_FileName::Option_FileName() : Option_StringVector() {
    myTypeName = "FILE";
}

Option_FileName::Option_FileName(const StringVector& value)
    : Option_StringVector(value) {
    myTypeName = "FILE";
}

Option_FileName::Option_FileName(const Option_FileName& s)
    : Option_StringVector(s) {}

Option_FileName::~Option_FileName() {}

Option_FileName& Option_FileName::operator=(const Option_FileName& s) {
    Option_StringVector::operator=(s);
    return (*this);
}

bool Option_FileName::isFileName() const {
    return true;
}

std::string
Option_FileName::getString() const {
    return Option_StringVector::getValueString();
}

std::string Option_FileName::getValueString() const {
    return StringUtils::urlEncode(Option_StringVector::getValueString(), " ;%");
}


/****************************************************************************/
