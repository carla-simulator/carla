---
title: Developer/HFileTemplate
permalink: /Developer/HFileTemplate/
---

```
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
#pragma once
#include <config.h> // you can leave this one out if you don't need any defines from config.h, especially in libsumo / utils

#include <somewhere/otherClass1.h>
#include "myOtherClass2.h"


// ===========================================================================
// class declarations
// ===========================================================================
class myOwnClass1; // always prefer a declaration over an include
class myOwnClass2;


// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class OwnClass (Can be preceded by GNE, MS, NB, etc. depending on the project)
 * brief description of the class
 */
class OwnClass : public ParentClass  {

public:
     /**
      * @class InnerClass
      * brief description of the inner class
      */
     class InnerClass {

     public:
         /// @brief Constructor
         InnerClass();

         /// @brief Destructor
         ~InnerClass();

     private:
         /// @brief description of parameter
         <parameterType> myPrivateParameter;
     };

    /** @brief Constructor
     * @param[in] parameter1 each parameter must be documented
     * @param[in] parameter2 each parameter must be documented
       ...
     */
    OwnClass(<parameterType> parameter1, <parameterType> parameter2,... <parameterType> parametern);

    /// @brief Destructor (don't need documentation)
    ~OwnClass();

    /// @brief Returns parameter1 (Simple functions should be documented with one line)
    <parameterType> getParameter1() const;

    /** @brief some functions can throw exceptions but don't use throw declarations
     * @param[in] parameter1 description of the parameter
     * @throw <exceptionName> description of the situation that provoque an exception
     * @return description of the returned value
     */
    bool setParameter1(<type> parameter1) const;

    /// @brief if the class is abstract, should have pure virtual functions (virtual .... = 0;)
    //  @note: Extra information about a function should be written using @note
    virtual void pureVirtualFunction() = 0;

protected:
    /// @brief description of the parameter
    <parameterType> myProtectedParameter1;

private:
    /// @brief every private parameter must start with "my"
    <parameterType> myPrivateParameter2;

    /// @brief C++ Standard Library classes must be preceded by std::
    std::vector<std::string> myPrivateParameter3;
};
```