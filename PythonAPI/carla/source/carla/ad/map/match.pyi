from typing import overload
import ad
from . import *

class AdMapMatching():
    @property
    def MaxHeadingHintFactor(self) -> float: ...

    @property
    def RouteHintFactor(self) -> float: ...

    @overload
    def addHeadingHint(self, arg1: AdMapMatching, yaw: point.ENUHeading, enuReferencePoint: point.GeoPoint) -> None: ...

    @overload
    def addHeadingHint(self, arg1: AdMapMatching, headingHint: point.ECEFHeading) -> None:
        '''

        addHeadingHint( (AdMapMatching)arg1, (ECEFHeading)headingHint) -> None :

            C++ signature :
                void addHeadingHint(ad::map::match::AdMapMatching {lvalue},ad::map::point::ECEFHeading)

        addHeadingHint( (AdMapMatching)arg1, (ENUHeading)yaw, (GeoPoint)enuReferencePoint) -> None :

            C++ signature :
                void addHeadingHint(ad::map::match::AdMapMatching {lvalue},ad::map::point::ENUHeading,ad::map::point::GeoPoint)
        '''
        ...

    def addRouteHint(self, arg1: AdMapMatching, routeHint: route.FullRoute) -> None:
        '''

        addRouteHint( (AdMapMatching)arg1, (FullRoute)routeHint) -> None :

            C++ signature :
                void addRouteHint(ad::map::match::AdMapMatching {lvalue},ad::map::route::FullRoute)
        '''
        ...

    def clearHeadingHints(self, arg1: AdMapMatching) -> None:
        '''

        clearHeadingHints( (AdMapMatching)arg1) -> None :

            C++ signature :
                void clearHeadingHints(ad::map::match::AdMapMatching {lvalue})
        '''
        ...

    def clearHints(self, arg1: AdMapMatching) -> None:
        '''

        clearHints( (AdMapMatching)arg1) -> None :

            C++ signature :
                void clearHints(ad::map::match::AdMapMatching {lvalue})
        '''
        ...

    def clearRelevantLanes(self, arg1: AdMapMatching) -> None:
        '''

        clearRelevantLanes( (AdMapMatching)arg1) -> None :

            C++ signature :
                void clearRelevantLanes(ad::map::match::AdMapMatching {lvalue})
        '''
        ...

    def clearRouteHints(self, arg1: AdMapMatching) -> None:
        '''

        clearRouteHints( (AdMapMatching)arg1) -> None :

            C++ signature :
                void clearRouteHints(ad::map::match::AdMapMatching {lvalue})
        '''
        ...

    @overload
    def findLanes(self, ecefPoint: point.ECEFPoint, distance: ad.physics.Distance, relevantLanes: lane.LaneIdSet, geoPoint: point.GeoPoint, distance: ad.physics.Distance, relevantLanes: LaneIdSet) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_:
        '''

        findLanes( (ECEFPoint)ecefPoint, (Distance)distance [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x79ee2c74fbe0>]) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_ :

            C++ signature :
                std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > findLanes(ad::map::point::ECEFPoint,ad::physics::Distance [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x79ee2c74fbe0>])

        findLanes( (GeoPoint)geoPoint, (Distance)distance [, (LaneIdSet)relevantLanes=<lane.LaneIdSet object at 0x79ee2c74fc70>]) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_ :

            C++ signature :
                std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > findLanes(ad::map::point::GeoPoint,ad::physics::Distance [,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >=<lane.LaneIdSet object at 0x79ee2c74fc70>])
        '''
        ...

    def findRouteLanes(self, ecefPoint: point.ECEFPoint, route: route.FullRoute) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_:
        '''

        findRouteLanes( (ECEFPoint)ecefPoint, (FullRoute)route) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_ :

            C++ signature :
                std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > findRouteLanes(ad::map::point::ECEFPoint,ad::map::route::FullRoute)
        '''
        ...

    def getLaneENUHeading(self, arg1: AdMapMatching, mapMatchedPosition: MapMatchedPosition) -> ENUHeading:
        '''

        getLaneENUHeading( (AdMapMatching)arg1, (MapMatchedPosition)mapMatchedPosition) -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading getLaneENUHeading(ad::map::match::AdMapMatching {lvalue},ad::map::match::MapMatchedPosition)
        '''
        ...

    def getLaneOccupiedRegions(self, arg1: AdMapMatching, enuObjectPositionList: ENUObjectPositionList, samplingDistance: Distance) -> LaneOccupiedRegionList:
        '''

        getLaneOccupiedRegions( (AdMapMatching)arg1, (ENUObjectPositionList)enuObjectPositionList [, (Distance)samplingDistance=<physics.Distance object at 0x79ee292a4120>]) -> LaneOccupiedRegionList :

            C++ signature :
                std::vector<ad::map::match::LaneOccupiedRegion, std::allocator<ad::map::match::LaneOccupiedRegion> > getLaneOccupiedRegions(ad::map::match::AdMapMatching {lvalue},std::vector<ad::map::match::ENUObjectPosition, std::allocator<ad::map::match::ENUObjectPosition> > [,ad::physics::Distance=<physics.Distance object at 0x79ee292a4120>])
        '''
        ...

    def getMapMatchedBoundingBox(self, arg1: AdMapMatching, enuObjectPosition: ENUObjectPosition, samplingDistance: Distance) -> MapMatchedObjectBoundingBox:
        '''

        getMapMatchedBoundingBox( (AdMapMatching)arg1, (ENUObjectPosition)enuObjectPosition [, (Distance)samplingDistance=<physics.Distance object at 0x79ee292a4190>]) -> MapMatchedObjectBoundingBox :

            C++ signature :
                ad::map::match::MapMatchedObjectBoundingBox getMapMatchedBoundingBox(ad::map::match::AdMapMatching {lvalue},ad::map::match::ENUObjectPosition [,ad::physics::Distance=<physics.Distance object at 0x79ee292a4190>])
        '''
        ...

    def getMapMatchedPositions(self, arg1: AdMapMatching, geoPoint: GeoPoint, distance: Distance, minProbability: Probabilityarg1: AdMapMatching, enuPoint: ENUPoint, enuReferencePoint: GeoPoint, distance: Distance, minProbability: Probabilityarg1: AdMapMatching, enuPoint: ENUPoint, distance: Distance, minProbability: Probabilityarg1: AdMapMatching, enuObjectPosition: ENUObjectPosition, distance: Distance, minProbability: Probability) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_:
        '''

        getMapMatchedPositions( (AdMapMatching)arg1, (GeoPoint)geoPoint, (Distance)distance, (Probability)minProbability) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_ :

            C++ signature :
                std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > getMapMatchedPositions(ad::map::match::AdMapMatching {lvalue},ad::map::point::GeoPoint,ad::physics::Distance,ad::physics::Probability)

        getMapMatchedPositions( (AdMapMatching)arg1, (ENUPoint)enuPoint, (GeoPoint)enuReferencePoint, (Distance)distance, (Probability)minProbability) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_ :

            C++ signature :
                std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > getMapMatchedPositions(ad::map::match::AdMapMatching {lvalue},ad::map::point::ENUPoint,ad::map::point::GeoPoint,ad::physics::Distance,ad::physics::Probability)

        getMapMatchedPositions( (AdMapMatching)arg1, (ENUPoint)enuPoint, (Distance)distance, (Probability)minProbability) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_ :

            C++ signature :
                std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > getMapMatchedPositions(ad::map::match::AdMapMatching {lvalue},ad::map::point::ENUPoint,ad::physics::Distance,ad::physics::Probability)

        getMapMatchedPositions( (AdMapMatching)arg1, (ENUObjectPosition)enuObjectPosition, (Distance)distance, (Probability)minProbability) -> vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_ :

            C++ signature :
                std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > getMapMatchedPositions(ad::map::match::AdMapMatching {lvalue},ad::map::match::ENUObjectPosition,ad::physics::Distance,ad::physics::Probability)
        '''
        ...

    def setRelevantLanes(self, arg1: AdMapMatching, relevantLanes: LaneIdSet) -> None:
        '''

        setRelevantLanes( (AdMapMatching)arg1, (LaneIdSet)relevantLanes) -> None :

            C++ signature :
                void setRelevantLanes(ad::map::match::AdMapMatching {lvalue},std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >)
        '''
        ...

