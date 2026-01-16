# --------- Linting ------------
# Not relevant for stubs
# pylint: disable=unused-argument,C0103,used-before-assignment
#
# needs change in API
# pylint: disable=too-many-locals,too-many-public-methods,too-many-arguments,too-many-public-methods,too-few-public-methods,too-many-lines,redefined-builtin
#
# Fixable
# pylint: disable=line-too-long
# Needs __all__ to be defined
# pylint: disable=useless-import-alias,unused-import
#
# False positives
# Should only trigger for class name used in itself
# pylint: disable=undefined-variable
#
# ruff: noqa: F401,F405,F403
# -------------------------------

from typing import Iterable, TypeAlias, overload

import ad
from ...libcarla import _CarlaEnum

from . import *


AllNeighborLanes = RouteCreationMode.AllNeighborLanes
AllRoutableLanes = RouteCreationMode.AllRoutableLanes
AllRouteLanes = RouteSectionCreationMode.AllRouteLanes
DONT_CARE = RoutingDirection.DONT_CARE
Differ = CompareRouteResult.Differ
DontCutIntersectionAndPrependIfSucceededBeforeRoute = ShortenRouteMode.DontCutIntersectionAndPrependIfSucceededBeforeRoute
Equal = CompareRouteResult.Equal
FailedRouteEmpty = ShortenRouteResult.FailedRouteEmpty
Following = ConnectingRouteType.Following
Invalid = LaneChangeDirection.Invalid
LeftToRight = LaneChangeDirection.LeftToRight
Longer = CompareRouteResult.Longer
Merging = ConnectingRouteType.Merging
NEGATIVE = RoutingDirection.NEGATIVE
Normal = ShortenRouteMode.Normal
Off = FilterDuplicatesMode.Off
OnlyEqual = FilterDuplicatesMode.OnlyEqual
Opposing = ConnectingRouteType.Opposing
POSITIVE = RoutingDirection.POSITIVE
PrependIfSucceededBeforeRoute = ShortenRouteMode.PrependIfSucceededBeforeRoute
RightToLeft = LaneChangeDirection.RightToLeft
SameDrivingDirection = RouteCreationMode.SameDrivingDirection
Shorter = CompareRouteResult.Shorter
SingleLane = RouteSectionCreationMode.SingleLane
SubRoutesPreferLongerOnes = FilterDuplicatesMode.SubRoutesPreferLongerOnes
SubRoutesPreferShorterOnes = FilterDuplicatesMode.SubRoutesPreferShorterOnes
Succeeded = ShortenRouteResult.Succeeded
SucceededBeforeRoute = ShortenRouteResult.SucceededBeforeRoute
SucceededIntersectionNotCut = ShortenRouteResult.SucceededIntersectionNotCut
SucceededRouteEmpty = ShortenRouteResult.SucceededRouteEmpty
Undefined = RouteCreationMode.Undefined

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

class LaneSegmentList(ad._VectorSequence[LaneSegment]):
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

class RoadSegmentList(ad._VectorSequence[ad.map.route.RoadSegment]):
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

    def getBasicRoute(self, routeIndex: int = 0) -> object:
        """

        getBasicRoute( (Route)arg1 [, (object)routeIndex=0]) -> object :

            C++ signature :
                std::vector<std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> >, std::allocator<std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> > > > getBasicRoute(ad::map::route::planning::Route {lvalue} [,unsigned long=0])
        """
        ...

    def getRawRoute(self, routeIndex: int = 0) -> Route.RawRoute:
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

class RouteCreationMode(int, _CarlaEnum):
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

class RouteSectionCreationMode(int, _CarlaEnum):
    AllRouteLanes = 1

    SingleLane = 0

class RoutingDirection(int, _CarlaEnum):
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


def calcLength(obj, /) -> ad.physics.Distance: ...

def calcDuration(obj, /) -> ad.physics.Duration: ...

def calcParametricLength(laneInterval: LaneInterval, /) -> ad.physics.ParametricValue: ...

def calculateBypassingRoute(route: FullRoute, bypassingRoute: FullRoute, /) -> bool: ...

def calculateConnectingRoute(*args, **kwargs):
    """
    calculateConnectingRoute( (Object)startObject, (Object)destObject, (Distance)maxDistance, (Duration)maxDuration [, (FullRouteList)startObjectPredictionHints=<route.FullRouteList object at 0x7752c8570640> [, (FullRouteList)destObjectPredictionHints=<route.FullRouteList object at 0x7752c8570c40> [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c8577370>]]]) -> ConnectingRoute :

        C++ signature :
            ad::map::route::ConnectingRoute calculateConnectingRoute(ad::map::match::Object,ad::map::match::Object,ad::physics::Distance,ad::physics::Duration [,std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> >=<route.FullRouteList object at 0x7752c8570640> [,std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> >=<route.FullRouteList object at 0x7752c8570c40> [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c8577370>]]])

    calculateConnectingRoute( (Object)startObject, (Object)destObject, (Distance)maxDistance [, (FullRouteList)startObjectPredictionHints=<route.FullRouteList object at 0x7752c85702c0> [, (FullRouteList)destObjectPredictionHints=<route.FullRouteList object at 0x7752c8571040> [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c8577910>]]]) -> ConnectingRoute :

        C++ signature :
            ad::map::route::ConnectingRoute calculateConnectingRoute(ad::map::match::Object,ad::map::match::Object,ad::physics::Distance [,std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> >=<route.FullRouteList object at 0x7752c85702c0> [,std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> >=<route.FullRouteList object at 0x7752c8571040> [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c8577910>]]])

    calculateConnectingRoute( (Object)startObject, (Object)destObject, (Duration)maxDuration [, (FullRouteList)startObjectPredictionHints=<route.FullRouteList object at 0x7752c8571a40> [, (FullRouteList)destObjectPredictionHints=<route.FullRouteList object at 0x7752c8570040> [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c8574ee0>]]]) -> ConnectingRoute :

        C++ signature :
            ad::map::route::ConnectingRoute calculateConnectingRoute(ad::map::match::Object,ad::map::match::Object,ad::physics::Duration [,std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> >=<route.FullRouteList object at 0x7752c8571a40> [,std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> >=<route.FullRouteList object at 0x7752c8570040> [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c8574ee0>]]])"""
