import ad
from ad import _Assignable
from ...libcarla import _CarlaEnum

from . import *

class LateralRelativePosition(int, _CarlaEnum):
    AtLeft = 0

    AtRight = 4

    Overlap = 2

    OverlapLeft = 1

    OverlapRight = 3

class LongitudinalRelativePosition(int, _CarlaEnum):
    AtBack = 4

    InFront = 0

    Overlap = 2

    OverlapBack = 3

    OverlapFront = 1

class RelativePosition(_Assignable):

    @property
    def lateralDistance(self) -> ad.physics.Distance: ...

    @property
    def lateralPosition(self) -> LateralRelativePosition: ...

    @property
    def longitudinalDistance(self) -> ad.physics.Distance: ...

    @property
    def longitudinalPosition(self) -> LongitudinalRelativePosition: ...

class Situation(_Assignable):

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

class SituationSnapshot(_Assignable):

    @property
    def defaultEgoVehicleRssDynamics(self) -> world.RssDynamics: ...

    @property
    def situations(self) -> SituationVector: ...

    @property
    def timeIndex(self) -> int: ...

class SituationType(int, _CarlaEnum):
    IntersectionEgoHasPriority = 3

    IntersectionObjectHasPriority = 4

    IntersectionSamePriority = 5

    NotRelevant = 0

    OppositeDirection = 2

    SameDirection = 1

    Unstructured = 6

class SituationVector(ad._VectorSequence[Situation]):
    ...

class VehicleState(_Assignable):

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

class VelocityRange(_Assignable):

    @property
    def speedLat(self) -> ad.physics.SpeedRange: ...

    @property
    def speedLon(self) -> ad.physics.SpeedRange: ...