class ENUObjectPosition():
    def assign(self, arg1: ENUObjectPosition, other: ENUObjectPosition) -> ENUObjectPosition:
        '''

        assign( (ENUObjectPosition)arg1, (ENUObjectPosition)other) -> ENUObjectPosition :

            C++ signature :
                ad::map::match::ENUObjectPosition {lvalue} assign(ad::map::match::ENUObjectPosition {lvalue},ad::map::match::ENUObjectPosition)
        '''
        ...

    @property
    def centerPoint(self) -> point.ENUPoint: ...

    @property
    def dimension(self) -> ad.physics.Dimension3D: ...

    @property
    def enuReferencePoint(self) -> point.GeoPoint: ...

    @property
    def heading(self) -> point.ENUHeading: ...

class ENUObjectPositionList():
    def append(self, arg1: ENUObjectPositionList, arg2: ENUObjectPosition) -> None:
        '''

        append( (ENUObjectPositionList)arg1, (ENUObjectPosition)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::match::ENUObjectPosition, std::allocator<ad::map::match::ENUObjectPosition> > {lvalue},ad::map::match::ENUObjectPosition)
        '''
        ...

    def count(self, arg1: ENUObjectPositionList, arg2: ENUObjectPosition) -> int:
        '''

        count( (ENUObjectPositionList)arg1, (ENUObjectPosition)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::match::ENUObjectPosition, std::allocator<ad::map::match::ENUObjectPosition> > {lvalue},ad::map::match::ENUObjectPosition)
        '''
        ...

    def extend(self, arg1: ENUObjectPositionList, arg2: object) -> None:
        '''

        extend( (ENUObjectPositionList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::match::ENUObjectPosition, std::allocator<ad::map::match::ENUObjectPosition> > {lvalue},boost::python::api::object)
        '''
        ...

    def index(self, arg1: ENUObjectPositionList, arg2: ENUObjectPosition) -> int:
        '''

        index( (ENUObjectPositionList)arg1, (ENUObjectPosition)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::match::ENUObjectPosition, std::allocator<ad::map::match::ENUObjectPosition> > {lvalue},ad::map::match::ENUObjectPosition)
        '''
        ...

    def insert(self, arg1: ENUObjectPositionList, arg2: int, arg3: ENUObjectPosition) -> None:
        '''

        insert( (ENUObjectPositionList)arg1, (int)arg2, (ENUObjectPosition)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::match::ENUObjectPosition, std::allocator<ad::map::match::ENUObjectPosition> > {lvalue},long,ad::map::match::ENUObjectPosition)
        '''
        ...

    def reverse(self, arg1: ENUObjectPositionList) -> None:
        '''

        reverse( (ENUObjectPositionList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::match::ENUObjectPosition, std::allocator<ad::map::match::ENUObjectPosition> > {lvalue})
        '''
        ...