def calculateRouteParaPointAtDistance(*args, **kwargs):
    """
    calculateRouteParaPointAtDistance( (FullRoute)route, (RouteParaPoint)origin, (Distance)distance, (RouteParaPoint)resultingPoint) -> bool :

        C++ signature :
            bool calculateRouteParaPointAtDistance(ad::map::route::FullRoute,ad::map::route::RouteParaPoint,ad::physics::Distance,ad::map::route::RouteParaPoint {lvalue})"""
def compareRoutesOnIntervalLevel(*args, **kwargs):
    """
    compareRoutesOnIntervalLevel( (FullRoute)left, (FullRoute)right) -> CompareRouteResult :

        C++ signature :
            ad::map::route::planning::CompareRouteResult compareRoutesOnIntervalLevel(ad::map::route::FullRoute,ad::map::route::FullRoute)"""
def createFullRoute(*args, **kwargs):
    """
    createFullRoute( (RawRoute)rawRoute, (RouteCreationMode)routeCreationMode, (LaneIdSet)relevantLanes) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute createFullRoute(ad::map::route::planning::Route::RawRoute,ad::map::route::RouteCreationMode,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >)"""
def createRoutingParaPoint(*args, **kwargs):
    """
    createRoutingParaPoint( (LaneId)laneId, (ParametricValue)parametricOffset [, (RoutingDirection)routingDirection=route.RoutingDirection.DONT_CARE]) -> RoutingParaPoint :

        C++ signature :
            ad::map::route::planning::RoutingParaPoint createRoutingParaPoint(ad::map::lane::LaneId,ad::physics::ParametricValue [,ad::map::route::planning::RoutingDirection=route.RoutingDirection.DONT_CARE])"""

@overload
def createRoutingPoint(ad) -> Any:
    """
    createRoutingPoint( (LaneId)laneId, (ParametricValue)parametricOffset [, (RoutingDirection)routingDirection=route.RoutingDirection.DONT_CARE]) -> RoutingParaPoint :

        C++ signature :
            ad::map::route::planning::RoutingParaPoint createRoutingPoint(ad::map::lane::LaneId,ad::physics::ParametricValue [,ad::map::route::planning::RoutingDirection=route.RoutingDirection.DONT_CARE])

    createRoutingPoint( (ParaPoint)paraPoint [, (RoutingDirection)routingDirection=route.RoutingDirection.DONT_CARE]) -> RoutingParaPoint :

        C++ signature :
            ad::map::route::planning::RoutingParaPoint createRoutingPoint(ad::map::point::ParaPoint [,ad::map::route::planning::RoutingDirection=route.RoutingDirection.DONT_CARE])

    createRoutingPoint( (LaneOccupiedRegion)occupiedRegion [, (RoutingDirection)routingDirection=route.RoutingDirection.DONT_CARE]) -> RoutingParaPoint :

        C++ signature :
            ad::map::route::planning::RoutingParaPoint createRoutingPoint(ad::map::match::LaneOccupiedRegion [,ad::map::route::planning::RoutingDirection=route.RoutingDirection.DONT_CARE])

    createRoutingPoint( (ParaPoint)paraPoint, (ENUHeading)heading) -> RoutingParaPoint :

        C++ signature :
            ad::map::route::planning::RoutingParaPoint createRoutingPoint(ad::map::point::ParaPoint,ad::map::point::ENUHeading)

    createRoutingPoint( (LaneOccupiedRegion)occupiedRegion, (ENUHeading)heading) -> RoutingParaPoint :

        C++ signature :
            ad::map::route::planning::RoutingParaPoint createRoutingPoint(ad::map::match::LaneOccupiedRegion,ad::map::point::ENUHeading)"""

@overload
def createRoutingPoint(obj, routingDirection: route.RoutingDirection=route.RoutingDirection.DONT_CARE) -> RoutingParaPoint: ...

@overload
def createRoutingPoint(obj: match.LaneOccupiedRegion | point.ParaPoint, heading: point.ENUHeading) -> RoutingParaPoint: ...

def cutIntervalAtEnd(*args, **kwargs):
    """
    cutIntervalAtEnd( (LaneInterval)laneInterval, (ParametricValue)newIntervalEnd) -> LaneInterval :

        C++ signature :
            ad::map::route::LaneInterval cutIntervalAtEnd(ad::map::route::LaneInterval,ad::physics::ParametricValue)"""
def cutIntervalAtStart(*args, **kwargs):
    """
    cutIntervalAtStart( (LaneInterval)laneInterval, (ParametricValue)newIntervalStart) -> LaneInterval :

        C++ signature :
            ad::map::route::LaneInterval cutIntervalAtStart(ad::map::route::LaneInterval,ad::physics::ParametricValue)"""
def extendIntervalFromEnd(*args, **kwargs):
    """
    extendIntervalFromEnd( (LaneInterval)laneInterval, (Distance)distance) -> LaneInterval :

        C++ signature :
            ad::map::route::LaneInterval extendIntervalFromEnd(ad::map::route::LaneInterval,ad::physics::Distance)"""
def extendIntervalFromStart(*args, **kwargs):
    """
    extendIntervalFromStart( (LaneInterval)laneInterval, (Distance)distance) -> LaneInterval :

        C++ signature :
            ad::map::route::LaneInterval extendIntervalFromStart(ad::map::route::LaneInterval,ad::physics::Distance)"""
