from carla.libcarla import _CarlaEnum

from . import *

INVALID = TrafficType.INVALID
LEFT_HAND_TRAFFIC = TrafficType.LEFT_HAND_TRAFFIC
RIGHT_HAND_TRAFFIC = TrafficType.RIGHT_HAND_TRAFFIC

class MapMetaData:
    def assign(self, arg1: MapMetaData, other: MapMetaData) -> MapMetaData:
        """

        assign( (MapMetaData)arg1, (MapMetaData)other) -> MapMetaData :

            C++ signature :
                ad::map::access::MapMetaData {lvalue} assign(ad::map::access::MapMetaData {lvalue},ad::map::access::MapMetaData)
        """
        ...

    @property
    def trafficType(self) -> TrafficType: ...

class PartitionId:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: PartitionId, other: PartitionId) -> PartitionId:
        """

        assign( (PartitionId)arg1, (PartitionId)other) -> PartitionId :

            C++ signature :
                ad::map::access::PartitionId {lvalue} assign(ad::map::access::PartitionId {lvalue},ad::map::access::PartitionId)
        """
        ...

    cMaxValue: float = 18446744073709551615

    cMinValue: float = 0

    def ensureValid(self, arg1: PartitionId) -> None:
        """

        ensureValid( (PartitionId)arg1) -> None :

            C++ signature :
                void ensureValid(ad::map::access::PartitionId {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: PartitionId) -> None:
        """

        ensureValidNonZero( (PartitionId)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::map::access::PartitionId {lvalue})
        """
        ...

    def getMax(self) -> PartitionId:
        """

        getMax() -> PartitionId :

            C++ signature :
                ad::map::access::PartitionId getMax()
        """
        ...

    def getMin(self) -> PartitionId:
        """

        getMin() -> PartitionId :

            C++ signature :
                ad::map::access::PartitionId getMin()
        """
        ...

class PartitionIdList:
    def append(self, arg1: PartitionIdList, arg2: PartitionId) -> None:
        """

        append( (PartitionIdList)arg1, (PartitionId)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::access::PartitionId, std::allocator<ad::map::access::PartitionId> > {lvalue},ad::map::access::PartitionId)
        """
        ...

    def count(self, arg1: PartitionIdList, arg2: PartitionId) -> int:
        """

        count( (PartitionIdList)arg1, (PartitionId)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::access::PartitionId, std::allocator<ad::map::access::PartitionId> > {lvalue},ad::map::access::PartitionId)
        """
        ...

    def extend(self, arg1: PartitionIdList, arg2: object) -> None:
        """

        extend( (PartitionIdList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::access::PartitionId, std::allocator<ad::map::access::PartitionId> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: PartitionIdList, arg2: PartitionId) -> int:
        """

        index( (PartitionIdList)arg1, (PartitionId)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::access::PartitionId, std::allocator<ad::map::access::PartitionId> > {lvalue},ad::map::access::PartitionId)
        """
        ...

    def insert(self, arg1: PartitionIdList, arg2: int, arg3: PartitionId) -> None:
        """

        insert( (PartitionIdList)arg1, (int)arg2, (PartitionId)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::access::PartitionId, std::allocator<ad::map::access::PartitionId> > {lvalue},long,ad::map::access::PartitionId)
        """
        ...

    def reverse(self, arg1: PartitionIdList) -> None:
        """

        reverse( (PartitionIdList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::access::PartitionId, std::allocator<ad::map::access::PartitionId> > {lvalue})
        """
        ...

    def sort(self, arg1: PartitionIdList) -> None:
        """

        sort( (PartitionIdList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::map::access::PartitionId, std::allocator<ad::map::access::PartitionId> > {lvalue})
        """
        ...

class TrafficType(int, _CarlaEnum):
    INVALID = 0
    LEFT_HAND_TRAFFIC = 1
    RIGHT_HAND_TRAFFIC = 2

class numeric_limits_less__ad_scope_map_scope_access_scope_PartitionId__greater_:
    def epsilon(self) -> PartitionId:
        """

        epsilon() -> PartitionId :

            C++ signature :
                ad::map::access::PartitionId epsilon()
        """
        ...

    def lowest(self) -> PartitionId:
        """

        lowest() -> PartitionId :

            C++ signature :
                ad::map::access::PartitionId lowest()
        """
        ...

    def max(self) -> PartitionId:
        """

        max() -> PartitionId :

            C++ signature :
                ad::map::access::PartitionId max()
        """
        ...
