"""
Warning:
    The stub files for the ad module are only a partial representation of the actual API,
    some functions and attributes are still missing.
"""

from typing import Iterable, Iterator, Protocol, TypeVar, overload, type_check_only

from typing_extensions import Self

from . import map, physics, rss

__all__ = [
    'map',
    'physics',
    'rss',
]

# ---------------------------------------------
# Below are helper classes to easier define stubs.

_T = TypeVar('_T')

@type_check_only
class _Vector(Protocol[_T]):
    # Note: __contains__, __iter__, and __reversed__ cann fall to __getitem__

    def append(self, item: _T, /) -> None:
        ...

    def extend(self, iterable: Iterable[_T], /) -> None:
        ...

    def insert(self, index: int, item: _T, /) -> None:
        ...

    def reverse(self) -> None:
        ...

    @overload
    def __getitem__(self, index: slice, /) -> list[_T]: ...

    @overload
    def __getitem__(self, index: int, /) -> _T:
        ...

    def __delitem__(self, index: int, /):
        ...

    @overload
    def __setitem__(self, arg2: slice, value: Iterable[_T], /) -> None: ...

    @overload
    def __setitem__(self, index: int, value: _T, /) -> None: ...

    def __len__(self) -> int:
        ...

    def __contains__(self, item: object, /) -> bool:
        ...

    def __iter__(self) -> Iterator[_T]:
        ...

@type_check_only
class _IndexableVector(_Vector[_T], Protocol):
    """add `index` methods."""

    def index(self, item: _T, /) -> int:
        ...

@type_check_only
class _VectorSequence(_IndexableVector[_T], Protocol):
    """Adds `count` and `index` methods."""

    def index(self, item: _T, /) -> int:
        ...

@type_check_only
class _Assignable(Protocol):

    def assign(self, other: Self) -> Self: ...

@type_check_only
class _FloatLike(Protocol):
    def __float__(self) -> float: ...

    def __add__(self, other: float | Self) -> Self: ...

    def __iadd__(self, other: float | Self) -> Self: ...

    def __truediv__(self, other: float | Self) -> Self: ...

    def __sub__(self, other: float | Self) -> Self: ...

    def __isub(self, other: float | Self) -> Self: ...

    def __lt__(self, other: float | Self) -> bool: ...

    def __le__(self, other: float | Self) -> bool: ...

    def __mul__(self, other: Self) -> Self: ...

    def __ge__(self, other: float | Self) -> bool: ...

    def __gt__(self, other: float | Self) -> bool: ...

    def __eq__(self, value: object) -> bool: ...

@type_check_only
class _Calculable(_Assignable, _FloatLike, Protocol):

    cMaxValue: float
    cMinValue: float
    cPrecisionValue: float

    @classmethod
    def getMin(cls) -> Self: ...

    @classmethod
    def getMax(cls) -> Self: ...

    @classmethod
    def getPrecision(cls) -> Self: ...

    @property
    def Valid(self) -> bool: ...

    def ensureValid(self, value: Self) -> Self: ...

    def ensureValidNonZero(self, value: Self) -> Self: ...

    def __hash__(self) -> int: ...