def extendIntervalUntilEnd(ad) -> Any:
    """
    extendIntervalUntilEnd( (LaneInterval)laneInterval) -> LaneInterval :

        C++ signature :
            ad::map::route::LaneInterval extendIntervalUntilEnd(ad::map::route::LaneInterval)"""
def extendIntervalUntilStart(ad) -> Any:
    """
    extendIntervalUntilStart( (LaneInterval)laneInterval) -> LaneInterval :

        C++ signature :
            ad::map::route::LaneInterval extendIntervalUntilStart(ad::map::route::LaneInterval)"""
def extendRouteToDestinations(*args, **kwargs):
    """
    extendRouteToDestinations( (FullRoute)route, (vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_)dest) -> bool :

        C++ signature :
            bool extendRouteToDestinations(ad::map::route::FullRoute {lvalue},std::vector<ad::map::route::planning::RoutingParaPoint, std::allocator<ad::map::route::planning::RoutingParaPoint> >)

    extendRouteToDestinations( (FullRoute)route, (GeoEdge)dest) -> bool :

        C++ signature :
            bool extendRouteToDestinations(ad::map::route::FullRoute {lvalue},std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> >)

    extendRouteToDestinations( (FullRoute)route, (ENUEdge)dest) -> bool :

        C++ signature :
            bool extendRouteToDestinations(ad::map::route::FullRoute {lvalue},std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> >)"""
def extendRouteToDistance(*args, **kwargs):
    """
    extendRouteToDistance( (FullRoute)route, (Distance)length, (FullRouteList)additionalRoutes [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c8574c10>]) -> bool :

        C++ signature :
            bool extendRouteToDistance(ad::map::route::FullRoute {lvalue},ad::physics::Distance,std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > {lvalue} [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c8574c10>])"""
def filterDuplicatedRoutes(*args, **kwargs):
    """
    filterDuplicatedRoutes( (FullRouteList)fullRoutes, (FilterDuplicatesMode)filterMode) -> FullRouteList :

        C++ signature :
            std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > filterDuplicatedRoutes(std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> >,ad::map::route::planning::FilterDuplicatesMode)"""
def findCenterWaypoint(*args, **kwargs):
    """
    findCenterWaypoint( (Object)object, (FullRoute)route) -> FindWaypointResult :

        C++ signature :
            ad::map::route::FindWaypointResult findCenterWaypoint(ad::map::match::Object,ad::map::route::FullRoute)"""
def findFirstLaneChange(*args, **kwargs):
    """
    findFirstLaneChange( (MapMatchedPosition)currentPositionEgoVehicle, (FullRoute)route) -> FindLaneChangeResult :

        C++ signature :
            ad::map::route::FindLaneChangeResult findFirstLaneChange(ad::map::match::MapMatchedPosition,ad::map::route::FullRoute)"""
def findNearestWaypoint(*args, **kwargs):
    """
    findNearestWaypoint( (ParaPointList)positions, (FullRoute)route) -> FindWaypointResult :

        C++ signature :
            ad::map::route::FindWaypointResult findNearestWaypoint(std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> >,ad::map::route::FullRoute)

    findNearestWaypoint( (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)mapMatchedPositions, (FullRoute)route) -> FindWaypointResult :

        C++ signature :
            ad::map::route::FindWaypointResult findNearestWaypoint(std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >,ad::map::route::FullRoute)"""
def findWaypoint(*args, **kwargs):
    """
    findWaypoint( (ParaPoint)position, (FullRoute)route) -> FindWaypointResult :

        C++ signature :
            ad::map::route::FindWaypointResult findWaypoint(ad::map::point::ParaPoint,ad::map::route::FullRoute)

    findWaypoint( (LaneId)laneId, (FullRoute)route) -> FindWaypointResult :

        C++ signature :
            ad::map::route::FindWaypointResult findWaypoint(ad::map::lane::LaneId,ad::map::route::FullRoute)"""
def fromString(*args, **kwargs):
    """
    fromString( (str)str) -> ConnectingRouteType :

        C++ signature :
            ad::map::route::ConnectingRouteType fromString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)

    fromString( (str)str) -> RouteCreationMode :

        C++ signature :
            ad::map::route::RouteCreationMode fromString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)

    fromString( (str)str) -> LaneChangeDirection :

        C++ signature :
            ad::map::route::LaneChangeDirection fromString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)"""
def getECEFBorder(ad) -> Any:
    """
    getECEFBorder( (LaneInterval)laneInterval) -> ECEFBorder :

        C++ signature :
            ad::map::lane::ECEFBorder getECEFBorder(ad::map::route::LaneInterval)"""
def getECEFBorderOfRoadSegment(ad) -> Any:
    """
    getECEFBorderOfRoadSegment( (RoadSegment)roadSegment [, (ParametricValue)parametricOffset]) -> ECEFBorder :

        C++ signature :
            ad::map::lane::ECEFBorder getECEFBorderOfRoadSegment(ad::map::route::RoadSegment [,ad::physics::ParametricValue])"""
def getECEFBorderOfRoute(ad) -> Any:
    """
    getECEFBorderOfRoute( (FullRoute)route) -> ECEFBorderList :

        C++ signature :
            std::vector<ad::map::lane::ECEFBorder, std::allocator<ad::map::lane::ECEFBorder> > getECEFBorderOfRoute(ad::map::route::FullRoute)"""
def getENUBorder(ad) -> Any:
    """
    getENUBorder( (LaneInterval)laneInterval) -> ENUBorder :

        C++ signature :
            ad::map::lane::ENUBorder getENUBorder(ad::map::route::LaneInterval)"""
