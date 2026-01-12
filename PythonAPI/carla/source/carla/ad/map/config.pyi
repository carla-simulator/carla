
from carla import ad

class MapConfigFileHandler:
    @property
    def Initialized(self) -> bool: ...

    @property
    def adMapEntry(self) -> MapEntry: ...

    @property
    def configFileName(self) -> str: ...

    @property
    def defaultEnuReference(self) -> ad.map.point.GeoPoint: ...

    @property
    def defaultEnuReferenceAvailable(self) -> bool: ...

    def isInitializedWithFilename(self, arg1: MapConfigFileHandler, configFileName: str) -> bool:
        """

        isInitializedWithFilename( (MapConfigFileHandler)arg1, (str)configFileName) -> bool :

            C++ signature :
                bool isInitializedWithFilename(ad::map::config::MapConfigFileHandler {lvalue},std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)
        """
        ...

    @property
    def pointsOfInterest(self) -> PointOfInterest: ...

    def readConfig(self, arg1: MapConfigFileHandler, configFileName: str) -> bool:
        """

        readConfig( (MapConfigFileHandler)arg1, (str)configFileName) -> bool :

            C++ signature :
                bool readConfig(ad::map::config::MapConfigFileHandler {lvalue},std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)
        """
        ...

    def reset(self, arg1: MapConfigFileHandler) -> None:
        """

        reset( (MapConfigFileHandler)arg1) -> None :

            C++ signature :
                void reset(ad::map::config::MapConfigFileHandler {lvalue})
        """
        ...

class MapEntry:
    def assign(self, arg1: MapEntry, other: MapEntry) -> MapEntry:
        """

        assign( (MapEntry)arg1, (MapEntry)other) -> MapEntry :

            C++ signature :
                ad::map::config::MapEntry {lvalue} assign(ad::map::config::MapEntry {lvalue},ad::map::config::MapEntry)
        """
        ...

    @property
    def filename(self) -> str: ...

    @property
    def openDriveDefaultIntersectionType(self) -> ad.map.intersection.IntersectionType: ...

    @property
    def openDriveDefaultTrafficLightType(self) -> ad.map.landmark.TrafficLightType: ...

    @property
    def openDriveOverlapMargin(self) -> ad.physics.Distance: ...

class PointOfInterest:
    def assign(self, arg1: PointOfInterest, other: PointOfInterest) -> PointOfInterest:
        """

        assign( (PointOfInterest)arg1, (PointOfInterest)other) -> PointOfInterest :

            C++ signature :
                ad::map::config::PointOfInterest {lvalue} assign(ad::map::config::PointOfInterest {lvalue},ad::map::config::PointOfInterest)
        """
        ...

    @property
    def geoPoint(self) -> ad.map.point.GeoPoint: ...

    @property
    def name(self) -> str: ...
