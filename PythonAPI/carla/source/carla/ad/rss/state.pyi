from ... import ad
from ...libcarla import _CarlaEnum
from . import *

def isDangerous(rssState: RssState) -> bool:
    ...

def isLateralSafe(rssState: RssState) -> bool:
    ...

def isLongitudinalSafe(rssState: RssState) -> bool:
    ...

class AccelerationRestriction():
    def assign(self, arg1: AccelerationRestriction, other: AccelerationRestriction) -> AccelerationRestriction:
        '''

        assign( (AccelerationRestriction)arg1, (AccelerationRestriction)other) -> AccelerationRestriction :

            C++ signature :
                ad::rss::state::AccelerationRestriction {lvalue} assign(ad::rss::state::AccelerationRestriction {lvalue},ad::rss::state::AccelerationRestriction)
        '''
        ...

    @property
    def lateralLeftRange(self) -> ad.physics.AccelerationRange: ...

    @property
    def lateralRightRange(self) -> ad.physics.AccelerationRange: ...

    @property
    def longitudinalRange(self) -> ad.physics.AccelerationRange: ...

class HeadingRange():
    def assign(self, arg1: HeadingRange, other: HeadingRange) -> HeadingRange:
        '''

        assign( (HeadingRange)arg1, (HeadingRange)other) -> HeadingRange :

            C++ signature :
                ad::rss::state::HeadingRange {lvalue} assign(ad::rss::state::HeadingRange {lvalue},ad::rss::state::HeadingRange)
        '''
        ...

    @property
    def begin(self) -> ad.physics.Angle: ...

    @property
    def end(self) -> ad.physics.Angle: ...

class HeadingRangeVector(ad._Vector[HeadingRange]):
    def append(self, arg1: HeadingRangeVector, arg2: HeadingRange) -> None:
        '''

        append( (HeadingRangeVector)arg1, (HeadingRange)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue},ad::rss::state::HeadingRange)
        '''
        ...

    def count(self, arg1: HeadingRangeVector, arg2: HeadingRange) -> int:
        '''

        count( (HeadingRangeVector)arg1, (HeadingRange)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue},ad::rss::state::HeadingRange)
        '''
        ...

    def extend(self, arg1: HeadingRangeVector, arg2: object) -> None:
        '''

        extend( (HeadingRangeVector)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue},boost::python::api::object)
        '''
        ...

    def index(self, arg1: HeadingRangeVector, arg2: HeadingRange) -> int:
        '''

        index( (HeadingRangeVector)arg1, (HeadingRange)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue},ad::rss::state::HeadingRange)
        '''
        ...

    def insert(self, arg1: HeadingRangeVector, arg2: int, arg3: HeadingRange) -> None:
        '''

        insert( (HeadingRangeVector)arg1, (int)arg2, (HeadingRange)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue},long,ad::rss::state::HeadingRange)
        '''
        ...

    def reverse(self, arg1: HeadingRangeVector) -> None:
        '''

        reverse( (HeadingRangeVector)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue})
        '''
        ...

class LateralResponse(int,):
    BrakeMin = 1

    None = 0  # type: ignore

class LateralRssState():
    @property
    def alphaLat(self) -> world.LateralRssAccelerationValues: ...

    def assign(self, arg1: LateralRssState, other: LateralRssState) -> LateralRssState:
        '''

        assign( (LateralRssState)arg1, (LateralRssState)other) -> LateralRssState :

            C++ signature :
                ad::rss::state::LateralRssState {lvalue} assign(ad::rss::state::LateralRssState {lvalue},ad::rss::state::LateralRssState)
        '''
        ...

    @property
    def isSafe(self) -> bool: ...

    @property
    def response(self) -> LateralResponse: ...

    @property
    def rssStateInformation(self) -> RssStateInformation: ...

class LongitudinalResponse(int,):
    BrakeMin = 2

    BrakeMinCorrect = 1

    None = 0  # type: ignore

class LongitudinalRssState():
    @property
    def alphaLon(self) -> world.LongitudinalRssAccelerationValues: ...

    def assign(self, arg1: LongitudinalRssState, other: LongitudinalRssState) -> LongitudinalRssState:
        '''

        assign( (LongitudinalRssState)arg1, (LongitudinalRssState)other) -> LongitudinalRssState :

            C++ signature :
                ad::rss::state::LongitudinalRssState {lvalue} assign(ad::rss::state::LongitudinalRssState {lvalue},ad::rss::state::LongitudinalRssState)
        '''
        ...

    @property
    def isSafe(self) -> bool: ...

    @property
    def response(self) -> LongitudinalResponse: ...

    @property
    def rssStateInformation(self) -> RssStateInformation: ...

class ProperResponse():
    @property
    def accelerationRestrictions(self) -> AccelerationRestriction: ...

    def assign(self, arg1: ProperResponse, other: ProperResponse) -> ProperResponse:
        '''

        assign( (ProperResponse)arg1, (ProperResponse)other) -> ProperResponse :

            C++ signature :
                ad::rss::state::ProperResponse {lvalue} assign(ad::rss::state::ProperResponse {lvalue},ad::rss::state::ProperResponse)
        '''
        ...

    @property
    def dangerousObjects(self) -> world.ObjectIdVector: ...

    @property
    def headingRanges(self) -> HeadingRangeVector: ...

    @property
    def isSafe(self) -> bool: ...

    @property
    def lateralResponseLeft(self) -> LateralResponse: ...

    @property
    def lateralResponseRight(self) -> LateralResponse: ...

    @property
    def longitudinalResponse(self) -> LongitudinalResponse: ...

    @property
    def timeIndex(self) -> int: ...

    @property
    def unstructuredSceneResponse(self) -> UnstructuredSceneResponse: ...

