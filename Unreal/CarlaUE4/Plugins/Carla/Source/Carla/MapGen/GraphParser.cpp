// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "GraphParser.h"

#include "DoublyConnectedEdgeList.h"

#include <type_traits>
#include <unordered_set>

namespace MapGen {

  using Graph = DoublyConnectedEdgeList;

  // ===========================================================================
  // -- Local static methods ---------------------------------------------------
  // ===========================================================================

  static int getQuadrant(float angle) {
    return static_cast<int>(std::round(angle/HALF_PI));
  }

  // Assumes angles are separated by half pi approx.
  static float getRotation(float angle0, float angle1) {
    const int min = getQuadrant(std::min(angle0, angle1));
    const int max = getQuadrant(std::max(angle0, angle1));
    return HALF_PI * std::min(min, min * max);
  }

  // Assumes angles are separated by half pi approx.
  static float getRotation(float angle0, float angle1, float angle2) {
    /// @todo There has to be a better way.
    switch (getQuadrant(angle0) + getQuadrant(angle1) + getQuadrant(angle2)) {
      case 0:
        return HALF_PI;
      case 1:
        return 0.0;
      case 2:
        return -1.0 * HALF_PI;
      case 3:
        return PI;
      default:
        UE_LOG(LogCarla, Error, TEXT("Wrong quadrants"));
        return 0.0;
    }
  }

  /// @todo This can probably be done at graph creation.
  static void fixGraphData(Graph &graph) {
    // Set the edge count for each node in the graph.
    for (auto &node : graph.GetNodes()) {
      std::vector<float> angles;
      angles.reserve(4u);
      // Iterate every half-edge in this node.
      auto &firstEdge = Graph::GetLeavingHalfEdge(node);
      auto *edge = &firstEdge;
      do {
        edge->Angle = Graph::GetAngle(*edge);
        angles.emplace_back(edge->Angle);
        edge = &Graph::GetNextInNode(*edge);
      } while (edge != &firstEdge);
      check(!angles.empty());
      node.EdgeCount = angles.size();
      node.bIsIntersection = true;
      switch (node.EdgeCount) {
        case 2:
          node.Rotation = getRotation(angles[0u], angles[1u]);
          node.IntersectionType = EIntersectionType::Turn90Deg;
          break;
        case 3:
          node.Rotation = getRotation(angles[0u], angles[1u], angles[2u]);
          node.IntersectionType = EIntersectionType::TIntersection;
          break;
        case 4:
        default:
          node.Rotation = 0.0;
          node.IntersectionType = EIntersectionType::XIntersection;
          break;
      }
      node.Rots.swap(angles);
    }
  }

  // ===========================================================================
  // -- RoadSegmentBuilder -----------------------------------------------------
  // ===========================================================================

  class RoadSegmentBuilder {
  public:

    std::vector<TUniquePtr<RoadSegmentDescription>> Segments;

    explicit RoadSegmentBuilder(const Graph &graph) : _graph(graph) {}

    void Add(Graph::HalfEdge &edge) {
      if (!insert(edge))
        return;
      if (Graph::GetSource(edge).bIsIntersection) {
        Segments.emplace_back(MakeUnique<RoadSegmentDescription>());
        _handlingInitial = false;
      }
      if (_handlingInitial) {
        _initial.emplace_back(&edge);
      } else {
        Segments.back()->Add(edge);
      }
    }

    void Close() {
      for (auto edge : _initial) {
        Segments.back()->Add(*edge);
      }
      _handlingInitial = true;
    }

  private:

    /// Insert both half-edges only if they haven't been visited yet.
    bool insert(Graph::HalfEdge &edge) {
      return _visitedEdges.insert(&edge).second &&
             _visitedEdges.insert(&Graph::GetPair(edge)).second;
    }

    const Graph &_graph;

    std::unordered_set<const Graph::HalfEdge *> _visitedEdges;

    bool _handlingInitial = true;

    std::vector<const Graph::HalfEdge *> _initial;
  };

  // ===========================================================================
  // -- GraphParser ------------------------------------------------------------
  // ===========================================================================

  GraphParser::GraphParser(DoublyConnectedEdgeList &graph) {
    check(graph.CountNodes() >= 4u);
    check(graph.CountHalfEdges() >= 8u);
    check(graph.CountFaces() >= 2u);

    fixGraphData(graph);

    CityAreas.reserve(graph.CountFaces() - 1);

    RoadSegmentBuilder rsb(graph);

    auto faceList = graph.GetFaces();
    auto it = faceList.begin();
    ++it; // Ignore first face (unbounded).
    for (; it != faceList.end(); ++it) {
      CityAreas.emplace_back(MakeUnique<CityAreaDescription>(*it));
      CityAreaDescription &cityArea = *CityAreas.back();
      // Iterate every half-edge in this face.
      auto &firstEdge = Graph::GetHalfEdge(*it);
      for (auto *edge = &Graph::GetNextInFace(firstEdge);
           edge != &firstEdge;
           edge = &Graph::GetNextInFace(*edge)) {
        cityArea.Add(Graph::GetSource(*edge));
        rsb.Add(*edge);
      }
      rsb.Close();
    }

    RoadSegments.swap(rsb.Segments);
  }

} // namespace MapGen
