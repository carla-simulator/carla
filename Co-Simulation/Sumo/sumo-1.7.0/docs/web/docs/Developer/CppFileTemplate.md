---
title: Developer/CppFileTemplate
permalink: /Developer/CppFileTemplate/
---

```cpp
/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see <https://eclipse.org/sumo>
// Copyright (C) <YEAR OF CREATION>-<CURRENT YEAR> German Aerospace Center (DLR) and others.
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
/// @file    <FILENAME>
/// @author  <AUTHOR'S NAME, ONE SEPARATE LINE FOR EACH AUTHOR>
/// @author  <AUTHOR'S NAME 2>
/// @author  <AUTHOR'S NAME 3>
/// @date    <FILE CREATION DATE>
///
// <A BRIEF DESCRIPTION OF THE FILE'S CONTENTS>
/****************************************************************************/
#include <config.h>

#include <StandardLibrary>     // Standard libraries
#include <SUMOLibrary>         // SUMO libraries

#include "myHeaderFile1"       // Header files from the same directory
#include "myHeaderFile2"


// ===========================================================================
// static member definitions
// ===========================================================================
<parameterType> OwnClass::myStaticMember = value;    // Define here values of static members


// ===========================================================================
// member method definitions
// ===========================================================================
OwnClass::OwnClass(<parameterType> parameter1, <parameterType> parameter2,... <parameterType> parametern) :
    parentClass(),             // Call to parent class (if inherit)
    myParameter1(parameter1),  // values of members should be initialized with this method
    myParameter2(parameter1),  // keep the order coherent with the order of the members in the header file
    ...
    myParametern(parametern) {
    // Code of constructor
}


void
OwnClass::function() const {
    // code
}


<parameterType>
OwnClass::getParameter1() const {
    return myParameter1;
}
```

```cpp
// ---------------------------------------------------------------------------
// Class::InnerClass - methods <LEAVE OUT IF METHODS ARE OF ONE CLASS ONLY>
// ---------------------------------------------------------------------------
OwnClass::InnerClass::InnerClass() :
    myPrivateParameter(0) {
    // code of constructor
}


OwnClass::InnerClass::~InnerClass() {}


/****************************************************************************/
```