def getENUBorderOfRoadSegment(ad) -> Any:
    """
    getENUBorderOfRoadSegment( (RoadSegment)roadSegment [, (ParametricValue)parametricOffset]) -> ENUBorder :

        C++ signature :
            ad::map::lane::ENUBorder getENUBorderOfRoadSegment(ad::map::route::RoadSegment [,ad::physics::ParametricValue])"""
def getENUBorderOfRoute(ad) -> Any:
    """
    getENUBorderOfRoute( (FullRoute)route) -> ENUBorderList :

        C++ signature :
            std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> > getENUBorderOfRoute(ad::map::route::FullRoute)"""
def getENUHeadingOfRoute(*args, **kwargs):
    """
    getENUHeadingOfRoute( (Object)object, (FullRoute)route) -> ENUHeading :

        C++ signature :
            ad::map::point::ENUHeading getENUHeadingOfRoute(ad::map::match::Object,ad::map::route::FullRoute)"""
def getENUProjectedBorder(ad) -> lane.ENUBorder:
    """
    getENUProjectedBorder( (LaneInterval)laneInterval) -> ENUBorder :

        C++ signature :
            ad::map::lane::ENUBorder getENUProjectedBorder(ad::map::route::LaneInterval)"""
def getGeoBorder(ad) -> lane.GeoBorder:
    """
    getGeoBorder( (LaneInterval)laneInterval) -> GeoBorder :

        C++ signature :
            ad::map::lane::GeoBorder getGeoBorder(ad::map::route::LaneInterval)"""
def getGeoBorderOfRoadSegment(ad) -> lane.GeoBorder:
    """
    getGeoBorderOfRoadSegment( (RoadSegment)roadSegment [, (ParametricValue)parametricOffset]) -> GeoBorder :

        C++ signature :
            ad::map::lane::GeoBorder getGeoBorderOfRoadSegment(ad::map::route::RoadSegment [,ad::physics::ParametricValue])"""
def getGeoBorderOfRoute(ad) -> lane.GeoBorderList:
    """
    getGeoBorderOfRoute( (FullRoute)route) -> GeoBorderList :

        C++ signature :
            std::vector<ad::map::lane::GeoBorder, std::allocator<ad::map::lane::GeoBorder> > getGeoBorderOfRoute(ad::map::route::FullRoute)"""
def getIntervalEnd(ad) -> point.ParaPoint:
    """
    getIntervalEnd( (LaneInterval)laneInterval) -> ParaPoint :

        C++ signature :
            ad::map::point::ParaPoint getIntervalEnd(ad::map::route::LaneInterval)"""
def getIntervalStart(ad) -> point.ParaPoint:
    """
    getIntervalStart( (LaneInterval)laneInterval) -> ParaPoint :

        C++ signature :
            ad::map::point::ParaPoint getIntervalStart(ad::map::route::LaneInterval)

    getIntervalStart( (FullRoute)route, (LaneId)laneId) -> ParaPoint :

        C++ signature :
            ad::map::point::ParaPoint getIntervalStart(ad::map::route::FullRoute,ad::map::lane::LaneId)"""
def getLaneParaPoint(*args, **kwargs):
    """
    getLaneParaPoint( (ParametricValue)routeParametricOffset, (LaneInterval)laneInterval) -> ParaPoint :

        C++ signature :
            ad::map::point::ParaPoint getLaneParaPoint(ad::physics::ParametricValue,ad::map::route::LaneInterval)"""
def getLaneParaPoints(*args, **kwargs):
    """
    getLaneParaPoints( (RouteParaPoint)routePosition, (FullRoute)route) -> ParaPointList :

        C++ signature :
            std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> > getLaneParaPoints(ad::map::route::RouteParaPoint,ad::map::route::FullRoute)"""
def getLeftECEFEdge(ad) -> point.ECEFEdge:
    """
    getLeftECEFEdge( (LaneInterval)laneInterval) -> ECEFEdge :

        C++ signature :
            std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > getLeftECEFEdge(ad::map::route::LaneInterval)"""
def getLeftENUEdge(ad) -> point.ENUEdge:
    """
    getLeftENUEdge( (LaneInterval)laneInterval) -> ENUEdge :

        C++ signature :
            std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > getLeftENUEdge(ad::map::route::LaneInterval)"""
def getLeftEdge(*args, **kwargs):
    """
    getLeftEdge( (LaneInterval)laneInterval, (ENUEdge)enuEdge) -> None :

        C++ signature :
            void getLeftEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > {lvalue})

    getLeftEdge( (LaneInterval)laneInterval, (ECEFEdge)ecefEdge) -> None :

        C++ signature :
            void getLeftEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > {lvalue})

    getLeftEdge( (LaneInterval)laneInterval, (GeoEdge)geoEdge) -> None :

        C++ signature :
            void getLeftEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue})"""
def getLeftGeoEdge(ad) -> point.GeoEdge:
    """
    getLeftGeoEdge( (LaneInterval)laneInterval) -> GeoEdge :

        C++ signature :
            std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > getLeftGeoEdge(ad::map::route::LaneInterval)"""
def getLeftProjectedECEFEdge(ad) -> point.ECEFEdge:
    """
    getLeftProjectedECEFEdge( (LaneInterval)laneInterval) -> ECEFEdge :

        C++ signature :
            std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > getLeftProjectedECEFEdge(ad::map::route::LaneInterval)"""
def getLeftProjectedENUEdge(laneInterval: LaneInterval) -> point.ENUEdge:
    """
    getLeftProjectedENUEdge( (LaneInterval)laneInterval) -> ENUEdge :

        C++ signature :
            std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > getLeftProjectedENUEdge(ad::map::route::LaneInterval)"""
