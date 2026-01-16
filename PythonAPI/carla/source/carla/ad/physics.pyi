
from collections.abc import MutableSequence

from ad import _FloatLike, _Calculable, _Assignable, _SortableSequence, _VectorSequence

class Acceleration(_Calculable):

    cMaxValue: float = 1000.0

    cMinValue: float = -1000.0

    cPrecisionValue: float = 0.0001

class Acceleration3D(_FloatLike, _Assignable):

    @property
    def x(self) -> Acceleration: ...

    @property
    def y(self) -> Acceleration: ...

    @property
    def z(self) -> Acceleration: ...

class Acceleration3DList(_VectorSequence[Acceleration3D]): ...
   
class AccelerationList(_VectorSequence[Acceleration]):
    def append(self, arg1: AccelerationList, arg2: Acceleration) -> None:
        """

        append( (AccelerationList)arg1, (Acceleration)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Acceleration, std::allocator<ad::physics::Acceleration> > {lvalue},ad::physics::Acceleration)
        """
        ...

    def count(self, arg1: AccelerationList, arg2: Acceleration) -> int:
        """

        count( (AccelerationList)arg1, (Acceleration)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Acceleration, std::allocator<ad::physics::Acceleration> > {lvalue},ad::physics::Acceleration)
        """
        ...

    def extend(self, arg1: AccelerationList, arg2: object) -> None:
        """

        extend( (AccelerationList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Acceleration, std::allocator<ad::physics::Acceleration> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: AccelerationList, arg2: Acceleration) -> int:
        """

        index( (AccelerationList)arg1, (Acceleration)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Acceleration, std::allocator<ad::physics::Acceleration> > {lvalue},ad::physics::Acceleration)
        """
        ...

    def insert(self, arg1: AccelerationList, arg2: int, arg3: Acceleration) -> None:
        """

        insert( (AccelerationList)arg1, (int)arg2, (Acceleration)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Acceleration, std::allocator<ad::physics::Acceleration> > {lvalue},long,ad::physics::Acceleration)
        """
        ...

    def reverse(self, arg1: AccelerationList) -> None:
        """

        reverse( (AccelerationList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Acceleration, std::allocator<ad::physics::Acceleration> > {lvalue})
        """
        ...

    def sort(self, arg1: AccelerationList) -> None:
        """

        sort( (AccelerationList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::Acceleration, std::allocator<ad::physics::Acceleration> > {lvalue})
        """
        ...

class AccelerationRange:
    def assign(self, arg1: AccelerationRange, other: AccelerationRange) -> AccelerationRange:
        """

        assign( (AccelerationRange)arg1, (AccelerationRange)other) -> AccelerationRange :

            C++ signature :
                ad::physics::AccelerationRange {lvalue} assign(ad::physics::AccelerationRange {lvalue},ad::physics::AccelerationRange)
        """
        ...

    @property
    def maximum(self) -> Acceleration: ...

    @property
    def minimum(self) -> Acceleration: ...

class AccelerationRangeList:
    def append(self, arg1: AccelerationRangeList, arg2: AccelerationRange) -> None:
        """

        append( (AccelerationRangeList)arg1, (AccelerationRange)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::AccelerationRange, std::allocator<ad::physics::AccelerationRange> > {lvalue},ad::physics::AccelerationRange)
        """
        ...

    def count(self, arg1: AccelerationRangeList, arg2: AccelerationRange) -> int:
        """

        count( (AccelerationRangeList)arg1, (AccelerationRange)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::AccelerationRange, std::allocator<ad::physics::AccelerationRange> > {lvalue},ad::physics::AccelerationRange)
        """
        ...

    def extend(self, arg1: AccelerationRangeList, arg2: object) -> None:
        """

        extend( (AccelerationRangeList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::AccelerationRange, std::allocator<ad::physics::AccelerationRange> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: AccelerationRangeList, arg2: AccelerationRange) -> int:
        """

        index( (AccelerationRangeList)arg1, (AccelerationRange)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::AccelerationRange, std::allocator<ad::physics::AccelerationRange> > {lvalue},ad::physics::AccelerationRange)
        """
        ...

    def insert(self, arg1: AccelerationRangeList, arg2: int, arg3: AccelerationRange) -> None:
        """

        insert( (AccelerationRangeList)arg1, (int)arg2, (AccelerationRange)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::AccelerationRange, std::allocator<ad::physics::AccelerationRange> > {lvalue},long,ad::physics::AccelerationRange)
        """
        ...

    def reverse(self, arg1: AccelerationRangeList) -> None:
        """

        reverse( (AccelerationRangeList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::AccelerationRange, std::allocator<ad::physics::AccelerationRange> > {lvalue})
        """
        ...

class Angle(_FloatLike):
    cMaxValue: float = ...  # likely unbound and depends on platform

    cMinValue: float = ...  # likely unbound and depends on platform

    cPrecisionValue: float = 0.001

class AngleList(_SortableSequence[Angle]):
    ...

class AngleRange:
    def assign(self, arg1: AngleRange, other: AngleRange) -> AngleRange:
        """

        assign( (AngleRange)arg1, (AngleRange)other) -> AngleRange :

            C++ signature :
                ad::physics::AngleRange {lvalue} assign(ad::physics::AngleRange {lvalue},ad::physics::AngleRange)
        """
        ...

    @property
    def maximum(self) -> Angle: ...

    @property
    def minimum(self) -> Angle: ...

class AngleRangeList(_VectorSequence[AngleRange]):
    ...

class AngularAcceleration(_Calculable):
    cMaxValue: float = 1000.0

    cMinValue: float = -1000.0

    cPrecisionValue: float = 0.0001


class AngularAccelerationList(_SortableSequence[AngularAcceleration]):
    ...
    
class AngularVelocity(_Calculable):
   
    cMaxValue: float = 1000.0

    cMinValue: float = -1000.0

    cPrecisionValue: float = 0.001

class AngularVelocity3D:
    def assign(self, arg1: AngularVelocity3D, other: AngularVelocity3D) -> AngularVelocity3D:
        """

        assign( (AngularVelocity3D)arg1, (AngularVelocity3D)other) -> AngularVelocity3D :

            C++ signature :
                ad::physics::AngularVelocity3D {lvalue} assign(ad::physics::AngularVelocity3D {lvalue},ad::physics::AngularVelocity3D)
        """
        ...

    @property
    def x(self) -> AngularVelocity: ...

    @property
    def y(self) -> AngularVelocity: ...

    @property
    def z(self) -> AngularVelocity: ...

class AngularVelocity3DList(_VectorSequence[AngularVelocity3D]):
    ...

class AngularVelocityList(_SortableSequence[AngularVelocity]):
    ...

class Dimension2D:
    def assign(self, arg1: Dimension2D, other: Dimension2D) -> Dimension2D:
        """

        assign( (Dimension2D)arg1, (Dimension2D)other) -> Dimension2D :

            C++ signature :
                ad::physics::Dimension2D {lvalue} assign(ad::physics::Dimension2D {lvalue},ad::physics::Dimension2D)
        """
        ...

    @property
    def length(self) -> Distance: ...

    @property
    def width(self) -> Distance: ...

class Dimension2DList(_VectorSequence[Dimension2D]):
    ...

class Dimension3D:
    def assign(self, arg1: Dimension3D, other: Dimension3D) -> Dimension3D:
        """

        assign( (Dimension3D)arg1, (Dimension3D)other) -> Dimension3D :

            C++ signature :
                ad::physics::Dimension3D {lvalue} assign(ad::physics::Dimension3D {lvalue},ad::physics::Dimension3D)
        """
        ...

    @property
    def height(self) -> Distance: ...

    @property
    def length(self) -> Distance: ...

    @property
    def width(self) -> Distance: ...

class Dimension3DList(_VectorSequence[Dimension3D]):
    ...

class Distance(_Calculable):
    cMaxValue: float = 1000000000.0

    cMinValue: float = -1000000000.0

    cPrecisionValue: float = 0.001


class Distance2D:
    def assign(self, other: Distance2D) -> Distance2D:
        """

        assign( (Distance2D)arg1, (Distance2D)other) -> Distance2D :

            C++ signature :
                ad::physics::Distance2D {lvalue} assign(ad::physics::Distance2D {lvalue},ad::physics::Distance2D)
        """
        ...

    @property
    def x(self) -> Distance: ...

    @property
    def y(self) -> Distance: ...

class Distance2DList(_VectorSequence[Distance2D]):
    ...
    
class Distance3D:
    def assign(self, arg1: Distance3D, other: Distance3D) -> Distance3D:
        """

        assign( (Distance3D)arg1, (Distance3D)other) -> Distance3D :

            C++ signature :
                ad::physics::Distance3D {lvalue} assign(ad::physics::Distance3D {lvalue},ad::physics::Distance3D)
        """
        ...

    @property
    def x(self) -> Distance: ...

    @property
    def y(self) -> Distance: ...

    @property
    def z(self) -> Distance: ...

class Distance3DList(_VectorSequence[Distance3D]):
    ...

class DistanceList(_SortableSequence[Distance]):
    ...

class DistanceSquared(_Calculable):

    cMaxValue: float = 1e+18

    cMinValue: float = -1e+18

    cPrecisionValue: float = 1e-06


class DistanceSquaredList:
    def append(self, arg1: DistanceSquaredList, arg2: DistanceSquared) -> None:
        """

        append( (DistanceSquaredList)arg1, (DistanceSquared)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::DistanceSquared, std::allocator<ad::physics::DistanceSquared> > {lvalue},ad::physics::DistanceSquared)
        """
        ...

    def count(self, arg1: DistanceSquaredList, arg2: DistanceSquared) -> int:
        """

        count( (DistanceSquaredList)arg1, (DistanceSquared)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::DistanceSquared, std::allocator<ad::physics::DistanceSquared> > {lvalue},ad::physics::DistanceSquared)
        """
        ...

    def extend(self, arg1: DistanceSquaredList, arg2: object) -> None:
        """

        extend( (DistanceSquaredList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::DistanceSquared, std::allocator<ad::physics::DistanceSquared> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: DistanceSquaredList, arg2: DistanceSquared) -> int:
        """

        index( (DistanceSquaredList)arg1, (DistanceSquared)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::DistanceSquared, std::allocator<ad::physics::DistanceSquared> > {lvalue},ad::physics::DistanceSquared)
        """
        ...

    def insert(self, arg1: DistanceSquaredList, arg2: int, arg3: DistanceSquared) -> None:
        """

        insert( (DistanceSquaredList)arg1, (int)arg2, (DistanceSquared)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::DistanceSquared, std::allocator<ad::physics::DistanceSquared> > {lvalue},long,ad::physics::DistanceSquared)
        """
        ...

    def reverse(self, arg1: DistanceSquaredList) -> None:
        """

        reverse( (DistanceSquaredList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::DistanceSquared, std::allocator<ad::physics::DistanceSquared> > {lvalue})
        """
        ...

    def sort(self, arg1: DistanceSquaredList) -> None:
        """

        sort( (DistanceSquaredList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::DistanceSquared, std::allocator<ad::physics::DistanceSquared> > {lvalue})
        """
        ...

class Duration(_Calculable):

    cMaxValue: float = 1000000.0

    cMinValue: float = -1000000.0

    cPrecisionValue: float = 0.001

class DurationList(_SortableSequence[Duration]):
    ...

class DurationSquared(_Calculable):

    cMaxValue: float = 1000000000000.0

    cMinValue: float = -1000000000000.0

    cPrecisionValue: float = 1e-06

class DurationSquaredList:
    def append(self, arg1: DurationSquaredList, arg2: DurationSquared) -> None:
        """

        append( (DurationSquaredList)arg1, (DurationSquared)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::DurationSquared, std::allocator<ad::physics::DurationSquared> > {lvalue},ad::physics::DurationSquared)
        """
        ...

    def count(self, arg1: DurationSquaredList, arg2: DurationSquared) -> int:
        """

        count( (DurationSquaredList)arg1, (DurationSquared)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::DurationSquared, std::allocator<ad::physics::DurationSquared> > {lvalue},ad::physics::DurationSquared)
        """
        ...

    def extend(self, arg1: DurationSquaredList, arg2: object) -> None:
        """

        extend( (DurationSquaredList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::DurationSquared, std::allocator<ad::physics::DurationSquared> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: DurationSquaredList, arg2: DurationSquared) -> int:
        """

        index( (DurationSquaredList)arg1, (DurationSquared)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::DurationSquared, std::allocator<ad::physics::DurationSquared> > {lvalue},ad::physics::DurationSquared)
        """
        ...

    def insert(self, arg1: DurationSquaredList, arg2: int, arg3: DurationSquared) -> None:
        """

        insert( (DurationSquaredList)arg1, (int)arg2, (DurationSquared)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::DurationSquared, std::allocator<ad::physics::DurationSquared> > {lvalue},long,ad::physics::DurationSquared)
        """
        ...

    def reverse(self, arg1: DurationSquaredList) -> None:
        """

        reverse( (DurationSquaredList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::DurationSquared, std::allocator<ad::physics::DurationSquared> > {lvalue})
        """
        ...

    def sort(self, arg1: DurationSquaredList) -> None:
        """

        sort( (DurationSquaredList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::DurationSquared, std::allocator<ad::physics::DurationSquared> > {lvalue})
        """
        ...

class MetricRange:
    def assign(self, arg1: MetricRange, other: MetricRange) -> MetricRange:
        """

        assign( (MetricRange)arg1, (MetricRange)other) -> MetricRange :

            C++ signature :
                ad::physics::MetricRange {lvalue} assign(ad::physics::MetricRange {lvalue},ad::physics::MetricRange)
        """
        ...

    @property
    def maximum(self) -> Distance: ...

    @property
    def minimum(self) -> Distance: ...

class MetricRangeList:
    def append(self, arg1: MetricRangeList, arg2: MetricRange) -> None:
        """

        append( (MetricRangeList)arg1, (MetricRange)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::MetricRange, std::allocator<ad::physics::MetricRange> > {lvalue},ad::physics::MetricRange)
        """
        ...

    def count(self, arg1: MetricRangeList, arg2: MetricRange) -> int:
        """

        count( (MetricRangeList)arg1, (MetricRange)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::MetricRange, std::allocator<ad::physics::MetricRange> > {lvalue},ad::physics::MetricRange)
        """
        ...

    def extend(self, arg1: MetricRangeList, arg2: object) -> None:
        """

        extend( (MetricRangeList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::MetricRange, std::allocator<ad::physics::MetricRange> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: MetricRangeList, arg2: MetricRange) -> int:
        """

        index( (MetricRangeList)arg1, (MetricRange)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::MetricRange, std::allocator<ad::physics::MetricRange> > {lvalue},ad::physics::MetricRange)
        """
        ...

    def insert(self, arg1: MetricRangeList, arg2: int, arg3: MetricRange) -> None:
        """

        insert( (MetricRangeList)arg1, (int)arg2, (MetricRange)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::MetricRange, std::allocator<ad::physics::MetricRange> > {lvalue},long,ad::physics::MetricRange)
        """
        ...

    def reverse(self, arg1: MetricRangeList) -> None:
        """

        reverse( (MetricRangeList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::MetricRange, std::allocator<ad::physics::MetricRange> > {lvalue})
        """
        ...

class ParametricRange:
    def assign(self, arg1: ParametricRange, other: ParametricRange) -> ParametricRange:
        """

        assign( (ParametricRange)arg1, (ParametricRange)other) -> ParametricRange :

            C++ signature :
                ad::physics::ParametricRange {lvalue} assign(ad::physics::ParametricRange {lvalue},ad::physics::ParametricRange)
        """
        ...

    @property
    def maximum(self) -> ParametricValue: ...

    @property
    def minimum(self) -> ParametricValue: ...

class ParametricRangeList:
    def append(self, arg1: ParametricRangeList, arg2: ParametricRange) -> None:
        """

        append( (ParametricRangeList)arg1, (ParametricRange)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::ParametricRange, std::allocator<ad::physics::ParametricRange> > {lvalue},ad::physics::ParametricRange)
        """
        ...

    def count(self, arg1: ParametricRangeList, arg2: ParametricRange) -> int:
        """

        count( (ParametricRangeList)arg1, (ParametricRange)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::ParametricRange, std::allocator<ad::physics::ParametricRange> > {lvalue},ad::physics::ParametricRange)
        """
        ...

    def extend(self, arg1: ParametricRangeList, arg2: object) -> None:
        """

        extend( (ParametricRangeList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::ParametricRange, std::allocator<ad::physics::ParametricRange> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ParametricRangeList, arg2: ParametricRange) -> int:
        """

        index( (ParametricRangeList)arg1, (ParametricRange)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::ParametricRange, std::allocator<ad::physics::ParametricRange> > {lvalue},ad::physics::ParametricRange)
        """
        ...

    def insert(self, arg1: ParametricRangeList, arg2: int, arg3: ParametricRange) -> None:
        """

        insert( (ParametricRangeList)arg1, (int)arg2, (ParametricRange)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::ParametricRange, std::allocator<ad::physics::ParametricRange> > {lvalue},long,ad::physics::ParametricRange)
        """
        ...

    def reverse(self, arg1: ParametricRangeList) -> None:
        """

        reverse( (ParametricRangeList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::ParametricRange, std::allocator<ad::physics::ParametricRange> > {lvalue})
        """
        ...

class ParametricValue(_Calculable):

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 1e-06

class ParametricValueList:
    def append(self, arg1: ParametricValueList, arg2: ParametricValue) -> None:
        """

        append( (ParametricValueList)arg1, (ParametricValue)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::ParametricValue, std::allocator<ad::physics::ParametricValue> > {lvalue},ad::physics::ParametricValue)
        """
        ...

    def count(self, arg1: ParametricValueList, arg2: ParametricValue) -> int:
        """

        count( (ParametricValueList)arg1, (ParametricValue)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::ParametricValue, std::allocator<ad::physics::ParametricValue> > {lvalue},ad::physics::ParametricValue)
        """
        ...

    def extend(self, arg1: ParametricValueList, arg2: object) -> None:
        """

        extend( (ParametricValueList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::ParametricValue, std::allocator<ad::physics::ParametricValue> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ParametricValueList, arg2: ParametricValue) -> int:
        """

        index( (ParametricValueList)arg1, (ParametricValue)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::ParametricValue, std::allocator<ad::physics::ParametricValue> > {lvalue},ad::physics::ParametricValue)
        """
        ...

    def insert(self, arg1: ParametricValueList, arg2: int, arg3: ParametricValue) -> None:
        """

        insert( (ParametricValueList)arg1, (int)arg2, (ParametricValue)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::ParametricValue, std::allocator<ad::physics::ParametricValue> > {lvalue},long,ad::physics::ParametricValue)
        """
        ...

    def reverse(self, arg1: ParametricValueList) -> None:
        """

        reverse( (ParametricValueList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::ParametricValue, std::allocator<ad::physics::ParametricValue> > {lvalue})
        """
        ...

    def sort(self, arg1: ParametricValueList) -> None:
        """

        sort( (ParametricValueList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::ParametricValue, std::allocator<ad::physics::ParametricValue> > {lvalue})
        """
        ...

class Probability(_Calculable):

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 1e-06

class ProbabilityList:
    def append(self, arg1: ProbabilityList, arg2: Probability) -> None:
        """

        append( (ProbabilityList)arg1, (Probability)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Probability, std::allocator<ad::physics::Probability> > {lvalue},ad::physics::Probability)
        """
        ...

    def count(self, arg1: ProbabilityList, arg2: Probability) -> int:
        """

        count( (ProbabilityList)arg1, (Probability)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Probability, std::allocator<ad::physics::Probability> > {lvalue},ad::physics::Probability)
        """
        ...

    def extend(self, arg1: ProbabilityList, arg2: object) -> None:
        """

        extend( (ProbabilityList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Probability, std::allocator<ad::physics::Probability> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: ProbabilityList, arg2: Probability) -> int:
        """

        index( (ProbabilityList)arg1, (Probability)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Probability, std::allocator<ad::physics::Probability> > {lvalue},ad::physics::Probability)
        """
        ...

    def insert(self, arg1: ProbabilityList, arg2: int, arg3: Probability) -> None:
        """

        insert( (ProbabilityList)arg1, (int)arg2, (Probability)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Probability, std::allocator<ad::physics::Probability> > {lvalue},long,ad::physics::Probability)
        """
        ...

    def reverse(self, arg1: ProbabilityList) -> None:
        """

        reverse( (ProbabilityList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Probability, std::allocator<ad::physics::Probability> > {lvalue})
        """
        ...

    def sort(self, arg1: ProbabilityList) -> None:
        """

        sort( (ProbabilityList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::Probability, std::allocator<ad::physics::Probability> > {lvalue})
        """
        ...

class RatioValue:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: RatioValue, other: RatioValue) -> RatioValue:
        """

        assign( (RatioValue)arg1, (RatioValue)other) -> RatioValue :

            C++ signature :
                ad::physics::RatioValue {lvalue} assign(ad::physics::RatioValue {lvalue},ad::physics::RatioValue)
        """
        ...

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 1e-09

    def ensureValid(self, arg1: RatioValue) -> None:
        """

        ensureValid( (RatioValue)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::RatioValue {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: RatioValue) -> None:
        """

        ensureValidNonZero( (RatioValue)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::RatioValue {lvalue})
        """
        ...

    def getMax(self) -> RatioValue:
        """

        getMax() -> RatioValue :

            C++ signature :
                ad::physics::RatioValue getMax()
        """
        ...

    def getMin(self) -> RatioValue:
        """

        getMin() -> RatioValue :

            C++ signature :
                ad::physics::RatioValue getMin()
        """
        ...

    def getPrecision(self) -> RatioValue:
        """

        getPrecision() -> RatioValue :

            C++ signature :
                ad::physics::RatioValue getPrecision()
        """
        ...

class RatioValueList:
    def append(self, arg1: RatioValueList, arg2: RatioValue) -> None:
        """

        append( (RatioValueList)arg1, (RatioValue)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::RatioValue, std::allocator<ad::physics::RatioValue> > {lvalue},ad::physics::RatioValue)
        """
        ...

    def count(self, arg1: RatioValueList, arg2: RatioValue) -> int:
        """

        count( (RatioValueList)arg1, (RatioValue)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::RatioValue, std::allocator<ad::physics::RatioValue> > {lvalue},ad::physics::RatioValue)
        """
        ...

    def extend(self, arg1: RatioValueList, arg2: object) -> None:
        """

        extend( (RatioValueList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::RatioValue, std::allocator<ad::physics::RatioValue> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: RatioValueList, arg2: RatioValue) -> int:
        """

        index( (RatioValueList)arg1, (RatioValue)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::RatioValue, std::allocator<ad::physics::RatioValue> > {lvalue},ad::physics::RatioValue)
        """
        ...

    def insert(self, arg1: RatioValueList, arg2: int, arg3: RatioValue) -> None:
        """

        insert( (RatioValueList)arg1, (int)arg2, (RatioValue)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::RatioValue, std::allocator<ad::physics::RatioValue> > {lvalue},long,ad::physics::RatioValue)
        """
        ...

    def reverse(self, arg1: RatioValueList) -> None:
        """

        reverse( (RatioValueList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::RatioValue, std::allocator<ad::physics::RatioValue> > {lvalue})
        """
        ...

    def sort(self, arg1: RatioValueList) -> None:
        """

        sort( (RatioValueList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::RatioValue, std::allocator<ad::physics::RatioValue> > {lvalue})
        """
        ...

class Speed(_Calculable):
    cMaxValue: float = 1000.0

    cMinValue: float = -1000.0

    cPrecisionValue: float = 0.001

    

class SpeedList(MutableSequence[Speed]):
    def append(self, arg1: SpeedList, arg2: Speed) -> None:
        """

        append( (SpeedList)arg1, (Speed)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Speed, std::allocator<ad::physics::Speed> > {lvalue},ad::physics::Speed)
        """
        ...

    def count(self, arg1: SpeedList, arg2: Speed) -> int:
        """

        count( (SpeedList)arg1, (Speed)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Speed, std::allocator<ad::physics::Speed> > {lvalue},ad::physics::Speed)
        """
        ...

    def extend(self, arg1: SpeedList, arg2: object) -> None:
        """

        extend( (SpeedList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Speed, std::allocator<ad::physics::Speed> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: SpeedList, arg2: Speed) -> int:
        """

        index( (SpeedList)arg1, (Speed)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Speed, std::allocator<ad::physics::Speed> > {lvalue},ad::physics::Speed)
        """
        ...

    def insert(self, arg1: SpeedList, arg2: int, arg3: Speed) -> None:
        """

        insert( (SpeedList)arg1, (int)arg2, (Speed)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Speed, std::allocator<ad::physics::Speed> > {lvalue},long,ad::physics::Speed)
        """
        ...

    def reverse(self, arg1: SpeedList) -> None:
        """

        reverse( (SpeedList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Speed, std::allocator<ad::physics::Speed> > {lvalue})
        """
        ...

    def sort(self, arg1: SpeedList) -> None:
        """

        sort( (SpeedList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::Speed, std::allocator<ad::physics::Speed> > {lvalue})
        """
        ...

class SpeedRange:
    def assign(self, arg1: SpeedRange, other: SpeedRange) -> SpeedRange:
        """

        assign( (SpeedRange)arg1, (SpeedRange)other) -> SpeedRange :

            C++ signature :
                ad::physics::SpeedRange {lvalue} assign(ad::physics::SpeedRange {lvalue},ad::physics::SpeedRange)
        """
        ...

    @property
    def maximum(self) -> Speed: ...

    @property
    def minimum(self) -> Speed: ...

class SpeedRangeList:
    def append(self, arg1: SpeedRangeList, arg2: SpeedRange) -> None:
        """

        append( (SpeedRangeList)arg1, (SpeedRange)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::SpeedRange, std::allocator<ad::physics::SpeedRange> > {lvalue},ad::physics::SpeedRange)
        """
        ...

    def count(self, arg1: SpeedRangeList, arg2: SpeedRange) -> int:
        """

        count( (SpeedRangeList)arg1, (SpeedRange)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::SpeedRange, std::allocator<ad::physics::SpeedRange> > {lvalue},ad::physics::SpeedRange)
        """
        ...

    def extend(self, arg1: SpeedRangeList, arg2: object) -> None:
        """

        extend( (SpeedRangeList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::SpeedRange, std::allocator<ad::physics::SpeedRange> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: SpeedRangeList, arg2: SpeedRange) -> int:
        """

        index( (SpeedRangeList)arg1, (SpeedRange)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::SpeedRange, std::allocator<ad::physics::SpeedRange> > {lvalue},ad::physics::SpeedRange)
        """
        ...

    def insert(self, arg1: SpeedRangeList, arg2: int, arg3: SpeedRange) -> None:
        """

        insert( (SpeedRangeList)arg1, (int)arg2, (SpeedRange)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::SpeedRange, std::allocator<ad::physics::SpeedRange> > {lvalue},long,ad::physics::SpeedRange)
        """
        ...

    def reverse(self, arg1: SpeedRangeList) -> None:
        """

        reverse( (SpeedRangeList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::SpeedRange, std::allocator<ad::physics::SpeedRange> > {lvalue})
        """
        ...

class SpeedSquared(_Calculable):
    cMaxValue: float = 1000000.0

    cMinValue: float = -1000000.0

    cPrecisionValue: float = 1e-06

class SpeedSquaredList:
    def append(self, arg1: SpeedSquaredList, arg2: SpeedSquared) -> None:
        """

        append( (SpeedSquaredList)arg1, (SpeedSquared)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::SpeedSquared, std::allocator<ad::physics::SpeedSquared> > {lvalue},ad::physics::SpeedSquared)
        """
        ...

    def count(self, arg1: SpeedSquaredList, arg2: SpeedSquared) -> int:
        """

        count( (SpeedSquaredList)arg1, (SpeedSquared)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::SpeedSquared, std::allocator<ad::physics::SpeedSquared> > {lvalue},ad::physics::SpeedSquared)
        """
        ...

    def extend(self, arg1: SpeedSquaredList, arg2: object) -> None:
        """

        extend( (SpeedSquaredList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::SpeedSquared, std::allocator<ad::physics::SpeedSquared> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: SpeedSquaredList, arg2: SpeedSquared) -> int:
        """

        index( (SpeedSquaredList)arg1, (SpeedSquared)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::SpeedSquared, std::allocator<ad::physics::SpeedSquared> > {lvalue},ad::physics::SpeedSquared)
        """
        ...

    def insert(self, arg1: SpeedSquaredList, arg2: int, arg3: SpeedSquared) -> None:
        """

        insert( (SpeedSquaredList)arg1, (int)arg2, (SpeedSquared)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::SpeedSquared, std::allocator<ad::physics::SpeedSquared> > {lvalue},long,ad::physics::SpeedSquared)
        """
        ...

    def reverse(self, arg1: SpeedSquaredList) -> None:
        """

        reverse( (SpeedSquaredList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::SpeedSquared, std::allocator<ad::physics::SpeedSquared> > {lvalue})
        """
        ...

    def sort(self, arg1: SpeedSquaredList) -> None:
        """

        sort( (SpeedSquaredList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::SpeedSquared, std::allocator<ad::physics::SpeedSquared> > {lvalue})
        """
        ...

class Velocity:
    def assign(self, arg1: Velocity, other: Velocity) -> Velocity:
        """

        assign( (Velocity)arg1, (Velocity)other) -> Velocity :

            C++ signature :
                ad::physics::Velocity {lvalue} assign(ad::physics::Velocity {lvalue},ad::physics::Velocity)
        """
        ...

    @property
    def x(self) -> Speed: ...

    @property
    def y(self) -> Speed: ...

    @property
    def z(self) -> Speed: ...

class VelocityList(_VectorSequence[Velocity]):
    ...

class Weight(_Calculable):

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 0.001

class WeightList:
    def append(self, arg1: WeightList, arg2: Weight) -> None:
        """

        append( (WeightList)arg1, (Weight)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Weight, std::allocator<ad::physics::Weight> > {lvalue},ad::physics::Weight)
        """
        ...

    def count(self, arg1: WeightList, arg2: Weight) -> int:
        """

        count( (WeightList)arg1, (Weight)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Weight, std::allocator<ad::physics::Weight> > {lvalue},ad::physics::Weight)
        """
        ...

    def extend(self, arg1: WeightList, arg2: object) -> None:
        """

        extend( (WeightList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Weight, std::allocator<ad::physics::Weight> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: WeightList, arg2: Weight) -> int:
        """

        index( (WeightList)arg1, (Weight)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Weight, std::allocator<ad::physics::Weight> > {lvalue},ad::physics::Weight)
        """
        ...

    def insert(self, arg1: WeightList, arg2: int, arg3: Weight) -> None:
        """

        insert( (WeightList)arg1, (int)arg2, (Weight)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Weight, std::allocator<ad::physics::Weight> > {lvalue},long,ad::physics::Weight)
        """
        ...

    def reverse(self, arg1: WeightList) -> None:
        """

        reverse( (WeightList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Weight, std::allocator<ad::physics::Weight> > {lvalue})
        """
        ...

    def sort(self, arg1: WeightList) -> None:
        """

        sort( (WeightList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::Weight, std::allocator<ad::physics::Weight> > {lvalue})
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_Acceleration__greater_:
    def epsilon(self) -> Acceleration:
        """

        epsilon() -> Acceleration :

            C++ signature :
                ad::physics::Acceleration epsilon()
        """
        ...

    def lowest(self) -> Acceleration:
        """

        lowest() -> Acceleration :

            C++ signature :
                ad::physics::Acceleration lowest()
        """
        ...

    def max(self) -> Acceleration:
        """

        max() -> Acceleration :

            C++ signature :
                ad::physics::Acceleration max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_Angle__greater_:
    def epsilon(self) -> Angle:
        """

        epsilon() -> Angle :

            C++ signature :
                ad::physics::Angle epsilon()
        """
        ...

    def lowest(self) -> Angle:
        """

        lowest() -> Angle :

            C++ signature :
                ad::physics::Angle lowest()
        """
        ...

    def max(self) -> Angle:
        """

        max() -> Angle :

            C++ signature :
                ad::physics::Angle max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_AngularAcceleration__greater_:
    def epsilon(self) -> AngularAcceleration:
        """

        epsilon() -> AngularAcceleration :

            C++ signature :
                ad::physics::AngularAcceleration epsilon()
        """
        ...

    def lowest(self) -> AngularAcceleration:
        """

        lowest() -> AngularAcceleration :

            C++ signature :
                ad::physics::AngularAcceleration lowest()
        """
        ...

    def max(self) -> AngularAcceleration:
        """

        max() -> AngularAcceleration :

            C++ signature :
                ad::physics::AngularAcceleration max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_AngularVelocity__greater_:
    def epsilon(self) -> AngularVelocity:
        """

        epsilon() -> AngularVelocity :

            C++ signature :
                ad::physics::AngularVelocity epsilon()
        """
        ...

    def lowest(self) -> AngularVelocity:
        """

        lowest() -> AngularVelocity :

            C++ signature :
                ad::physics::AngularVelocity lowest()
        """
        ...

    def max(self) -> AngularVelocity:
        """

        max() -> AngularVelocity :

            C++ signature :
                ad::physics::AngularVelocity max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_DistanceSquared__greater_:
    def epsilon(self) -> DistanceSquared:
        """

        epsilon() -> DistanceSquared :

            C++ signature :
                ad::physics::DistanceSquared epsilon()
        """
        ...

    def lowest(self) -> DistanceSquared:
        """

        lowest() -> DistanceSquared :

            C++ signature :
                ad::physics::DistanceSquared lowest()
        """
        ...

    def max(self) -> DistanceSquared:
        """

        max() -> DistanceSquared :

            C++ signature :
                ad::physics::DistanceSquared max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_Distance__greater_:
    def epsilon(self) -> Distance:
        """

        epsilon() -> Distance :

            C++ signature :
                ad::physics::Distance epsilon()
        """
        ...

    def lowest(self) -> Distance:
        """

        lowest() -> Distance :

            C++ signature :
                ad::physics::Distance lowest()
        """
        ...

    def max(self) -> Distance:
        """

        max() -> Distance :

            C++ signature :
                ad::physics::Distance max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_DurationSquared__greater_:
    def epsilon(self) -> DurationSquared:
        """

        epsilon() -> DurationSquared :

            C++ signature :
                ad::physics::DurationSquared epsilon()
        """
        ...

    def lowest(self) -> DurationSquared:
        """

        lowest() -> DurationSquared :

            C++ signature :
                ad::physics::DurationSquared lowest()
        """
        ...

    def max(self) -> DurationSquared:
        """

        max() -> DurationSquared :

            C++ signature :
                ad::physics::DurationSquared max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_Duration__greater_:
    def epsilon(self) -> Duration:
        """

        epsilon() -> Duration :

            C++ signature :
                ad::physics::Duration epsilon()
        """
        ...

    def lowest(self) -> Duration:
        """

        lowest() -> Duration :

            C++ signature :
                ad::physics::Duration lowest()
        """
        ...

    def max(self) -> Duration:
        """

        max() -> Duration :

            C++ signature :
                ad::physics::Duration max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_ParametricValue__greater_:
    def epsilon(self) -> ParametricValue:
        """

        epsilon() -> ParametricValue :

            C++ signature :
                ad::physics::ParametricValue epsilon()
        """
        ...

    def lowest(self) -> ParametricValue:
        """

        lowest() -> ParametricValue :

            C++ signature :
                ad::physics::ParametricValue lowest()
        """
        ...

    def max(self) -> ParametricValue:
        """

        max() -> ParametricValue :

            C++ signature :
                ad::physics::ParametricValue max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_Probability__greater_:
    def epsilon(self) -> Probability:
        """

        epsilon() -> Probability :

            C++ signature :
                ad::physics::Probability epsilon()
        """
        ...

    def lowest(self) -> Probability:
        """

        lowest() -> Probability :

            C++ signature :
                ad::physics::Probability lowest()
        """
        ...

    def max(self) -> Probability:
        """

        max() -> Probability :

            C++ signature :
                ad::physics::Probability max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_RatioValue__greater_:
    def epsilon(self) -> RatioValue:
        """

        epsilon() -> RatioValue :

            C++ signature :
                ad::physics::RatioValue epsilon()
        """
        ...

    def lowest(self) -> RatioValue:
        """

        lowest() -> RatioValue :

            C++ signature :
                ad::physics::RatioValue lowest()
        """
        ...

    def max(self) -> RatioValue:
        """

        max() -> RatioValue :

            C++ signature :
                ad::physics::RatioValue max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_SpeedSquared__greater_:
    def epsilon(self) -> SpeedSquared:
        """

        epsilon() -> SpeedSquared :

            C++ signature :
                ad::physics::SpeedSquared epsilon()
        """
        ...

    def lowest(self) -> SpeedSquared:
        """

        lowest() -> SpeedSquared :

            C++ signature :
                ad::physics::SpeedSquared lowest()
        """
        ...

    def max(self) -> SpeedSquared:
        """

        max() -> SpeedSquared :

            C++ signature :
                ad::physics::SpeedSquared max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_Speed__greater_:
    def epsilon(self) -> Speed:
        """

        epsilon() -> Speed :

            C++ signature :
                ad::physics::Speed epsilon()
        """
        ...

    def lowest(self) -> Speed:
        """

        lowest() -> Speed :

            C++ signature :
                ad::physics::Speed lowest()
        """
        ...

    def max(self) -> Speed:
        """

        max() -> Speed :

            C++ signature :
                ad::physics::Speed max()
        """
        ...

class numeric_limits_less__ad_scope_physics_scope_Weight__greater_:
    def epsilon(self) -> Weight:
        """

        epsilon() -> Weight :

            C++ signature :
                ad::physics::Weight epsilon()
        """
        ...

    def lowest(self) -> Weight:
        """

        lowest() -> Weight :

            C++ signature :
                ad::physics::Weight lowest()
        """
        ...

    def max(self) -> Weight:
        """

        max() -> Weight :

            C++ signature :
                ad::physics::Weight max()
        """
        ...
