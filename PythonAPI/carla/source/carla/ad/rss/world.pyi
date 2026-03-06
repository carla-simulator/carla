from typing import ClassVar, overload
from carla import ad

from . import *

class LaneDrivingDirection(int):
    Bidirectional = 0

    Negative = 2

    Positive = 1

class LaneSegment:
    def assign(self, arg1: LaneSegment, other: LaneSegment) -> LaneSegment:
        """

        assign( (LaneSegment)arg1, (LaneSegment)other) -> LaneSegment :

            C++ signature :
                ad::rss::world::LaneSegment {lvalue} assign(ad::rss::world::LaneSegment {lvalue},ad::rss::world::LaneSegment)
        """
        ...

    @property
    def drivingDirection(self) -> LaneDrivingDirection: ...

    @property
    def id(self) -> int: ...

    @property
    def length(self) -> ad.physics.MetricRange: ...

    @property
    def type(self) -> LaneSegmentType: ...

    @property
    def width(self) -> ad.physics.MetricRange: ...

class LaneSegmentType(int):
    Intersection = 1

    Normal = 0

class LateralRssAccelerationValues:
    @property
    def accelMax(self) -> ad.physics.Acceleration: ...

    def assign(self, arg1: LateralRssAccelerationValues, other: LateralRssAccelerationValues) -> LateralRssAccelerationValues:
        """

        assign( (LateralRssAccelerationValues)arg1, (LateralRssAccelerationValues)other) -> LateralRssAccelerationValues :

            C++ signature :
                ad::rss::world::LateralRssAccelerationValues {lvalue} assign(ad::rss::world::LateralRssAccelerationValues {lvalue},ad::rss::world::LateralRssAccelerationValues)
        """
        ...

    @property
    def brakeMin(self) -> ad.physics.Acceleration: ...

class LongitudinalRssAccelerationValues:
    @property
    def accelMax(self) -> ad.physics.Acceleration: ...

    def assign(self, arg1: LongitudinalRssAccelerationValues, other: LongitudinalRssAccelerationValues) -> LongitudinalRssAccelerationValues:
        """

        assign( (LongitudinalRssAccelerationValues)arg1, (LongitudinalRssAccelerationValues)other) -> LongitudinalRssAccelerationValues :

            C++ signature :
                ad::rss::world::LongitudinalRssAccelerationValues {lvalue} assign(ad::rss::world::LongitudinalRssAccelerationValues {lvalue},ad::rss::world::LongitudinalRssAccelerationValues)
        """
        ...

    @property
    def brakeMax(self) -> ad.physics.Acceleration: ...

    @property
    def brakeMin(self) -> ad.physics.Acceleration: ...

    @property
    def brakeMinCorrect(self) -> ad.physics.Acceleration: ...

class Object:
    def assign(self, arg1: Object, other: Object) -> Object:
        """

        assign( (Object)arg1, (Object)other) -> Object :

            C++ signature :
                ad::rss::world::Object {lvalue} assign(ad::rss::world::Object {lvalue},ad::rss::world::Object)
        """
        ...

    @property
    def objectId(self) -> int: ...

    @property
    def objectType(self) -> ObjectType: ...

    @property
    def occupiedRegions(self) -> OccupiedRegionVector: ...

    @property
    def state(self) -> ObjectState: ...

    @property
    def velocity(self) -> Velocity: ...

