import ad

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

class OccupiedRegionVector(ad._Vector[OccupiedRegion]):
    def append(self, arg1: OccupiedRegionVector, arg2: OccupiedRegion) -> None:
        """

        append( (OccupiedRegionVector)arg1, (OccupiedRegion)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::rss::world::OccupiedRegion, std::allocator<ad::rss::world::OccupiedRegion> > {lvalue},ad::rss::world::OccupiedRegion)
        """
        ...

    def count(self, arg1: OccupiedRegionVector, arg2: OccupiedRegion) -> int:
        """

        count( (OccupiedRegionVector)arg1, (OccupiedRegion)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::rss::world::OccupiedRegion, std::allocator<ad::rss::world::OccupiedRegion> > {lvalue},ad::rss::world::OccupiedRegion)
        """
        ...

    def extend(self, arg1: OccupiedRegionVector, arg2: object) -> None:
        """

        extend( (OccupiedRegionVector)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::rss::world::OccupiedRegion, std::allocator<ad::rss::world::OccupiedRegion> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: OccupiedRegionVector, arg2: OccupiedRegion) -> int:
        """

        index( (OccupiedRegionVector)arg1, (OccupiedRegion)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::rss::world::OccupiedRegion, std::allocator<ad::rss::world::OccupiedRegion> > {lvalue},ad::rss::world::OccupiedRegion)
        """
        ...

    def insert(self, arg1: OccupiedRegionVector, arg2: int, arg3: OccupiedRegion) -> None:
        """

        insert( (OccupiedRegionVector)arg1, (int)arg2, (OccupiedRegion)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::rss::world::OccupiedRegion, std::allocator<ad::rss::world::OccupiedRegion> > {lvalue},long,ad::rss::world::OccupiedRegion)
        """
        ...

    def reverse(self, arg1: OccupiedRegionVector) -> None:
        """

        reverse( (OccupiedRegionVector)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::rss::world::OccupiedRegion, std::allocator<ad::rss::world::OccupiedRegion> > {lvalue})
        """
        ...

class RoadArea:
    def append(self, arg1: RoadArea, arg2: RoadSegment) -> None:
        """

        append( (RoadArea)arg1, (RoadSegment)arg2) -> None :

            C++ signature :
                void append(std::vector<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> >, std::allocator<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > > > {lvalue},std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> >)
        """
        ...

    def extend(self, arg1: RoadArea, arg2: object) -> None:
        """

        extend( (RoadArea)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> >, std::allocator<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > > > {lvalue},boost::python::api::object)
        """
        ...

    def insert(self, arg1: RoadArea, arg2: int, arg3: RoadSegment) -> None:
        """

        insert( (RoadArea)arg1, (int)arg2, (RoadSegment)arg3) -> None :

            C++ signature :
                void insert(std::vector<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> >, std::allocator<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > > > {lvalue},long,std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> >)
        """
        ...

    def reverse(self, arg1: RoadArea) -> None:
        """

        reverse( (RoadArea)arg1) -> None :

            C++ signature :
                void reverse(std::vector<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> >, std::allocator<std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > > > {lvalue})
        """
        ...

class RoadSegment:
    def append(self, arg1: RoadSegment, arg2: LaneSegment) -> None:
        """

        append( (RoadSegment)arg1, (LaneSegment)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > {lvalue},ad::rss::world::LaneSegment)
        """
        ...

    def count(self, arg1: RoadSegment, arg2: LaneSegment) -> int:
        """

        count( (RoadSegment)arg1, (LaneSegment)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > {lvalue},ad::rss::world::LaneSegment)
        """
        ...

    def extend(self, arg1: RoadSegment, arg2: object) -> None:
        """

        extend( (RoadSegment)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: RoadSegment, arg2: LaneSegment) -> int:
        """

        index( (RoadSegment)arg1, (LaneSegment)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > {lvalue},ad::rss::world::LaneSegment)
        """
        ...

    def insert(self, arg1: RoadSegment, arg2: int, arg3: LaneSegment) -> None:
        """

        insert( (RoadSegment)arg1, (int)arg2, (LaneSegment)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > {lvalue},long,ad::rss::world::LaneSegment)
        """
        ...

    def reverse(self, arg1: RoadSegment) -> None:
        """

        reverse( (RoadSegment)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::rss::world::LaneSegment, std::allocator<ad::rss::world::LaneSegment> > {lvalue})
        """
        ...

