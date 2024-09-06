from typing import Iterable, TypeAlias, overload

import ad
from ...libcarla import _CarlaEnum

from . import *

class CompareRouteResult(int, _CarlaEnum):
    Differ = 3
    Equal = 0
    Longer = 2
    Shorter = 1

class ConnectingRoute:
    def assign(self, other: ConnectingRoute) -> ConnectingRoute:
        """

        assign( (ConnectingRoute)arg1, (ConnectingRoute)other) -> ConnectingRoute :

            C++ signature :
                ad::map::route::ConnectingRoute {lvalue} assign(ad::map::route::ConnectingRoute {lvalue},ad::map::route::ConnectingRoute)
        """
        ...

    @property
    def routeA(self) -> FullRoute: ...

    @property
    def routeB(self) -> FullRoute: ...

    @property
    def type(self) -> ConnectingRouteType: ...

class ConnectingRouteType(int, _CarlaEnum):
    Following = 1

    Invalid = 0

    Merging = 3

    Opposing = 2

class FilterDuplicatesMode(int, _CarlaEnum):
    Off = 0

    OnlyEqual = 1

    SubRoutesPreferLongerOnes = 3

    SubRoutesPreferShorterOnes = 2

class FindLaneChangeResult:
    @property
    def Valid(self) -> bool: ...

    @property
    def calcZoneLength(self) -> ad.physics.Distance: ...

    def get_queryRoute(self) -> FullRoute:
        """

        get_queryRoute( (FindLaneChangeResult)arg1) -> FullRoute :

            C++ signature :
                ad::map::route::FullRoute get_queryRoute(ad::map::route::FindLaneChangeResult {lvalue})
        """
        ...

    @property
    def laneChangeDirection(self) -> LaneChangeDirection: ...

    @property
    def laneChangeEndLaneSegmentIterator(self) -> Unknown: ...

    @property
    def laneChangeEndRouteIterator(self) -> Unknown: ...

    @property
    def laneChangeStartLaneSegmentIterator(self) -> Unknown: ...

    @property
    def laneChangeStartRouteIterator(self) -> Unknown: ...

    @property
    def numberOfConnectedLaneChanges(self) -> int: ...

class FindWaypointResult:
    @property
    def LeftLane(self) -> FindWaypointResult: ...

    @property
    def PredecessorLanes(self) -> FindWaypointResult: ...

    @property
    def RightLane(self) -> FindWaypointResult: ...

    @property
    def SuccessorLanes(self) -> FindWaypointResult: ...

    @property
    def Valid(self) -> FindWaypointResult: ...

    def assign(self, other: FindWaypointResult) -> FindWaypointResult:
        """

        assign( (FindWaypointResult)arg1, (FindWaypointResult)other) -> FindWaypointResult :

            C++ signature :
                ad::map::route::FindWaypointResult {lvalue} assign(ad::map::route::FindWaypointResult {lvalue},ad::map::route::FindWaypointResult)
        """
        ...

    def get_queryRoute(self) -> FullRoute:
        """

        get_queryRoute( (FindWaypointResult)arg1) -> FullRoute :

            C++ signature :
                ad::map::route::FullRoute get_queryRoute(ad::map::route::FindWaypointResult {lvalue})
        """
        ...

    @property
    def laneSegmentIterator(self) -> FindWaypointResult: ...

    @property
    def queryPosition(self) -> FindWaypointResult: ...

    @property
    def roadSegmentIterator(self) -> FindWaypointResult: ...

class FullRoute:
    def assign(self, other: FullRoute) -> FullRoute:
        """

        assign( (FullRoute)arg1, (FullRoute)other) -> FullRoute :

            C++ signature :
                ad::map::route::FullRoute {lvalue} assign(ad::map::route::FullRoute {lvalue},ad::map::route::FullRoute)
        """
        ...

    @property
    def destinationLaneOffset(self) -> int: ...

    @property
    def fullRouteSegmentCount(self) -> int: ...

    @property
    def maxLaneOffset(self) -> int: ...

    @property
    def minLaneOffset(self) -> int: ...

    @property
    def roadSegments(self) -> RoadSegmentList: ...

    @property
    def routeCreationMode(self) -> RouteCreationMode: ...

    @property
    def routePlanningCounter(self) -> int: ...

