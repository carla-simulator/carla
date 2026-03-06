from ...libcarla import _CarlaEnum
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

class RestrictionList(ad._VectorSequence[Restriction]):
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

class RoadUserType(int, _CarlaEnum):
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

class RoadUserTypeList(ad._VectorSequence[RoadUserType]):
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

class SpeedLimitList(ad._VectorSequence[SpeedLimit]):
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
