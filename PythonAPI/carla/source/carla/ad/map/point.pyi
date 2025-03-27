from typing import overload

import ad

from . import *

class Altitude(ad._Calculable):
    cMaxValue: float = ...
    cMinValue: float = ...
    cPrecisionValue: float = 0.001

class BoundingSphere:
    def assign(self, arg1: BoundingSphere, other: BoundingSphere) -> BoundingSphere:
        """

        assign( (BoundingSphere)arg1, (BoundingSphere)other) -> BoundingSphere :

            C++ signature :
                ad::map::point::BoundingSphere {lvalue} assign(ad::map::point::BoundingSphere {lvalue},ad::map::point::BoundingSphere)
        """
        ...

    @property
    def center(self) -> ECEFPoint: ...

    @property
    def radius(self) -> ad.physics.Distance: ...

class CoordinateTransform:
    def ECEF2ENU(self, arg1: CoordinateTransform, pt: ECEFPoint) -> ENUPoint:
        """

        ECEF2ENU( (CoordinateTransform)arg1, (ECEFPoint)pt) -> ENUPoint :

            C++ signature :
                ad::map::point::ENUPoint ECEF2ENU(ad::map::point::CoordinateTransform {lvalue},ad::map::point::ECEFPoint)
        """
        ...

    def ECEF2Geo(self, arg1: CoordinateTransform, pt: ECEFPoint) -> GeoPoint:
        """

        ECEF2Geo( (CoordinateTransform)arg1, (ECEFPoint)pt) -> GeoPoint :

            C++ signature :
                ad::map::point::GeoPoint ECEF2Geo(ad::map::point::CoordinateTransform {lvalue},ad::map::point::ECEFPoint)
        """
        ...

    def ENU2ECEF(self, arg1: CoordinateTransform, pt: ENUPoint) -> ECEFPoint:
        """

        ENU2ECEF( (CoordinateTransform)arg1, (ENUPoint)pt) -> ECEFPoint :

            C++ signature :
                ad::map::point::ECEFPoint ENU2ECEF(ad::map::point::CoordinateTransform {lvalue},ad::map::point::ENUPoint)
        """
        ...

    def ENU2Geo(self, arg1: CoordinateTransform, pt: ENUPoint) -> GeoPoint:
        """

        ENU2Geo( (CoordinateTransform)arg1, (ENUPoint)pt) -> GeoPoint :

            C++ signature :
                ad::map::point::GeoPoint ENU2Geo(ad::map::point::CoordinateTransform {lvalue},ad::map::point::ENUPoint)
        """
        ...

    @property
    def ENURef(self) -> int: ...

    @property
    def ENUReferencePoint(self) -> GeoPoint: ...

    @property
    def ENUValid(self) -> bool: ...

    def Geo2ECEF(self, arg1: CoordinateTransform, pt: GeoPoint) -> ECEFPoint:
        """

        Geo2ECEF( (CoordinateTransform)arg1, (GeoPoint)pt) -> ECEFPoint :

            C++ signature :
                ad::map::point::ECEFPoint Geo2ECEF(ad::map::point::CoordinateTransform {lvalue},ad::map::point::GeoPoint)
        """
        ...

    def Geo2ENU(self, arg1: CoordinateTransform, pt: GeoPoint) -> ENUPoint:
        """

        Geo2ENU( (CoordinateTransform)arg1, (GeoPoint)pt) -> ENUPoint :

            C++ signature :
                ad::map::point::ENUPoint Geo2ENU(ad::map::point::CoordinateTransform {lvalue},ad::map::point::GeoPoint)
        """
        ...

    def WGS84_R(self, lat: Latitude) -> ad.physics.Distance:
        """

        WGS84_R( (Latitude)lat) -> Distance :

            C++ signature :
                ad::physics::Distance WGS84_R(ad::map::point::Latitude)
        """
        ...

    # NOTE: Overloads might not be correct, # TODO

    @overload
    def convert(self, arg1: CoordinateTransform, x: ENUPoint, y: GeoPoint) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, x: GeoPoint, y: ECEFPoint) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, x: ECEFPoint, y: GeoPoint) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, x: ECEFPoint, y: ENUPoint) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, x: ENUPoint, y: ECEFPoint) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, x: GeoPoint, y: ENUPoint) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, x: ENUPoint, y: GeoPoint) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, xs: GeoEdge, ys: ECEFEdge) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, xs: ENUEdge, ys: ECEFEdge) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, xs: ECEFEdge, ys: GeoEdge) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, xs: ENUEdge, ys: GeoEdge) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, xs: ECEFEdge, ys: ENUEdge) -> None:
        ...

    @overload
    def convert(self, arg1: CoordinateTransform, xs: GeoEdge, ys: ENUEdge) -> None:
        ...

    def convert(self, arg1: CoordinateTransform, x, y) -> None:
        """
        convert( (CoordinateTransform)arg1, (ECEFPoint)x, (ENUPoint)y) -> None :

            C++ signature :
                void convert(ad::map::point::CoordinateTransform {lvalue},ad::map::point::ECEFPoint,ad::map::point::ENUPoint {lvalue})

        convert( (CoordinateTransform)arg1, (ENUPoint)x, (ECEFPoint)y) -> None :

            C++ signature :
                void convert(ad::map::point::CoordinateTransform {lvalue},ad::map::point::ENUPoint,ad::map::point::ECEFPoint {lvalue})

        convert( (CoordinateTransform)arg1, (GeoEdge)xs, (ECEFEdge)ys) -> None :

            C++ signature :
                void convert(ad::map::point::CoordinateTransform {lvalue},std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> >,std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > {lvalue})

        convert( (CoordinateTransform)arg1, (ENUEdge)xs, (ECEFEdge)ys) -> None :

            C++ signature :
                void convert(ad::map::point::CoordinateTransform {lvalue},std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> >,std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> > {lvalue})

        convert( (CoordinateTransform)arg1, (ECEFEdge)xs, (GeoEdge)ys) -> None :

            C++ signature :
                void convert(ad::map::point::CoordinateTransform {lvalue},std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> >,std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue})

        convert( (CoordinateTransform)arg1, (ENUEdge)xs, (GeoEdge)ys) -> None :

            C++ signature :
                void convert(ad::map::point::CoordinateTransform {lvalue},std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> >,std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue})

        convert( (CoordinateTransform)arg1, (ECEFEdge)xs, (ENUEdge)ys) -> None :

            C++ signature :
                void convert(ad::map::point::CoordinateTransform {lvalue},std::vector<ad::map::point::ECEFPoint, std::allocator<ad::map::point::ECEFPoint> >,std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > {lvalue})

        convert( (CoordinateTransform)arg1, (GeoEdge)xs, (ENUEdge)ys) -> None :

            C++ signature :
                void convert(ad::map::point::CoordinateTransform {lvalue},std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> >,std::vector<ad::map::point::ENUPoint, std::allocator<ad::map::point::ENUPoint> > {lvalue})
        """
        ...

    def geocentricLatitude(self, lat: Latitude) -> float:
        """

        geocentricLatitude( (Latitude)lat) -> float :

            C++ signature :
                double geocentricLatitude(ad::map::point::Latitude)
        """
        ...

    def setGeoProjection(self, arg1: CoordinateTransform, geo_projection: str) -> bool:
        """

        setGeoProjection( (CoordinateTransform)arg1, (str)geo_projection) -> bool :

            C++ signature :
                bool setGeoProjection(ad::map::point::CoordinateTransform {lvalue},std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)
        """
        ...

