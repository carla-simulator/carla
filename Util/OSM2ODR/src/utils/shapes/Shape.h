/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    Shape.h
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Oct 2012
///
// A 2D- or 3D-Shape
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/common/Named.h>
#include <utils/common/RGBColor.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Shape
 * @brief A 2D- or 3D-Shape
 */
class Shape : public Named {
public:
    /// @nane default shape's values
    /// @{
    static const std::string DEFAULT_TYPE;
    static const double DEFAULT_LAYER;
    static const double DEFAULT_LINEWIDTH;
    static const double DEFAULT_LAYER_POI;
    static const double DEFAULT_ANGLE;
    static const std::string DEFAULT_IMG_FILE;
    static const bool DEFAULT_RELATIVEPATH;
    static const double DEFAULT_IMG_WIDTH;
    static const double DEFAULT_IMG_HEIGHT;
    /// @}

    /** @brief Constructor
     * @param[in] id The name of the shape
     * @param[in] type The (abstract) type of the shape
     * @param[in] color The color of the shape
     * @param[in] layer The layer of the shape
     * @param[in] angle The rotation of the shape in navigational degrees
     * @param[in] imgFile The raster image of the shape
     * @param[in] relativePath set image file as relative path
     */
    Shape(const std::string& id, const std::string& type,
          const RGBColor& color, double layer,
          double angle, const std::string& imgFile, bool relativePath);

    /// @brief Destructor
    virtual ~Shape();

    /// @name Getter
    /// @{

    /** @brief Returns the (abstract) type of the Shape
     * @return The Shape's (abstract) type
     */
    inline const std::string& getShapeType() const {
        return myType;
    }

    /** @brief Returns the color of the Shape
     * @return The Shape's color
     */
    inline const RGBColor& getShapeColor() const {
        return myColor;
    }

    /** @brief Returns the layer of the Shape
     * @return The Shape's layer
     */
    inline double getShapeLayer() const {
        return myLayer;
    }

    /** @brief Returns the angle of the Shape in navigational degrees
     * @return The Shape's rotation angle
     */
    inline double getShapeNaviDegree() const {
        return myNaviDegreeAngle;
    }

    /** @brief Returns the imgFile of the Shape
     * @return The Shape's rotation imgFile
     */
    inline const std::string& getShapeImgFile() const {
        return myImgFile;
    }

    /** @brief Returns the relativePath of the Shape
    * @return The Shape's relativePath
    */
    inline bool getShapeRelativePath() const {
        return myRelativePath;
    }
    /// @}


    /// @name Setter
    /// @{

    /** @brief Sets a new type
     * @param[in] type The new type to use
     */
    inline void setShapeType(const std::string& type) {
        myType = type;
    }

    /** @brief Sets a new color
     * @param[in] col The new color to use
     */
    inline void setShapeColor(const RGBColor& col) {
        myColor = col;
    }

    /** @brief Sets a new alpha value
     * @param[in] alpha The new value to use
     */
    inline void setShapeAlpha(unsigned char alpha) {
        myColor.setAlpha(alpha);
    }

    /** @brief Sets a new layer
     * @param[in] layer The new layer to use
     */
    inline void setShapeLayer(const double layer) {
        myLayer = layer;
    }

    /** @brief Sets a new angle in navigational degrees
     * @param[in] layer The new angle to use
     */
    virtual void setShapeNaviDegree(const double angle) {
        myNaviDegreeAngle = angle;
    }

    /** @brief Sets a new imgFile
     * @param[in] imgFile The new imgFile to use
     */
    inline void setShapeImgFile(const std::string& imgFile) {
        myImgFile = imgFile;
    }

    /** @brief Sets a new relativePath value
    * @param[in] relativePath The new relative path to set
    */
    inline void setShapeRelativePath(bool relativePath) {
        myRelativePath = relativePath;
    }
    /// @}

private:
    /// @brief The type of the Shape
    std::string myType;

    /// @brief The color of the Shape
    RGBColor myColor;

    /// @brief The layer of the Shape
    double myLayer;

    /// @brief The angle of the Shape
    double myNaviDegreeAngle;

    /// @brief The img file (include path)
    std::string myImgFile;

    /// @brief Enable or disable save imgFile as relative path
    bool myRelativePath;
};
