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
/// @file    GNEChange_DataSet.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// A network change in which a data set element is created or deleted
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/elements/data/GNEDataSet.h>
#include <netedit/GNEViewNet.h>

#include "GNEChange_DataSet.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEChange_DataSet, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================

GNEChange_DataSet::GNEChange_DataSet(GNEDataSet* dataSet, bool forward) :
    GNEChange(forward, dataSet->isAttributeCarrierSelected()),
    myDataSet(dataSet) {
    myDataSet->incRef("GNEChange_DataSet");
}


GNEChange_DataSet::~GNEChange_DataSet() {
    assert(myDataSet);
    myDataSet->decRef("GNEChange_DataSet");
    if (myDataSet->unreferenced()) {
        // show extra information for tests
        WRITE_DEBUG("Deleting unreferenced " + myDataSet->getTagStr() + " '" + myDataSet->getID() + "'");
        // make sure that element isn't in net before removing
        if (myDataSet->getNet()->getAttributeCarriers()->dataSetExist(myDataSet)) {
            myDataSet->getNet()->getAttributeCarriers()->deleteDataSet(myDataSet);
        }
        // delete data set
        delete myDataSet;
    }
}


void
GNEChange_DataSet::undo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Removing " + myDataSet->getTagStr() + " '" + myDataSet->getID() + "' in GNEChange_DataSet");
        // delete data set from net
        myDataSet->getNet()->getAttributeCarriers()->deleteDataSet(myDataSet);
    } else {
        // show extra information for tests
        WRITE_DEBUG("Adding " + myDataSet->getTagStr() + " '" + myDataSet->getID() + "' in GNEChange_DataSet");
        // insert data set into net
        myDataSet->getNet()->getAttributeCarriers()->insertDataSet(myDataSet);
    }
    // update toolbar
    myDataSet->getNet()->getViewNet()->getIntervalBar().updateIntervalBar();
    // Requiere always save elements
    myDataSet->getNet()->requireSaveDataElements(true);
}


void
GNEChange_DataSet::redo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Adding " + myDataSet->getTagStr() + " '" + myDataSet->getID() + "' in GNEChange_DataSet");
        // insert data set into net
        myDataSet->getNet()->getAttributeCarriers()->insertDataSet(myDataSet);
    } else {
        // show extra information for tests
        WRITE_DEBUG("Removing " + myDataSet->getTagStr() + " '" + myDataSet->getID() + "' in GNEChange_DataSet");
        // delete data set from net
        myDataSet->getNet()->getAttributeCarriers()->deleteDataSet(myDataSet);
    }
    // update toolbar
    myDataSet->getNet()->getViewNet()->getIntervalBar().updateIntervalBar();
    // Requiere always save elements
    myDataSet->getNet()->requireSaveDataElements(true);
}


FXString
GNEChange_DataSet::undoName() const {
    if (myForward) {
        return ("Undo create " + myDataSet->getTagStr()).c_str();
    } else {
        return ("Undo delete " + myDataSet->getTagStr()).c_str();
    }
}


FXString
GNEChange_DataSet::redoName() const {
    if (myForward) {
        return ("Redo create " + myDataSet->getTagStr()).c_str();
    } else {
        return ("Redo delete " + myDataSet->getTagStr()).c_str();
    }
}


/****************************************************************************/