class ECEFCoordinate(ad._Calculable):
    cMaxValue: float = 1000000000.0
    cMinValue: float = -1000000000.0
    cPrecisionValue: float = 0.001

class ECEFEdge(ad._VectorSequence[ECEFPoint]):
    ...

class ECEFHeading:
    def assign(self, arg1: ECEFHeading, other: ECEFHeading) -> ECEFHeading:
        """

        assign( (ECEFHeading)arg1, (ECEFHeading)other) -> ECEFHeading :

            C++ signature :
                ad::map::point::ECEFHeading {lvalue} assign(ad::map::point::ECEFHeading {lvalue},ad::map::point::ECEFHeading)
        """
        ...

    @property
    def x(self) -> ECEFCoordinate: ...

    @property
    def y(self) -> ECEFCoordinate: ...

    @property
    def z(self) -> ECEFCoordinate: ...

class ECEFPoint:
    def assign(self, arg1: ECEFPoint, other: ECEFPoint) -> ECEFPoint:
        """

        assign( (ECEFPoint)arg1, (ECEFPoint)other) -> ECEFPoint :

            C++ signature :
                ad::map::point::ECEFPoint {lvalue} assign(ad::map::point::ECEFPoint {lvalue},ad::map::point::ECEFPoint)
        """
        ...

    @property
    def x(self) -> ECEFCoordinate: ...

    @property
    def y(self) -> ECEFCoordinate: ...

    @property
    def z(self) -> ECEFCoordinate: ...

