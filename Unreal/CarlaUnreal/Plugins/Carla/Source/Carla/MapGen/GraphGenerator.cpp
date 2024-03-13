// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "GraphGenerator.h"
#include "Carla.h"

#include <vector>

namespace MapGen {

  using Graph = DoublyConnectedEdgeList;

  constexpr static int32 MARGIN = 6;

  // ===========================================================================
  // -- Static local methods ---------------------------------------------------
  // ===========================================================================

  static int32 signOf(int32 val) {
    return (0 < val) - (val < 0);
  }

  static const Graph::Position &getSourcePosition(const Graph::HalfEdge &edge) {
    return Graph::GetSource(edge).GetPosition();
  }

  static const Graph::Position &getTargetPosition(const Graph::HalfEdge &edge) {
    return Graph::GetTarget(edge).GetPosition();
  }

  static Graph::Position getDirection(const Graph::HalfEdge &edge) {
    return getTargetPosition(edge) - getSourcePosition(edge);
  }

  static std::pair<Graph::HalfEdge *, Graph::HalfEdge *> getRandomOpposingEdges(
      Graph::Face &face,
      FRandomStream &random) {
    // Get all the edges in the face.
    std::vector<Graph::HalfEdge *> edges;
    edges.reserve(4u);
    auto &firstEdge = Graph::GetHalfEdge(face);
    auto *edge = &firstEdge;
    do {
      edges.emplace_back(edge);
      edge = &Graph::GetNextInFace(*edge);
    } while (edge != &firstEdge);
    check(edges.size() == 4u);
    auto randomIndex = random.RandRange(0, edges.size() - 1);
    return {edges[randomIndex], edges[(randomIndex + 2u) % edges.size()]};
  }

  static Graph::Face *splitFace(Graph &graph, Graph::Face &face, FRandomStream &random) {
    auto edgePair = getRandomOpposingEdges(face, random);
    auto dir = getDirection(*edgePair.first);
    // Assumes both edges are opposing faces on a rectangle.
    auto otherDir = getDirection(*edgePair.second);
    check((dir.x == -1 * otherDir.x) && (dir.y == -1 * otherDir.y));
    // If the rectangle is not big enough do not split it.
    if ((std::abs(dir.x) < 2*MARGIN+1) && (std::abs(dir.y) < 2*MARGIN+1))
      return nullptr;
    // Get a random point along the edges.
    auto randX = (dir.x != 0 ? signOf(dir.x) * random.RandRange(MARGIN, std::abs(dir.x) - MARGIN) : 0);
    auto randY = (dir.y != 0 ? signOf(dir.y) * random.RandRange(MARGIN, std::abs(dir.y) - MARGIN) : 0);
    auto position0 = getSourcePosition(*edgePair.first) + Graph::Position{randX, randY};
    auto position1 = getTargetPosition(*edgePair.second) + Graph::Position{randX, randY};
    // Split the edges and connect.
    Graph::Node &node0 = graph.SplitEdge(position0, *edgePair.first);
    Graph::Node &node1 = graph.SplitEdge(position1, *edgePair.second);
    return &graph.ConnectNodes(node0, node1);
  }

  static void randomize(Graph &graph, const int32 seed)
  {
    check(graph.CountNodes() == 4u);
    check(graph.CountHalfEdges() == 8u);
    check(graph.CountFaces() == 2u);
    FRandomStream random(seed);
    /// @todo We skip first face because is the surrounding face. But this won't
    /// be always the case, if the graph is generated differently it might be a
    /// different one.
    Graph::Face *face = &*(++graph.GetFaces().begin());
    do {
      face = splitFace(graph, *face, random);
#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
      graph.PrintToLog();
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG
    } while (face != nullptr);
  }

 // =============================================================================
 // -- GraphGenerator -----------------------------------------------------------
 // =============================================================================

  TUniquePtr<DoublyConnectedEdgeList> GraphGenerator::Generate(
      const uint32 SizeX,
      const uint32 SizeY,
      const int32 Seed)
  {
    using Position = typename DoublyConnectedEdgeList::Position;
    std::array<Position, 4u> box;
    box[0u] = Position(0, 0);
    box[1u] = Position(0, SizeY);
    box[2u] = Position(SizeX, SizeY);
    box[3u] = Position(SizeX, 0);
    auto Dcel = MakeUnique<DoublyConnectedEdgeList>(box);
    randomize(*Dcel, Seed);
    return Dcel;
  }

} // namespace MapGen
