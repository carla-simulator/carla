from typing import Any, ClassVar
from ...libcarla import _CarlaEnum

from . import access, config, intersection, landmark, lane, match, point, restriction, route
from .. import rss

NotRelevant: RssMode
Structured: RssMode
Unstructured: RssMode

class RssMode(int, _CarlaEnum):
    NotRelevant = 0
    Structured = 1
    Unstructured = 2
    
class RssObjectData:
    __instance_size__: ClassVar[int] = 360
    
    # properties
    id: Unknown
    matchObject: Unknown
    rssDynamics: Unknown
    speed: Unknown
    steeringAngle: Unknown
    type: Unknown
    yawRate: Unknown
    
    @classmethod
    def __init__(cls, *args, **kwargs) -> None:
        """
        __init__( (object)arg1) -> None :

            C++ signature :
                void __init__(_object*)"""
    @classmethod
    def __reduce__(cls): ...

class RssSceneCreation:
    class AppendRoadBoundariesMode(int, _CarlaEnum):
        RouteOnly = 0
        ExpandRouteToOppositeLanes = 1
        ExpandRouteToAllNeighbors = 2

    class RestrictSpeedLimitMode(int, _CarlaEnum):
        None = 0  # type: ignore
        ExactSpeedLimit = 1
        IncreasedSpeedLimit10 = 2
        IncreasedSpeedLimit5 = 3

    ExactSpeedLimit = RestrictSpeedLimitMode.ExactSpeedLimit
    ExpandRouteToAllNeighbors = AppendRoadBoundariesMode.ExpandRouteToAllNeighbors
    ExpandRouteToOppositeLanes = AppendRoadBoundariesMode.ExpandRouteToOppositeLanes
    IncreasedSpeedLimit10 = RestrictSpeedLimitMode.IncreasedSpeedLimit10
    IncreasedSpeedLimit5 = RestrictSpeedLimitMode.IncreasedSpeedLimit5
    RouteOnly = AppendRoadBoundariesMode.RouteOnly
    
    __instance_size__: ClassVar[int] = 256
    def __init__(self, timeIndex: int, defaultEgoRssDynamics: rss.world.RssDynamics) -> None:
        """
        __init__( (object)arg1, (object)timeIndex, (RssDynamics)defaultEgoRssDynamics) -> None :

            C++ signature :
                void __init__(_object*,unsigned long,ad::rss::world::RssDynamics)"""
    @classmethod
    def appendRoadBoundaries(cls, egoObjectData: RssObjectData, route: route.FullRoute, operationMode: AppendRoadBoundariesMode) -> bool:
        """
        appendRoadBoundaries( (RssSceneCreation)arg1, (RssObjectData)egoObjectData, (FullRoute)route, (AppendRoadBoundariesMode)operationMode) -> bool :

            C++ signature :
                bool appendRoadBoundaries(ad::rss::map::RssSceneCreation {lvalue},ad::rss::map::RssObjectData,ad::map::route::FullRoute,ad::rss::map::RssSceneCreation::AppendRoadBoundariesMode)"""

    def appendScenes(self, *args, **kwargs) -> bool:
        """
        appendScenes( (RssSceneCreation)arg1, (RssObjectData)egoObjectData, (FullRoute)egoRoute, (RssObjectData)otherObjectData, (RestrictSpeedLimitMode)restrictSpeedLimitMode, (LandmarkIdSet)greenTrafficLights, (RssMode)mode) -> bool :

            C++ signature :
                bool appendScenes(ad::rss::map::RssSceneCreation {lvalue},ad::rss::map::RssObjectData,ad::map::route::FullRoute,ad::rss::map::RssObjectData,ad::rss::map::RssSceneCreation::RestrictSpeedLimitMode,std::set<ad::map::landmark::LandmarkId, std::less<ad::map::landmark::LandmarkId>, std::allocator<ad::map::landmark::LandmarkId> >,ad::rss::map::RssMode)"""

    def getWorldModel(self) -> rss.world.WorldModel:
        """
        getWorldModel( (RssSceneCreation)arg1) -> WorldModel :

            C++ signature :
                ad::rss::world::WorldModel getWorldModel(ad::rss::map::RssSceneCreation {lvalue})"""
    @classmethod
    def __reduce__(cls): ...