def getLeftProjectedEdge(*args, **kwargs):
    """
    getLeftProjectedEdge( (LaneInterval)laneInterval, (ENUEdge)enuEdge) -> None :

        C++ signature :
            void getLeftProjectedEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > {lvalue})

    getLeftProjectedEdge( (LaneInterval)laneInterval, (ECEFEdge)ecefEdge) -> None :

        C++ signature :
            void getLeftProjectedEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > {lvalue})

    getLeftProjectedEdge( (LaneInterval)laneInterval, (GeoEdge)geoEdge) -> None :

        C++ signature :
            void getLeftProjectedEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue})"""
def getLeftProjectedGeoEdge(ad) -> Any:
    """
    getLeftProjectedGeoEdge( (LaneInterval)laneInterval) -> GeoEdge :

        C++ signature :
            std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > getLeftProjectedGeoEdge(ad::map::route::LaneInterval)"""
def getMetricRanges(*args, **kwargs):
    """
    getMetricRanges( (LaneInterval)laneInterval, (MetricRange)lengthRange, (MetricRange)widthRange) -> None :

        C++ signature :
            void getMetricRanges(ad::map::route::LaneInterval,ad::physics::MetricRange {lvalue},ad::physics::MetricRange {lvalue})"""
def getProjectedParametricOffsetOnNeighborLane(*args, **kwargs):
    """
    getProjectedParametricOffsetOnNeighborLane( (LaneInterval)currentInterval, (LaneInterval)neighborInterval, (ParametricValue)parametricOffset) -> ParametricValue :

        C++ signature :
            ad::physics::ParametricValue getProjectedParametricOffsetOnNeighborLane(ad::map::route::LaneInterval,ad::map::route::LaneInterval,ad::physics::ParametricValue)"""
def getRightECEFEdge(ad) -> Any:
    """
    getRightECEFEdge( (LaneInterval)laneInterval) -> ECEFEdge :

        C++ signature :
            std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > getRightECEFEdge(ad::map::route::LaneInterval)"""
def getRightENUEdge(ad) -> Any:
    """
    getRightENUEdge( (LaneInterval)laneInterval) -> ENUEdge :

        C++ signature :
            std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > getRightENUEdge(ad::map::route::LaneInterval)"""
def getRightEdge(*args, **kwargs):
    """
    getRightEdge( (LaneInterval)laneInterval, (ENUEdge)enuEdge) -> None :

        C++ signature :
            void getRightEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > {lvalue})

    getRightEdge( (LaneInterval)laneInterval, (ECEFEdge)ecefEdge) -> None :

        C++ signature :
            void getRightEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > {lvalue})

    getRightEdge( (LaneInterval)laneInterval, (GeoEdge)geoEdge) -> None :

        C++ signature :
            void getRightEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue})"""
def getRightGeoEdge(laneInterval: LaneInterval) -> point.GeoEdge:
    """
    getRightGeoEdge( (LaneInterval)laneInterval) -> GeoEdge :

        C++ signature :
            std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > getRightGeoEdge(ad::map::route::LaneInterval)"""
def getRightProjectedECEFEdge(laneInterval: LaneInterval) -> point.ECEFEdge:
    """
    getRightProjectedECEFEdge( (LaneInterval)laneInterval) -> ECEFEdge :

        C++ signature :
            std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > getRightProjectedECEFEdge(ad::map::route::LaneInterval)"""
def getRightProjectedENUEdge(laneInterval: LaneInterval) -> point.ENUEdge:
    """
    getRightProjectedENUEdge( (LaneInterval)laneInterval) -> ENUEdge :

        C++ signature :
            std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > getRightProjectedENUEdge(ad::map::route::LaneInterval)"""
def getRightProjectedEdge(*args, **kwargs):
    """
    getRightProjectedEdge( (LaneInterval)laneInterval, (ENUEdge)enuEdge) -> None :

        C++ signature :
            void getRightProjectedEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > {lvalue})

    getRightProjectedEdge( (LaneInterval)laneInterval, (ECEFEdge)ecefEdge) -> None :

        C++ signature :
            void getRightProjectedEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > {lvalue})

    getRightProjectedEdge( (LaneInterval)laneInterval, (GeoEdge)geoEdge) -> None :

        C++ signature :
            void getRightProjectedEdge(ad::map::route::LaneInterval,std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue})"""
def getRightProjectedGeoEdge(ad) -> Any:
    """
    getRightProjectedGeoEdge( (LaneInterval)laneInterval) -> GeoEdge :

        C++ signature :
            std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > getRightProjectedGeoEdge(ad::map::route::LaneInterval)"""
def getRouteExpandedToAllNeighborLanes(ad) -> Any:
    """
    getRouteExpandedToAllNeighborLanes( (FullRoute)route) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute getRouteExpandedToAllNeighborLanes(ad::map::route::FullRoute)"""
def getRouteExpandedToOppositeLanes(ad) -> Any:
    """
    getRouteExpandedToOppositeLanes( (FullRoute)route) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute getRouteExpandedToOppositeLanes(ad::map::route::FullRoute)"""
def getRouteIterator(*args, **kwargs):
    """
    getRouteIterator( (RouteParaPoint)routePosition, (FullRoute)route) -> RouteIterator :

        C++ signature :
            ad::map::route::RouteIterator getRouteIterator(ad::map::route::RouteParaPoint,ad::map::route::FullRoute)"""
def getRouteParaPointFromParaPoint(*args, **kwargs):
    """
    getRouteParaPointFromParaPoint( (ParaPoint)paraPoint, (FullRoute)route, (RouteParaPoint)routeParaPoint) -> bool :

        C++ signature :
            bool getRouteParaPointFromParaPoint(ad::map::point::ParaPoint,ad::map::route::FullRoute,ad::map::route::RouteParaPoint {lvalue})"""
