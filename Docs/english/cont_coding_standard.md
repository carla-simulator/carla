# 코딩 표준

*   [__일반__](#일반)  
*   [__Python__](#python)  
*   [__C++__](#c)  

---
## 일반

  * 탭이 아닌 스페이스를 사용하세요.
  * diff에 불필요한 변화를 만들 수 있으므로 끝에 공백을 추가하지 마세요.

---
## Python

  * 주석은 80열을 넘지 않아야 하며, 코드는 120열을 넘지 않아야 합니다.
  * 모든 코드는 Python 2.7과 3.7과 호환되어야 합니다.
  * [Pylint][pylintlink]는 어떠한 오류나 경고도 표시하지 않아야 합니다(`.pylintrc`에서 확인할 수 있는 `numpy`와 `pygame` 같은 외부 클래스에 대해서는 몇 가지 예외가 적용됩니다).
  * Python 코드는 [PEP8 스타일 가이드][pep8link]를 따릅니다(가능한 한 `autopep8` 사용).

[pylintlink]: https://www.pylint.org/
[pep8link]: https://www.python.org/dev/peps/pep-0008/

---
## C++

  * 주석은 80열을 넘지 않아야 하며, 코드는 더 명확한 코드가 될 경우 드물게 이 제한을 약간 초과할 수 있습니다.
  * 컴파일 시 어떠한 오류나 경고도 발생하지 않아야 합니다
    (`clang++-8 -Wall -Wextra -std=C++14 -Wno-missing-braces`).
  * `throw` 사용은 금지되어 있으며, 대신 `carla::throw_exception`을 사용하세요.
  * 언리얼 C++ 코드(CarlaUE4와 Carla 플러그인)는 탭 대신 스페이스를 사용하는 것을 제외하고
    [언리얼 엔진의 코딩 표준][ue4link]을 따릅니다.
  * LibCarla는 [Google의 스타일 가이드][googlelink]의 변형을 사용합니다.
  * `try-catch` 블록의 사용은 서버 측에서 코드가 사용되는 경우
    `#ifndef LIBCARLA_NO_EXCEPTIONS`로 둘러싸여 있어야 합니다.

[ue4link]: https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/
[googlelink]: https://google.github.io/styleguide/cppguide.html