class LaneOccupiedRegion():
    def assign(self, arg1: LaneOccupiedRegion, other: LaneOccupiedRegion) -> LaneOccupiedRegion:
        '''

        assign( (LaneOccupiedRegion)arg1, (LaneOccupiedRegion)other) -> LaneOccupiedRegion :

            C++ signature :
                ad::map::match::LaneOccupiedRegion {lvalue} assign(ad::map::match::LaneOccupiedRegion {lvalue},ad::map::match::LaneOccupiedRegion)
        '''
        ...

    @property
    def laneId(self) -> lane.LaneId: ...

    @property
    def lateralRange(self) -> ad.physics.ParametricRange: ...

    @property
    def longitudinalRange(self) -> ad.physics.ParametricRange: ...

class LaneOccupiedRegionList():
    def append(self, arg1: LaneOccupiedRegionList, arg2: LaneOccupiedRegion) -> None:
        '''

        append( (LaneOccupiedRegionList)arg1, (LaneOccupiedRegion)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::match::LaneOccupiedRegion, std::allocator<ad::map::match::LaneOccupiedRegion> > {lvalue},ad::map::match::LaneOccupiedRegion)
        '''
        ...

    def count(self, arg1: LaneOccupiedRegionList, arg2: LaneOccupiedRegion) -> int:
        '''

        count( (LaneOccupiedRegionList)arg1, (LaneOccupiedRegion)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::match::LaneOccupiedRegion, std::allocator<ad::map::match::LaneOccupiedRegion> > {lvalue},ad::map::match::LaneOccupiedRegion)
        '''
        ...

    def extend(self, arg1: LaneOccupiedRegionList, arg2: object) -> None:
        '''

        extend( (LaneOccupiedRegionList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::match::LaneOccupiedRegion, std::allocator<ad::map::match::LaneOccupiedRegion> > {lvalue},boost::python::api::object)
        '''
        ...

    def index(self, arg1: LaneOccupiedRegionList, arg2: LaneOccupiedRegion) -> int:
        '''

        index( (LaneOccupiedRegionList)arg1, (LaneOccupiedRegion)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::match::LaneOccupiedRegion, std::allocator<ad::map::match::LaneOccupiedRegion> > {lvalue},ad::map::match::LaneOccupiedRegion)
        '''
        ...

    def insert(self, arg1: LaneOccupiedRegionList, arg2: int, arg3: LaneOccupiedRegion) -> None:
        '''

        insert( (LaneOccupiedRegionList)arg1, (int)arg2, (LaneOccupiedRegion)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::match::LaneOccupiedRegion, std::allocator<ad::map::match::LaneOccupiedRegion> > {lvalue},long,ad::map::match::LaneOccupiedRegion)
        '''
        ...

    def reverse(self, arg1: LaneOccupiedRegionList) -> None:
        '''

        reverse( (LaneOccupiedRegionList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::match::LaneOccupiedRegion, std::allocator<ad::map::match::LaneOccupiedRegion> > {lvalue})
        '''
        ...

    def __getitem__(self, key: int) -> LaneOccupiedRegion: ...

    def __contains__(self, item: LaneOccupiedRegion) -> bool: ...

    def __delitem__(self, key: int) -> None: ...

    def __hash__(self) -> int: ...

    def __setitem__(self, key: int, value: LaneOccupiedRegion) -> None: ...

