from libcarla import ad

from . import *

class LateralRelativePosition(int):
    AtLeft = 0

    AtRight = 4

    Overlap = 2

    OverlapLeft = 1

    OverlapRight = 3

class LongitudinalRelativePosition(int):
    AtBack = 4

    InFront = 0

    Overlap = 2

    OverlapBack = 3

    OverlapFront = 1

class RelativePosition:
    def assign(self, arg1: RelativePosition, other: RelativePosition) -> RelativePosition:
        """

        assign( (RelativePosition)arg1, (RelativePosition)other) -> RelativePosition :

            C++ signature :
                ad::rss::situation::RelativePosition {lvalue} assign(ad::rss::situation::RelativePosition {lvalue},ad::rss::situation::RelativePosition)
        """
        ...

    @property
    def lateralDistance(self) -> ad.physics.Distance: ...

    @property
    def lateralPosition(self) -> LateralRelativePosition: ...

    @property
    def longitudinalDistance(self) -> ad.physics.Distance: ...

    @property
    def longitudinalPosition(self) -> LongitudinalRelativePosition: ...

class Situation:
    def assign(self, arg1: Situation, other: Situation) -> Situation:
        """

        assign( (Situation)arg1, (Situation)other) -> Situation :

            C++ signature :
                ad::rss::situation::Situation {lvalue} assign(ad::rss::situation::Situation {lvalue},ad::rss::situation::Situation)
        """
        ...

    @property
    def egoVehicleState(self) -> VehicleState: ...

    @property
    def objectId(self) -> int: ...

    @property
    def otherVehicleState(self) -> VehicleState: ...

    @property
    def relativePosition(self) -> RelativePosition: ...

    @property
    def situationId(self) -> int: ...

    @property
    def situationType(self) -> SituationType: ...

class SituationSnapshot:
    def assign(self, arg1: SituationSnapshot, other: SituationSnapshot) -> SituationSnapshot:
        """

        assign( (SituationSnapshot)arg1, (SituationSnapshot)other) -> SituationSnapshot :

            C++ signature :
                ad::rss::situation::SituationSnapshot {lvalue} assign(ad::rss::situation::SituationSnapshot {lvalue},ad::rss::situation::SituationSnapshot)
        """
        ...

    @property
    def defaultEgoVehicleRssDynamics(self) -> world.RssDynamics: ...

    @property
    def situations(self) -> SituationVector: ...

    @property
    def timeIndex(self) -> int: ...

class SituationType(int):
    IntersectionEgoHasPriority = 3

    IntersectionObjectHasPriority = 4

    IntersectionSamePriority = 5

    NotRelevant = 0

    OppositeDirection = 2

    SameDirection = 1

    Unstructured = 6

class SituationVector(ad._Vector[Situation]):
    def append(self, arg1: SituationVector, arg2: Situation) -> None:
        """

        append( (SituationVector)arg1, (Situation)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::rss::situation::Situation, std::allocator<ad::rss::situation::Situation> > {lvalue},ad::rss::situation::Situation)
        """
        ...

    def count(self, arg1: SituationVector, arg2: Situation) -> int:
        """

        count( (SituationVector)arg1, (Situation)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::rss::situation::Situation, std::allocator<ad::rss::situation::Situation> > {lvalue},ad::rss::situation::Situation)
        """
        ...

    def extend(self, arg1: SituationVector, arg2: object) -> None:
        """

        extend( (SituationVector)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::rss::situation::Situation, std::allocator<ad::rss::situation::Situation> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: SituationVector, arg2: Situation) -> int:
        """

        index( (SituationVector)arg1, (Situation)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::rss::situation::Situation, std::allocator<ad::rss::situation::Situation> > {lvalue},ad::rss::situation::Situation)
        """
        ...

    def insert(self, arg1: SituationVector, arg2: int, arg3: Situation) -> None:
        """

        insert( (SituationVector)arg1, (int)arg2, (Situation)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::rss::situation::Situation, std::allocator<ad::rss::situation::Situation> > {lvalue},long,ad::rss::situation::Situation)
        """
        ...

    def reverse(self, arg1: SituationVector) -> None:
        """

        reverse( (SituationVector)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::rss::situation::Situation, std::allocator<ad::rss::situation::Situation> > {lvalue})
        """
        ...

class VehicleState:
    def assign(self, arg1: VehicleState, other: VehicleState) -> VehicleState:
        """

        assign( (VehicleState)arg1, (VehicleState)other) -> VehicleState :

            C++ signature :
                ad::rss::situation::VehicleState {lvalue} assign(ad::rss::situation::VehicleState {lvalue},ad::rss::situation::VehicleState)
        """
        ...

    @property
    def distanceToEnterIntersection(self) -> ad.physics.Distance: ...

    @property
    def distanceToLeaveIntersection(self) -> ad.physics.Distance: ...

    @property
    def dynamics(self) -> world.RssDynamics: ...

    @property
    def hasPriority(self) -> bool: ...

    @property
    def isInCorrectLane(self) -> bool: ...

    @property
    def objectState(self) -> world.ObjectState: ...

    @property
    def objectType(self) -> world.ObjectType: ...

    @property
    def velocity(self) -> VelocityRange: ...

class VelocityRange:
    def assign(self, arg1: VelocityRange, other: VelocityRange) -> VelocityRange:
        """

        assign( (VelocityRange)arg1, (VelocityRange)other) -> VelocityRange :

            C++ signature :
                ad::rss::situation::VelocityRange {lvalue} assign(ad::rss::situation::VelocityRange {lvalue},ad::rss::situation::VelocityRange)
        """
        ...

    @property
    def speedLat(self) -> ad.physics.SpeedRange: ...

    @property
    def speedLon(self) -> ad.physics.SpeedRange: ...
