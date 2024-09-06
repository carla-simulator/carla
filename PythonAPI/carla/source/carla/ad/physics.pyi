
from collections.abc import MutableSequence

from ad import _FloatLike

class Acceleration:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: Acceleration, other: Acceleration) -> Acceleration:
        """

        assign( (Acceleration)arg1, (Acceleration)other) -> Acceleration :

            C++ signature :
                ad::physics::Acceleration {lvalue} assign(ad::physics::Acceleration {lvalue},ad::physics::Acceleration)
        """
        ...

    cMaxValue: float = 1000.0

    cMinValue: float = -1000.0

    cPrecisionValue: float = 0.0001

    def ensureValid(self, arg1: Acceleration) -> None:
        """

        ensureValid( (Acceleration)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::Acceleration {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: Acceleration) -> None:
        """

        ensureValidNonZero( (Acceleration)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::Acceleration {lvalue})
        """
        ...

    def getMax(self) -> Acceleration:
        """

        getMax() -> Acceleration :

            C++ signature :
                ad::physics::Acceleration getMax()
        """
        ...

    def getMin(self) -> Acceleration:
        """

        getMin() -> Acceleration :

            C++ signature :
                ad::physics::Acceleration getMin()
        """
        ...

    def getPrecision(self) -> Acceleration:
        """

        getPrecision() -> Acceleration :

            C++ signature :
                ad::physics::Acceleration getPrecision()
        """
        ...

    def __float__(self) -> float: ...

    def __gt__(self, other: float | Acceleration) -> bool: ...

    def __ge__(self, other: float | Acceleration) -> bool: ...

    def __lt__(self, other: float | Acceleration) -> bool: ...

    def __le__(self, other: float | Acceleration) -> bool: ...

    def __eq__(self, other: float | Acceleration) -> bool: ...

class Acceleration3D:
    def __float__(self) -> float: ...

    def assign(self, arg1: Acceleration3D, other: Acceleration3D) -> Acceleration3D:
        """

        assign( (Acceleration3D)arg1, (Acceleration3D)other) -> Acceleration3D :

            C++ signature :
                ad::physics::Acceleration3D {lvalue} assign(ad::physics::Acceleration3D {lvalue},ad::physics::Acceleration3D)
        """
        ...

    @property
    def x(self) -> Acceleration: ...

    @property
    def y(self) -> Acceleration: ...

    @property
    def z(self) -> Acceleration: ...

class Acceleration3DList:
    def append(self, arg1: Acceleration3DList, arg2: Acceleration3D) -> None:
        """

        append( (Acceleration3DList)arg1, (Acceleration3D)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Acceleration3D, std::allocator<ad::physics::Acceleration3D> > {lvalue},ad::physics::Acceleration3D)
        """
        ...

    def count(self, arg1: Acceleration3DList, arg2: Acceleration3D) -> int:
        """

        count( (Acceleration3DList)arg1, (Acceleration3D)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Acceleration3D, std::allocator<ad::physics::Acceleration3D> > {lvalue},ad::physics::Acceleration3D)
        """
        ...

    def extend(self, arg1: Acceleration3DList, arg2: object) -> None:
        """

        extend( (Acceleration3DList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Acceleration3D, std::allocator<ad::physics::Acceleration3D> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: Acceleration3DList, arg2: Acceleration3D) -> int:
        """

        index( (Acceleration3DList)arg1, (Acceleration3D)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Acceleration3D, std::allocator<ad::physics::Acceleration3D> > {lvalue},ad::physics::Acceleration3D)
        """
        ...

    def insert(self, arg1: Acceleration3DList, arg2: int, arg3: Acceleration3D) -> None:
        """

        insert( (Acceleration3DList)arg1, (int)arg2, (Acceleration3D)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Acceleration3D, std::allocator<ad::physics::Acceleration3D> > {lvalue},long,ad::physics::Acceleration3D)
        """
        ...

    def reverse(self, arg1: Acceleration3DList) -> None:
        """

        reverse( (Acceleration3DList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Acceleration3D, std::allocator<ad::physics::Acceleration3D> > {lvalue})
        """
        ...

class AccelerationList:
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

class Angle:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: Angle, other: Angle) -> Angle:
        """

        assign( (Angle)arg1, (Angle)other) -> Angle :

            C++ signature :
                ad::physics::Angle {lvalue} assign(ad::physics::Angle {lvalue},ad::physics::Angle)
        """
        ...

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 0.001

    def ensureValid(self, arg1: Angle) -> None:
        """

        ensureValid( (Angle)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::Angle {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: Angle) -> None:
        """

        ensureValidNonZero( (Angle)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::Angle {lvalue})
        """
        ...

    def getMax(self) -> Angle:
        """

        getMax() -> Angle :

            C++ signature :
                ad::physics::Angle getMax()
        """
        ...

    def getMin(self) -> Angle:
        """

        getMin() -> Angle :

            C++ signature :
                ad::physics::Angle getMin()
        """
        ...

    def getPrecision(self) -> Angle:
        """

        getPrecision() -> Angle :

            C++ signature :
                ad::physics::Angle getPrecision()
        """
        ...

class AngleList:
    def append(self, arg1: AngleList, arg2: Angle) -> None:
        """

        append( (AngleList)arg1, (Angle)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Angle, std::allocator<ad::physics::Angle> > {lvalue},ad::physics::Angle)
        """
        ...

    def count(self, arg1: AngleList, arg2: Angle) -> int:
        """

        count( (AngleList)arg1, (Angle)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Angle, std::allocator<ad::physics::Angle> > {lvalue},ad::physics::Angle)
        """
        ...

    def extend(self, arg1: AngleList, arg2: object) -> None:
        """

        extend( (AngleList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Angle, std::allocator<ad::physics::Angle> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: AngleList, arg2: Angle) -> int:
        """

        index( (AngleList)arg1, (Angle)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Angle, std::allocator<ad::physics::Angle> > {lvalue},ad::physics::Angle)
        """
        ...

    def insert(self, arg1: AngleList, arg2: int, arg3: Angle) -> None:
        """

        insert( (AngleList)arg1, (int)arg2, (Angle)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Angle, std::allocator<ad::physics::Angle> > {lvalue},long,ad::physics::Angle)
        """
        ...

    def reverse(self, arg1: AngleList) -> None:
        """

        reverse( (AngleList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Angle, std::allocator<ad::physics::Angle> > {lvalue})
        """
        ...

    def sort(self, arg1: AngleList) -> None:
        """

        sort( (AngleList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::Angle, std::allocator<ad::physics::Angle> > {lvalue})
        """
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

class AngleRangeList:
    def append(self, arg1: AngleRangeList, arg2: AngleRange) -> None:
        """

        append( (AngleRangeList)arg1, (AngleRange)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::AngleRange, std::allocator<ad::physics::AngleRange> > {lvalue},ad::physics::AngleRange)
        """
        ...

    def count(self, arg1: AngleRangeList, arg2: AngleRange) -> int:
        """

        count( (AngleRangeList)arg1, (AngleRange)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::AngleRange, std::allocator<ad::physics::AngleRange> > {lvalue},ad::physics::AngleRange)
        """
        ...

    def extend(self, arg1: AngleRangeList, arg2: object) -> None:
        """

        extend( (AngleRangeList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::AngleRange, std::allocator<ad::physics::AngleRange> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: AngleRangeList, arg2: AngleRange) -> int:
        """

        index( (AngleRangeList)arg1, (AngleRange)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::AngleRange, std::allocator<ad::physics::AngleRange> > {lvalue},ad::physics::AngleRange)
        """
        ...

    def insert(self, arg1: AngleRangeList, arg2: int, arg3: AngleRange) -> None:
        """

        insert( (AngleRangeList)arg1, (int)arg2, (AngleRange)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::AngleRange, std::allocator<ad::physics::AngleRange> > {lvalue},long,ad::physics::AngleRange)
        """
        ...

    def reverse(self, arg1: AngleRangeList) -> None:
        """

        reverse( (AngleRangeList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::AngleRange, std::allocator<ad::physics::AngleRange> > {lvalue})
        """
        ...

class AngularAcceleration:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: AngularAcceleration, other: AngularAcceleration) -> AngularAcceleration:
        """

        assign( (AngularAcceleration)arg1, (AngularAcceleration)other) -> AngularAcceleration :

            C++ signature :
                ad::physics::AngularAcceleration {lvalue} assign(ad::physics::AngularAcceleration {lvalue},ad::physics::AngularAcceleration)
        """
        ...

    cMaxValue: float = 1000.0

    cMinValue: float = -1000.0

    cPrecisionValue: float = 0.0001

    def ensureValid(self, arg1: AngularAcceleration) -> None:
        """

        ensureValid( (AngularAcceleration)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::AngularAcceleration {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: AngularAcceleration) -> None:
        """

        ensureValidNonZero( (AngularAcceleration)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::AngularAcceleration {lvalue})
        """
        ...

    def getMax(self) -> AngularAcceleration:
        """

        getMax() -> AngularAcceleration :

            C++ signature :
                ad::physics::AngularAcceleration getMax()
        """
        ...

    def getMin(self) -> AngularAcceleration:
        """

        getMin() -> AngularAcceleration :

            C++ signature :
                ad::physics::AngularAcceleration getMin()
        """
        ...

    def getPrecision(self) -> AngularAcceleration:
        """

        getPrecision() -> AngularAcceleration :

            C++ signature :
                ad::physics::AngularAcceleration getPrecision()
        """
        ...

class AngularAccelerationList:
    def append(self, arg1: AngularAccelerationList, arg2: AngularAcceleration) -> None:
        """

        append( (AngularAccelerationList)arg1, (AngularAcceleration)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::AngularAcceleration, std::allocator<ad::physics::AngularAcceleration> > {lvalue},ad::physics::AngularAcceleration)
        """
        ...

    def count(self, arg1: AngularAccelerationList, arg2: AngularAcceleration) -> int:
        """

        count( (AngularAccelerationList)arg1, (AngularAcceleration)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::AngularAcceleration, std::allocator<ad::physics::AngularAcceleration> > {lvalue},ad::physics::AngularAcceleration)
        """
        ...

    def extend(self, arg1: AngularAccelerationList, arg2: object) -> None:
        """

        extend( (AngularAccelerationList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::AngularAcceleration, std::allocator<ad::physics::AngularAcceleration> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: AngularAccelerationList, arg2: AngularAcceleration) -> int:
        """

        index( (AngularAccelerationList)arg1, (AngularAcceleration)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::AngularAcceleration, std::allocator<ad::physics::AngularAcceleration> > {lvalue},ad::physics::AngularAcceleration)
        """
        ...

    def insert(self, arg1: AngularAccelerationList, arg2: int, arg3: AngularAcceleration) -> None:
        """

        insert( (AngularAccelerationList)arg1, (int)arg2, (AngularAcceleration)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::AngularAcceleration, std::allocator<ad::physics::AngularAcceleration> > {lvalue},long,ad::physics::AngularAcceleration)
        """
        ...

    def reverse(self, arg1: AngularAccelerationList) -> None:
        """

        reverse( (AngularAccelerationList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::AngularAcceleration, std::allocator<ad::physics::AngularAcceleration> > {lvalue})
        """
        ...

    def sort(self, arg1: AngularAccelerationList) -> None:
        """

        sort( (AngularAccelerationList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::AngularAcceleration, std::allocator<ad::physics::AngularAcceleration> > {lvalue})
        """
        ...

class AngularVelocity:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: AngularVelocity, other: AngularVelocity) -> AngularVelocity:
        """

        assign( (AngularVelocity)arg1, (AngularVelocity)other) -> AngularVelocity :

            C++ signature :
                ad::physics::AngularVelocity {lvalue} assign(ad::physics::AngularVelocity {lvalue},ad::physics::AngularVelocity)
        """
        ...

    cMaxValue: float = 1000.0

    cMinValue: float = -1000.0

    cPrecisionValue: float = 0.001

    def ensureValid(self, arg1: AngularVelocity) -> None:
        """

        ensureValid( (AngularVelocity)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::AngularVelocity {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: AngularVelocity) -> None:
        """

        ensureValidNonZero( (AngularVelocity)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::AngularVelocity {lvalue})
        """
        ...

    def getMax(self) -> AngularVelocity:
        """

        getMax() -> AngularVelocity :

            C++ signature :
                ad::physics::AngularVelocity getMax()
        """
        ...

    def getMin(self) -> AngularVelocity:
        """

        getMin() -> AngularVelocity :

            C++ signature :
                ad::physics::AngularVelocity getMin()
        """
        ...

    def getPrecision(self) -> AngularVelocity:
        """

        getPrecision() -> AngularVelocity :

            C++ signature :
                ad::physics::AngularVelocity getPrecision()
        """
        ...

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

class AngularVelocity3DList:
    def append(self, arg1: AngularVelocity3DList, arg2: AngularVelocity3D) -> None:
        """

        append( (AngularVelocity3DList)arg1, (AngularVelocity3D)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::AngularVelocity3D, std::allocator<ad::physics::AngularVelocity3D> > {lvalue},ad::physics::AngularVelocity3D)
        """
        ...

    def count(self, arg1: AngularVelocity3DList, arg2: AngularVelocity3D) -> int:
        """

        count( (AngularVelocity3DList)arg1, (AngularVelocity3D)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::AngularVelocity3D, std::allocator<ad::physics::AngularVelocity3D> > {lvalue},ad::physics::AngularVelocity3D)
        """
        ...

    def extend(self, arg1: AngularVelocity3DList, arg2: object) -> None:
        """

        extend( (AngularVelocity3DList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::AngularVelocity3D, std::allocator<ad::physics::AngularVelocity3D> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: AngularVelocity3DList, arg2: AngularVelocity3D) -> int:
        """

        index( (AngularVelocity3DList)arg1, (AngularVelocity3D)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::AngularVelocity3D, std::allocator<ad::physics::AngularVelocity3D> > {lvalue},ad::physics::AngularVelocity3D)
        """
        ...

    def insert(self, arg1: AngularVelocity3DList, arg2: int, arg3: AngularVelocity3D) -> None:
        """

        insert( (AngularVelocity3DList)arg1, (int)arg2, (AngularVelocity3D)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::AngularVelocity3D, std::allocator<ad::physics::AngularVelocity3D> > {lvalue},long,ad::physics::AngularVelocity3D)
        """
        ...

    def reverse(self, arg1: AngularVelocity3DList) -> None:
        """

        reverse( (AngularVelocity3DList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::AngularVelocity3D, std::allocator<ad::physics::AngularVelocity3D> > {lvalue})
        """
        ...

class AngularVelocityList:
    def append(self, arg1: AngularVelocityList, arg2: AngularVelocity) -> None:
        """

        append( (AngularVelocityList)arg1, (AngularVelocity)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::AngularVelocity, std::allocator<ad::physics::AngularVelocity> > {lvalue},ad::physics::AngularVelocity)
        """
        ...

    def count(self, arg1: AngularVelocityList, arg2: AngularVelocity) -> int:
        """

        count( (AngularVelocityList)arg1, (AngularVelocity)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::AngularVelocity, std::allocator<ad::physics::AngularVelocity> > {lvalue},ad::physics::AngularVelocity)
        """
        ...

    def extend(self, arg1: AngularVelocityList, arg2: object) -> None:
        """

        extend( (AngularVelocityList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::AngularVelocity, std::allocator<ad::physics::AngularVelocity> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: AngularVelocityList, arg2: AngularVelocity) -> int:
        """

        index( (AngularVelocityList)arg1, (AngularVelocity)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::AngularVelocity, std::allocator<ad::physics::AngularVelocity> > {lvalue},ad::physics::AngularVelocity)
        """
        ...

    def insert(self, arg1: AngularVelocityList, arg2: int, arg3: AngularVelocity) -> None:
        """

        insert( (AngularVelocityList)arg1, (int)arg2, (AngularVelocity)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::AngularVelocity, std::allocator<ad::physics::AngularVelocity> > {lvalue},long,ad::physics::AngularVelocity)
        """
        ...

    def reverse(self, arg1: AngularVelocityList) -> None:
        """

        reverse( (AngularVelocityList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::AngularVelocity, std::allocator<ad::physics::AngularVelocity> > {lvalue})
        """
        ...

    def sort(self, arg1: AngularVelocityList) -> None:
        """

        sort( (AngularVelocityList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::AngularVelocity, std::allocator<ad::physics::AngularVelocity> > {lvalue})
        """
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

class Dimension2DList:
    def append(self, arg1: Dimension2DList, arg2: Dimension2D) -> None:
        """

        append( (Dimension2DList)arg1, (Dimension2D)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Dimension2D, std::allocator<ad::physics::Dimension2D> > {lvalue},ad::physics::Dimension2D)
        """
        ...

    def count(self, arg1: Dimension2DList, arg2: Dimension2D) -> int:
        """

        count( (Dimension2DList)arg1, (Dimension2D)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Dimension2D, std::allocator<ad::physics::Dimension2D> > {lvalue},ad::physics::Dimension2D)
        """
        ...

    def extend(self, arg1: Dimension2DList, arg2: object) -> None:
        """

        extend( (Dimension2DList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Dimension2D, std::allocator<ad::physics::Dimension2D> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: Dimension2DList, arg2: Dimension2D) -> int:
        """

        index( (Dimension2DList)arg1, (Dimension2D)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Dimension2D, std::allocator<ad::physics::Dimension2D> > {lvalue},ad::physics::Dimension2D)
        """
        ...

    def insert(self, arg1: Dimension2DList, arg2: int, arg3: Dimension2D) -> None:
        """

        insert( (Dimension2DList)arg1, (int)arg2, (Dimension2D)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Dimension2D, std::allocator<ad::physics::Dimension2D> > {lvalue},long,ad::physics::Dimension2D)
        """
        ...

    def reverse(self, arg1: Dimension2DList) -> None:
        """

        reverse( (Dimension2DList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Dimension2D, std::allocator<ad::physics::Dimension2D> > {lvalue})
        """
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

class Dimension3DList:
    def append(self, arg1: Dimension3DList, arg2: Dimension3D) -> None:
        """

        append( (Dimension3DList)arg1, (Dimension3D)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Dimension3D, std::allocator<ad::physics::Dimension3D> > {lvalue},ad::physics::Dimension3D)
        """
        ...

    def count(self, arg1: Dimension3DList, arg2: Dimension3D) -> int:
        """

        count( (Dimension3DList)arg1, (Dimension3D)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Dimension3D, std::allocator<ad::physics::Dimension3D> > {lvalue},ad::physics::Dimension3D)
        """
        ...

    def extend(self, arg1: Dimension3DList, arg2: object) -> None:
        """

        extend( (Dimension3DList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Dimension3D, std::allocator<ad::physics::Dimension3D> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: Dimension3DList, arg2: Dimension3D) -> int:
        """

        index( (Dimension3DList)arg1, (Dimension3D)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Dimension3D, std::allocator<ad::physics::Dimension3D> > {lvalue},ad::physics::Dimension3D)
        """
        ...

    def insert(self, arg1: Dimension3DList, arg2: int, arg3: Dimension3D) -> None:
        """

        insert( (Dimension3DList)arg1, (int)arg2, (Dimension3D)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Dimension3D, std::allocator<ad::physics::Dimension3D> > {lvalue},long,ad::physics::Dimension3D)
        """
        ...

    def reverse(self, arg1: Dimension3DList) -> None:
        """

        reverse( (Dimension3DList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Dimension3D, std::allocator<ad::physics::Dimension3D> > {lvalue})
        """
        ...

class Distance(_FloatLike):
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: Distance, other: Distance) -> Distance:
        """

        assign( (Distance)arg1, (Distance)other) -> Distance :

            C++ signature :
                ad::physics::Distance {lvalue} assign(ad::physics::Distance {lvalue},ad::physics::Distance)
        """
        ...

    cMaxValue: float = 1000000000.0

    cMinValue: float = -1000000000.0

    cPrecisionValue: float = 0.001

    def ensureValid(self, arg1: Distance) -> None:
        """

        ensureValid( (Distance)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::Distance {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: Distance) -> None:
        """

        ensureValidNonZero( (Distance)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::Distance {lvalue})
        """
        ...

    def getMax(self) -> Distance:
        """

        getMax() -> Distance :

            C++ signature :
                ad::physics::Distance getMax()
        """
        ...

    def getMin(self) -> Distance:
        """

        getMin() -> Distance :

            C++ signature :
                ad::physics::Distance getMin()
        """
        ...

    def getPrecision(self) -> Distance:
        """

        getPrecision() -> Distance :

            C++ signature :
                ad::physics::Distance getPrecision()
        """
        ...

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

class Distance2DList:
    def append(self, arg1: Distance2DList, arg2: Distance2D) -> None:
        """

        append( (Distance2DList)arg1, (Distance2D)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> > {lvalue},ad::physics::Distance2D)
        """
        ...

    def count(self, arg1: Distance2DList, arg2: Distance2D) -> int:
        """

        count( (Distance2DList)arg1, (Distance2D)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> > {lvalue},ad::physics::Distance2D)
        """
        ...

    def extend(self, arg1: Distance2DList, arg2: object) -> None:
        """

        extend( (Distance2DList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: Distance2DList, arg2: Distance2D) -> int:
        """

        index( (Distance2DList)arg1, (Distance2D)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> > {lvalue},ad::physics::Distance2D)
        """
        ...

    def insert(self, arg1: Distance2DList, arg2: int, arg3: Distance2D) -> None:
        """

        insert( (Distance2DList)arg1, (int)arg2, (Distance2D)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> > {lvalue},long,ad::physics::Distance2D)
        """
        ...

    def reverse(self, arg1: Distance2DList) -> None:
        """

        reverse( (Distance2DList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Distance2D, std::allocator<ad::physics::Distance2D> > {lvalue})
        """
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

class Distance3DList:
    def append(self, arg1: Distance3DList, arg2: Distance3D) -> None:
        """

        append( (Distance3DList)arg1, (Distance3D)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Distance3D, std::allocator<ad::physics::Distance3D> > {lvalue},ad::physics::Distance3D)
        """
        ...

    def count(self, arg1: Distance3DList, arg2: Distance3D) -> int:
        """

        count( (Distance3DList)arg1, (Distance3D)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Distance3D, std::allocator<ad::physics::Distance3D> > {lvalue},ad::physics::Distance3D)
        """
        ...

    def extend(self, arg1: Distance3DList, arg2: object) -> None:
        """

        extend( (Distance3DList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Distance3D, std::allocator<ad::physics::Distance3D> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: Distance3DList, arg2: Distance3D) -> int:
        """

        index( (Distance3DList)arg1, (Distance3D)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Distance3D, std::allocator<ad::physics::Distance3D> > {lvalue},ad::physics::Distance3D)
        """
        ...

    def insert(self, arg1: Distance3DList, arg2: int, arg3: Distance3D) -> None:
        """

        insert( (Distance3DList)arg1, (int)arg2, (Distance3D)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Distance3D, std::allocator<ad::physics::Distance3D> > {lvalue},long,ad::physics::Distance3D)
        """
        ...

    def reverse(self, arg1: Distance3DList) -> None:
        """

        reverse( (Distance3DList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Distance3D, std::allocator<ad::physics::Distance3D> > {lvalue})
        """
        ...

class DistanceList:
    def append(self, arg1: DistanceList, arg2: Distance) -> None:
        """

        append( (DistanceList)arg1, (Distance)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Distance, std::allocator<ad::physics::Distance> > {lvalue},ad::physics::Distance)
        """
        ...

    def count(self, arg1: DistanceList, arg2: Distance) -> int:
        """

        count( (DistanceList)arg1, (Distance)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Distance, std::allocator<ad::physics::Distance> > {lvalue},ad::physics::Distance)
        """
        ...

    def extend(self, arg1: DistanceList, arg2: object) -> None:
        """

        extend( (DistanceList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Distance, std::allocator<ad::physics::Distance> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: DistanceList, arg2: Distance) -> int:
        """

        index( (DistanceList)arg1, (Distance)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Distance, std::allocator<ad::physics::Distance> > {lvalue},ad::physics::Distance)
        """
        ...

    def insert(self, arg1: DistanceList, arg2: int, arg3: Distance) -> None:
        """

        insert( (DistanceList)arg1, (int)arg2, (Distance)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Distance, std::allocator<ad::physics::Distance> > {lvalue},long,ad::physics::Distance)
        """
        ...

    def reverse(self, arg1: DistanceList) -> None:
        """

        reverse( (DistanceList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Distance, std::allocator<ad::physics::Distance> > {lvalue})
        """
        ...

    def sort(self, arg1: DistanceList) -> None:
        """

        sort( (DistanceList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::Distance, std::allocator<ad::physics::Distance> > {lvalue})
        """
        ...

class DistanceSquared:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: DistanceSquared, other: DistanceSquared) -> DistanceSquared:
        """

        assign( (DistanceSquared)arg1, (DistanceSquared)other) -> DistanceSquared :

            C++ signature :
                ad::physics::DistanceSquared {lvalue} assign(ad::physics::DistanceSquared {lvalue},ad::physics::DistanceSquared)
        """
        ...

    cMaxValue: float = 1e+18

    cMinValue: float = -1e+18

    cPrecisionValue: float = 1e-06

    def ensureValid(self, arg1: DistanceSquared) -> None:
        """

        ensureValid( (DistanceSquared)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::DistanceSquared {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: DistanceSquared) -> None:
        """

        ensureValidNonZero( (DistanceSquared)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::DistanceSquared {lvalue})
        """
        ...

    def getMax(self) -> DistanceSquared:
        """

        getMax() -> DistanceSquared :

            C++ signature :
                ad::physics::DistanceSquared getMax()
        """
        ...

    def getMin(self) -> DistanceSquared:
        """

        getMin() -> DistanceSquared :

            C++ signature :
                ad::physics::DistanceSquared getMin()
        """
        ...

    def getPrecision(self) -> DistanceSquared:
        """

        getPrecision() -> DistanceSquared :

            C++ signature :
                ad::physics::DistanceSquared getPrecision()
        """
        ...

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

class Duration:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: Duration, other: Duration) -> Duration:
        """

        assign( (Duration)arg1, (Duration)other) -> Duration :

            C++ signature :
                ad::physics::Duration {lvalue} assign(ad::physics::Duration {lvalue},ad::physics::Duration)
        """
        ...

    cMaxValue: float = 1000000.0

    cMinValue: float = -1000000.0

    cPrecisionValue: float = 0.001

    def ensureValid(self, arg1: Duration) -> None:
        """

        ensureValid( (Duration)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::Duration {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: Duration) -> None:
        """

        ensureValidNonZero( (Duration)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::Duration {lvalue})
        """
        ...

    def getMax(self) -> Duration:
        """

        getMax() -> Duration :

            C++ signature :
                ad::physics::Duration getMax()
        """
        ...

    def getMin(self) -> Duration:
        """

        getMin() -> Duration :

            C++ signature :
                ad::physics::Duration getMin()
        """
        ...

    def getPrecision(self) -> Duration:
        """

        getPrecision() -> Duration :

            C++ signature :
                ad::physics::Duration getPrecision()
        """
        ...

class DurationList:
    def append(self, arg1: DurationList, arg2: Duration) -> None:
        """

        append( (DurationList)arg1, (Duration)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Duration, std::allocator<ad::physics::Duration> > {lvalue},ad::physics::Duration)
        """
        ...

    def count(self, arg1: DurationList, arg2: Duration) -> int:
        """

        count( (DurationList)arg1, (Duration)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Duration, std::allocator<ad::physics::Duration> > {lvalue},ad::physics::Duration)
        """
        ...

    def extend(self, arg1: DurationList, arg2: object) -> None:
        """

        extend( (DurationList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Duration, std::allocator<ad::physics::Duration> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: DurationList, arg2: Duration) -> int:
        """

        index( (DurationList)arg1, (Duration)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Duration, std::allocator<ad::physics::Duration> > {lvalue},ad::physics::Duration)
        """
        ...

    def insert(self, arg1: DurationList, arg2: int, arg3: Duration) -> None:
        """

        insert( (DurationList)arg1, (int)arg2, (Duration)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Duration, std::allocator<ad::physics::Duration> > {lvalue},long,ad::physics::Duration)
        """
        ...

    def reverse(self, arg1: DurationList) -> None:
        """

        reverse( (DurationList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Duration, std::allocator<ad::physics::Duration> > {lvalue})
        """
        ...

    def sort(self, arg1: DurationList) -> None:
        """

        sort( (DurationList)arg1) -> None :

            C++ signature :
                void sort(std::vector<ad::physics::Duration, std::allocator<ad::physics::Duration> > {lvalue})
        """
        ...

class DurationSquared:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: DurationSquared, other: DurationSquared) -> DurationSquared:
        """

        assign( (DurationSquared)arg1, (DurationSquared)other) -> DurationSquared :

            C++ signature :
                ad::physics::DurationSquared {lvalue} assign(ad::physics::DurationSquared {lvalue},ad::physics::DurationSquared)
        """
        ...

    cMaxValue: float = 1000000000000.0

    cMinValue: float = -1000000000000.0

    cPrecisionValue: float = 1e-06

    def ensureValid(self, arg1: DurationSquared) -> None:
        """

        ensureValid( (DurationSquared)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::DurationSquared {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: DurationSquared) -> None:
        """

        ensureValidNonZero( (DurationSquared)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::DurationSquared {lvalue})
        """
        ...

    def getMax(self) -> DurationSquared:
        """

        getMax() -> DurationSquared :

            C++ signature :
                ad::physics::DurationSquared getMax()
        """
        ...

    def getMin(self) -> DurationSquared:
        """

        getMin() -> DurationSquared :

            C++ signature :
                ad::physics::DurationSquared getMin()
        """
        ...

    def getPrecision(self) -> DurationSquared:
        """

        getPrecision() -> DurationSquared :

            C++ signature :
                ad::physics::DurationSquared getPrecision()
        """
        ...

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

class ParametricValue:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: ParametricValue, other: ParametricValue) -> ParametricValue:
        """

        assign( (ParametricValue)arg1, (ParametricValue)other) -> ParametricValue :

            C++ signature :
                ad::physics::ParametricValue {lvalue} assign(ad::physics::ParametricValue {lvalue},ad::physics::ParametricValue)
        """
        ...

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 1e-06

    def ensureValid(self, arg1: ParametricValue) -> None:
        """

        ensureValid( (ParametricValue)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::ParametricValue {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: ParametricValue) -> None:
        """

        ensureValidNonZero( (ParametricValue)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::ParametricValue {lvalue})
        """
        ...

    def getMax(self) -> ParametricValue:
        """

        getMax() -> ParametricValue :

            C++ signature :
                ad::physics::ParametricValue getMax()
        """
        ...

    def getMin(self) -> ParametricValue:
        """

        getMin() -> ParametricValue :

            C++ signature :
                ad::physics::ParametricValue getMin()
        """
        ...

    def getPrecision(self) -> ParametricValue:
        """

        getPrecision() -> ParametricValue :

            C++ signature :
                ad::physics::ParametricValue getPrecision()
        """
        ...

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

class Probability:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: Probability, other: Probability) -> Probability:
        """

        assign( (Probability)arg1, (Probability)other) -> Probability :

            C++ signature :
                ad::physics::Probability {lvalue} assign(ad::physics::Probability {lvalue},ad::physics::Probability)
        """
        ...

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 1e-06

    def ensureValid(self, arg1: Probability) -> None:
        """

        ensureValid( (Probability)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::Probability {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: Probability) -> None:
        """

        ensureValidNonZero( (Probability)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::Probability {lvalue})
        """
        ...

    def getMax(self) -> Probability:
        """

        getMax() -> Probability :

            C++ signature :
                ad::physics::Probability getMax()
        """
        ...

    def getMin(self) -> Probability:
        """

        getMin() -> Probability :

            C++ signature :
                ad::physics::Probability getMin()
        """
        ...

    def getPrecision(self) -> Probability:
        """

        getPrecision() -> Probability :

            C++ signature :
                ad::physics::Probability getPrecision()
        """
        ...

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

class Speed:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: Speed, other: Speed) -> Speed:
        """

        assign( (Speed)arg1, (Speed)other) -> Speed :

            C++ signature :
                ad::physics::Speed {lvalue} assign(ad::physics::Speed {lvalue},ad::physics::Speed)
        """
        ...

    cMaxValue: float = 1000.0

    cMinValue: float = -1000.0

    cPrecisionValue: float = 0.001

    def ensureValid(self, arg1: Speed) -> None:
        """

        ensureValid( (Speed)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::Speed {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: Speed) -> None:
        """

        ensureValidNonZero( (Speed)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::Speed {lvalue})
        """
        ...

    def getMax(self) -> Speed:
        """

        getMax() -> Speed :

            C++ signature :
                ad::physics::Speed getMax()
        """
        ...

    def getMin(self) -> Speed:
        """

        getMin() -> Speed :

            C++ signature :
                ad::physics::Speed getMin()
        """
        ...

    def getPrecision(self) -> Speed:
        """

        getPrecision() -> Speed :

            C++ signature :
                ad::physics::Speed getPrecision()
        """
        ...

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

class SpeedSquared:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: SpeedSquared, other: SpeedSquared) -> SpeedSquared:
        """

        assign( (SpeedSquared)arg1, (SpeedSquared)other) -> SpeedSquared :

            C++ signature :
                ad::physics::SpeedSquared {lvalue} assign(ad::physics::SpeedSquared {lvalue},ad::physics::SpeedSquared)
        """
        ...

    cMaxValue: float = 1000000.0

    cMinValue: float = -1000000.0

    cPrecisionValue: float = 1e-06

    def ensureValid(self, arg1: SpeedSquared) -> None:
        """

        ensureValid( (SpeedSquared)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::SpeedSquared {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: SpeedSquared) -> None:
        """

        ensureValidNonZero( (SpeedSquared)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::SpeedSquared {lvalue})
        """
        ...

    def getMax(self) -> SpeedSquared:
        """

        getMax() -> SpeedSquared :

            C++ signature :
                ad::physics::SpeedSquared getMax()
        """
        ...

    def getMin(self) -> SpeedSquared:
        """

        getMin() -> SpeedSquared :

            C++ signature :
                ad::physics::SpeedSquared getMin()
        """
        ...

    def getPrecision(self) -> SpeedSquared:
        """

        getPrecision() -> SpeedSquared :

            C++ signature :
                ad::physics::SpeedSquared getPrecision()
        """
        ...

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

class VelocityList:
    def append(self, arg1: VelocityList, arg2: Velocity) -> None:
        """

        append( (VelocityList)arg1, (Velocity)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::physics::Velocity, std::allocator<ad::physics::Velocity> > {lvalue},ad::physics::Velocity)
        """
        ...

    def count(self, arg1: VelocityList, arg2: Velocity) -> int:
        """

        count( (VelocityList)arg1, (Velocity)arg2) -> int :

            C++ signature :
                unsigned long count(std::vector<ad::physics::Velocity, std::allocator<ad::physics::Velocity> > {lvalue},ad::physics::Velocity)
        """
        ...

    def extend(self, arg1: VelocityList, arg2: object) -> None:
        """

        extend( (VelocityList)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::physics::Velocity, std::allocator<ad::physics::Velocity> > {lvalue},boost::python::api::object)
        """
        ...

    def index(self, arg1: VelocityList, arg2: Velocity) -> int:
        """

        index( (VelocityList)arg1, (Velocity)arg2) -> int :

            C++ signature :
                unsigned long index(std::vector<ad::physics::Velocity, std::allocator<ad::physics::Velocity> > {lvalue},ad::physics::Velocity)
        """
        ...

    def insert(self, arg1: VelocityList, arg2: int, arg3: Velocity) -> None:
        """

        insert( (VelocityList)arg1, (int)arg2, (Velocity)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::physics::Velocity, std::allocator<ad::physics::Velocity> > {lvalue},long,ad::physics::Velocity)
        """
        ...

    def reverse(self, arg1: VelocityList) -> None:
        """

        reverse( (VelocityList)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::physics::Velocity, std::allocator<ad::physics::Velocity> > {lvalue})
        """
        ...

class Weight:
    @property
    def Valid(self) -> bool: ...

    def assign(self, arg1: Weight, other: Weight) -> Weight:
        """

        assign( (Weight)arg1, (Weight)other) -> Weight :

            C++ signature :
                ad::physics::Weight {lvalue} assign(ad::physics::Weight {lvalue},ad::physics::Weight)
        """
        ...

    cMaxValue: float = ...

    cMinValue: float = ...

    cPrecisionValue: float = 0.001

    def ensureValid(self, arg1: Weight) -> None:
        """

        ensureValid( (Weight)arg1) -> None :

            C++ signature :
                void ensureValid(ad::physics::Weight {lvalue})
        """
        ...

    def ensureValidNonZero(self, arg1: Weight) -> None:
        """

        ensureValidNonZero( (Weight)arg1) -> None :

            C++ signature :
                void ensureValidNonZero(ad::physics::Weight {lvalue})
        """
        ...

    def getMax(self) -> Weight:
        """

        getMax() -> Weight :

            C++ signature :
                ad::physics::Weight getMax()
        """
        ...

    def getMin(self) -> Weight:
        """

        getMin() -> Weight :

            C++ signature :
                ad::physics::Weight getMin()
        """
        ...

    def getPrecision(self) -> Weight:
        """

        getPrecision() -> Weight :

            C++ signature :
                ad::physics::Weight getPrecision()
        """
        ...

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