class ObjectIdVector(ad._Vector[object]):
    def append(self, arg1: ObjectIdVector, arg2: object) -> None:
        """

        append( (ObjectIdVector)arg1, (object)arg2) -> None :

            C++ signature :
                void append(std::vector<unsigned long, std::allocator<unsigned long> > {lvalue},unsigned long)
        """
        ...

    def count(self, arg1: ObjectIdVector, arg2: object) -> int:
        """

        count( (ObjectIdVector)arg1, (object)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<unsigned long, std::allocator<unsigned long> > {lvalue},unsigned long)
        """
        ...

    def extend(self, arg1: ObjectIdVector, arg2: object) -> None:
        """

        extend( (ObjectIdVector)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<unsigned long, std::allocator<unsigned long> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ObjectIdVector, arg2: object) -> int:
        """

        index( (ObjectIdVector)arg1, (object)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<unsigned long, std::allocator<unsigned long> > {lvalue},unsigned long)
        """
        ...

    def insert(self, arg1: ObjectIdVector, arg2: int, arg3: object) -> None:
        """

        insert( (ObjectIdVector)arg1, (int)arg2, (object)arg3) -> None :

            C++ signature :
                void insert(std::vector<unsigned long, std::allocator<unsigned long> > {lvalue},long,unsigned long)
        """
        ...

    def reverse(self, arg1: ObjectIdVector) -> None:
        """

        reverse( (ObjectIdVector)arg1) -> None :

            C++ signature :
                void reverse(std::vector<unsigned long, std::allocator<unsigned long> > {lvalue})
        """
        ...

    def sort(self, arg1: ObjectIdVector) -> None:
        """

        sort( (ObjectIdVector)arg1) -> None :

            C++ signature :
                void sort(std::vector<unsigned long, std::allocator<unsigned long> > {lvalue})
        """
        ...

class ObjectState:
    def assign(self, arg1: ObjectState, other: ObjectState) -> ObjectState:
        """

        assign( (ObjectState)arg1, (ObjectState)other) -> ObjectState :

            C++ signature :
                ad::rss::world::ObjectState {lvalue} assign(ad::rss::world::ObjectState {lvalue},ad::rss::world::ObjectState)
        """
        ...

    @property
    def centerPoint(self) -> ad.physics.Distance2D: ...

    @property
    def dimension(self) -> ad.physics.Dimension2D: ...

    @property
    def speed(self) -> ad.physics.Speed: ...

    @property
    def steeringAngle(self) -> ad.physics.Angle: ...

    @property
    def yaw(self) -> ad.physics.Angle: ...

    @property
    def yawRate(self) -> ad.physics.AngularVelocity: ...

class ObjectType(int):
    ArtificialObject = 4

    EgoVehicle = 1

    Invalid = 0

    OtherVehicle = 2

    Pedestrian = 3

class OccupiedRegion:
    __instance_size__: ClassVar[int] = ...

    def assign(self, arg1: OccupiedRegion, other: OccupiedRegion) -> OccupiedRegion:
        """

        assign( (OccupiedRegion)arg1, (OccupiedRegion)other) -> OccupiedRegion :

            C++ signature :
                ad::rss::world::OccupiedRegion {lvalue} assign(ad::rss::world::OccupiedRegion {lvalue},ad::rss::world::OccupiedRegion)
        """
        ...

    @property
    def latRange(self) -> ad.physics.ParametricRange: ...

    @property
    def lonRange(self) -> ad.physics.ParametricRange: ...

    @property
    def segmentId(self) -> int: ...

    @classmethod
    def __eq__(cls, other: object, /) -> bool:
        """
        __eq__( (OccupiedRegion)arg1, (OccupiedRegion)arg2) -> object :

            C++ signature :
                _object* __eq__(ad::rss::world::OccupiedRegion {lvalue},ad::rss::world::OccupiedRegion)
        """

    @classmethod
    def __ne__(cls, other: object, /) -> bool:
        """
        __ne__( (OccupiedRegion)arg1, (OccupiedRegion)arg2) -> object :

            C++ signature :
                _object* __ne__(ad::rss::world::OccupiedRegion {lvalue},ad::rss::world::OccupiedRegion)
        """

    @classmethod
    def __reduce__(cls): ...

class OccupiedRegionVector(ad._VectorSequence[OccupiedRegion]):
    ...

class RoadArea(ad._VectorSequence[RoadSegment]):
    __instance_size__: ClassVar[int] = ...

class RoadSegment(ad._VectorSequence[LaneSegment]):
    __instance_size__: ClassVar[int] = ...

class RssDynamics(ad._Assignable):
    __instance_size__: ClassVar[int] = ...

    @property
    def alphaLat(self) -> LateralRssAccelerationValues: ...

    @property
    def alphaLon(self) -> LongitudinalRssAccelerationValues: ...

    @property
    def lateralFluctuationMargin(self) -> ad.physics.Distance: ...

    @property
    def maxSpeedOnAcceleration(self) -> ad.physics.Speed: ...

    @property
    def responseTime(self) -> ad.physics.Duration: ...

    @property
    def unstructuredSettings(self) -> UnstructuredSettings: ...

class Scene(ad._Assignable):
    @property
    def egoVehicle(self) -> Object: ...

    @property
    def egoVehicleRoad(self) -> RoadArea: ...

    @property
    def egoVehicleRssDynamics(self) -> RssDynamics: ...

    @property
    def intersectingRoad(self) -> RoadArea: ...

    @property
    def object(self) -> Object: ...

    @property
    def objectRssDynamics(self) -> RssDynamics: ...

    @property
    def situationType(self) -> situation.SituationType: ...

class SceneVector(ad._VectorSequence[Scene]):
    ...

class UnstructuredSettings(ad._Assignable):
    @property
    def driveAwayMaxAngle(self) -> ad.physics.Angle: ...

    @property
    def pedestrianBackIntermediateHeadingChangeRatioSteps(self) -> int: ...

    @property
    def pedestrianBrakeIntermediateAccelerationSteps(self) -> int: ...

    @property
    def pedestrianContinueForwardIntermediateAccelerationSteps(self) -> int: ...

    @property
    def pedestrianContinueForwardIntermediateHeadingChangeRatioSteps(self) -> int: ...

    @property
    def pedestrianFrontIntermediateHeadingChangeRatioSteps(self) -> int: ...

    @property
    def pedestrianTurningRadius(self) -> ad.physics.Distance: ...

    @property
    def vehicleBackIntermediateYawRateChangeRatioSteps(self) -> int: ...

    @property
    def vehicleBrakeIntermediateAccelerationSteps(self) -> int: ...

    @property
    def vehicleContinueForwardIntermediateAccelerationSteps(self) -> int: ...

    @property
    def vehicleContinueForwardIntermediateYawRateChangeRatioSteps(self) -> int: ...

    @property
    def vehicleFrontIntermediateYawRateChangeRatioSteps(self) -> int: ...

    @property
    def vehicleMinRadius(self) -> ad.physics.Distance: ...

    @property
    def vehicleTrajectoryCalculationStep(self) -> ad.physics.Duration: ...

    @property
    def vehicleYawRateChange(self) -> ad.physics.AngularAcceleration: ...

class Velocity(ad._Assignable):

    @property
    def speedLatMax(self) -> ad.physics.Speed: ...

    @property
    def speedLatMin(self) -> ad.physics.Speed: ...

    @property
    def speedLonMax(self) -> ad.physics.Speed: ...

    @property
    def speedLonMin(self) -> ad.physics.Speed: ...

class WorldModel(ad._Assignable):
    @property
    def defaultEgoVehicleRssDynamics(self) -> RssDynamics: ...

    @property
    def scenes(self) -> SceneVector: ...

    @property
    def timeIndex(self) -> int: ...

@overload
def to_string(value: (OccupiedRegion | OccupiedRegionVector | ObjectIdVector | LongitudinalRssAccelerationValues
                      | Velocity | LaneDrivingDirection | LaneSegment | LaneSegmentType | RoadSegment
                      | LateralRssAccelerationValues | UnstructuredSettings | RssDynamics | ObjectState | ObjectType
                      | Object | RoadArea | Scene | SceneVector | WorldModel)) -> str:
    """
    to_string( (OccupiedRegion)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::OccupiedRegion)

    to_string( (OccupiedRegionVector)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::rss::world::OccupiedRegion, std::allocator<ad::rss::world::OccupiedRegion> >)

    to_string( (ObjectIdVector)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<unsigned long, std::allocator<unsigned long> >)

    to_string( (LongitudinalRssAccelerationValues)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::LongitudinalRssAccelerationValues)

    to_string( (Velocity)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::Velocity)

    to_string( (LaneDrivingDirection)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::LaneDrivingDirection)

    to_string( (LaneSegmentType)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::LaneSegmentType)

    to_string( (LaneSegment)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::LaneSegment)

    to_string( (RoadSegment)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> >)

    to_string( (LateralRssAccelerationValues)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::LateralRssAccelerationValues)

    to_string( (UnstructuredSettings)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::UnstructuredSettings)

    to_string( (RssDynamics)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::RssDynamics)

    to_string( (ObjectState)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::ObjectState)

    to_string( (ObjectType)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::ObjectType)

    to_string( (Object)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::Object)

    to_string( (RoadArea)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> >, std::allocator<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > > >)

    to_string( (Scene)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::Scene)

    to_string( (SceneVector)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(std::vector<ad::rss::world::Scene, std::allocator<ad::rss::world::Scene> >)

    to_string( (WorldModel)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(ad::rss::world::WorldModel)
    """
@overload
def to_string(ad) -> str:
   ...