class RssState():
    def assign(self, other: RssState) -> RssState:
        '''

        assign( (RssState)arg1, (RssState)other) -> RssState :

            C++ signature :
                ad::rss::state::RssState {lvalue} assign(ad::rss::state::RssState {lvalue},ad::rss::state::RssState)
        '''
        ...

    @property
    def lateralStateLeft(self) -> LateralRssState: ...

    @property
    def lateralStateRight(self) -> LateralRssState: ...

    @property
    def longitudinalState(self) -> LongitudinalRssState: ...

    @property
    def objectId(self) -> int: ...

    @property
    def situationId(self) -> int: ...

    @property
    def situationType(self) -> situation.SituationType: ...

    @property
    def unstructuredSceneState(self) -> UnstructuredSceneRssState: ...

class RssStateEvaluator(int, _CarlaEnum):
    None = 0  # type: ignore
    LongitudinalDistanceOppositeDirectionEgoCorrectLane = 1
    LongitudinalDistanceOppositeDirection = 2
    LongitudinalDistanceSameDirectionEgoFront = 3
    LongitudinalDistanceSameDirectionOtherInFront = 4
    LateralDistance = 5
    IntersectionOtherPriorityEgoAbleToStop = 6
    IntersectionEgoPriorityOtherAbleToStop = 7
    IntersectionEgoInFront = 8
    IntersectionOtherInFront = 9
    IntersectionOverlap = 10

class RssStateInformation():
    def assign(self, arg1: RssStateInformation, other: RssStateInformation) -> RssStateInformation:
        '''

        assign( (RssStateInformation)arg1, (RssStateInformation)other) -> RssStateInformation :

            C++ signature :
                ad::rss::state::RssStateInformation {lvalue} assign(ad::rss::state::RssStateInformation {lvalue},ad::rss::state::RssStateInformation)
        '''
        ...

    @property
    def currentDistance(self) -> ad.physics.Distance: ...

    @property
    def evaluator(self) -> RssStateEvaluator: ...

    @property
    def safeDistance(self) -> ad.physics.Distance: ...

class RssStateSnapshot():
    def assign(self, other: RssStateSnapshot) -> RssStateSnapshot:
        '''

        assign( (RssStateSnapshot)arg1, (RssStateSnapshot)other) -> RssStateSnapshot :

            C++ signature :
                ad::rss::state::RssStateSnapshot {lvalue} assign(ad::rss::state::RssStateSnapshot {lvalue},ad::rss::state::RssStateSnapshot)
        '''
        ...

    @property
    def defaultEgoVehicleRssDynamics(self) -> world.RssDynamics: ...

    @property
    def individualResponses(self) -> RssStateVector: ...

    @property
    def timeIndex(self) -> int: ...

    @property
    def unstructuredSceneEgoInformation(self) -> UnstructuredSceneStateInformation: ...

class RssStateVector(ad._VectorSequence[RssState]):
   ...

class UnstructuredSceneResponse(int, _CarlaEnum):
    None = 0  # type: ignore # is named None in the C++ code
    ContinueForward = 1
    DriveAway = 2
    Brake = 3

class UnstructuredSceneRssState():
    @property
    def alphaLon(self) -> world.LongitudinalRssAccelerationValues: ...

    def assign(self, arg1: UnstructuredSceneRssState, other: UnstructuredSceneRssState) -> UnstructuredSceneRssState:
        '''

        assign( (UnstructuredSceneRssState)arg1, (UnstructuredSceneRssState)other) -> UnstructuredSceneRssState :

            C++ signature :
                ad::rss::state::UnstructuredSceneRssState {lvalue} assign(ad::rss::state::UnstructuredSceneRssState {lvalue},ad::rss::state::UnstructuredSceneRssState)
        '''
        ...

    @property
    def headingRange(self) -> HeadingRange: ...

    @property
    def isSafe(self) -> bool: ...

    @property
    def response(self) -> UnstructuredSceneResponse: ...

    @property
    def rssStateInformation(self) -> UnstructuredSceneStateInformation: ...

class UnstructuredSceneStateInformation():
    def assign(self, arg1: UnstructuredSceneStateInformation, other: UnstructuredSceneStateInformation) -> UnstructuredSceneStateInformation:
        '''

        assign( (UnstructuredSceneStateInformation)arg1, (UnstructuredSceneStateInformation)other) -> UnstructuredSceneStateInformation :

            C++ signature :
                ad::rss::state::UnstructuredSceneStateInformation {lvalue} assign(ad::rss::state::UnstructuredSceneStateInformation {lvalue},ad::rss::state::UnstructuredSceneStateInformation)
        '''
        ...

    @property
    def brakeTrajectorySet(self) -> ad.physics.Distance2DList: ...

    @property
    def continueForwardTrajectorySet(self) -> ad.physics.Distance2DList: ...
