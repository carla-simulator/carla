import ad

class Restriction:
    def assign(self, arg1: Restriction, other: Restriction) -> Restriction:
        """

        assign( (Restriction)arg1, (Restriction)other) -> Restriction :

            C++ signature :
                ad::map::restriction::Restriction {lvalue} assign(ad::map::restriction::Restriction {lvalue},ad::map::restriction::Restriction)
        """
        ...

    @property
    def negated(self) -> bool: ...

    @property
    def passengersMin(self) -> int: ...

    @property
    def roadUserTypes(self) -> RoadUserTypeList: ...

class RestrictionList:
    def append(self, arg1: RestrictionList, arg2: Restriction) -> None:
        """

        append( (RestrictionList)arg1, (Restriction)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::restriction::Restriction, std::allocator<ad::map::restriction::Restriction> > {lvalue},ad::map::restriction::Restriction)
        """
        ...

    def count(self, arg1: RestrictionList, arg2: Restriction) -> int:
        """

        count( (RestrictionList)arg1, (Restriction)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::restriction::Restriction, std::allocator<ad::map::restriction::Restriction> > {lvalue},ad::map::restriction::Restriction)
        """
        ...

    def extend(self, arg1: RestrictionList, arg2: object) -> None:
        """

        extend( (RestrictionList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::restriction::Restriction, std::allocator<ad::map::restriction::Restriction> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: RestrictionList, arg2: Restriction) -> int:
        """

        index( (RestrictionList)arg1, (Restriction)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::restriction::Restriction, std::allocator<ad::map::restriction::Restriction> > {lvalue},ad::map::restriction::Restriction)
        """
        ...

    def insert(self, arg1: RestrictionList, arg2: int, arg3: Restriction) -> None:
        """

        insert( (RestrictionList)arg1, (int)arg2, (Restriction)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::restriction::Restriction, std::allocator<ad::map::restriction::Restriction> > {lvalue},long,ad::map::restriction::Restriction)
        """
        ...

    def reverse(self, arg1: RestrictionList) -> None:
        """

        reverse( (RestrictionList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::restriction::Restriction, std::allocator<ad::map::restriction::Restriction> > {lvalue})
        """
        ...

class Restrictions:
    def assign(self, arg1: Restrictions, other: Restrictions) -> Restrictions:
        """

        assign( (Restrictions)arg1, (Restrictions)other) -> Restrictions :

            C++ signature :
                ad::map::restriction::Restrictions {lvalue} assign(ad::map::restriction::Restrictions {lvalue},ad::map::restriction::Restrictions)
        """
        ...

    @property
    def conjunctions(self) -> RestrictionList: ...

    @property
    def disjunctions(self) -> RestrictionList: ...

class RoadUserType(int):
    BICYCLE = 7

    BUS = 3

    CAR = 2

    CAR_DIESEL = 11

    CAR_ELECTRIC = 8

    CAR_HYBRID = 9

    CAR_PETROL = 10

    INVALID = 0

    MOTORBIKE = 6

    PEDESTRIAN = 5

    TRUCK = 4

    UNKNOWN = 1

class RoadUserTypeList:
    def append(self, arg1: RoadUserTypeList, arg2: RoadUserType) -> None:
        """

        append( (RoadUserTypeList)arg1, (RoadUserType)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::restriction::RoadUserType, std::allocator<ad::map::restriction::RoadUserType> > {lvalue},ad::map::restriction::RoadUserType)
        """
        ...

    def count(self, arg1: RoadUserTypeList, arg2: RoadUserType) -> int:
        """

        count( (RoadUserTypeList)arg1, (RoadUserType)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::restriction::RoadUserType, std::allocator<ad::map::restriction::RoadUserType> > {lvalue},ad::map::restriction::RoadUserType)
        """
        ...

    def extend(self, arg1: RoadUserTypeList, arg2: object) -> None:
        """

        extend( (RoadUserTypeList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::restriction::RoadUserType, std::allocator<ad::map::restriction::RoadUserType> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: RoadUserTypeList, arg2: RoadUserType) -> int:
        """

        index( (RoadUserTypeList)arg1, (RoadUserType)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::restriction::RoadUserType, std::allocator<ad::map::restriction::RoadUserType> > {lvalue},ad::map::restriction::RoadUserType)
        """
        ...

    def insert(self, arg1: RoadUserTypeList, arg2: int, arg3: RoadUserType) -> None:
        """

        insert( (RoadUserTypeList)arg1, (int)arg2, (RoadUserType)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::restriction::RoadUserType, std::allocator<ad::map::restriction::RoadUserType> > {lvalue},long,ad::map::restriction::RoadUserType)
        """
        ...

    def reverse(self, arg1: RoadUserTypeList) -> None:
        """

        reverse( (RoadUserTypeList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::restriction::RoadUserType, std::allocator<ad::map::restriction::RoadUserType> > {lvalue})
        """
        ...

    def sort(self, arg1: RoadUserTypeList) -> None:
        """

        sort( (RoadUserTypeList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::map::restriction::RoadUserType, std::allocator<ad::map::restriction::RoadUserType> > {lvalue})
        """
        ...

class SpeedLimit:
    def assign(self, arg1: SpeedLimit, other: SpeedLimit) -> SpeedLimit:
        """

        assign( (SpeedLimit)arg1, (SpeedLimit)other) -> SpeedLimit :

            C++ signature :
                ad::map::restriction::SpeedLimit {lvalue} assign(ad::map::restriction::SpeedLimit {lvalue},ad::map::restriction::SpeedLimit)
        """
        ...

    @property
    def lanePiece(self) -> ad.physics.ParametricRange: ...

    @property
    def speedLimit(self) -> ad.physics.Speed: ...

class SpeedLimitList:
    def append(self, arg1: SpeedLimitList, arg2: SpeedLimit) -> None:
        """

        append( (SpeedLimitList)arg1, (SpeedLimit)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::restriction::SpeedLimit, std::allocator<ad::map::restriction::SpeedLimit> > {lvalue},ad::map::restriction::SpeedLimit)
        """
        ...

    def count(self, arg1: SpeedLimitList, arg2: SpeedLimit) -> int:
        """

        count( (SpeedLimitList)arg1, (SpeedLimit)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::restriction::SpeedLimit, std::allocator<ad::map::restriction::SpeedLimit> > {lvalue},ad::map::restriction::SpeedLimit)
        """
        ...

    def extend(self, arg1: SpeedLimitList, arg2: object) -> None:
        """

        extend( (SpeedLimitList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::restriction::SpeedLimit, std::allocator<ad::map::restriction::SpeedLimit> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: SpeedLimitList, arg2: SpeedLimit) -> int:
        """

        index( (SpeedLimitList)arg1, (SpeedLimit)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::restriction::SpeedLimit, std::allocator<ad::map::restriction::SpeedLimit> > {lvalue},ad::map::restriction::SpeedLimit)
        """
        ...

    def insert(self, arg1: SpeedLimitList, arg2: int, arg3: SpeedLimit) -> None:
        """

        insert( (SpeedLimitList)arg1, (int)arg2, (SpeedLimit)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::restriction::SpeedLimit, std::allocator<ad::map::restriction::SpeedLimit> > {lvalue},long,ad::map::restriction::SpeedLimit)
        """
        ...

    def reverse(self, arg1: SpeedLimitList) -> None:
        """

        reverse( (SpeedLimitList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::restriction::SpeedLimit, std::allocator<ad::map::restriction::SpeedLimit> > {lvalue})
        """
        ...

class VehicleDescriptor:
    def assign(self, arg1: VehicleDescriptor, other: VehicleDescriptor) -> VehicleDescriptor:
        """

        assign( (VehicleDescriptor)arg1, (VehicleDescriptor)other) -> VehicleDescriptor :

            C++ signature :
                ad::map::restriction::VehicleDescriptor {lvalue} assign(ad::map::restriction::VehicleDescriptor {lvalue},ad::map::restriction::VehicleDescriptor)
        """
        ...

    @property
    def height(self) -> ad.physics.Distance: ...

    @property
    def length(self) -> ad.physics.Distance: ...

    @property
    def passengers(self) -> int: ...

    @property
    def type(self) -> RoadUserType: ...

    @property
    def weight(self) -> ad.physics.Weight: ...

    @property
    def width(self) -> ad.physics.Distance: ...
