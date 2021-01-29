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
/// @file    RODFDetectorHandler.h
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// missing_desc
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/options/OptionsCont.h>
#include "RODFDetector.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODFDetectorHandler
 * @brief SAX2-Handler for loading DFROUTER-detector definitions
 */
class RODFDetectorHandler : public SUMOSAXHandler {
public:
    /// Constructor
    RODFDetectorHandler(RODFNet* optNet, bool ignoreErrors, RODFDetectorCon& con,
                        const std::string& file);

    /// Destructor
    virtual ~RODFDetectorHandler();

protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element,
                        const SUMOSAXAttributes& attrs);
    //@}

private:
    /// the net
    RODFNet* myNet;

    /// whether to ignore errors on parsing
    bool myIgnoreErrors;

    /// the container to put the detectors into
    RODFDetectorCon& myContainer;


private:
    /// invalidated copy constructor
    RODFDetectorHandler(const RODFDetectorHandler& src);

    /// invalidated assignment operator
    RODFDetectorHandler& operator=(const RODFDetectorHandler& src);

};
