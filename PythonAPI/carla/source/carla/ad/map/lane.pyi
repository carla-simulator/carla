from typing import overload
import ad

from . import *

# TODO constants and functions

def getLane(laneId: LaneId) -> Lane:
    """
    getLane( (LaneId)laneId) -> Lane :

        C++ signature :
            ad::map::lane::Lane getLane(ad::map::lane::LaneId)
    """
    ...

class ContactLane:
    def assign(self, arg1: ContactLane, other: ContactLane) -> ContactLane:
        """

        assign( (ContactLane)arg1, (ContactLane)other) -> ContactLane :

            C++ signature :
                ad::map::lane::ContactLane {lvalue} assign(ad::map::lane::ContactLane {lvalue},ad::map::lane::ContactLane)
        """
        ...

    @property
    def location(self) -> ContactLocation: ...

    @property
    def restrictions(self) -> restriction.Restrictions: ...

    @property
    def toLane(self) -> LaneId: ...

    @property
    def trafficLightId(self) -> landmark.LandmarkId: ...

    @property
    def types(self) -> ContactTypeList: ...

class ContactLaneList:
    def append(self, arg1: ContactLaneList, arg2: ContactLane) -> None:
        """

        append( (ContactLaneList)arg1, (ContactLane)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::lane::ContactLane, std::allocator<ad::map::lane::ContactLane> > {lvalue},ad::map::lane::ContactLane)
        """
        ...

    def count(self, arg1: ContactLaneList, arg2: ContactLane) -> int:
        """

        count( (ContactLaneList)arg1, (ContactLane)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::lane::ContactLane, std::allocator<ad::map::lane::ContactLane> > {lvalue},ad::map::lane::ContactLane)
        """
        ...

    def extend(self, arg1: ContactLaneList, arg2: object) -> None:
        """

        extend( (ContactLaneList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::lane::ContactLane, std::allocator<ad::map::lane::ContactLane> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ContactLaneList, arg2: ContactLane) -> int:
        """

        index( (ContactLaneList)arg1, (ContactLane)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::lane::ContactLane, std::allocator<ad::map::lane::ContactLane> > {lvalue},ad::map::lane::ContactLane)
        """
        ...

    def insert(self, arg1: ContactLaneList, arg2: int, arg3: ContactLane) -> None:
        """

        insert( (ContactLaneList)arg1, (int)arg2, (ContactLane)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::lane::ContactLane, std::allocator<ad::map::lane::ContactLane> > {lvalue},long,ad::map::lane::ContactLane)
        """
        ...

    def reverse(self, arg1: ContactLaneList) -> None:
        """

        reverse( (ContactLaneList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::lane::ContactLane, std::allocator<ad::map::lane::ContactLane> > {lvalue})
        """
        ...

class ContactLocation(int):
    INVALID = 0

    LEFT = 2

    OVERLAP = 6

    PREDECESSOR = 5

    RIGHT = 3

    SUCCESSOR = 4

    UNKNOWN = 1

class ContactLocationList:
    def append(self, arg1: ContactLocationList, arg2: ContactLocation) -> None:
        """

        append( (ContactLocationList)arg1, (ContactLocation)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::lane::ContactLocation, std::allocator<ad::map::lane::ContactLocation> > {lvalue},ad::map::lane::ContactLocation)
        """
        ...

    def count(self, arg1: ContactLocationList, arg2: ContactLocation) -> int:
        """

        count( (ContactLocationList)arg1, (ContactLocation)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::lane::ContactLocation, std::allocator<ad::map::lane::ContactLocation> > {lvalue},ad::map::lane::ContactLocation)
        """
        ...

    def extend(self, arg1: ContactLocationList, arg2: object) -> None:
        """

        extend( (ContactLocationList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::lane::ContactLocation, std::allocator<ad::map::lane::ContactLocation> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ContactLocationList, arg2: ContactLocation) -> int:
        """

        index( (ContactLocationList)arg1, (ContactLocation)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::lane::ContactLocation, std::allocator<ad::map::lane::ContactLocation> > {lvalue},ad::map::lane::ContactLocation)
        """
        ...

    def insert(self, arg1: ContactLocationList, arg2: int, arg3: ContactLocation) -> None:
        """

        insert( (ContactLocationList)arg1, (int)arg2, (ContactLocation)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::lane::ContactLocation, std::allocator<ad::map::lane::ContactLocation> > {lvalue},long,ad::map::lane::ContactLocation)
        """
        ...

    def reverse(self, arg1: ContactLocationList) -> None:
        """

        reverse( (ContactLocationList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::lane::ContactLocation, std::allocator<ad::map::lane::ContactLocation> > {lvalue})
        """
        ...

    def sort(self, arg1: ContactLocationList) -> None:
        """

        sort( (ContactLocationList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::map::lane::ContactLocation, std::allocator<ad::map::lane::ContactLocation> > {lvalue})
        """
        ...

class ContactType(int):
    CROSSWALK = 18

    CURB_DOWN = 15

    CURB_UP = 14

    FREE = 2

    GATE_BARRIER = 10

    GATE_SPIKES = 12

    GATE_SPIKES_CONTRA = 13

    GATE_TOLBOOTH = 11

    INVALID = 0

    LANE_CHANGE = 3

    LANE_CONTINUATION = 4

    LANE_END = 5

    PRIO_TO_RIGHT = 19

    PRIO_TO_RIGHT_AND_STRAIGHT = 21

    RIGHT_OF_WAY = 20

    SINGLE_POINT = 6

    SPEED_BUMP = 16

    STOP = 7

    STOP_ALL = 8

    TRAFFIC_LIGHT = 17

    UNKNOWN = 1

    YIELD = 9

class ContactTypeList:
    def append(self, arg1: ContactTypeList, arg2: ContactType) -> None:
        """

        append( (ContactTypeList)arg1, (ContactType)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::lane::ContactType, std::allocator<ad::map::lane::ContactType> > {lvalue},ad::map::lane::ContactType)
        """
        ...

    def count(self, arg1: ContactTypeList, arg2: ContactType) -> int:
        """

        count( (ContactTypeList)arg1, (ContactType)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::lane::ContactType, std::allocator<ad::map::lane::ContactType> > {lvalue},ad::map::lane::ContactType)
        """
        ...

    def extend(self, arg1: ContactTypeList, arg2: object) -> None:
        """

        extend( (ContactTypeList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::lane::ContactType, std::allocator<ad::map::lane::ContactType> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ContactTypeList, arg2: ContactType) -> int:
        """

        index( (ContactTypeList)arg1, (ContactType)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::lane::ContactType, std::allocator<ad::map::lane::ContactType> > {lvalue},ad::map::lane::ContactType)
        """
        ...

    def insert(self, arg1: ContactTypeList, arg2: int, arg3: ContactType) -> None:
        """

        insert( (ContactTypeList)arg1, (int)arg2, (ContactType)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::lane::ContactType, std::allocator<ad::map::lane::ContactType> > {lvalue},long,ad::map::lane::ContactType)
        """
        ...

    def reverse(self, arg1: ContactTypeList) -> None:
        """

        reverse( (ContactTypeList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::lane::ContactType, std::allocator<ad::map::lane::ContactType> > {lvalue})
        """
        ...

    def sort(self, arg1: ContactTypeList) -> None:
        """

        sort( (ContactTypeList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::map::lane::ContactType, std::allocator<ad::map::lane::ContactType> > {lvalue})
        """
        ...

class ECEFBorder:
    def assign(self, arg1: ECEFBorder, other: ECEFBorder) -> ECEFBorder:
        """

        assign( (ECEFBorder)arg1, (ECEFBorder)other) -> ECEFBorder :

            C++ signature :
                ad::map::lane::ECEFBorder {lvalue} assign(ad::map::lane::ECEFBorder {lvalue},ad::map::lane::ECEFBorder)
        """
        ...

    @property
    def left(self) -> point.ECEFEdge: ...

    @property
    def right(self) -> point.ECEFEdge: ...

class ECEFBorderList:
    def append(self, arg1: ECEFBorderList, arg2: ECEFBorder) -> None:
        """

        append( (ECEFBorderList)arg1, (ECEFBorder)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::lane::ECEFBorder, std::allocator<ad::map::lane::ECEFBorder> > {lvalue},ad::map::lane::ECEFBorder)
        """
        ...

    def count(self, arg1: ECEFBorderList, arg2: ECEFBorder) -> int:
        """

        count( (ECEFBorderList)arg1, (ECEFBorder)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::lane::ECEFBorder, std::allocator<ad::map::lane::ECEFBorder> > {lvalue},ad::map::lane::ECEFBorder)
        """
        ...

    def extend(self, arg1: ECEFBorderList, arg2: object) -> None:
        """

        extend( (ECEFBorderList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::lane::ECEFBorder, std::allocator<ad::map::lane::ECEFBorder> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ECEFBorderList, arg2: ECEFBorder) -> int:
        """

        index( (ECEFBorderList)arg1, (ECEFBorder)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::lane::ECEFBorder, std::allocator<ad::map::lane::ECEFBorder> > {lvalue},ad::map::lane::ECEFBorder)
        """
        ...

    def insert(self, arg1: ECEFBorderList, arg2: int, arg3: ECEFBorder) -> None:
        """

        insert( (ECEFBorderList)arg1, (int)arg2, (ECEFBorder)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::lane::ECEFBorder, std::allocator<ad::map::lane::ECEFBorder> > {lvalue},long,ad::map::lane::ECEFBorder)
        """
        ...

    def reverse(self, arg1: ECEFBorderList) -> None:
        """

        reverse( (ECEFBorderList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::lane::ECEFBorder, std::allocator<ad::map::lane::ECEFBorder> > {lvalue})
        """
        ...

class ENUBorder:
    def assign(self, arg1: ENUBorder, other: ENUBorder) -> ENUBorder:
        """

        assign( (ENUBorder)arg1, (ENUBorder)other) -> ENUBorder :

            C++ signature :
                ad::map::lane::ENUBorder {lvalue} assign(ad::map::lane::ENUBorder {lvalue},ad::map::lane::ENUBorder)
        """
        ...

    @property
    def left(self) -> point.ENUEdge: ...

    @property
    def right(self) -> point.ENUEdge: ...

class ENUBorderList:
    def append(self, arg1: ENUBorderList, arg2: ENUBorder) -> None:
        """

        append( (ENUBorderList)arg1, (ENUBorder)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> > {lvalue},ad::map::lane::ENUBorder)
        """
        ...

    def count(self, arg1: ENUBorderList, arg2: ENUBorder) -> int:
        """

        count( (ENUBorderList)arg1, (ENUBorder)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> > {lvalue},ad::map::lane::ENUBorder)
        """
        ...

    def extend(self, arg1: ENUBorderList, arg2: object) -> None:
        """

        extend( (ENUBorderList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ENUBorderList, arg2: ENUBorder) -> int:
        """

        index( (ENUBorderList)arg1, (ENUBorder)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> > {lvalue},ad::map::lane::ENUBorder)
        """
        ...

    def insert(self, arg1: ENUBorderList, arg2: int, arg3: ENUBorder) -> None:
        """

        insert( (ENUBorderList)arg1, (int)arg2, (ENUBorder)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> > {lvalue},long,ad::map::lane::ENUBorder)
        """
        ...

    def reverse(self, arg1: ENUBorderList) -> None:
        """

        reverse( (ENUBorderList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> > {lvalue})
        """
        ...

class GeoBorder:
    def assign(self, arg1: GeoBorder, other: GeoBorder) -> GeoBorder:
        """

        assign( (GeoBorder)arg1, (GeoBorder)other) -> GeoBorder :

            C++ signature :
                ad::map::lane::GeoBorder {lvalue} assign(ad::map::lane::GeoBorder {lvalue},ad::map::lane::GeoBorder)
        """
        ...

    @property
    def left(self) -> point.GeoEdge: ...

    @property
    def right(self) -> point.GeoEdge: ...

class GeoBorderList:
    def append(self, arg1: GeoBorderList, arg2: GeoBorder) -> None:
        """

        append( (GeoBorderList)arg1, (GeoBorder)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::lane::GeoBorder, std::allocator<ad::map::lane::GeoBorder> > {lvalue},ad::map::lane::GeoBorder)
        """
        ...

    def count(self, arg1: GeoBorderList, arg2: GeoBorder) -> int:
        """

        count( (GeoBorderList)arg1, (GeoBorder)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::lane::GeoBorder, std::allocator<ad::map::lane::GeoBorder> > {lvalue},ad::map::lane::GeoBorder)
        """
        ...

    def extend(self, arg1: GeoBorderList, arg2: object) -> None:
        """

        extend( (GeoBorderList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::lane::GeoBorder, std::allocator<ad::map::lane::GeoBorder> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: GeoBorderList, arg2: GeoBorder) -> int:
        """

        index( (GeoBorderList)arg1, (GeoBorder)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::lane::GeoBorder, std::allocator<ad::map::lane::GeoBorder> > {lvalue},ad::map::lane::GeoBorder)
        """
        ...

    def insert(self, arg1: GeoBorderList, arg2: int, arg3: GeoBorder) -> None:
        """

        insert( (GeoBorderList)arg1, (int)arg2, (GeoBorder)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::lane::GeoBorder, std::allocator<ad::map::lane::GeoBorder> > {lvalue},long,ad::map::lane::GeoBorder)
        """
        ...

    def reverse(self, arg1: GeoBorderList) -> None:
        """

        reverse( (GeoBorderList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::lane::GeoBorder, std::allocator<ad::map::lane::GeoBorder> > {lvalue})
        """
        ...

class Lane:
    def assign(self, arg1: Lane, other: Lane) -> Lane:
        """

        assign( (Lane)arg1, (Lane)other) -> Lane :

            C++ signature :
                ad::map::lane::Lane {lvalue} assign(ad::map::lane::Lane {lvalue},ad::map::lane::Lane)
        """
        ...

    @property
    def boundingSphere(self) -> point.BoundingSphere: ...

    @property
    def complianceVersion(self) -> int: ...

    @property
    def contactLanes(self) -> ContactLaneList: ...

    @property
    def direction(self) -> LaneDirection: ...

    @property
    def edgeLeft(self) -> point.Geometry: ...

    @property
    def edgeRight(self) -> point.Geometry: ...

    @property
    def id(self) -> LaneId: ...

    @property
    def length(self) -> ad.physics.Distance: ...

    @property
    def lengthRange(self) -> ad.physics.MetricRange: ...

    @property
    def restrictions(self) -> restriction.Restrictions: ...

    @property
    def speedLimits(self) -> restriction.SpeedLimitList: ...

    @property
    def type(self) -> LaneType: ...

    @property
    def visibleLandmarks(self) -> landmark.LandmarkIdList: ...

    @property
    def width(self) -> ad.physics.Distance: ...

    @property
    def widthRange(self) -> ad.physics.MetricRange: ...

class LaneAltitudeRange:
    @property
    def maximum(self) -> point.Altitude: ...

    @property
    def minimum(self) -> point.Altitude: ...

class LaneDirection(int):
    BIDIRECTIONAL = 5

    INVALID = 0

    NEGATIVE = 3

    NONE = 6

    POSITIVE = 2

    REVERSABLE = 4

    UNKNOWN = 1

class LaneId:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: LaneId, other: LaneId) -> LaneId:
        """

        assign( (LaneId)arg1, (LaneId)other) -> LaneId :

            C++ signature :
                ad::map::lane::LaneId {lvalue} assign(ad::map::lane::LaneId {lvalue},ad::map::lane::LaneId)
        """
        ...

    cMaxValue: float = 18446744073709551615

    cMinValue: float = 0

    def ensureValid(self, arg1: LaneId) -> None:
        """

        ensureValid( (LaneId)arg1) -> None :

            C++ signature :
                void ensureValid(ad::map::lane::LaneId {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: LaneId) -> None:
        """

        ensureValidNonZero( (LaneId)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::map::lane::LaneId {lvalue})
        """
        ...

    def getMax(self) -> LaneId:
        """

        getMax() -> LaneId :

            C++ signature :
                ad::map::lane::LaneId getMax()
        """
        ...

    def getMin(self) -> LaneId:
        """

        getMin() -> LaneId :

            C++ signature :
                ad::map::lane::LaneId getMin()
        """
        ...

class LaneIdList:
    def append(self, arg1: LaneIdList, arg2: LaneId) -> None:
        """

        append( (LaneIdList)arg1, (LaneId)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> > {lvalue},ad::map::lane::LaneId)
        """
        ...

    def count(self, arg1: LaneIdList, arg2: LaneId) -> int:
        """

        count( (LaneIdList)arg1, (LaneId)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> > {lvalue},ad::map::lane::LaneId)
        """
        ...

    def extend(self, arg1: LaneIdList, arg2: object) -> None:
        """

        extend( (LaneIdList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: LaneIdList, arg2: LaneId) -> int:
        """

        index( (LaneIdList)arg1, (LaneId)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> > {lvalue},ad::map::lane::LaneId)
        """
        ...

    def insert(self, arg1: LaneIdList, arg2: int, arg3: LaneId) -> None:
        """

        insert( (LaneIdList)arg1, (int)arg2, (LaneId)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> > {lvalue},long,ad::map::lane::LaneId)
        """
        ...

    def reverse(self, arg1: LaneIdList) -> None:
        """

        reverse( (LaneIdList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> > {lvalue})
        """
        ...

    def sort(self, arg1: LaneIdList) -> None:
        """

        sort( (LaneIdList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> > {lvalue})
        """
        ...

class LaneIdSet:
    def add(self, arg1: LaneIdSet, arg2: LaneId) -> None:
        """

        add( (LaneIdSet)arg1, (LaneId)arg2) -> None :

            C++ signature :
                void add(std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> > {lvalue},ad::map::lane::LaneId)
        """
        ...

    def count(self, arg1: LaneIdSet, arg2: LaneId) -> int:
        """

        count( (LaneIdSet)arg1, (LaneId)arg2) -> int :

            C++ signature :
                unsigned long count(std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> > {lvalue},ad::map::lane::LaneId)
        """
        ...

    def has_key(self, arg1: LaneIdSet, arg2: LaneId) -> bool:
        """

        has_key( (LaneIdSet)arg1, (LaneId)arg2) -> bool :

            C++ signature :
                bool has_key(std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> > {lvalue},ad::map::lane::LaneId)
        """
        ...

    def insert(self, arg1: LaneIdSet, arg2: LaneId) -> None:
        """

        insert( (LaneIdSet)arg1, (LaneId)arg2) -> None :

            C++ signature :
                void insert(std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> > {lvalue},ad::map::lane::LaneId)
        """
        ...

class LaneType(int):
    BIKE = 10

    EMERGENCY = 5

    INTERSECTION = 3

    INVALID = 0

    MULTI = 6

    NORMAL = 2

    OVERTAKING = 8

    PEDESTRIAN = 7

    SHOULDER = 4

    TURN = 9

    UNKNOWN = 1

class numeric_limits_less__ad_scope_map_scope_lane_scope_LaneId__greater_:
    def epsilon(self) -> LaneId:
        """

        epsilon() -> LaneId :

            C++ signature :
                ad::map::lane::LaneId epsilon()
        """
        ...

    def lowest(self) -> LaneId:
        """

        lowest() -> LaneId :

            C++ signature :
                ad::map::lane::LaneId lowest()
        """
        ...

    def max(self) -> LaneId:
        """

        max() -> LaneId :

            C++ signature :
                ad::map::lane::LaneId max()
        """
        ...

def calcLaneAltitudeRange(lane: Lane) -> LaneAltitudeRange:
    """

        calcLaneAltitudeRange( (Lane)lane) -> LaneAltitudeRange :

            C++ signature :
                ad::map::lane::LaneAltitudeRange calcLaneAltitudeRange(ad::map::lane::Lane)
    """

def calcLength(laneId: LaneId | match.LaneOccupiedRegion | ENUBorder | ECEFBorder | GeoBorder | ENUBorderList | ECEFBorderList | GeoBorderList) -> ad.physics.Distance:
    """

        calcLength( (LaneId)laneId) -> Distance :

            C++ signature :
                ad::physics::Distance calcLength(ad::map::lane::LaneId)

        calcLength( (LaneOccupiedRegion)laneOccupiedRegion) -> Distance :

            C++ signature :
                ad::physics::Distance calcLength(ad::map::match::LaneOccupiedRegion)

        calcLength( (ENUBorder)border) -> Distance :

            C++ signature :
                ad::physics::Distance calcLength(ad::map::lane::ENUBorder)

        calcLength( (ECEFBorder)border) -> Distance :

            C++ signature :
                ad::physics::Distance calcLength(ad::map::lane::ECEFBorder)

        calcLength( (GeoBorder)border) -> Distance :

            C++ signature :
                ad::physics::Distance calcLength(ad::map::lane::GeoBorder)

        calcLength( (ENUBorderList)borderList) -> Distance :

            C++ signature :
                ad::physics::Distance calcLength(std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> >)

        calcLength( (ECEFBorderList)borderList) -> Distance :

            C++ signature :
                ad::physics::Distance calcLength(std::vector<ad::map::lane::ECEFBorder, std::allocator<ad::map::lane::ECEFBorder> >)

        calcLength( (GeoBorderList)borderList) -> Distance :

            C++ signature :
                ad::physics::Distance calcLength(std::vector<ad::map::lane::GeoBorder, std::allocator<ad::map::lane::GeoBorder> >)
    """

def calcWidth(obj: point.ParaPoint | LaneId | ad.physics.ParametricValue | point.ENUPoint | match.LaneOccupiedRegion) -> ad.physics.Distance:
    """

        calcWidth( (ParaPoint)paraPoint) -> Distance :

            C++ signature :
                ad::physics::Distance calcWidth(ad::map::point::ParaPoint)

        calcWidth( (LaneId)laneId, (ParametricValue)longOffset) -> Distance :

            C++ signature :
                ad::physics::Distance calcWidth(ad::map::lane::LaneId,ad::physics::ParametricValue)

        calcWidth( (ENUPoint)enuPoint) -> Distance :

            C++ signature :
                ad::physics::Distance calcWidth(ad::map::point::ENUPoint)

        calcWidth( (LaneOccupiedRegion)laneOccupiedRegion) -> Distance :

            C++ signature :
                ad::physics::Distance calcWidth(ad::map::match::LaneOccupiedRegion)
    """

def findNearestPointOnLane(lane: Lane, pt: point.ECEFPoint, mmpos: match.MapMatchedPosition) -> bool:
    """

        findNearestPointOnLane( (Lane)lane, (ECEFPoint)pt, (MapMatchedPosition)mmpos) -> bool :

            C++ signature :
                bool findNearestPointOnLane(ad::map::lane::Lane,ad::map::point::ECEFPoint,ad::map::match::MapMatchedPosition {lvalue})
    """

def findNearestPointOnLaneInterval(laneInterval: route.LaneInterval, pt: point.ECEFPoint, mmpos: match.MapMatchedPosition) -> bool:
    """

        findNearestPointOnLaneInterval( (LaneInterval)laneInterval, (ECEFPoint)pt, (MapMatchedPosition)mmpos) -> bool :

            C++ signature :
                bool findNearestPointOnLaneInterval(ad::map::route::LaneInterval,ad::map::point::ECEFPoint,ad::map::match::MapMatchedPosition {lvalue})
    """

def fromString(str: str) -> ContactLocation | LaneType | ContactType | LaneDirection:
    """

        fromString( (str)str) -> ContactLocation :

            C++ signature :
                ad::map::lane::ContactLocation fromString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)

        fromString( (str)str) -> LaneType :

            C++ signature :
                ad::map::lane::LaneType fromString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)

        fromString( (str)str) -> ContactType :

            C++ signature :
                ad::map::lane::ContactType fromString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)

        fromString( (str)str) -> LaneDirection :

            C++ signature :
                ad::map::lane::LaneDirection fromString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)
    """

def getContactLanes(lane: Lane, location: ContactLocation | ContactLocationList) -> ContactLaneList:
    """

        getContactLanes( (Lane)lane, (ContactLocation)location) -> ContactLaneList :

            C++ signature :
                std::vector<ad::map::lane::ContactLane, std::allocator<ad::map::lane::ContactLane> > getContactLanes(ad::map::lane::Lane,ad::map::lane::ContactLocation)

        getContactLanes( (Lane)lane, (ContactLocationList)locations) -> ContactLaneList :

            C++ signature :
                std::vector<ad::map::lane::ContactLane, std::allocator<ad::map::lane::ContactLane> > getContactLanes(ad::map::lane::Lane,std::vector<ad::map::lane::ContactLocation, std::allocator<ad::map::lane::ContactLocation> >)
    """

def getContactLocation(lane: Lane, to_lane_id: LaneId) -> ContactLocation:
    """

        getContactLocation( (Lane)lane, (LaneId)to_lane_id) -> ContactLocation :

            C++ signature :
                ad::map::lane::ContactLocation getContactLocation(ad::map::lane::Lane,ad::map::lane::LaneId)
    """

def getDirectNeighborhoodRelation(laneId: LaneId, checkLaneId: LaneId) -> ContactLocation:
    """

        getDirectNeighborhoodRelation( (LaneId)laneId, (LaneId)checkLaneId) -> ContactLocation :

            C++ signature :
                ad::map::lane::ContactLocation getDirectNeighborhoodRelation(ad::map::lane::LaneId,ad::map::lane::LaneId)
    """

def getDistanceEnuPointToLateralAlignmentEdge(enuPoint: point.ENUPoint, lateralAlignmentEdge: point.ENUEdge) -> ad.physics.Distance:
    """

        getDistanceEnuPointToLateralAlignmentEdge( (ENUPoint)enuPoint, (ENUEdge)lateralAlignmentEdge) -> Distance :

            C++ signature :
                ad::physics::Distance getDistanceEnuPointToLateralAlignmentEdge(ad::map::point::ENUPoint,std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> >)
    """

def getDistanceToLane(laneId: LaneId, object: rss.world.Object) -> ad.physics.Distance:
    """

        getDistanceToLane( (LaneId)laneId, (Object)object) -> Distance :

            C++ signature :
                ad::physics::Distance getDistanceToLane(ad::map::lane::LaneId,ad::map::match::Object)
    """

def getDuration(lane: Lane, range: ad.physics.ParametricRange) -> ad.physics.Duration:
    """

        getDuration( (Lane)lane, (ParametricRange)range) -> Duration :

            C++ signature :
                ad::physics::Duration getDuration(ad::map::lane::Lane,ad::physics::ParametricRange)
    """

def getENUHeading(borderList: ENUBorderList, enuPoint: point.ENUPoint) -> point.ENUHeading:
    """

        getENUHeading( (ENUBorderList)borderList, (ENUPoint)enuPoint) -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading getENUHeading(std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> >,ad::map::point::ENUPoint)
    """

def getENULanePoint(parametricPoint: point.ParaPoint, lateralOffset: ad.physics.ParametricValue) -> point.ENUHeading:
    """

        getENULanePoint( (ParaPoint)parametricPoint [, (ParametricValue)lateralOffset=<physics.ParametricValue object at 0x7752c8de28f0>]) -> ENUPoint :

            C++ signature :
                ad::map::point::ENUPoint getENULanePoint(ad::map::point::ParaPoint [,ad::physics::ParametricValue=<physics.ParametricValue object at 0x7752c8de28f0>])
    """

def getEndPoint(lane: Lane) -> point.ECEFPoint:
    """

        getEndPoint( (Lane)lane) -> ECEFPoint :

            C++ signature :
                ad::map::point::ECEFPoint getEndPoint(ad::map::lane::Lane)
    """

def getHOV(lane: Lane) -> int:
    """

        getHOV( (Lane)lane) -> int :

            C++ signature :
                unsigned short getHOV(ad::map::lane::Lane)
    """

def getLane(id: LaneId) -> Lane:
    """

        getLane( (LaneId)id) -> Lane :

            C++ signature :
                ad::map::lane::Lane getLane(ad::map::lane::LaneId)
    """

def getLaneECEFHeading(position: match.MapMatchedPosition | point.ParaPoint, /) -> point.ECEFHeading:
    """

        getLaneECEFHeading( (MapMatchedPosition)mapMatchedPosition) -> ECEFHeading :

            C++ signature :
                ad::map::point::ECEFHeading getLaneECEFHeading(ad::map::match::MapMatchedPosition)

        getLaneECEFHeading( (ParaPoint)paraPoint) -> ECEFHeading :

            C++ signature :
                ad::map::point::ECEFHeading getLaneECEFHeading(ad::map::point::ParaPoint)
    """


@overload
def getLaneENUHeading(position: match.MapMatchedPosition | point.ParaPoint, /):
    """

        getLaneENUHeading( (MapMatchedPosition)mapMatchedPosition) -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading getLaneENUHeading(ad::map::match::MapMatchedPosition)

        getLaneENUHeading( (ParaPoint)paraPoint, (GeoPoint)gnssReference) -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading getLaneENUHeading(ad::map::point::ParaPoint,ad::map::point::GeoPoint)

        getLaneENUHeading( (ParaPoint)position) -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading getLaneENUHeading(ad::map::point::ParaPoint)
    """

@overload
def getLaneENUHeading(paraPoint: point.ParaPoint, gnssReference: point.GeoPoint) -> point.ENUHeading:
    """

        getLaneENUHeading( (MapMatchedPosition)mapMatchedPosition) -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading getLaneENUHeading(ad::map::match::MapMatchedPosition)

        getLaneENUHeading( (ParaPoint)paraPoint, (GeoPoint)gnssReference) -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading getLaneENUHeading(ad::map::point::ParaPoint,ad::map::point::GeoPoint)

        getLaneENUHeading( (ParaPoint)position) -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading getLaneENUHeading(ad::map::point::ParaPoint)
    """

def getLanePtr(id: LaneId) -> Lane:
    """

        getLanePtr( (LaneId)id) -> Lane :

            C++ signature :
                std::shared_ptr<ad::map::lane::Lane const> getLanePtr(ad::map::lane::LaneId)
    """

def getLanes() -> LaneIdList:
    """

        getLanes() -> LaneIdList :

            C++ signature :
                std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> > getLanes()
    """

def getLateralAlignmentEdge(border: ENUBorder, lateralAlignment: ad.physics.ParametricValue) -> point.ENUEdge:
    """

        getLateralAlignmentEdge( (ENUBorder)border, (ParametricValue)lateralAlignment) -> ENUEdge :

            C++ signature :
                std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > getLateralAlignmentEdge(ad::map::lane::ENUBorder,ad::physics::ParametricValue)
    """

def getMaxSpeed(lane: Lane, range: ad.physics.ParametricRange) -> ad.physics.Speed:
    """

        getMaxSpeed( (Lane)lane, (ParametricRange)range) -> Speed :

            C++ signature :
                ad::physics::Speed getMaxSpeed(ad::map::lane::Lane,ad::physics::ParametricRange)
    """

def getParametricPoint(lane: Lane, longitudinalOffset: ad.physics.ParametricValue, lateralOffset: ad.physics.ParametricValue) -> point.ECEFPoint:
    """

        getParametricPoint( (Lane)lane, (ParametricValue)longitudinalOffset, (ParametricValue)lateralOffset) -> ECEFPoint :

            C++ signature :
                ad::map::point::ECEFPoint getParametricPoint(ad::map::lane::Lane,ad::physics::ParametricValue,ad::physics::ParametricValue)
    """

def getProjectedParametricPoint(lane: Lane, longitudinalOffset: ad.physics.ParametricValue, lateralOffset: ad.physics.ParametricValue) -> point.ECEFPoint:
    """

        getProjectedParametricPoint( (Lane)lane, (ParametricValue)longitudinalOffset, (ParametricValue)lateralOffset) -> ECEFPoint :

            C++ signature :
                ad::map::point::ECEFPoint getProjectedParametricPoint(ad::map::lane::Lane,ad::physics::ParametricValue,ad::physics::ParametricValue)
    """

def getSpeedLimits(lane: Lane, range: ad.physics.ParametricRange) -> restriction.SpeedLimitList:
    """

        getSpeedLimits( (Lane)lane, (ParametricRange)range) -> SpeedLimitList :

            C++ signature :
                std::vector<ad::map::restriction::SpeedLimit, std::allocator<ad::map::restriction::SpeedLimit> > getSpeedLimits(ad::map::lane::Lane,ad::physics::ParametricRange)
    """

def getStartPoint(lane: Lane) -> point.ECEFPoint:
    """

        getStartPoint( (Lane)lane) -> ECEFPoint :

            C++ signature :
                ad::map::point::ECEFPoint getStartPoint(ad::map::lane::Lane)
    """

def getWidth(lane: Lane, longitudinalOffset: ad.physics.ParametricValue) -> ad.physics.Distance:
    """

        getWidth( (Lane)lane, (ParametricValue)longitudinalOffset) -> Distance :

            C++ signature :
                ad::physics::Distance getWidth(ad::map::lane::Lane,ad::physics::ParametricValue)
    """

def isAccessOk(lane: Lane | ContactLane, vehicle: restriction.VehicleDescriptor) -> bool:
    """

        isAccessOk( (Lane)lane, (VehicleDescriptor)vehicle) -> bool :

            C++ signature :
                bool isAccessOk(ad::map::lane::Lane,ad::map::restriction::VehicleDescriptor)

        isAccessOk( (ContactLane)contactLane, (VehicleDescriptor)vehicle) -> bool :

            C++ signature :
                bool isAccessOk(ad::map::lane::ContactLane,ad::map::restriction::VehicleDescriptor)
    """

def isHeadingInLaneDirection(position: point.ParaPoint, heading: point.ENUHeading) -> bool:
    """

        isHeadingInLaneDirection( (ParaPoint)position, (ENUHeading)heading) -> bool :

            C++ signature :
                bool isHeadingInLaneDirection(ad::map::point::ParaPoint,ad::map::point::ENUHeading)
    """


def isLaneDirectionNegative(lane: Lane | LaneId) -> bool:
    """

        isLaneDirectionNegative( (Lane)lane) -> bool :

            C++ signature :
                bool isLaneDirectionNegative(ad::map::lane::Lane)

        isLaneDirectionNegative( (LaneId)laneId) -> bool :

            C++ signature :
                bool isLaneDirectionNegative(ad::map::lane::LaneId)
    """

def isLaneDirectionPositive(lane: Lane | LaneId) -> bool:
    """

        isLaneDirectionPositive( (Lane)lane) -> bool :

            C++ signature :
                bool isLaneDirectionPositive(ad::map::lane::Lane)

        isLaneDirectionPositive( (LaneId)laneId) -> bool :

            C++ signature :
                bool isLaneDirectionPositive(ad::map::lane::LaneId)
    """

def isLanePartOfAnIntersection(lane: Lane) -> bool:
    """

        isLanePartOfAnIntersection( (Lane)lane) -> bool :

            C++ signature :
                bool isLanePartOfAnIntersection(ad::map::lane::Lane)
    """

def isLaneRelevantForExpansion(laneId: LaneId, relevantLanes: LaneIdSet) -> bool:
    """

        isLaneRelevantForExpansion( (LaneId)laneId, (LaneIdSet)relevantLanes) -> bool :

            C++ signature :
                bool isLaneRelevantForExpansion(ad::map::lane::LaneId,std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >)
    """

def isLeftMost(lane: Lane) -> bool:
    """

        isLeftMost( (Lane)lane) -> bool :

            C++ signature :
                bool isLeftMost(ad::map::lane::Lane)
    """

def isNear(lane: Lane, boundingSphere: point.BoundingSphere) -> bool:
    """

        isNear( (Lane)lane, (BoundingSphere)boundingSphere) -> bool :

            C++ signature :
                bool isNear(ad::map::lane::Lane,ad::map::point::BoundingSphere)
    """

def isPhysicalPredecessor(lane: Lane, other: Lane) -> bool:
    """

        isPhysicalPredecessor( (Lane)lane, (Lane)other) -> bool :

            C++ signature :
                bool isPhysicalPredecessor(ad::map::lane::Lane,ad::map::lane::Lane)
    """

def isPyhsicalSuccessor(lane: Lane, other: Lane) -> bool:
    """

        isPyhsicalSuccessor( (Lane)lane, (Lane)other) -> bool :

            C++ signature :
                bool isPyhsicalSuccessor(ad::map::lane::Lane,ad::map::lane::Lane)
    """

def isRightMost(lane: Lane) -> bool:
    """

        isRightMost( (Lane)lane) -> bool :

            C++ signature :
                bool isRightMost(ad::map::lane::Lane)
    """

def isRouteable(lane: Lane) -> bool:
    """

        isRouteable( (Lane)lane) -> bool :

            C++ signature :
                bool isRouteable(ad::map::lane::Lane)
    """

def isSameOrDirectNeighbor(id: LaneId, neighbor: LaneId) -> bool:
    """

        isSameOrDirectNeighbor( (LaneId)id, (LaneId)neighbor) -> bool :

            C++ signature :
                bool isSameOrDirectNeighbor(ad::map::lane::LaneId,ad::map::lane::LaneId)
    """

def isSuccessorOrPredecessor(laneId: LaneId, checkLaneId: LaneId) -> bool:
    """

        isSuccessorOrPredecessor( (LaneId)laneId, (LaneId)checkLaneId) -> bool :

            C++ signature :
                bool isSuccessorOrPredecessor(ad::map::lane::LaneId,ad::map::lane::LaneId)
    """

def isValid(laneId: LaneId | Lane | ContactLane, /, logErrors: bool=True) -> bool:
    """

        isValid( (LaneId)laneId [, (bool)logErrors=True]) -> bool :

            C++ signature :
                bool isValid(ad::map::lane::LaneId [,bool=True])

        isValid( (Lane)lane [, (bool)logErrors=True]) -> bool :

            C++ signature :
                bool isValid(ad::map::lane::Lane [,bool=True])

        isValid( (ContactLane)contactLane [, (bool)logErrors=True]) -> bool :

            C++ signature :
                bool isValid(ad::map::lane::ContactLane [,bool=True])
    """

def isVanishingLaneEnd(lane: Lane) -> bool:
    """

        isVanishingLaneEnd( (Lane)lane) -> bool :

            C++ signature :
                bool isVanishingLaneEnd(ad::map::lane::Lane)
    """

def isVanishingLaneStart(lane: Lane) -> bool:
    """

        isVanishingLaneStart( (Lane)lane) -> bool :

            C++ signature :
                bool isVanishingLaneStart(ad::map::lane::Lane)
    """

def makeTransitionFromFirstBorderContinuous(first: ENUBorder, second: ENUBorder) -> None:
    """

        makeTransitionFromFirstBorderContinuous( (ENUBorder)first, (ENUBorder)second) -> None :

            C++ signature :
                void makeTransitionFromFirstBorderContinuous(ad::map::lane::ENUBorder {lvalue},ad::map::lane::ENUBorder)
    """

def makeTransitionFromFirstEdgeContinuous(first: point.ENUEdge, second: point.ENUEdge) -> None:
    """

        makeTransitionFromFirstEdgeContinuous( (ENUEdge)first, (ENUEdge)second) -> None :

            C++ signature :
                void makeTransitionFromFirstEdgeContinuous(std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > {lvalue},std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> >)
    """

def makeTransitionToSecondBorderContinuous(first: ENUBorder, second: ENUBorder) -> None:
    """

        makeTransitionToSecondBorderContinuous( (ENUBorder)first, (ENUBorder)second) -> None :

            C++ signature :
                void makeTransitionToSecondBorderContinuous(ad::map::lane::ENUBorder,ad::map::lane::ENUBorder {lvalue})
    """

def makeTransitionToSecondEdgeContinuous(first: point.ENUEdge, second: point.ENUEdge) -> None:
    """

        makeTransitionToSecondEdgeContinuous( (ENUEdge)first, (ENUEdge)second) -> None :

            C++ signature :
                void makeTransitionToSecondEdgeContinuous(std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> >,std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > {lvalue})
    """

def normalizeBorder(border: ENUBorder, previousBorder: ENUBorder) -> None:
    """

        normalizeBorder( (ENUBorder)border [, (ENUBorder)previousBorder=0]) -> None :

            C++ signature :
                void normalizeBorder(ad::map::lane::ENUBorder {lvalue} [,ad::map::lane::ENUBorder const*=0])
    """

def oppositeLocation(e: ContactLocation) -> ContactLocation:
    """

        oppositeLocation( (ContactLocation)e) -> ContactLocation :

            C++ signature :
                ad::map::lane::ContactLocation oppositeLocation(ad::map::lane::ContactLocation)
    """

def projectParametricPointToEdges(lane: Lane, referencePoint: point.ECEFPoint | ad.physics.ParametricValue, point_on_left_edge: point.ECEFPoint, point_on_right_edge: point.ECEFPoint) -> bool:
    """

        projectParametricPointToEdges( (Lane)lane, (ECEFPoint)referencePoint, (ECEFPoint)point_on_left_edge, (ECEFPoint)point_on_right_edge) -> bool :

            C++ signature :
                bool projectParametricPointToEdges(ad::map::lane::Lane,ad::map::point::ECEFPoint,ad::map::point::ECEFPoint {lvalue},ad::map::point::ECEFPoint {lvalue})

        projectParametricPointToEdges( (Lane)lane, (ParametricValue)longitudinalOffset, (ECEFPoint)point_on_left_edge, (ECEFPoint)point_on_right_edge) -> bool :

            C++ signature :
                bool projectParametricPointToEdges(ad::map::lane::Lane,ad::physics::ParametricValue,ad::map::point::ECEFPoint {lvalue},ad::map::point::ECEFPoint {lvalue})
    """

def projectPositionToLaneInHeadingDirection(position: point.ParaPoint, heading: point.ENUHeading, projectedPosition: point.ParaPoint) -> bool:
    """

        projectPositionToLaneInHeadingDirection( (ParaPoint)position, (ENUHeading)heading, (ParaPoint)projectedPosition) -> bool :

            C++ signature :
                bool projectPositionToLaneInHeadingDirection(ad::map::point::ParaPoint,ad::map::point::ENUHeading,ad::map::point::ParaPoint {lvalue})
    """

def satisfiesFilter(lane: Lane, typeFilter: str, isHov: bool) -> bool:
    """

        satisfiesFilter( (Lane)lane, (str)typeFilter, (bool)isHov) -> bool :

            C++ signature :
                bool satisfiesFilter(ad::map::lane::Lane,std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >,bool)
    """

def toString(e: ContactLocation | LaneType | ContactType | LaneDirection) -> str:
    """

        toString( (ContactLocation)e) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > toString(ad::map::lane::ContactLocation)

        toString( (LaneType)e) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > toString(ad::map::lane::LaneType)

        toString( (ContactType)e) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > toString(ad::map::lane::ContactType)

        toString( (LaneDirection)e) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > toString(ad::map::lane::LaneDirection)
    """

def to_string(value: object) -> str:
    """

        to_string( (LaneId)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::LaneId)

        to_string( (LaneIdList)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::map::lane::LaneId, std::allocator<ad::map::lane::LaneId> >)

        to_string( (ContactLocation)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::ContactLocation)

        to_string( (LaneType)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::LaneType)

        to_string( (ENUBorder)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::ENUBorder)

        to_string( (GeoBorder)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::GeoBorder)

        to_string( (ContactType)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::ContactType)

        to_string( (ContactTypeList)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::map::lane::ContactType, std::allocator<ad::map::lane::ContactType> >)

        to_string( (ContactLane)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::ContactLane)

        to_string( (ContactLaneList)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::map::lane::ContactLane, std::allocator<ad::map::lane::ContactLane> >)

        to_string( (LaneDirection)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::LaneDirection)

        to_string( (Lane)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::Lane)

        to_string( (GeoBorderList)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::map::lane::GeoBorder, std::allocator<ad::map::lane::GeoBorder> >)

        to_string( (ECEFBorder)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::map::lane::ECEFBorder)

        to_string( (ENUBorderList)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::map::lane::ENUBorder, std::allocator<ad::map::lane::ENUBorder> >)

        to_string( (ECEFBorderList)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::map::lane::ECEFBorder, std::allocator<ad::map::lane::ECEFBorder> >)

        to_string( (ContactLocationList)value) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::map::lane::ContactLocation, std::allocator<ad::map::lane::ContactLocation> >)

        to_string( (LaneIdSet)laneIdSet) -> str :

            C++ signature :
                std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::set<ad::map::lane::LaneId, std::less<ad::map::lane::LaneId>, std::allocator<ad::map::lane::LaneId> >)
    """

def uniqueLaneId(point: point.GeoPoint) -> LaneId:
    """

        uniqueLaneId( (GeoPoint)point) -> LaneId :

            C++ signature :
                ad::map::lane::LaneId uniqueLaneId(ad::map::point::GeoPoint)
    """

def uniqueParaPoint(point: point.GeoPoint) -> point.ParaPoint:
    """

        uniqueParaPoint( (GeoPoint)point) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint uniqueParaPoint(ad::map::point::GeoPoint)
    """
