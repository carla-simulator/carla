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
/// @file    GUISettingsHandler.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Fri, 24. Apr 2009
///
// The handler for parsing gui settings from xml.
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/xml/SUMOSAXHandler.h>
#include <utils/distribution/RandomDistributor.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUISUMOAbstractView;
class Position;


// ===========================================================================
// class definitions
// ===========================================================================
/** @class GUISettingsHandler
 * @brief An XML-handler for visualisation schemes
 */
class GUISettingsHandler : public SUMOSAXHandler {
public:
    /** @brief Constructor
     * @param[in] file the file to parse
     */
    GUISettingsHandler(const std::string& content, bool isFile = true, bool netedit = false);


    /// @brief Destructor
    ~GUISettingsHandler();



    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element, const SUMOSAXAttributes& attrs);
    //@}



    /** @brief Adds the parsed settings to the global list of settings
     * @return the name of the parsed settings
     */
    std::string addSettings(GUISUMOAbstractView* view = 0) const;


    /** @brief Sets the viewport which has been parsed
     * @param[in] parent the view for which the viewport has to be set
     */
    void applyViewport(GUISUMOAbstractView* view) const;


    /** @brief Makes a snapshot if it has been parsed
     * @param[in] parent the view which needs to be shot
     * @todo Please describe why the snapshots are only set if no other existed before (see code)
     */
    void setSnapshots(GUISUMOAbstractView* view) const;


    /** @brief Returns whether any decals have been parsed
     * @return whether decals have been parsed
     */
    bool hasDecals() const;


    /** @brief Returns the parsed decals
     * @return the parsed decals
     */
    const std::vector<GUISUMOAbstractView::Decal>& getDecals() const;


    /** @brief Returns the parsed delay
     * @return the parsed delay
     */
    double getDelay() const;


    /** @brief Returns the parsed breakpoints
     * @return the parsed breakpoints
     */
    const std::vector<SUMOTime>& getBreakpoints() const {
        return myBreakpoints;
    }


    /// @brief loads breakpoints from the specified file
    static std::vector<SUMOTime> loadBreakpoints(const std::string& file);


    /** @brief Returns the parsed view type
     * @return the parsed view type
     */
    const std::string& getViewType() const {
        return myViewType;
    }

    RandomDistributor<std::string> getEventDistribution(const std::string& id);
    double getJamSoundTime() {
        return myJamSoundTime;
    }

private:
    /// @brief The settings to fill
    GUIVisualizationSettings mySettings;

    /// @brief The view type (osg, opengl, default) loaded
    std::string myViewType;

    /// @brief The delay loaded
    double myDelay;

    /// @brief The viewport loaded, zoom is stored in z coordinate
    Position myLookFrom;

    /// @brief The point to look at, only needed for osg view
    Position myLookAt;

    /// @brief View rotation
    double myRotation;

    /// @brief mappig of time steps to filenames for potential snapshots
    std::map<SUMOTime, std::vector<std::string> > mySnapshots;

    /// @brief The decals list to fill
    std::vector<GUISUMOAbstractView::Decal> myDecals;

    /// @brief The last color scheme category (edges or vehicles)
    int myCurrentColorer;

    /// @brief The current color scheme
    GUIColorScheme* myCurrentScheme;

    /// @brief The current scaling scheme
    GUIScaleScheme* myCurrentScaleScheme;

    /// @brief The parsed breakpoints
    std::vector<SUMOTime> myBreakpoints;

    /// @brief The parsed event distributions
    std::map<std::string, RandomDistributor<std::string> > myEventDistributions;
    double myJamSoundTime;

private:

    /// @brief parse attributes for textSettings
    GUIVisualizationTextSettings parseTextSettings(
        const std::string& prefix, const SUMOSAXAttributes& attrs,
        GUIVisualizationTextSettings defaults);

    /// @brief parse attributes for sizeSettings
    GUIVisualizationSizeSettings parseSizeSettings(
        const std::string& prefix, const SUMOSAXAttributes& attrs,
        GUIVisualizationSizeSettings defaults);

};