def getRouteSection(*args, **kwargs):
    """
    getRouteSection( (FindWaypointResult)currentLane, (Distance)distanceFront, (Distance)distanceEnd, (FullRoute)route [, (RouteSectionCreationMode)routeSectionCreationMode=route.RouteSectionCreationMode.SingleLane]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute getRouteSection(ad::map::route::FindWaypointResult,ad::physics::Distance,ad::physics::Distance,ad::map::route::FullRoute [,ad::map::route::RouteSectionCreationMode=route.RouteSectionCreationMode.SingleLane])

    getRouteSection( (ParaPoint)centerPoint, (Distance)distanceFront, (Distance)distanceEnd, (FullRoute)route [, (RouteSectionCreationMode)routeSectionCreationMode=route.RouteSectionCreationMode.SingleLane]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute getRouteSection(ad::map::point::ParaPoint,ad::physics::Distance,ad::physics::Distance,ad::map::route::FullRoute [,ad::map::route::RouteSectionCreationMode=route.RouteSectionCreationMode.SingleLane])

    getRouteSection( (Object)object, (FullRoute)route [, (RouteSectionCreationMode)routeSectionCreationMode=route.RouteSectionCreationMode.SingleLane]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute getRouteSection(ad::map::match::Object,ad::map::route::FullRoute [,ad::map::route::RouteSectionCreationMode=route.RouteSectionCreationMode.SingleLane])"""
def getSignedDistance(*args, **kwargs):
    """
    getSignedDistance( (LaneInterval)laneInterval, (ParaPoint)first, (ParaPoint)second) -> ParametricValue :

        C++ signature :
            ad::physics::ParametricValue getSignedDistance(ad::map::route::LaneInterval,ad::map::point::ParaPoint,ad::map::point::ParaPoint)"""


@overload
def getSpeedLimits(ad) -> restriction.SpeedLimitList: ...
    
@overload
def getSpeedLimits(*args) -> restriction.SpeedLimitList: ...


def getUnsignedDistance(*args, **kwargs):
    """
    getUnsignedDistance( (LaneInterval)laneInterval, (ParaPoint)first, (ParaPoint)second) -> ParametricValue :

        C++ signature :
            ad::physics::ParametricValue getUnsignedDistance(ad::map::route::LaneInterval,ad::map::point::ParaPoint,ad::map::point::ParaPoint)"""
def intersectionOnRoute(*args, **kwargs):
    """
    intersectionOnRoute( (Intersection)intersection, (FullRoute)route) -> FindWaypointResult :

        C++ signature :
            ad::map::route::FindWaypointResult intersectionOnRoute(ad::map::intersection::Intersection,ad::map::route::FullRoute)"""
def isAfterInterval(*args, **kwargs):
    """
    isAfterInterval( (LaneInterval)laneInterval, (ParametricValue)parametricOffset) -> bool :

        C++ signature :
            bool isAfterInterval(ad::map::route::LaneInterval,ad::physics::ParametricValue)

    isAfterInterval( (LaneInterval)laneInterval, (ParaPoint)point) -> bool :

        C++ signature :
            bool isAfterInterval(ad::map::route::LaneInterval,ad::map::point::ParaPoint)"""
def isBeforeInterval(*args, **kwargs):
    """
    isBeforeInterval( (LaneInterval)laneInterval, (ParametricValue)parametricOffset) -> bool :

        C++ signature :
            bool isBeforeInterval(ad::map::route::LaneInterval,ad::physics::ParametricValue)

    isBeforeInterval( (LaneInterval)laneInterval, (ParaPoint)point) -> bool :

        C++ signature :
            bool isBeforeInterval(ad::map::route::LaneInterval,ad::map::point::ParaPoint)"""
def isConnectedRoutePartOfAnIntersection(ad) -> Any:
    """
    isConnectedRoutePartOfAnIntersection( (ConnectingRoute)connectingRoute) -> bool :

        C++ signature :
            bool isConnectedRoutePartOfAnIntersection(ad::map::route::ConnectingRoute)"""
def isDegenerated(ad) -> Any:
    """
    isDegenerated( (LaneInterval)laneInterval) -> bool :

        C++ signature :
            bool isDegenerated(ad::map::route::LaneInterval)"""
def isEndOfInterval(*args, **kwargs):
    """
    isEndOfInterval( (LaneInterval)laneInterval, (ParaPoint)point) -> bool :

        C++ signature :
            bool isEndOfInterval(ad::map::route::LaneInterval,ad::map::point::ParaPoint)"""
def isObjectHeadingInRouteDirection(*args, **kwargs):
    """
    isObjectHeadingInRouteDirection( (Object)object, (FullRoute)route) -> bool :

        C++ signature :
            bool isObjectHeadingInRouteDirection(ad::map::match::Object,ad::map::route::FullRoute)"""
def isRouteDirectionAlignedWithDrivingDirection(ad) -> Any:
    """
    isRouteDirectionAlignedWithDrivingDirection( (LaneInterval)laneInterval) -> bool :

        C++ signature :
            bool isRouteDirectionAlignedWithDrivingDirection(ad::map::route::LaneInterval)"""
def isRouteDirectionNegative(ad) -> Any:
    """
    isRouteDirectionNegative( (LaneInterval)laneInterval) -> bool :

        C++ signature :
            bool isRouteDirectionNegative(ad::map::route::LaneInterval)"""
def isRouteDirectionPositive(ad) -> Any:
    """
    isRouteDirectionPositive( (LaneInterval)laneInterval) -> bool :

        C++ signature :
            bool isRouteDirectionPositive(ad::map::route::LaneInterval)"""
def isStartOfInterval(*args, **kwargs):
    """
    isStartOfInterval( (LaneInterval)laneInterval, (ParaPoint)point) -> bool :

        C++ signature :
            bool isStartOfInterval(ad::map::route::LaneInterval,ad::map::point::ParaPoint)"""
