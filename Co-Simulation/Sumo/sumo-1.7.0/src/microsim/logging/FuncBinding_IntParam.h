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
/// @file    FuncBinding_IntParam.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Fri, 29.04.2005
///
// �Function type template
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/ValueSource.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class FuncBinding_IntParam
 */
template< class T, typename R  >
class FuncBinding_IntParam : public ValueSource<R> {
public:
    /// Type of the function to execute.
    typedef R(T::* Operation)(int) const;

    FuncBinding_IntParam(T* source, Operation operation,
                         int param)
        :
        mySource(source),
        myOperation(operation),
        myParam(param) {}

    /// Destructor.
    ~FuncBinding_IntParam() {}

    double getValue() const {
        return (mySource->*myOperation)(myParam);
    }

    ValueSource<R>* copy() const {
        return new FuncBinding_IntParam<T, R>(
                   mySource, myOperation, myParam);
    }

    ValueSource<double>* makedoubleReturningCopy() const {
        return new FuncBinding_IntParam<T, double>(mySource, myOperation, myParam);
    }

protected:

private:
    /// The object the action is directed to.
    T* mySource;

    /// The object's operation to perform.
    Operation myOperation;

    int myParam;

};