class LanePoint():
    def assign(self, arg1: LanePoint, other: LanePoint) -> LanePoint:
        '''

        assign( (LanePoint)arg1, (LanePoint)other) -> LanePoint :

            C++ signature :
                ad::map::match::LanePoint {lvalue} assign(ad::map::match::LanePoint {lvalue},ad::map::match::LanePoint)
        '''
        ...

    @property
    def laneLength(self) -> ad.physics.Distance: ...

    @property
    def laneWidth(self) -> ad.physics.Distance: ...

    @property
    def lateralT(self) -> ad.physics.RatioValue: ...

    @property
    def paraPoint(self) -> point.ParaPoint: ...

class MapMatchedObjectBoundingBox():
    def assign(self, arg1: MapMatchedObjectBoundingBox, other: MapMatchedObjectBoundingBox) -> MapMatchedObjectBoundingBox:
        '''

        assign( (MapMatchedObjectBoundingBox)arg1, (MapMatchedObjectBoundingBox)other) -> MapMatchedObjectBoundingBox :

            C++ signature :
                ad::map::match::MapMatchedObjectBoundingBox {lvalue} assign(ad::map::match::MapMatchedObjectBoundingBox {lvalue},ad::map::match::MapMatchedObjectBoundingBox)
        '''
        ...

    @property
    def laneOccupiedRegions(self) -> LaneOccupiedRegionList: ...

    @property
    def matchRadius(self) -> ad.physics.Distance: ...

    @property
    def referencePointPositions(self) -> MapMatchedObjectReferencePositionList: ...

    @property
    def samplingDistance(self) -> ad.physics.Distance: ...