def isWithinInterval(*args, **kwargs):
    """
    isWithinInterval( (RoadSegment)roadSegment, (ParaPoint)point) -> bool :

        C++ signature :
            bool isWithinInterval(ad::map::route::RoadSegment,ad::map::point::ParaPoint)

    isWithinInterval( (LaneInterval)laneInterval, (ParametricValue)parametricOffset) -> bool :

        C++ signature :
            bool isWithinInterval(ad::map::route::LaneInterval,ad::physics::ParametricValue)

    isWithinInterval( (LaneInterval)laneInterval, (ParaPoint)point) -> bool :

        C++ signature :
            bool isWithinInterval(ad::map::route::LaneInterval,ad::map::point::ParaPoint)"""
def objectOnRoute(*args, **kwargs):
    """
    objectOnRoute( (MapMatchedObjectBoundingBox)object, (FullRoute)route) -> FindWaypointResult :

        C++ signature :
            ad::map::route::FindWaypointResult objectOnRoute(ad::map::match::MapMatchedObjectBoundingBox,ad::map::route::FullRoute)"""
def overlapsInterval(*args, **kwargs):
    """
    overlapsInterval( (LaneInterval)laneInterval, (ParametricRange)range) -> bool :

        C++ signature :
            bool overlapsInterval(ad::map::route::LaneInterval,ad::physics::ParametricRange)"""
def planRoute(*args, **kwargs):
    """
    planRoute( (RoutingParaPoint)start, (RoutingParaPoint)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::route::planning::RoutingParaPoint,ad::map::route::planning::RoutingParaPoint [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (ParaPoint)start, (ParaPoint)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::point::ParaPoint,ad::map::point::ParaPoint [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (ParaPoint)start, (ENUHeading)startHeading, (ParaPoint)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::point::ParaPoint,ad::map::point::ENUHeading,ad::map::point::ParaPoint [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (ParaPoint)start, (ENUHeading)startHeading, (ParaPoint)dest, (ENUHeading)destHeading [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::point::ParaPoint,ad::map::point::ENUHeading,ad::map::point::ParaPoint,ad::map::point::ENUHeading [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (RoutingParaPoint)start, (GeoPoint)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::route::planning::RoutingParaPoint,ad::map::point::GeoPoint [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (RoutingParaPoint)start, (ENUPoint)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::route::planning::RoutingParaPoint,ad::map::point::ENUPoint [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (ParaPoint)start, (GeoPoint)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::point::ParaPoint,ad::map::point::GeoPoint [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (ParaPoint)start, (ENUHeading)startHeading, (GeoPoint)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::point::ParaPoint,ad::map::point::ENUHeading,ad::map::point::GeoPoint [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (ParaPoint)start, (PointOfInterest)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::point::ParaPoint,ad::map::config::PointOfInterest [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (RoutingParaPoint)start, (GeoEdge)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::route::planning::RoutingParaPoint,std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (RoutingParaPoint)start, (ENUEdge)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::route::planning::RoutingParaPoint,std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (RoutingParaPoint)start, (vector_less_ad_scope_map_scope_route_scope_planning_scope_RoutingParaPoint_greater_)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::route::planning::RoutingParaPoint,std::vector<ad::map::route::planning::RoutingParaPoint, std::allocator<ad::map::route::planning::RoutingParaPoint> > [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])

    planRoute( (ParaPoint)start, (ENUHeading)startHeading, (GeoEdge)dest [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection]) -> FullRoute :

        C++ signature :
            ad::map::route::FullRoute planRoute(ad::map::point::ParaPoint,ad::map::point::ENUHeading,std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection])"""
def predictRoutes(*args, **kwargs):
    """
    predictRoutes( (RoutingParaPoint)start, (Distance)predictionDistance, (Duration)predictionDuration [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection [, (FilterDuplicatesMode)filterMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c8577880>]]]) -> FullRouteList :

        C++ signature :
            std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > predictRoutes(ad::map::route::planning::RoutingParaPoint,ad::physics::Distance,ad::physics::Duration [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection [,ad::map::route::planning::FilterDuplicatesMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c8577880>]]])

    predictRoutes( (MapMatchedObjectBoundingBox)start, (Distance)predictionDistance, (Duration)predictionDuration [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection [, (FilterDuplicatesMode)filterMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c8574700>]]]) -> FullRouteList :

        C++ signature :
            std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > predictRoutes(ad::map::match::MapMatchedObjectBoundingBox,ad::physics::Distance,ad::physics::Duration [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection [,ad::map::route::planning::FilterDuplicatesMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c8574700>]]])"""
def predictRoutesDirectionless(*args, **kwargs):
    """
    predictRoutesDirectionless( (ParaPoint)start, (Distance)predictionDistance, (Duration)predictionDuration [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.AllRoutableLanes [, (FilterDuplicatesMode)filterMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c8574430>]]]) -> FullRouteList :

        C++ signature :
            std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > predictRoutesDirectionless(ad::map::point::ParaPoint,ad::physics::Distance,ad::physics::Duration [,ad::map::route::RouteCreationMode=route.RouteCreationMode.AllRoutableLanes [,ad::map::route::planning::FilterDuplicatesMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c8574430>]]])"""
def predictRoutesOnDistance(*args, **kwargs):
    """
    predictRoutesOnDistance( (RoutingParaPoint)start, (Distance)predictionDistance [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection [, (FilterDuplicatesMode)filterMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c8575f30>]]]) -> FullRouteList :

        C++ signature :
            std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > predictRoutesOnDistance(ad::map::route::planning::RoutingParaPoint,ad::physics::Distance [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection [,ad::map::route::planning::FilterDuplicatesMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c8575f30>]]])

    predictRoutesOnDistance( (MapMatchedObjectBoundingBox)start, (Distance)predictionDistance [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection [, (FilterDuplicatesMode)filterMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c85779a0>]]]) -> FullRouteList :

        C++ signature :
            std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > predictRoutesOnDistance(ad::map::match::MapMatchedObjectBoundingBox,ad::physics::Distance [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection [,ad::map::route::planning::FilterDuplicatesMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c85779a0>]]])"""
