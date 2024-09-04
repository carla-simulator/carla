"""
Warning:
    The stub files for the ad module are only a partial representation of the actual API,
    some functions and attributes are still missing.
"""

from typing import Iterable, Iterator, Protocol, TypeVar, type_check_only

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
class _FloatLike(Protocol):
    def __float__(self) -> float: ...

    def __add__(self, other: float | Self) -> Self: ...

    def __iadd__(self, other: float | Self) -> Self: ...

    def __truediv__(self, other: float | Self) -> Self: ...

    def __sub__(self, other: float | Self) -> Self: ...

    def __isub(self, other: float | Self) -> Self: ...

    def __lt__(self, other: float | Self) -> bool: ...

    def __le__(self, other: float | Self) -> bool: ...

    def __ge__(self, other: float | Self) -> bool: ...

    def __gt__(self, other: float | Self) -> bool: ...

    def __eq__(self, value: object) -> bool: ...


@type_check_only
class _Vector(Protocol[_T]):

    def __getitem__(self, index: int) -> _T:
        ...

    def __delitem__(self, index: int):
        ...

    def __setitem__(self, index: int, value: _T) -> None:
        ...

    def __len__(self) -> int:
        ...

    def append(self, item: _T) -> None:
        ...

    def count(self, item: _T) -> int:
        ...

    def extend(self, iterable: Iterable[_T]) -> None:
        ...

    def index(self, item: _T) -> int:
        ...

    def insert(self, index: int, item: _T) -> None:
        ...

    def reverse(self) -> None:
        ...

    def __contains__(self, item: object) -> bool:
        ...

    def __iter__(self) -> Iterator[_T]:
        ...

class _Calculable(Protocol):

    cMaxValue: float
    cMinValue: float
    cPrecisionValue: float

    @classmethod
    def getMin(cls) -> Self:
        pass

    @classmethod
    def getMax(cls) -> Self:
        pass

    @classmethod
    def getPrecision(cls) -> Self:
        pass

    def Valid(self) -> bool:
        pass

    def ensureValid(self, value: Self) -> Self:
        pass

    def ensureValidNonZero(self, value: Self) -> Self:
        pass

    def assign(self, other: Self) -> None:
        pass

    def __truediv__(self, other: Self) -> Self:
        pass

    def __sub__(self, other: Self) -> Self:
        pass

    def __isub__(self, other: Self) -> None:
        pass

    def __le__(self, other: Self) -> bool:
        pass

    def __lt__(self, other: Self) -> bool:
        pass

    def __mul__(self, other: Self) -> Self:
        pass

    def __ne__(self, other: Self) -> bool:
        pass

    def __add__(self, other: Self) -> Self:
        pass

    def __eq__(self, other: Self) -> bool:
        pass

    def __float__(self) -> float:
        pass

    def __ge__(self, other: Self) -> bool:
        pass

    def __gt__(self, other: Self) -> bool:
        pass

    def __hash__(self) -> int:
        pass