class MapMatchedObjectReferencePositionList():
    def append(self, arg1: MapMatchedObjectReferencePositionList, arg2: vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_) -> None:
        '''

        append( (MapMatchedObjectReferencePositionList)arg1, (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)arg2) -> None :

            C++ signature :
                void append(std::vector<std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >, std::allocator<std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > > > {lvalue},std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >)
        '''
        ...

    def extend(self, arg1: MapMatchedObjectReferencePositionList, arg2: object) -> None:
        '''

        extend( (MapMatchedObjectReferencePositionList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >, std::allocator<std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > > > {lvalue},boost::python::api::object)
        '''
        ...

    def insert(self, arg1: MapMatchedObjectReferencePositionList, arg2: int, arg3: vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_) -> None:
        '''

        insert( (MapMatchedObjectReferencePositionList)arg1, (int)arg2, (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)arg3) -> None :

            C++ signature :
                void insert(std::vector<std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >, std::allocator<std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > > > {lvalue},long,std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >)
        '''
        ...

    def reverse(self, arg1: MapMatchedObjectReferencePositionList) -> None:
        '''

        reverse( (MapMatchedObjectReferencePositionList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> >, std::allocator<std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > > > {lvalue})
        '''
        ...

class MapMatchedPosition():
    def assign(self, arg1: MapMatchedPosition, other: MapMatchedPosition) -> MapMatchedPosition:
        '''

        assign( (MapMatchedPosition)arg1, (MapMatchedPosition)other) -> MapMatchedPosition :

            C++ signature :
                ad::map::match::MapMatchedPosition {lvalue} assign(ad::map::match::MapMatchedPosition {lvalue},ad::map::match::MapMatchedPosition)
        '''
        ...

    @property
    def lanePoint(self) -> LanePoint: ...

    @property
    def matchedPoint(self) -> point.ECEFPoint: ...

    @property
    def matchedPointDistance(self) -> ad.physics.Distance: ...

    @property
    def probability(self) -> ad.physics.Probability: ...

    @property
    def queryPoint(self) -> point.ECEFPoint: ...

    @property
    def type(self) -> MapMatchedPositionType: ...

class MapMatchedPositionType(int,):
    INVALID = 0

    LANE_IN = 2

    LANE_LEFT = 3

    LANE_RIGHT = 4

    UNKNOWN = 1

class Object():
    def assign(self, arg1: Object, other: Object) -> Object:
        '''

        assign( (Object)arg1, (Object)other) -> Object :

            C++ signature :
                ad::map::match::Object {lvalue} assign(ad::map::match::Object {lvalue},ad::map::match::Object)
        '''
        ...

    @property
    def enuPosition(self) -> ENUObjectPosition: ...

    @property
    def mapMatchedBoundingBox(self) -> MapMatchedObjectBoundingBox: ...

class ObjectReferencePoints(int,):
    Center = 4

    FrontLeft = 0

    FrontRight = 1

    NumPoints = 5

    RearLeft = 2

    RearRight = 3

class vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_():
    def append(self, arg1: vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_, arg2: MapMatchedPosition) -> None:
        '''

        append( (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)arg1, (MapMatchedPosition)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > {lvalue},ad::map::match::MapMatchedPosition)
        '''
        ...

    def count(self, arg1: vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_, arg2: MapMatchedPosition) -> int:
        '''

        count( (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)arg1, (MapMatchedPosition)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > {lvalue},ad::map::match::MapMatchedPosition)
        '''
        ...

    def extend(self, arg1: vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_, arg2: object) -> None:
        '''

        extend( (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > {lvalue},boost::python::api::object)
        '''
        ...

    def index(self, arg1: vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_, arg2: MapMatchedPosition) -> int:
        '''

        index( (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)arg1, (MapMatchedPosition)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > {lvalue},ad::map::match::MapMatchedPosition)
        '''
        ...

    def insert(self, arg1: vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_, arg2: int, arg3: MapMatchedPosition) -> None:
        '''

        insert( (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)arg1, (int)arg2, (MapMatchedPosition)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > {lvalue},long,ad::map::match::MapMatchedPosition)
        '''
        ...

    def reverse(self, arg1: vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_) -> None:
        '''

        reverse( (vector_less_ad_scope_map_scope_match_scope_MapMatchedPosition_greater_)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::match::MapMatchedPosition, std::allocator<ad::map::match::MapMatchedPosition> > {lvalue})
        '''
        ...