def predictRoutesOnDuration(*args, **kwargs):
    """
    predictRoutesOnDuration( (RoutingParaPoint)start, (Duration)predictionDuration [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection [, (FilterDuplicatesMode)filterMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c85753f0>]]]) -> FullRouteList :

        C++ signature :
            std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > predictRoutesOnDuration(ad::map::route::planning::RoutingParaPoint,ad::physics::Duration [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection [,ad::map::route::planning::FilterDuplicatesMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c85753f0>]]])

    predictRoutesOnDuration( (MapMatchedObjectBoundingBox)start, (Duration)predictionDuration [, (RouteCreationMode)routeCreationMode=route.RouteCreationMode.SameDrivingDirection [, (FilterDuplicatesMode)filterMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x7752c85756c0>]]]) -> FullRouteList :

        C++ signature :
            std::vector<ad::map::route::FullRoute, std::allocator<ad::map::route::FullRoute> > predictRoutesOnDuration(ad::map::match::MapMatchedObjectBoundingBox,ad::physics::Duration [,ad::map::route::RouteCreationMode=route.RouteCreationMode.SameDrivingDirection [,ad::map::route::planning::FilterDuplicatesMode=route.FilterDuplicatesMode.SubRoutesPreferLongerOnes [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x7752c85756c0>]]])"""
def restrictIntervalFromBegin(*args, **kwargs):
    """
    restrictIntervalFromBegin( (LaneInterval)laneInterval, (Distance)distance) -> LaneInterval :

        C++ signature :
            ad::map::route::LaneInterval restrictIntervalFromBegin(ad::map::route::LaneInterval,ad::physics::Distance)"""
def shortenIntervalFromBegin(*args, **kwargs):
    """
    shortenIntervalFromBegin( (LaneInterval)laneInterval, (Distance)distance) -> LaneInterval :

        C++ signature :
            ad::map::route::LaneInterval shortenIntervalFromBegin(ad::map::route::LaneInterval,ad::physics::Distance)"""
def shortenIntervalFromEnd(*args, **kwargs):
    """
    shortenIntervalFromEnd( (LaneInterval)laneInterval, (Distance)distance) -> LaneInterval :

        C++ signature :
            ad::map::route::LaneInterval shortenIntervalFromEnd(ad::map::route::LaneInterval,ad::physics::Distance)"""
def shortenRoute(*args, **kwargs):
    """
    shortenRoute( (ParaPoint)currentPosition, (FullRoute)route [, (ShortenRouteMode)shortenRouteMode=route.ShortenRouteMode.Normal]) -> ShortenRouteResult :

        C++ signature :
            ad::map::route::ShortenRouteResult shortenRoute(ad::map::point::ParaPoint,ad::map::route::FullRoute {lvalue} [,ad::map::route::ShortenRouteMode=route.ShortenRouteMode.Normal])

    shortenRoute( (ParaPointList)currentPositions, (FullRoute)route [, (ShortenRouteMode)shortenRouteMode=route.ShortenRouteMode.Normal]) -> ShortenRouteResult :

        C++ signature :
            ad::map::route::ShortenRouteResult shortenRoute(std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> >,ad::map::route::FullRoute {lvalue} [,ad::map::route::ShortenRouteMode=route.ShortenRouteMode.Normal])

    shortenRoute( (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)mapMatchedPositions, (FullRoute)route [, (ShortenRouteMode)shortenRouteMode=route.ShortenRouteMode.Normal]) -> ShortenRouteResult :

        C++ signature :
            ad::map::route::ShortenRouteResult shortenRoute(std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >,ad::map::route::FullRoute {lvalue} [,ad::map::route::ShortenRouteMode=route.ShortenRouteMode.Normal])"""
def shortenRouteToDistance(*args, **kwargs):
    """
    shortenRouteToDistance( (FullRoute)route, (Distance)length) -> None :

        C++ signature :
            void shortenRouteToDistance(ad::map::route::FullRoute {lvalue},ad::physics::Distance)"""
def shortenSegmentFromBegin(*args, **kwargs):
    """
    shortenSegmentFromBegin( (RoadSegment)roadSegment, (Distance)distance) -> None :

        C++ signature :
            void shortenSegmentFromBegin(ad::map::route::RoadSegment {lvalue},ad::physics::Distance)"""
def shortenSegmentFromEnd(*args, **kwargs):
    """
    shortenSegmentFromEnd( (RoadSegment)roadSegment, (Distance)distance) -> None :

        C++ signature :
            void shortenSegmentFromEnd(ad::map::route::RoadSegment {lvalue},ad::physics::Distance)"""
def signedDistanceToLane(*args, **kwargs):
    """
    signedDistanceToLane( (LaneId)checkLaneId, (FullRoute)route, (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)mapMatchedPositions) -> Distance :

        C++ signature :
            ad::physics::Distance signedDistanceToLane(ad::map::lane::LaneId,ad::map::route::FullRoute,std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >)"""
def toParametricRange(ad) -> ad.physics.ParametricRange:
    """
    toParametricRange( (LaneInterval)laneInterval) -> ParametricRange :

        C++ signature :
            ad::physics::ParametricRange toParametricRange(ad::map::route::LaneInterval)"""

def toString(ad) -> str: ...

def to_string(ad) -> str: ...

def updateRoutePlanningCounters(route: FullRoute) -> None: ...
