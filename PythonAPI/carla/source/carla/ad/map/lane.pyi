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