class ENUCoordinate(ad._Calculable):
    cMaxValue: float = 1000000.0
    cMinValue: float = -1000000.0
    cPrecisionValue: float = 0.001

class ENUEdge(ad._VectorSequence[ENUPoint]):
    ...

class ENUEdgeCache:
    def assign(self, arg1: ENUEdgeCache, other: ENUEdgeCache) -> ENUEdgeCache:
        """

        assign( (ENUEdgeCache)arg1, (ENUEdgeCache)other) -> ENUEdgeCache :

            C++ signature :
                ad::map::point::ENUEdgeCache {lvalue} assign(ad::map::point::ENUEdgeCache {lvalue},ad::map::point::ENUEdgeCache)
        """
        ...

    @property
    def enuEdge(self) -> ENUEdge: ...

    @property
    def enuVersion(self) -> int: ...

class ENUHeading(ad._Calculable):
    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 0.0001

class ENUPoint(ad._FloatLike):
    def assign(self, arg1: ENUPoint, other: ENUPoint) -> ENUPoint:
        """

        assign( (ENUPoint)arg1, (ENUPoint)other) -> ENUPoint :

            C++ signature :
                ad::map::point::ENUPoint {lvalue} assign(ad::map::point::ENUPoint {lvalue},ad::map::point::ENUPoint)
        """
        ...

    @property
    def x(self) -> ENUCoordinate: ...

    @property
    def y(self) -> ENUCoordinate: ...

    @property
    def z(self) -> ENUCoordinate: ...

