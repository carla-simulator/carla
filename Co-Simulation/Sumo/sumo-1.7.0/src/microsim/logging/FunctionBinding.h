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
/// @file    FunctionBinding.h
/// @author  Daniel Krajzewicz
/// @author  Christian Roessel
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Fri, 29.04.2005
///
//  Function type template
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/ValueSource.h>
#include "CastingFunctionBinding.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class FunctionBinding
 */
template< class T, typename R  >
class FunctionBinding : public ValueSource<R> {
public:
    /// Type of the function to execute.
    typedef R(T::* Operation)() const;

    FunctionBinding(T* const source, Operation operation, const R scale = 1) :
        mySource(source),
        myOperation(operation),
        myScale(scale) {}

    /// Destructor.
    ~FunctionBinding() {}

    R getValue() const {
        return myScale * (mySource->*myOperation)();
    }

    ValueSource<R>* copy() const {
        return new FunctionBinding<T, R>(mySource, myOperation, myScale);
    }

    ValueSource<double>* makedoubleReturningCopy() const {
        return new CastingFunctionBinding<T, double, R>(mySource, myOperation, myScale);
    }

private:
    /// The object the action is directed to.
    T* mySource;

    /// The object's operation to perform.
    Operation myOperation;

    /// The scale to apply.
    const R myScale;

private:
    /// @brief invalidated assignment operator
    FunctionBinding<T, R>& operator=(const FunctionBinding<T, R>&);

};

template<class T>
class FunctionBindingString : public ValueSource<std::string> {
public:
    /// Type of the function to execute.
    typedef std::string(T::* Operation)() const;

    FunctionBindingString(T* const source, Operation operation) :
        mySource(source),
        myOperation(operation)
    {}

    /// Destructor.
    ~FunctionBindingString() {}

    std::string getValue() const {
        return (mySource->*myOperation)();
    }

    ValueSource<std::string>* copy() const {
        return new FunctionBindingString<T>(mySource, myOperation);
    }

    ValueSource<double>* makedoubleReturningCopy() const {
        return nullptr;
    }

private:
    /// The object the action is directed to.
    T* mySource;

    /// The object's operation to perform.
    Operation myOperation;

private:
    /// @brief invalidated assignment operator
    FunctionBindingString<T>& operator=(const FunctionBindingString<T>&);

};
