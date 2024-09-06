
class ENULandmark:
    def assign(self, arg1: ENULandmark, other: ENULandmark) -> ENULandmark:
        """

        assign( (ENULandmark)arg1, (ENULandmark)other) -> ENULandmark :

            C++ signature :
                ad::map::landmark::ENULandmark {lvalue} assign(ad::map::landmark::ENULandmark {lvalue},ad::map::landmark::ENULandmark)
        """
        ...

    @property
    def heading(self) -> point.ENUHeading: ...

    @property
    def id(self) -> LandmarkId: ...

    @property
    def position(self) -> point.ENUPoint: ...

    @property
    def trafficLightType(self) -> TrafficLightType: ...

    @property
    def type(self) -> LandmarkType: ...

class ENULandmarkList:
    def append(self, arg1: ENULandmarkList, arg2: ENULandmark) -> None:
        """

        append( (ENULandmarkList)arg1, (ENULandmark)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::landmark::ENULandmark, std::allocator<ad::map::landmark::ENULandmark> > {lvalue},ad::map::landmark::ENULandmark)
        """
        ...

    def count(self, arg1: ENULandmarkList, arg2: ENULandmark) -> int:
        """

        count( (ENULandmarkList)arg1, (ENULandmark)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::landmark::ENULandmark, std::allocator<ad::map::landmark::ENULandmark> > {lvalue},ad::map::landmark::ENULandmark)
        """
        ...

    def extend(self, arg1: ENULandmarkList, arg2: object) -> None:
        """

        extend( (ENULandmarkList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::landmark::ENULandmark, std::allocator<ad::map::landmark::ENULandmark> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ENULandmarkList, arg2: ENULandmark) -> int:
        """

        index( (ENULandmarkList)arg1, (ENULandmark)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::landmark::ENULandmark, std::allocator<ad::map::landmark::ENULandmark> > {lvalue},ad::map::landmark::ENULandmark)
        """
        ...

    def insert(self, arg1: ENULandmarkList, arg2: int, arg3: ENULandmark) -> None:
        """

        insert( (ENULandmarkList)arg1, (int)arg2, (ENULandmark)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::landmark::ENULandmark, std::allocator<ad::map::landmark::ENULandmark> > {lvalue},long,ad::map::landmark::ENULandmark)
        """
        ...

    def reverse(self, arg1: ENULandmarkList) -> None:
        """

        reverse( (ENULandmarkList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::landmark::ENULandmark, std::allocator<ad::map::landmark::ENULandmark> > {lvalue})
        """
        ...

class Landmark:
    def assign(self, arg1: Landmark, other: Landmark) -> Landmark:
        """

        assign( (Landmark)arg1, (Landmark)other) -> Landmark :

            C++ signature :
                ad::map::landmark::Landmark {lvalue} assign(ad::map::landmark::Landmark {lvalue},ad::map::landmark::Landmark)
        """
        ...

    @property
    def boundingBox(self) -> point.Geometry: ...

    @property
    def id(self) -> LandmarkId: ...

    @property
    def orientation(self) -> point.ECEFPoint: ...

    @property
    def position(self) -> point.ECEFPoint: ...

    @property
    def supplementaryText(self) -> str: ...

    @property
    def trafficLightType(self) -> TrafficLightType: ...

    @property
    def trafficSignType(self) -> TrafficSignType: ...

    @property
    def type(self) -> LandmarkType: ...

class LandmarkId:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: LandmarkId, other: LandmarkId) -> LandmarkId:
        """

        assign( (LandmarkId)arg1, (LandmarkId)other) -> LandmarkId :

            C++ signature :
                ad::map::landmark::LandmarkId {lvalue} assign(ad::map::landmark::LandmarkId {lvalue},ad::map::landmark::LandmarkId)
        """
        ...

    cMaxValue: float = 18446744073709551615

    cMinValue: float = 0

    def ensureValid(self, arg1: LandmarkId) -> None:
        """

        ensureValid( (LandmarkId)arg1) -> None :

            C++ signature :
                void ensureValid(ad::map::landmark::LandmarkId {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: LandmarkId) -> None:
        """

        ensureValidNonZero( (LandmarkId)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::map::landmark::LandmarkId {lvalue})
        """
        ...

    def getMax(self) -> LandmarkId:
        """

        getMax() -> LandmarkId :

            C++ signature :
                ad::map::landmark::LandmarkId getMax()
        """
        ...

    def getMin(self) -> LandmarkId:
        """

        getMin() -> LandmarkId :

            C++ signature :
                ad::map::landmark::LandmarkId getMin()
        """
        ...

class LandmarkIdList:
    def append(self, arg1: LandmarkIdList, arg2: LandmarkId) -> None:
        """

        append( (LandmarkIdList)arg1, (LandmarkId)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::landmark::LandmarkId, std::allocator<ad::map::landmark::LandmarkId> > {lvalue},ad::map::landmark::LandmarkId)
        """
        ...

    def count(self, arg1: LandmarkIdList, arg2: LandmarkId) -> int:
        """

        count( (LandmarkIdList)arg1, (LandmarkId)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::landmark::LandmarkId, std::allocator<ad::map::landmark::LandmarkId> > {lvalue},ad::map::landmark::LandmarkId)
        """
        ...

    def extend(self, arg1: LandmarkIdList, arg2: object) -> None:
        """

        extend( (LandmarkIdList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::landmark::LandmarkId, std::allocator<ad::map::landmark::LandmarkId> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: LandmarkIdList, arg2: LandmarkId) -> int:
        """

        index( (LandmarkIdList)arg1, (LandmarkId)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::landmark::LandmarkId, std::allocator<ad::map::landmark::LandmarkId> > {lvalue},ad::map::landmark::LandmarkId)
        """
        ...

    def insert(self, arg1: LandmarkIdList, arg2: int, arg3: LandmarkId) -> None:
        """

        insert( (LandmarkIdList)arg1, (int)arg2, (LandmarkId)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::landmark::LandmarkId, std::allocator<ad::map::landmark::LandmarkId> > {lvalue},long,ad::map::landmark::LandmarkId)
        """
        ...

    def reverse(self, arg1: LandmarkIdList) -> None:
        """

        reverse( (LandmarkIdList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::landmark::LandmarkId, std::allocator<ad::map::landmark::LandmarkId> > {lvalue})
        """
        ...

    def sort(self, arg1: LandmarkIdList) -> None:
        """

        sort( (LandmarkIdList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::map::landmark::LandmarkId, std::allocator<ad::map::landmark::LandmarkId> > {lvalue})
        """
        ...

class LandmarkIdSet:
    def add(self, arg1: LandmarkIdSet, arg2: LandmarkId) -> None:
        """

        add( (LandmarkIdSet)arg1, (LandmarkId)arg2) -> None :

            C++ signature :
                void add(std::set<ad::map::landmark::LandmarkId, std::less<ad::map::landmark::LandmarkId>, std::allocator<ad::map::landmark::LandmarkId> > {lvalue},ad::map::landmark::LandmarkId)
        """
        ...

    def count(self, arg1: LandmarkIdSet, arg2: LandmarkId) -> int:
        """

        count( (LandmarkIdSet)arg1, (LandmarkId)arg2) -> int :

            C++ signature :
                unsigned long count(std::set<ad::map::landmark::LandmarkId, std::less<ad::map::landmark::LandmarkId>, std::allocator<ad::map::landmark::LandmarkId> > {lvalue},ad::map::landmark::LandmarkId)
        """
        ...

    def has_key(self, arg1: LandmarkIdSet, arg2: LandmarkId) -> bool:
        """

        has_key( (LandmarkIdSet)arg1, (LandmarkId)arg2) -> bool :

            C++ signature :
                bool has_key(std::set<ad::map::landmark::LandmarkId, std::less<ad::map::landmark::LandmarkId>, std::allocator<ad::map::landmark::LandmarkId> > {lvalue},ad::map::landmark::LandmarkId)
        """
        ...

    def insert(self, arg1: LandmarkIdSet, arg2: LandmarkId) -> None:
        """

        insert( (LandmarkIdSet)arg1, (LandmarkId)arg2) -> None :

            C++ signature :
                void insert(std::set<ad::map::landmark::LandmarkId, std::less<ad::map::landmark::LandmarkId>, std::allocator<ad::map::landmark::LandmarkId> > {lvalue},ad::map::landmark::LandmarkId)
        """
        ...

class LandmarkType(int):
    BOLLARD = 12

    FIRE_HYDRANT = 11

    GUIDE_POST = 5

    INVALID = 0

    MANHOLE = 9

    OTHER = 13

    POLE = 4

    POSTBOX = 8

    POWERCABINET = 10

    STREET_LAMP = 7

    TRAFFIC_LIGHT = 3

    TRAFFIC_SIGN = 2

    TREE = 6

    UNKNOWN = 1

class TrafficLightType(int):
    BIKE_PEDESTRIAN_RED_GREEN = 11

    BIKE_PEDESTRIAN_RED_YELLOW_GREEN = 14

    BIKE_RED_GREEN = 10

    BIKE_RED_YELLOW_GREEN = 13

    INVALID = 0

    LEFT_RED_YELLOW_GREEN = 4

    LEFT_STRAIGHT_RED_YELLOW_GREEN = 7

    PEDESTRIAN_RED_GREEN = 9

    PEDESTRIAN_RED_YELLOW_GREEN = 12

    RIGHT_RED_YELLOW_GREEN = 5

    RIGHT_STRAIGHT_RED_YELLOW_GREEN = 8

    SOLID_RED_YELLOW = 2

    SOLID_RED_YELLOW_GREEN = 3

    STRAIGHT_RED_YELLOW_GREEN = 6

    UNKNOWN = 1

class TrafficSignType(int):
    ACCESS_FORBIDDEN = 43

    ACCESS_FORBIDDEN_BICYCLE = 45

    ACCESS_FORBIDDEN_HEIGHT = 49

    ACCESS_FORBIDDEN_MOTORVEHICLES = 46

    ACCESS_FORBIDDEN_TRUCKS = 44

    ACCESS_FORBIDDEN_WEIGHT = 47

    ACCESS_FORBIDDEN_WIDTH = 48

    ACCESS_FORBIDDEN_WRONG_DIR = 50

    BYBICLE_PATH = 37

    CAUTION_ANIMALS = 23

    CAUTION_BICYCLE = 22

    CAUTION_CHILDREN = 21

    CAUTION_PEDESTRIAN = 20

    CAUTION_RAIL_CROSSING = 25

    CAUTION_RAIL_CROSSING_WITH_BARRIER = 24

    CITY_BEGIN = 58

    CITY_END = 59

    CUL_DE_SAC = 65

    CUL_DE_SAC_EXCEPT_PED_BICYCLE = 66

    DANGER = 18

    DESTINATION_BOARD = 70

    DIRECTION_TURN_TO_AUTOBAHN = 68

    DIRECTION_TURN_TO_LOCAL = 69

    ENVIORNMENT_ZONE_BEGIN = 51

    ENVIORNMENT_ZONE_END = 52

    FOOTWALK = 38

    FOOTWALK_BICYCLE_SEP_LEFT = 41

    FOOTWALK_BICYCLE_SEP_RIGHT = 40

    FOOTWALK_BICYCLE_SHARED = 39

    FREE_TEXT = 71

    HAS_WAY_NEXT_INTERSECTION = 56

    INFO_MOTORWAY_INFO = 64

    INFO_NUMBER_OF_AUTOBAHN = 67

    INVALID = 0

    LANES_MERGING = 19

    MAX_SPEED = 53

    MOTORVEHICLE_BEGIN = 62

    MOTORVEHICLE_END = 63

    MOTORWAY_BEGIN = 60

    MOTORWAY_END = 61

    PASS_LEFT = 36

    PASS_RIGHT = 35

    PEDESTRIAN_AREA_BEGIN = 42

    PRIORITY_WAY = 57

    REQUIRED_LEFT_TURN = 30

    REQUIRED_RIGHT_TURN = 29

    REQUIRED_STRAIGHT = 31

    REQUIRED_STRAIGHT_OR_LEFT_TURN = 33

    REQUIRED_STRAIGHT_OR_RIGHT_TURN = 32

    ROUNDABOUT = 34

    SPEED_ZONE_30_BEGIN = 54

    SPEED_ZONE_30_END = 55

    STOP = 28

    SUPPLEMENT_APPLIES_FOR_WEIGHT = 17

    SUPPLEMENT_APPLIES_NEXT_N_KM_TIME = 7

    SUPPLEMENT_ARROW_APPLIES_LEFT = 1

    SUPPLEMENT_ARROW_APPLIES_LEFT_RIGHT = 3

    SUPPLEMENT_ARROW_APPLIES_LEFT_RIGHT_BICYCLE = 5

    SUPPLEMENT_ARROW_APPLIES_RIGHT = 2

    SUPPLEMENT_ARROW_APPLIES_UP_DOWN = 4

    SUPPLEMENT_ARROW_APPLIES_UP_DOWN_BICYCLE = 6

    SUPPLEMENT_BICYCLE_ALLOWED = 10

    SUPPLEMENT_CONSTRUCTION_VEHICLE_ALLOWED = 14

    SUPPLEMENT_ENDS = 8

    SUPPLEMENT_ENVIRONMENT_ZONE_YELLOW_GREEN = 15

    SUPPLEMENT_FORESTAL_ALLOWED = 13

    SUPPLEMENT_MOPED_ALLOWED = 11

    SUPPLEMENT_RAILWAY_ONLY = 16

    SUPPLEMENT_RESIDENTS_ALLOWED = 9

    SUPPLEMENT_TRAM_ALLOWED = 12

    UNKNOWN = 72

    YIELD = 27

    YIELD_TRAIN = 26

class numeric_limits_less__ad_scope_map_scope_landmark_scope_LandmarkId__greater_:
    def epsilon(self) -> LandmarkId:
        """

        epsilon() -> LandmarkId :

            C++ signature :
                ad::map::landmark::LandmarkId epsilon()
        """
        ...

    def lowest(self) -> LandmarkId:
        """

        lowest() -> LandmarkId :

            C++ signature :
                ad::map::landmark::LandmarkId lowest()
        """
        ...

    def max(self) -> LandmarkId:
        """

        max() -> LandmarkId :

            C++ signature :
                ad::map::landmark::LandmarkId max()
        """
        ...