class RssDynamics:
    @property
    def alphaLat(self) -> LateralRssAccelerationValues: ...

    @property
    def alphaLon(self) -> LongitudinalRssAccelerationValues: ...

    def assign(self, arg1: RssDynamics, other: RssDynamics) -> RssDynamics:
        """

        assign( (RssDynamics)arg1, (RssDynamics)other) -> RssDynamics :

            C++ signature :
                ad::rss::world::RssDynamics {lvalue} assign(ad::rss::world::RssDynamics {lvalue},ad::rss::world::RssDynamics)
        """
        ...

    @property
    def lateralFluctuationMargin(self) -> ad.physics.Distance: ...

    @property
    def maxSpeedOnAcceleration(self) -> ad.physics.Speed: ...

    @property
    def responseTime(self) -> ad.physics.Duration: ...

    @property
    def unstructuredSettings(self) -> UnstructuredSettings: ...

class Scene:
    def assign(self, arg1: Scene, other: Scene) -> Scene:
        """

        assign( (Scene)arg1, (Scene)other) -> Scene :

            C++ signature :
                ad::rss::world::Scene {lvalue} assign(ad::rss::world::Scene {lvalue},ad::rss::world::Scene)
        """
        ...

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

class SceneVector(ad._Vector[Scene]):
    def append(self, arg2: Scene) -> None:
        """

        append( (SceneVector)arg1, (Scene)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::rss::world::Scene, std::allocator<ad::rss::world::Scene> > {lvalue},ad::rss::world::Scene)
        """
        ...

    def count(self, arg2: Scene) -> int:
        """

        count( (SceneVector)arg1, (Scene)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::rss::world::Scene, std::allocator<ad::rss::world::Scene> > {lvalue},ad::rss::world::Scene)
        """
        ...

    def extend(self, arg2: object) -> None:
        """

        extend( (SceneVector)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::rss::world::Scene, std::allocator<ad::rss::world::Scene> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg2: Scene) -> int:
        """

        index( (SceneVector)arg1, (Scene)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::rss::world::Scene, std::allocator<ad::rss::world::Scene> > {lvalue},ad::rss::world::Scene)
        """
        ...

    def insert(self, arg2: int, arg3: Scene) -> None:
        """

        insert( (SceneVector)arg1, (int)arg2, (Scene)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::rss::world::Scene, std::allocator<ad::rss::world::Scene> > {lvalue},long,ad::rss::world::Scene)
        """
        ...

    def reverse(self) -> None:
        """

        reverse( (SceneVector)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::rss::world::Scene, std::allocator<ad::rss::world::Scene> > {lvalue})
        """
        ...

class UnstructuredSettings:
    def assign(self, other: UnstructuredSettings) -> UnstructuredSettings:
        """

        assign( (UnstructuredSettings)arg1, (UnstructuredSettings)other) -> UnstructuredSettings :

            C++ signature :
                ad::rss::world::UnstructuredSettings {lvalue} assign(ad::rss::world::UnstructuredSettings {lvalue},ad::rss::world::UnstructuredSettings)
        """
        ...

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

class Velocity:
    def assign(self, arg1: Velocity, other: Velocity) -> Velocity:
        """

        assign( (Velocity)arg1, (Velocity)other) -> Velocity :

            C++ signature :
                ad::rss::world::Velocity {lvalue} assign(ad::rss::world::Velocity {lvalue},ad::rss::world::Velocity)
        """
        ...

    @property
    def speedLatMax(self) -> ad.physics.Speed: ...

    @property
    def speedLatMin(self) -> ad.physics.Speed: ...

    @property
    def speedLonMax(self) -> ad.physics.Speed: ...

    @property
    def speedLonMin(self) -> ad.physics.Speed: ...

class WorldModel:
    def assign(self, arg1: WorldModel, other: WorldModel) -> WorldModel:
        """

        assign( (WorldModel)arg1, (WorldModel)other) -> WorldModel :

            C++ signature :
                ad::rss::world::WorldModel {lvalue} assign(ad::rss::world::WorldModel {lvalue},ad::rss::world::WorldModel)
        """
        ...

    @property
    def defaultEgoVehicleRssDynamics(self) -> RssDynamics: ...

    @property
    def scenes(self) -> SceneVector: ...

    @property
    def timeIndex(self) -> int: ...