class GeoEdge:
    def append(self, arg1: GeoEdge, arg2: GeoPoint) -> None:
        """

        append( (GeoEdge)arg1, (GeoPoint)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue},ad::map::point::GeoPoint)
        """
        ...

    def count(self, arg1: GeoEdge, arg2: GeoPoint) -> int:
        """

        count( (GeoEdge)arg1, (GeoPoint)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue},ad::map::point::GeoPoint)
        """
        ...

    def extend(self, arg1: GeoEdge, arg2: object) -> None:
        """

        extend( (GeoEdge)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: GeoEdge, arg2: GeoPoint) -> int:
        """

        index( (GeoEdge)arg1, (GeoPoint)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue},ad::map::point::GeoPoint)
        """
        ...

    def insert(self, arg1: GeoEdge, arg2: int, arg3: GeoPoint) -> None:
        """

        insert( (GeoEdge)arg1, (int)arg2, (GeoPoint)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue},long,ad::map::point::GeoPoint)
        """
        ...

    def reverse(self, arg1: GeoEdge) -> None:
        """

        reverse( (GeoEdge)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::point::GeoPoint, std::allocator<ad::map::point::GeoPoint> > {lvalue})
        """
        ...

class GeoPoint:
    @property
    def altitude(self) -> Altitude: ...

    def assign(self, arg1: GeoPoint, other: GeoPoint) -> GeoPoint:
        """

        assign( (GeoPoint)arg1, (GeoPoint)other) -> GeoPoint :

            C++ signature :
                ad::map::point::GeoPoint {lvalue} assign(ad::map::point::GeoPoint {lvalue},ad::map::point::GeoPoint)
        """
        ...

    @property
    def latitude(self) -> Latitude: ...

    @property
    def longitude(self) -> Longitude: ...

class Geometry:
    def assign(self, arg1: Geometry, other: Geometry) -> Geometry:
        """

        assign( (Geometry)arg1, (Geometry)other) -> Geometry :

            C++ signature :
                ad::map::point::Geometry {lvalue} assign(ad::map::point::Geometry {lvalue},ad::map::point::Geometry)
        """
        ...

    @property
    def ecefEdge(self) -> ECEFEdge: ...

    @property
    def isClosed(self) -> bool: ...

    @property
    def isValid(self) -> bool: ...

    @property
    def length(self) -> ad.physics.Distance: ...

    @property
    def private_enuEdgeCache(self) -> ENUEdgeCache: ...

class Latitude(ad._FloatLike):
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: Latitude, other: Latitude) -> Latitude:
        """

        assign( (Latitude)arg1, (Latitude)other) -> Latitude :

            C++ signature :
                ad::map::point::Latitude {lvalue} assign(ad::map::point::Latitude {lvalue},ad::map::point::Latitude)
        """
        ...

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 1e-08

    def ensureValid(self, arg1: Latitude) -> None:
        """

        ensureValid( (Latitude)arg1) -> None :

            C++ signature :
                void ensureValid(ad::map::point::Latitude {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: Latitude) -> None:
        """

        ensureValidNonZero( (Latitude)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::map::point::Latitude {lvalue})
        """
        ...

    def getMax(self) -> Latitude:
        """

        getMax() -> Latitude :

            C++ signature :
                ad::map::point::Latitude getMax()
        """
        ...

    def getMin(self) -> Latitude:
        """

        getMin() -> Latitude :

            C++ signature :
                ad::map::point::Latitude getMin()
        """
        ...

    def getPrecision(self) -> Latitude:
        """

        getPrecision() -> Latitude :

            C++ signature :
                ad::map::point::Latitude getPrecision()
        """
        ...

class Longitude(ad._FloatLike):
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: Longitude, other: Longitude) -> Longitude:
        """

        assign( (Longitude)arg1, (Longitude)other) -> Longitude :

            C++ signature :
                ad::map::point::Longitude {lvalue} assign(ad::map::point::Longitude {lvalue},ad::map::point::Longitude)
        """
        ...

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 1e-08

    def ensureValid(self, arg1: Longitude) -> None:
        """

        ensureValid( (Longitude)arg1) -> None :

            C++ signature :
                void ensureValid(ad::map::point::Longitude {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: Longitude) -> None:
        """

        ensureValidNonZero( (Longitude)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::map::point::Longitude {lvalue})
        """
        ...

    def getMax(self) -> Longitude:
        """

        getMax() -> Longitude :

            C++ signature :
                ad::map::point::Longitude getMax()
        """
        ...

    def getMin(self) -> Longitude:
        """

        getMin() -> Longitude :

            C++ signature :
                ad::map::point::Longitude getMin()
        """
        ...

    def getPrecision(self) -> Longitude:
        """

        getPrecision() -> Longitude :

            C++ signature :
                ad::map::point::Longitude getPrecision()
        """
        ...

class ParaPoint:
    def assign(self, arg1: ParaPoint, other: ParaPoint) -> ParaPoint:
        """

        assign( (ParaPoint)arg1, (ParaPoint)other) -> ParaPoint :

            C++ signature :
                ad::map::point::ParaPoint {lvalue} assign(ad::map::point::ParaPoint {lvalue},ad::map::point::ParaPoint)
        """
        ...

    @property
    def laneId(self) -> lane.LaneId: ...

    @property
    def parametricOffset(self) -> ad.physics.ParametricValue: ...

class ParaPointList:
    def append(self, arg1: ParaPointList, arg2: ParaPoint) -> None:
        """

        append( (ParaPointList)arg1, (ParaPoint)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> > {lvalue},ad::map::point::ParaPoint)
        """
        ...

    def count(self, arg1: ParaPointList, arg2: ParaPoint) -> int:
        """

        count( (ParaPointList)arg1, (ParaPoint)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> > {lvalue},ad::map::point::ParaPoint)
        """
        ...

    def extend(self, arg1: ParaPointList, arg2: object) -> None:
        """

        extend( (ParaPointList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ParaPointList, arg2: ParaPoint) -> int:
        """

        index( (ParaPointList)arg1, (ParaPoint)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> > {lvalue},ad::map::point::ParaPoint)
        """
        ...

    def insert(self, arg1: ParaPointList, arg2: int, arg3: ParaPoint) -> None:
        """

        insert( (ParaPointList)arg1, (int)arg2, (ParaPoint)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> > {lvalue},long,ad::map::point::ParaPoint)
        """
        ...

    def reverse(self, arg1: ParaPointList) -> None:
        """

        reverse( (ParaPointList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::map::point::ParaPoint, std::allocator<ad::map::point::ParaPoint> > {lvalue})
        """
        ...

class numeric_limits_less__ad_scope_map_scope_point_scope_Altitude__greater_:
    def epsilon(self) -> Altitude:
        """

        epsilon() -> Altitude :

            C++ signature :
                ad::map::point::Altitude epsilon()
        """
        ...

    def lowest(self) -> Altitude:
        """

        lowest() -> Altitude :

            C++ signature :
                ad::map::point::Altitude lowest()
        """
        ...

    def max(self) -> Altitude:
        """

        max() -> Altitude :

            C++ signature :
                ad::map::point::Altitude max()
        """
        ...

class numeric_limits_less__ad_scope_map_scope_point_scope_ECEFCoordinate__greater_:
    def epsilon(self) -> ECEFCoordinate:
        """

        epsilon() -> ECEFCoordinate :

            C++ signature :
                ad::map::point::ECEFCoordinate epsilon()
        """
        ...

    def lowest(self) -> ECEFCoordinate:
        """

        lowest() -> ECEFCoordinate :

            C++ signature :
                ad::map::point::ECEFCoordinate lowest()
        """
        ...

    def max(self) -> ECEFCoordinate:
        """

        max() -> ECEFCoordinate :

            C++ signature :
                ad::map::point::ECEFCoordinate max()
        """
        ...

class numeric_limits_less__ad_scope_map_scope_point_scope_ENUCoordinate__greater_:
    def epsilon(self) -> ENUCoordinate:
        """

        epsilon() -> ENUCoordinate :

            C++ signature :
                ad::map::point::ENUCoordinate epsilon()
        """
        ...

    def lowest(self) -> ENUCoordinate:
        """

        lowest() -> ENUCoordinate :

            C++ signature :
                ad::map::point::ENUCoordinate lowest()
        """
        ...

    def max(self) -> ENUCoordinate:
        """

        max() -> ENUCoordinate :

            C++ signature :
                ad::map::point::ENUCoordinate max()
        """
        ...

class numeric_limits_less__ad_scope_map_scope_point_scope_ENUHeading__greater_:
    def epsilon(self) -> ENUHeading:
        """

        epsilon() -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading epsilon()
        """
        ...

    def lowest(self) -> ENUHeading:
        """

        lowest() -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading lowest()
        """
        ...

    def max(self) -> ENUHeading:
        """

        max() -> ENUHeading :

            C++ signature :
                ad::map::point::ENUHeading max()
        """
        ...

class numeric_limits_less__ad_scope_map_scope_point_scope_Latitude__greater_:
    def epsilon(self) -> Latitude:
        """

        epsilon() -> Latitude :

            C++ signature :
                ad::map::point::Latitude epsilon()
        """
        ...

    def lowest(self) -> Latitude:
        """

        lowest() -> Latitude :

            C++ signature :
                ad::map::point::Latitude lowest()
        """
        ...

    def max(self) -> Latitude:
        """

        max() -> Latitude :

            C++ signature :
                ad::map::point::Latitude max()
        """
        ...

class numeric_limits_less__ad_scope_map_scope_point_scope_Longitude__greater_:
    def epsilon(self) -> Longitude:
        """

        epsilon() -> Longitude :

            C++ signature :
                ad::map::point::Longitude epsilon()
        """
        ...

    def lowest(self) -> Longitude:
        """

        lowest() -> Longitude :

            C++ signature :
                ad::map::point::Longitude lowest()
        """
        ...

    def max(self) -> Longitude:
        """

        max() -> Longitude :

            C++ signature :
                ad::map::point::Longitude max()
        """
        ...