class FullRouteList:
    def append(self, arg2: FullRoute) -> None:
        """

        append( (FullRouteList)arg1, (FullRoute)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > {lvalue},ad::map::route::FullRoute)
        """
        ...

    def count(self, arg2: FullRoute) -> int:
        """

        count( (FullRouteList)arg1, (FullRoute)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > {lvalue},ad::map::route::FullRoute)
        """
        ...

    def extend(self, arg2: Iterable[FullRoute]) -> None:
        """

        extend( (FullRouteList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg2: FullRoute) -> int:
        """

        index( (FullRouteList)arg1, (FullRoute)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > {lvalue},ad::map::route::FullRoute)
        """
        ...

    def insert(self, arg2: int, arg3: FullRoute) -> None:
        """

        insert( (FullRouteList)arg1, (int)arg2, (FullRoute)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > {lvalue},long,ad::map::route::FullRoute)
        """
        ...

    def reverse(self) -> None:
        """

        reverse( (FullRouteList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > {lvalue})
        """
        ...

class LaneChangeDirection(int, _CarlaEnum):
    Invalid = 2

    LeftToRight = 0

    RightToLeft = 1

class LaneInterval:
    def assign(self, arg1: LaneInterval, other: LaneInterval) -> LaneInterval:
        """

        assign( (LaneInterval)arg1, (LaneInterval)other) -> LaneInterval :

            C++ signature :
                ad::map::route::LaneInterval {lvalue} assign(ad::map::route::LaneInterval {lvalue},ad::map::route::LaneInterval)
        """
        ...

    @property
    def end(self) -> ad.physics.ParametricValue: ...

    @property
    def laneId(self) -> lane.LaneId: ...

    @property
    def start(self) -> ad.physics.ParametricValue: ...

    @property
    def wrongWay(self) -> bool: ...

class LaneSegment:
    def assign(self, other: LaneSegment) -> LaneSegment:
        """

        assign( (LaneSegment)arg1, (LaneSegment)other) -> LaneSegment :

            C++ signature :
                ad::map::route::LaneSegment {lvalue} assign(ad::map::route::LaneSegment {lvalue},ad::map::route::LaneSegment)
        """
        ...

    @property
    def laneInterval(self) -> LaneInterval: ...

    @property
    def leftNeighbor(self) -> lane.LaneId: ...

    @property
    def predecessors(self) -> lane.LaneIdList: ...

    @property
    def rightNeighbor(self) -> lane.LaneId: ...

    @property
    def routeLaneOffset(self) -> int: ...

    @property
    def successors(self) -> lane.LaneIdList: ...

class LaneSegmentList:
    def append(self, arg2: LaneSegment) -> None:
        """

        append( (LaneSegmentList)arg1, (LaneSegment)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::route::LaneSegment, std::allocator<ad::map::route::LaneSegment> > {lvalue},ad::map::route::LaneSegment)
        """
        ...

    def count(self, arg2: LaneSegment) -> int:
        """

        count( (LaneSegmentList)arg1, (LaneSegment)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::route::LaneSegment, std::allocator<ad::map::route::LaneSegment> > {lvalue},ad::map::route::LaneSegment)
        """
        ...

    def extend(self, arg2: object) -> None:
        """

        extend( (LaneSegmentList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::route::LaneSegment, std::allocator<ad::map::route::LaneSegment> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg2: LaneSegment) -> int:
        """

        index( (LaneSegmentList)arg1, (LaneSegment)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::route::LaneSegment, std::allocator<ad::map::route::LaneSegment> > {lvalue},ad::map::route::LaneSegment)
        """
        ...

    def insert(self, arg2: int, arg3: LaneSegment) -> None:
        """

        insert( (LaneSegmentList)arg1, (int)arg2, (LaneSegment)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::route::LaneSegment, std::allocator<ad::map::route::LaneSegment> > {lvalue},long,ad::map::route::LaneSegment)
        """
        ...

    def reverse(self) -> None:
        """

        reverse( (LaneSegmentList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::route::LaneSegment, std::allocator<ad::map::route::LaneSegment> > {lvalue})
        """
        ...

class RoadSegment:
    def assign(self, other: RoadSegment) -> RoadSegment:
        """

        assign( (RoadSegment)arg1, (RoadSegment)other) -> RoadSegment :

            C++ signature :
                ad::map::route::RoadSegment {lvalue} assign(ad::map::route::RoadSegment {lvalue},ad::map::route::RoadSegment)
        """
        ...

    @property
    def boundingSphere(self) -> point.BoundingSphere: ...

    @property
    def drivableLaneSegments(self) -> LaneSegmentList: ...

    @property
    def segmentCountFromDestination(self) -> int: ...

class RoadSegmentList:
    def append(self, arg2: RoadSegment) -> None:
        """

        append( (RoadSegmentList)arg1, (RoadSegment)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::route::RoadSegment, std::allocator<ad::map::route::RoadSegment> > {lvalue},ad::map::route::RoadSegment)
        """
        ...

    def count(self, arg2: RoadSegment) -> int:
        """

        count( (RoadSegmentList)arg1, (RoadSegment)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::route::RoadSegment, std::allocator<ad::map::route::RoadSegment> > {lvalue},ad::map::route::RoadSegment)
        """
        ...

    def extend(self, arg2: object) -> None:
        """

        extend( (RoadSegmentList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::route::RoadSegment, std::allocator<ad::map::route::RoadSegment> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg2: RoadSegment) -> int:
        """

        index( (RoadSegmentList)arg1, (RoadSegment)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::route::RoadSegment, std::allocator<ad::map::route::RoadSegment> > {lvalue},ad::map::route::RoadSegment)
        """
        ...

    def insert(self, arg2: int, arg3: RoadSegment) -> None:
        """

        insert( (RoadSegmentList)arg1, (int)arg2, (RoadSegment)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::route::RoadSegment, std::allocator<ad::map::route::RoadSegment> > {lvalue},long,ad::map::route::RoadSegment)
        """
        ...

    def reverse(self) -> None:
        """

        reverse( (RoadSegmentList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::route::RoadSegment, std::allocator<ad::map::route::RoadSegment> > {lvalue})
        """
        ...

class Route:
    @property
    def BasicRoutes(self) -> Unknown: ...

    @property
    def Dest(self) -> Unknown: ...

    INVALID = 0

    RawRoute: TypeAlias = Route

    @property
    def RawRoutes(self) -> Unknown: ...

    @property
    def RoutingDest(self) -> Unknown: ...

    @property
    def RoutingStart(self) -> Unknown: ...

    SHORTEST = 1

    SHORTEST_IGNORE_DIRECTION = 2

    @property
    def Start(self) -> Unknown: ...

    Type: TypeAlias = Route

    @property
    def Valid(self) -> Unknown: ...

    @overload
    def calculate(self) -> None: ...

    @overload
    def calculate(self) -> bool: ...

    def calculate(self) -> bool | None:
        """

        calculate( (Route)arg1) -> bool :

            C++ signature :
                bool calculate(ad::map::route::planning::Route {lvalue})

        calculate( (Route)arg1) -> None :

            C++ signature :
                void calculate(Route_wrapper {lvalue})
        """
        ...

    def getBasicRoute(self, routeIndex: object = 0) -> object:
        """

        getBasicRoute( (Route)arg1 [, (object)routeIndex=0]) -> object :

            C++ signature :
                std::vector<std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> >, std::allocator<std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> > > > getBasicRoute(ad::map::route::planning::Route {lvalue} [,unsigned long=0])
        """
        ...

    def getRawRoute(self, routeIndex: object = 0) -> Route.RawRoute:
        """

        getRawRoute( (Route)arg1 [, (object)routeIndex=0]) -> RawRoute :

            C++ signature :
                ad::map::route::planning::Route::RawRoute getRawRoute(ad::map::route::planning::Route {lvalue} [,unsigned long=0])
        """
        ...

    def getType(self) -> Route.Type:
        """

        getType( (Route)arg1) -> Type :

            C++ signature :
                ad::map::route::planning::Route::Type getType(ad::map::route::planning::Route {lvalue})
        """
        ...

    @property
    def laneDirectionIsIgnored(self) -> Unknown: ...

class RouteCreationMode(int):
    AllNeighborLanes = 3

    AllRoutableLanes = 2

    SameDrivingDirection = 1

    Undefined = 0

class RouteIterator:
    @property
    def Valid(self) -> FullRoute: ...

    def get_route(self) -> FullRoute:
        """

        get_route( (RouteIterator)arg1) -> FullRoute :

            C++ signature :
                ad::map::route::FullRoute get_route(ad::map::route::RouteIterator {lvalue})
        """
        ...

    @property
    def roadSegmentIterator(self) -> FullRoute: ...

class RouteParaPoint:
    def assign(self, other: RouteParaPoint) -> RouteParaPoint:
        """

        assign( (RouteParaPoint)arg1, (RouteParaPoint)other) -> RouteParaPoint :

            C++ signature :
                ad::map::route::RouteParaPoint {lvalue} assign(ad::map::route::RouteParaPoint {lvalue},ad::map::route::RouteParaPoint)
        """
        ...

    @property
    def parametricOffset(self) -> ad.physics.ParametricValue: ...

    @property
    def routePlanningCounter(self) -> int: ...

    @property
    def segmentCountFromDestination(self) -> int: ...

class RouteSectionCreationMode(int):
    AllRouteLanes = 1

    SingleLane = 0

class RoutingDirection(int):
    DONT_CARE = 0

    NEGATIVE = 2

    POSITIVE = 1

class RoutingParaPoint:
    @property
    def direction(self) -> RoutingDirection: ...

    @property
    def point(self) -> point.ParaPoint: ...

class ShortenRouteMode(int):
    DontCutIntersectionAndPrependIfSucceededBeforeRoute = 2

    Normal = 0

    PrependIfSucceededBeforeRoute = 1

class ShortenRouteResult(int):
    FailedRouteEmpty = 4

    Succeeded = 0

    SucceededBeforeRoute = 1

    SucceededIntersectionNotCut = 3

    SucceededRouteEmpty = 2

class vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_:
    def append(self, arg2: RoutingParaPoint) -> None:
        """

        append( (vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_)arg1, (RoutingParaPoint)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::route::planning::RoutingParaPoint, std::allocator<ad::map::route::planning::RoutingParaPoint> > {lvalue},ad::map::route::planning::RoutingParaPoint)
        """
        ...

    def count(self, arg2: RoutingParaPoint) -> int:
        """

        count( (vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_)arg1, (RoutingParaPoint)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::route::planning::RoutingParaPoint, std::allocator<ad::map::route::planning::RoutingParaPoint> > {lvalue},ad::map::route::planning::RoutingParaPoint)
        """
        ...

    def extend(self, arg2: object) -> None:
        """

        extend( (vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::route::planning::RoutingParaPoint, std::allocator<ad::map::route::planning::RoutingParaPoint> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg2: RoutingParaPoint) -> int:
        """

        index( (vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_)arg1, (RoutingParaPoint)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::route::planning::RoutingParaPoint, std::allocator<ad::map::route::planning::RoutingParaPoint> > {lvalue},ad::map::route::planning::RoutingParaPoint)
        """
        ...

    def insert(self, arg2: int, arg3: RoutingParaPoint) -> None:
        """

        insert( (vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_)arg1, (int)arg2, (RoutingParaPoint)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::route::planning::RoutingParaPoint, std::allocator<ad::map::route::planning::RoutingParaPoint> > {lvalue},long,ad::map::route::planning::RoutingParaPoint)
        """
        ...

    def reverse(self) -> None:
        """

        reverse( (vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::route::planning::RoutingParaPoint, std::allocator<ad::map::route::planning::RoutingParaPoint> > {lvalue})
        """
        ...

    def sort(self) -> None:
        """

        sort( (vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::map::route::planning::RoutingParaPoint, std::allocator<ad::map::route::planning::RoutingParaPoint> > {lvalue})
        """
        ...
