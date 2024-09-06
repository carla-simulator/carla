from typing import TypeAlias, overload

import ad

from . import *

Unknown: TypeAlias = UnknownType

class CoreIntersection():
    @property
    def BoundingSphere(self) -> point.BoundingSphere: ...

    @property
    def entryLanes(self) -> lane.LaneIdSet: ...

    @property
    def entryParaPoints(self) -> point.ParaPointList: ...

    @property
    def exitLanes(self) -> lane.LaneIdSet: ...

    @property
    def exitParaPoints(self) -> point.ParaPointList: ...

    def extractLanesOfCoreIntersection(self, arg1: CoreIntersection, laneId: lane.LaneId) -> None:
        """

        extractLanesOfCoreIntersection( (CoreIntersection)arg1, (LaneId)laneId) -> None :

            C++ signature :
                void extractLanesOfCoreIntersection(CoreIntersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    @overload
    @staticmethod
    def getCoreIntersectionFor(laneId: lane.LaneId) -> CoreIntersection: ...

    @overload
    @staticmethod
    def getCoreIntersectionFor(mapMatchedPosition: match.MapMatchedPosition) -> CoreIntersection:
        """

        getCoreIntersectionFor( (LaneId)laneId) -> CoreIntersection :

            C++ signature :
                std::shared_ptr<ad::map::intersection::CoreIntersection> getCoreIntersectionFor(ad::map::lane::LaneId)

        getCoreIntersectionFor( (MapMatchedPosition)mapMatchedPosition) -> CoreIntersection :

            C++ signature :
                std::shared_ptr<ad::map::intersection::CoreIntersection> getCoreIntersectionFor(ad::map::match::MapMatchedPosition)
        """
        ...

    @staticmethod
    def getCoreIntersectionsFor(laneIds: lane.LaneIdSet | lane.LaneIdList) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_:
        """

        getCoreIntersectionsFor( (LaneIdSet)laneIds) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_ :

            C++ signature :
                std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > getCoreIntersectionsFor(std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >)

        getCoreIntersectionsFor( (LaneIdList)laneIds) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_ :

            C++ signature :
                std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > getCoreIntersectionsFor(std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> >)
        """
        ...

    def getCoreIntersectionsForInLaneMatches(self, position: point.ENUPoint, mapMatchedPositionConfidenceList: match.vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_, object: match.MapMatchedObjectBoundingBox) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_:
        """

        getCoreIntersectionsForInLaneMatches( (ENUPoint)position) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_ :

            C++ signature :
                std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > getCoreIntersectionsForInLaneMatches(ad::map::point::ENUPoint)

        getCoreIntersectionsForInLaneMatches( (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)mapMatchedPositionConfidenceList) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_ :

            C++ signature :
                std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > getCoreIntersectionsForInLaneMatches(std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >)

        getCoreIntersectionsForInLaneMatches( (MapMatchedObjectBoundingBox)object) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_ :

            C++ signature :
                std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > getCoreIntersectionsForInLaneMatches(ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def getCoreIntersectionsForMap(self, ) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_:
        """

        getCoreIntersectionsForMap() -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_ :

            C++ signature :
                std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > getCoreIntersectionsForMap()
        """
        ...

    def getEntryParaPointOfExternalLane(self, arg1: CoreIntersection, laneId: lane.LaneId) -> point.ParaPoint:
        """

        getEntryParaPointOfExternalLane( (CoreIntersection)arg1, (LaneId)laneId) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint getEntryParaPointOfExternalLane(CoreIntersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    def getEntryParaPointOfInternalLane(self, arg1: CoreIntersection, laneId: lane.LaneId) -> point.ParaPoint:
        """

        getEntryParaPointOfInternalLane( (CoreIntersection)arg1, (LaneId)laneId) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint getEntryParaPointOfInternalLane(CoreIntersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    def getExitParaPointOfExternalLane(self, arg1: CoreIntersection, laneId: lane.LaneId) -> point.ParaPoint:
        """

        getExitParaPointOfExternalLane( (CoreIntersection)arg1, (LaneId)laneId) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint getExitParaPointOfExternalLane(CoreIntersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    def getExitParaPointOfInternalLane(self, arg1: CoreIntersection, laneId: lane.LaneId) -> point.ParaPoint:
        """

        getExitParaPointOfInternalLane( (CoreIntersection)arg1, (LaneId)laneId) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint getExitParaPointOfInternalLane(CoreIntersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    @property
    def internalLanes(self) -> lane.LaneIdSet: ...

    def isIntersectionOnRoute(self, route: route.FullRoute) -> bool:
        """

        isIntersectionOnRoute( (FullRoute)route) -> bool :

            C++ signature :
                bool isIntersectionOnRoute(ad::map::route::FullRoute)
        """
        ...

    @staticmethod
    def isLanePartOfAnIntersection(laneId: lane.LaneId) -> bool:
        """

        isLanePartOfAnIntersection( (LaneId)laneId) -> bool :

            C++ signature :
                bool isLanePartOfAnIntersection(ad::map::lane::LaneId)
        """
        ...

    @staticmethod
    def isLanePartOfCoreIntersection(arg1: CoreIntersection, laneId: lane.LaneId) -> bool:
        """

        isLanePartOfCoreIntersection( (CoreIntersection)arg1, (LaneId)laneId) -> bool :

            C++ signature :
                bool isLanePartOfCoreIntersection(CoreIntersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    def isRoadSegmentEnteringIntersection(self, routeIterator: route.RouteIterator, routePreviousSegmentIter: object) -> bool:
        """

        isRoadSegmentEnteringIntersection( (RouteIterator)routeIterator, (object)routePreviousSegmentIter) -> bool :

            C++ signature :
                bool isRoadSegmentEnteringIntersection(ad::map::route::RouteIterator,__gnu_cxx::__normal_iterator<ad::map::route::RoadSegment const*, std::vector<ad::map::route::RoadSegment, std::allocator<ad::map::route::RoadSegment> > > {lvalue})
        """
        ...

    def isRoutePartOfAnIntersection(self, route: route.FullRoute) -> bool:
        """

        isRoutePartOfAnIntersection( (FullRoute)route) -> bool :

            C++ signature :
                bool isRoutePartOfAnIntersection(ad::map::route::FullRoute)
        """
        ...

    def objectDistanceToIntersection(self, arg1: CoreIntersection, object: match.Object) -> ad.physics.Distance:
        """

        objectDistanceToIntersection( (CoreIntersection)arg1, (Object)object) -> Distance :

            C++ signature :
                ad::physics::Distance objectDistanceToIntersection(ad::map::intersection::CoreIntersection {lvalue},ad::map::match::Object)
        """
        ...

    def objectRouteCrossesIntersection(self, arg1: CoreIntersection, objectRoute: route.FullRoute) -> bool:
        """

        objectRouteCrossesIntersection( (CoreIntersection)arg1, (FullRoute)objectRoute) -> bool :

            C++ signature :
                bool objectRouteCrossesIntersection(ad::map::intersection::CoreIntersection {lvalue},ad::map::route::FullRoute)
        """
        ...

    def objectWithinIntersection(self, arg1: CoreIntersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectWithinIntersection( (CoreIntersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectWithinIntersection(ad::map::intersection::CoreIntersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

class Intersection(CoreIntersection,):
    @property
    def BoundingSphere(self) -> Unknown: ...

    @property
    def IntersectionStartOnRoute(self) -> Unknown: ...

    @property
    def RoutePlanningCounter(self) -> Unknown: ...

    @property
    def RouteSegmentCountFromDestination(self) -> Unknown: ...

    @property
    def SpeedLimit(self) -> Unknown: ...

    @property
    def applicableTrafficLights(self) -> Unknown: ...

    @property
    def crossingLanes(self) -> Unknown: ...

    @property
    def entryLanes(self) -> Unknown: ...

    @property
    def entryParaPoints(self) -> Unknown: ...

    @property
    def exitLanes(self) -> Unknown: ...

    @property
    def exitParaPoints(self) -> Unknown: ...

    def extractLanesOfCoreIntersection(self, laneId: lane.LaneId) -> None:
        """

        extractLanesOfCoreIntersection( (Intersection)arg1, (LaneId)laneId) -> None :

            C++ signature :
                void extractLanesOfCoreIntersection(Intersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    def getEntryParaPointOfExternalLane(self, laneId: lane.LaneId) -> point.ParaPoint:
        """

        getEntryParaPointOfExternalLane( (Intersection)arg1, (LaneId)laneId) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint getEntryParaPointOfExternalLane(Intersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    def getEntryParaPointOfInternalLane(self, laneId: lane.LaneId) -> point.ParaPoint:
        """

        getEntryParaPointOfInternalLane( (Intersection)arg1, (LaneId)laneId) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint getEntryParaPointOfInternalLane(Intersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    def getExitParaPointOfExternalLane(self, laneId: lane.LaneId) -> point.ParaPoint:
        """

        getExitParaPointOfExternalLane( (Intersection)arg1, (LaneId)laneId) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint getExitParaPointOfExternalLane(Intersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    def getExitParaPointOfInternalLane(self, laneId: lane.LaneId) -> point.ParaPoint:
        """

        getExitParaPointOfInternalLane( (Intersection)arg1, (LaneId)laneId) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint getExitParaPointOfInternalLane(Intersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    @staticmethod
    def getIntersectionForRoadSegment(routeIterator: route.RouteIterator) -> Intersection:
        """

        getIntersectionForRoadSegment( (RouteIterator)routeIterator) -> Intersection :

            C++ signature :
                std::shared_ptr<ad::map::intersection::Intersection> getIntersectionForRoadSegment(ad::map::route::RouteIterator)
        """
        ...

    @staticmethod
    def getIntersectionsForRoute(route: route.FullRoute) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_Intersection_greater__greater_:
        """

        getIntersectionsForRoute( (FullRoute)route) -> vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_Intersection_greater__greater_ :

            C++ signature :
                std::vector<std::shared_ptr<ad::map::intersection::Intersection>, std::allocator<std::shared_ptr<ad::map::intersection::Intersection> > > getIntersectionsForRoute(ad::map::route::FullRoute)
        """
        ...

    @staticmethod
    def getNextIntersectionOnRoute(route: route.FullRoute) -> Intersection:
        """

        getNextIntersectionOnRoute( (FullRoute)route) -> Intersection :

            C++ signature :
                std::shared_ptr<ad::map::intersection::Intersection> getNextIntersectionOnRoute(ad::map::route::FullRoute)
        """
        ...

    @property
    def incomingLanes(self) -> Unknown: ...

    @property
    def incomingLanesOnRoute(self) -> Unknown: ...

    @property
    def incomingLanesWithHigherPriority(self) -> Unknown: ...

    @property
    def incomingLanesWithLowerPriority(self) -> Unknown: ...

    @property
    def incomingParaPoints(self) -> Unknown: ...

    @property
    def incomingParaPointsOnRoute(self) -> Unknown: ...

    @property
    def incomingParaPointsWithHigherPriority(self) -> Unknown: ...

    @property
    def incomingParaPointsWithLowerPriority(self) -> Unknown: ...

    @property
    def internalLanes(self) -> Unknown: ...

    @property
    def internalLanesWithHigherPriority(self) -> Unknown: ...

    @property
    def internalLanesWithLowerPriority(self) -> Unknown: ...

    @property
    def intersectionType(self) -> Unknown: ...

    def isLanePartOfCoreIntersection(self, arg1: Intersection, laneId: lane.LaneId) -> bool:
        """

        isLanePartOfCoreIntersection( (Intersection)arg1, (LaneId)laneId) -> bool :

            C++ signature :
                bool isLanePartOfCoreIntersection(Intersection_wrapper {lvalue},ad::map::lane::LaneId)
        """
        ...

    @property
    def lanesOnRoute(self) -> Unknown: ...

    def objectInterpenetrationDistanceWithIntersection(self, arg1: Intersection, object: match.Object) -> ad.physics.Distance:
        """

        objectInterpenetrationDistanceWithIntersection( (Intersection)arg1, (Object)object) -> Distance :

            C++ signature :
                ad::physics::Distance objectInterpenetrationDistanceWithIntersection(ad::map::intersection::Intersection {lvalue},ad::map::match::Object)
        """
        ...

    def objectOnCrossingLane(self, arg1: Intersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectOnCrossingLane( (Intersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectOnCrossingLane(ad::map::intersection::Intersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def objectOnIncomingLane(self, arg1: Intersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectOnIncomingLane( (Intersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectOnIncomingLane(ad::map::intersection::Intersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def objectOnIncomingLaneWithHigherPriority(self, arg1: Intersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectOnIncomingLaneWithHigherPriority( (Intersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectOnIncomingLaneWithHigherPriority(ad::map::intersection::Intersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def objectOnIncomingLaneWithLowerPriority(self, arg1: Intersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectOnIncomingLaneWithLowerPriority( (Intersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectOnIncomingLaneWithLowerPriority(ad::map::intersection::Intersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def objectOnInternalLaneWithHigherPriority(self, arg1: Intersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectOnInternalLaneWithHigherPriority( (Intersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectOnInternalLaneWithHigherPriority(ad::map::intersection::Intersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def objectOnInternalLaneWithLowerPriority(self, arg1: Intersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectOnInternalLaneWithLowerPriority( (Intersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectOnInternalLaneWithLowerPriority(ad::map::intersection::Intersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def objectOnIntersectionRoute(self, arg1: Intersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectOnIntersectionRoute( (Intersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectOnIntersectionRoute(ad::map::intersection::Intersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def objectOnLaneWithHigherPriority(self, arg1: Intersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectOnLaneWithHigherPriority( (Intersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectOnLaneWithHigherPriority(ad::map::intersection::Intersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def objectOnLaneWithLowerPriority(self, arg1: Intersection, object: match.MapMatchedObjectBoundingBox) -> bool:
        """

        objectOnLaneWithLowerPriority( (Intersection)arg1, (MapMatchedObjectBoundingBox)object) -> bool :

            C++ signature :
                bool objectOnLaneWithLowerPriority(ad::map::intersection::Intersection {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        """
        ...

    def objectRouteCrossesIntersectionRoute(self, arg1: Intersection, objectRoute: route.FullRoute) -> bool:
        """

        objectRouteCrossesIntersectionRoute( (Intersection)arg1, (FullRoute)objectRoute) -> bool :

            C++ signature :
                bool objectRouteCrossesIntersectionRoute(ad::map::intersection::Intersection {lvalue},ad::map::route::FullRoute)
        """
        ...

    def objectRouteCrossesLanesWithHigherPriority(self, arg1: Intersection, objectRoute: route.FullRoute) -> bool:
        """

        objectRouteCrossesLanesWithHigherPriority( (Intersection)arg1, (FullRoute)objectRoute) -> bool :

            C++ signature :
                bool objectRouteCrossesLanesWithHigherPriority(ad::map::intersection::Intersection {lvalue},ad::map::route::FullRoute)
        """
        ...

    def objectRouteFromSameArmAsIntersectionRoute(self, arg1: Intersection, objectRoute: route.FullRoute) -> bool:
        """

        objectRouteFromSameArmAsIntersectionRoute( (Intersection)arg1, (FullRoute)objectRoute) -> bool :

            C++ signature :
                bool objectRouteFromSameArmAsIntersectionRoute(ad::map::intersection::Intersection {lvalue},ad::map::route::FullRoute)
        """
        ...

    def objectRouteOppositeToIntersectionRoute(self, arg1: Intersection, objectRoute: route.FullRoute) -> bool:
        """

        objectRouteOppositeToIntersectionRoute( (Intersection)arg1, (FullRoute)objectRoute) -> bool :

            C++ signature :
                bool objectRouteOppositeToIntersectionRoute(ad::map::intersection::Intersection {lvalue},ad::map::route::FullRoute)
        """
        ...

    def onlySolidTrafficLightsOnRoute(self, arg1: Intersection) -> bool:
        """

        onlySolidTrafficLightsOnRoute( (Intersection)arg1) -> bool :

            C++ signature :
                bool onlySolidTrafficLightsOnRoute(ad::map::intersection::Intersection {lvalue})
        """
        ...

    @property
    def outgoingLanes(self) -> Unknown: ...

    @property
    def outgoingLanesOnRoute(self) -> Unknown: ...

    @property
    def outgoingParaPoints(self) -> Unknown: ...

    @property
    def outgoingParaPointsOnRoute(self) -> Unknown: ...

    @property
    def paraPointsOnRoute(self) -> Unknown: ...

    @property
    def turnDirection(self) -> Unknown: ...

    def updateRouteCounters(self, arg1: Intersection, newRoutePlanningCounter: object, newRouteSegmentCounter: object) -> None:
        """

        updateRouteCounters( (Intersection)arg1, (object)newRoutePlanningCounter, (object)newRouteSegmentCounter) -> None :

            C++ signature :
                void updateRouteCounters(ad::map::intersection::Intersection {lvalue},unsigned long,unsigned long)
        """
        ...

class IntersectionType(int,):
    AllWayStop = 3

    Crosswalk = 5

    HasWay = 4

    PriorityToRight = 6

    PriorityToRightAndStraight = 7

    Stop = 2

    TrafficLight = 8

    Unknown = 0

    Yield = 1

class TurnDirection(int,):
    Left = 3

    Right = 1

    Straight = 2

    UTurn = 4

    Unknown = 0

class vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_():
    def append(self, arg1: vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_, arg2: object) -> None:
        """

        append( (vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_)arg1, (object)arg2) -> None :

            C++ signature :
                void append(std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > {lvalue},std::shared_ptr<ad::map::intersection::CoreIntersection>)
        """
        ...

    def count(self, arg1: vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_, arg2: object) -> int:
        """

        count( (vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_)arg1, (object)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > {lvalue},std::shared_ptr<ad::map::intersection::CoreIntersection>)
        """
        ...

    def extend(self, arg1: vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_, arg2: object) -> None:
        """

        extend( (vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_, arg2: object) -> int:
        """

        index( (vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_)arg1, (object)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > {lvalue},std::shared_ptr<ad::map::intersection::CoreIntersection>)
        """
        ...

    def insert(self, arg1: vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_, arg2: int, arg3: object) -> None:
        """

        insert( (vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_)arg1, (int)arg2, (object)arg3) -> None :

            C++ signature :
                void insert(std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > {lvalue},long,std::shared_ptr<ad::map::intersection::CoreIntersection>)
        """
        ...

    def reverse(self, arg1: vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_) -> None:
        """

        reverse( (vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_)arg1) -> None :

            C++ signature :
                void reverse(std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > {lvalue})
        """
        ...

    def sort(self, arg1: vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_) -> None:
        """

        sort( (vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_CoreIntersection_greater__greater_)arg1) -> None :

            C++ signature :
                void sort(std::vector<std::shared_ptr<ad::map::intersection::CoreIntersection>, std::allocator<std::shared_ptr<ad::map::intersection::CoreIntersection> > > {lvalue})
        """
        ...

class vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_Intersection_greater__greater_(ad._VectorSequence[Intersection]):
    def sort(self, arg1: vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_Intersection_greater__greater_) -> None:
        """

        sort( (vector_less_std_scope_shared_ptr_less_ad_scope_map_scope_intersection_scope_Intersection_greater__greater_)arg1) -> None :

            C++ signature :
                void sort(std::vector<std::shared_ptr<ad::map::intersection::Intersection>, std::allocator<std::shared_ptr<ad::map::intersection::Intersection> > > {lvalue})
        """
        ...

