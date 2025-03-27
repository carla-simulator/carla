from typing import overload
from ... import ad
from .. import _Vector
from . import *

class DebugDrawing:
    class DebugLine:
        ...

    class DebugPoint:
        ...

    class DebugPolygon:
        ...

    class NullDeleter:
        ...

    def drawLine(self, arg1: DebugDrawing, line: object, color: str, ns: str) -> None:
        """

        drawLine( (DebugDrawing)arg1, (object)line [, (str)color='white' [, (str)ns='']]) -> None :

            C++ signature :
                void drawLine(ad::rss::unstructured::DebugDrawing {lvalue},boost::geometry::model::linestring<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, std::vector, std::allocator> [,std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >='white' [,std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >='']])
        """
        ...

    def drawPolygon(self, arg1: DebugDrawing, polygon: object, color: str, ns: str) -> None:
        """

        drawPolygon( (DebugDrawing)arg1, (object)polygon [, (str)color='white' [, (str)ns='']]) -> None :

            C++ signature :
                void drawPolygon(ad::rss::unstructured::DebugDrawing {lvalue},boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, false, true, std::vector, std::vector, std::allocator, std::allocator> [,std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >='white' [,std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >='']])
        """
        ...

    def enable(self, arg1: DebugDrawing, value: bool) -> None:
        """

        enable( (DebugDrawing)arg1, (bool)value) -> None :

            C++ signature :
                void enable(ad::rss::unstructured::DebugDrawing {lvalue},bool)
        """
        ...

    def getInstance(self) -> DebugDrawing:
        """

        getInstance() -> DebugDrawing :

            C++ signature :
                std::shared_ptr<ad::rss::unstructured::DebugDrawing> getInstance()
        """
        ...

    def isEnabled(self, arg1: DebugDrawing) -> bool:
        """

        isEnabled( (DebugDrawing)arg1) -> bool :

            C++ signature :
                bool isEnabled(ad::rss::unstructured::DebugDrawing {lvalue})
        """
        ...

    @property
    def mEnabled(self) -> bool: ...

    @property
    def mLines(self) -> vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_: ...

    @property
    def mPolygons(self) -> vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_: ...

    def reset(self, arg1: DebugDrawing) -> None:
        """

        reset( (DebugDrawing)arg1) -> None :

            C++ signature :
                void reset(ad::rss::unstructured::DebugDrawing {lvalue})
        """
        ...

class vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_(_Vector[DebugDrawing.DebugLine]):
    ...

class vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_(_Vector[DebugDrawing.DebugPoint]):
    ...

class vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_(_Vector[DebugDrawing.DebugPolygon]):
    ...

def collides(trajectorySet1: ad.physics.Distance2DList, trajectorySet2: ad.physics.Distance2DList):
    """
    collides( (Distance2DList)trajectorySet1, (Distance2DList)trajectorySet2) -> bool :

        C++ signature :
            bool collides(std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> >,std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> >)"""
def combinePolygon(*args, **kwargs):
    """
    combinePolygon( (object)a, (object)b, (object)result) -> bool :

        C++ signature :
            bool combinePolygon(boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, false, true, std::vector, std::vector, std::allocator, std::allocator>,boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, false, true, std::vector, std::vector, std::allocator, std::allocator>,boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, false, true, std::vector, std::vector, std::allocator, std::allocator> {lvalue})"""
def getCircleOrigin(*args, **kwargs):
    """
    getCircleOrigin( (object)point, (Distance)radius, (Angle)angle) -> object :

        C++ signature :
            boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> getCircleOrigin(boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>,ad::physics::Distance,ad::physics::Angle)"""

@overload
def getHeadingOverlap(a: state.HeadingRange, b: state.HeadingRange, overlapRanges: state.HeadingRangeVector) -> bool:
    """
    get the overlap between two angle ranges 
    [out] : overlapRanges
    
    getHeadingOverlap( (HeadingRange)a, (HeadingRange)b, (HeadingRangeVector)overlapRanges) -> bool :

        C++ signature :
            bool getHeadingOverlap(ad::rss::state::HeadingRange,ad::rss::state::HeadingRange,std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue})

    getHeadingOverlap( (HeadingRange)headingRange, (HeadingRangeVector)overlapRanges) -> bool :

        C++ signature :
            bool getHeadingOverlap(ad::rss::state::HeadingRange,std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue})"""

