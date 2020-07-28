/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
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
/// @file    NBHeightMapper.h
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Sept 2011
///
// Set z-values for all network positions based on data from a height map
/****************************************************************************/
#pragma once
#include <config.h>

#ifdef WIN32
typedef __int16 int16_t;
#else
#include <stdint.h>
#endif

#include <string>
#include <foreign/rtree/RTree.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/Boundary.h>
#include <utils/common/UtilExceptions.h>

#define TRIANGLE_RTREE_QUAL RTree<NBHeightMapper::Triangle*, NBHeightMapper::Triangle, float, 2, NBHeightMapper::QueryResult>

// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBHeightMapper
 * @brief Set z-values for all network positions based on data from a height map
 *
 * Importing data from '.shp'-files works only if SUMO was compiled with GDAL-support.
 *  If not, an error message is generated.
 */
class NBHeightMapper {

    friend class NBHeightMapperTest;

public:
    /** @brief loads height map data if any loading options are set
     *
     * @param[in] oc The options container to get further options from
     * @exception ProcessError if something fails
     */
    static void loadIfSet(OptionsCont& oc);

    /// @brief return the singleton instance (maybe 0)
    static const NBHeightMapper& get();

    /// @brief returns whether the NBHeightMapper has data
    bool ready() const;

    /// @brief returns the convex boundary of all known triangles
    const Boundary& getBoundary() {
        return myInstance.myBoundary;
    }

    /// @brief returns height for the given geo coordinate (WGS84)
    double getZ(const Position& geo) const;

    class QueryResult;
    /* @brief content class for the rtree. Since we wish to be able to use the
     * rtree for spatial querying we have to jump through some minor hoops:
     * We let each found triangle callback the NBHeightMapper and add itself the
     * the query result
     * */
    class Triangle {

    public:
        Triangle(const PositionVector& corners);
        ~Triangle() {};

        /// @brief callback for RTree search
        void addSelf(const QueryResult& queryResult) const;

        /// @brief checks whether pos lies within triangle (only checks x,y)
        bool contains(const Position& pos) const;

        /// @brief returns the projection of the give geoCoordinate (WGS84) onto triangle plane
        double getZ(const Position& geo) const;

        /// @brief returns the normal vector for this triangles plane
        Position normalVector() const;

        /// @brief the corners of the triangle
        PositionVector myCorners;

    };

    typedef std::vector<const Triangle*> Triangles;

    /// @brief class for cirumventing the const-restriction of RTree::Search-context
    class QueryResult {
    public:
        QueryResult() {};
        ~QueryResult() {};
        // @brief method not realy const
        void add(Triangle* triangle) const {
            triangles.push_back(triangle);
        };
        mutable Triangles triangles;
    };

private:
    /// @brief the singleton instance
    static NBHeightMapper myInstance;

    Triangles myTriangles;

    /// @brief The RTree for spatial queries
    TRIANGLE_RTREE_QUAL myRTree;

    /// @brief raster height information in m for all loaded files
    std::vector<std::pair<Boundary, int16_t*> > myRasters;

    /// @brief dimensions of one pixel in raster data
    Position mySizeOfPixel;

    /// @brief convex boundary of all known triangles;
    Boundary myBoundary;

private:
    /// @brief private constructor and destructor (Singleton)
    NBHeightMapper();
    ~NBHeightMapper();

    /// @brief adds one triangles worth of height data
    void addTriangle(PositionVector corners);

    /** @brief load height data from Arcgis-shape file and returns the number of parsed features
     * @return The number of parsed features
     * @throws ProcessError
     */
    int loadShapeFile(const std::string& file);

    /** @brief load height data from GeoTIFF file and returns the number of non void pixels
     * @return The number of valid pixels
     * @throws ProcessError
     */
    int loadTiff(const std::string& file);

    /// @brief clears loaded data
    void clearData();

    /// @brief Invalidated copy constructor.
    NBHeightMapper(const NBHeightMapper&);

    /// @brief Invalidated assignment operator.
    NBHeightMapper& operator=(const NBHeightMapper&);

};


// ===========================================================================
// RTree specialization for speedup and avoiding warnings (ripped from SUMORTree.h)
// ===========================================================================
template<>
inline float TRIANGLE_RTREE_QUAL::RectSphericalVolume(Rect* a_rect) {
    ASSERT(a_rect);
    const float extent0 = a_rect->m_max[0] - a_rect->m_min[0];
    const float extent1 = a_rect->m_max[1] - a_rect->m_min[1];
    return .78539816f * (extent0 * extent0 + extent1 * extent1);
}

template<>
inline TRIANGLE_RTREE_QUAL::Rect TRIANGLE_RTREE_QUAL::CombineRect(Rect* a_rectA, Rect* a_rectB) {
    ASSERT(a_rectA && a_rectB);
    Rect newRect;
    newRect.m_min[0] = rtree_min(a_rectA->m_min[0], a_rectB->m_min[0]);
    newRect.m_max[0] = rtree_max(a_rectA->m_max[0], a_rectB->m_max[0]);
    newRect.m_min[1] = rtree_min(a_rectA->m_min[1], a_rectB->m_min[1]);
    newRect.m_max[1] = rtree_max(a_rectA->m_max[1], a_rectB->m_max[1]);
    return newRect;
}
