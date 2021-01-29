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
/// @file    NILoader.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// Perfoms network import
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <xercesc/sax2/SAX2XMLReader.hpp>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class NBNetBuilder;
class Position;
class PositionVector;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NILoader
 * @brief Perfoms network import
 *
 * A plain loader which encapsulates calls to the import modules.
 */
class NILoader {
public:
    /** @brief Constructor
     * @param[in] nb The network builder to fill with loaded data
     */
    NILoader(NBNetBuilder& nb);


    /// @brief Destructor
    ~NILoader();


    /** loads data from the files specified in the given option container */
    void load(OptionsCont& oc);


private:
    /** loads data from sumo-files */
    //void loadSUMO(OptionsCont &oc);

    /** loads data from XML-files */
    bool loadXML(OptionsCont& oc);


private:
    /// @brief The network builder to fill with loaded data
    NBNetBuilder& myNetBuilder;


private:
    /// @brief Invalidated copy constructor.
    NILoader(const NILoader&);

    /// @brief Invalidated assignment operator.
    NILoader& operator=(const NILoader&);


};