@overload
def getHeadingOverlap(headingRange: state.HeadingRange, overlapRanges: state.HeadingRangeVector) -> bool:
    """
    get the overlap between an angle range and a heading range
    [in,out]: overlapRanges
    
    getHeadingOverlap( (HeadingRange)a, (HeadingRange)b, (HeadingRangeVector)overlapRanges) -> bool :

        C++ signature :
            bool getHeadingOverlap(ad::rss::state::HeadingRange,ad::rss::state::HeadingRange,std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue})

    getHeadingOverlap( (HeadingRange)headingRange, (HeadingRangeVector)overlapRanges) -> bool :

        C++ signature :
            bool getHeadingOverlap(ad::rss::state::HeadingRange,std::vector<ad::rss::state::HeadingRange, std::allocator<ad::rss::state::HeadingRange> > {lvalue})"""
            
            
def getPointOnCircle(origin: Unknown, radius: ad.physics.Distance, angle: ad.physics.Angle) -> Unknown:
    """
    getPointOnCircle( (object)origin, (Distance)radius, (Angle)angle) -> object :

        C++ signature :
            boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> getPointOnCircle(boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>,ad::physics::Distance,ad::physics::Angle)"""
def isInsideHeadingRange(angle: ad.physics.Angle, range: ad.rss.state.HeadingRange):
    """
    isInsideHeadingRange( (Angle)angle, (HeadingRange)range) -> bool :

        C++ signature :
            bool isInsideHeadingRange(ad::physics::Angle,ad::rss::state::HeadingRange)"""
def rotateAroundPoint(*args, **kwargs):
    """
    rotateAroundPoint( (object)origin, (object)relativePoint, (Angle)angle) -> object :

        C++ signature :
            boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> rotateAroundPoint(boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>,boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>,ad::physics::Angle)"""
def toDistance(*args, **kwargs):
    """
    toDistance( (object)point) -> Distance2D :

        C++ signature :
            ad::physics::Distance2D toDistance(boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>)"""
            

@overload
def toPoint(distance: ad.physics.Distance2D) -> Unknown:
    """
    toPoint( (Distance2D)distance) -> object :

        C++ signature :
            boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> toPoint(ad::physics::Distance2D)

    toPoint( (Distance)distanceX, (Distance)distanceY) -> object :

        C++ signature :
            boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> toPoint(ad::physics::Distance,ad::physics::Distance)
    """
            
@overload
def toPoint(distanceX: ad.physics.Distance, distanceY: ad.physics.Distance) -> Unknown:
    """
    toPoint( (Distance2D)distance) -> object :

        C++ signature :
            boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> toPoint(ad::physics::Distance2D)

    toPoint( (Distance)distanceX, (Distance)distanceY) -> object :

        C++ signature :
            boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> toPoint(ad::physics::Distance,ad::physics::Distance)
    """

def toPolygon(trajectorySet: ad.physics.Distance2DList, polygon):
    """
    toPolygon( (Distance2DList)trajectorySet, (object)polygon) -> None :

        C++ signature :
            void toPolygon(std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> >,boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, false, true, std::vector, std::vector, std::allocator, std::allocator> {lvalue})"""
def toTrajectorySet(polygon, trajectorySet: ad.physics.Distance2DList) -> None:
    """
    toTrajectorySet( (object)polygon, (Distance2DList)trajectorySet) -> None :

        C++ signature :
            void toTrajectorySet(boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, false, true, std::vector, std::vector, std::allocator, std::allocator>,std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> > {lvalue})"""

def to_string(value: object) -> str:
    """
    to_string( (object)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>)

    to_string( (object)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, false, true, std::vector, std::vector, std::allocator, std::allocator>)

    to_string( (object)value) -> str :

        C++ signature :
            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > to_string(boost::geometry::model::linestring<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, std::vector, std::allocator>)"""
