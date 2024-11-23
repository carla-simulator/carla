# 문서 표준

이 문서는 문서 작성에 기여할 때 따라야 할 규칙의 가이드이자 예시로 사용됩니다.

*   [__문서 구조__](#문서-구조)  
*   [__규칙__](#규칙)  
*   [__예외__](#예외)  

---
## 문서 구조

우리는 [`extra.css`](https://github.com/carla-simulator/carla/tree/master/Docs/extra.css) 파일과 함께 마크다운과 HTML 태그를 혼합하여 문서를 커스터마이즈합니다.
Python API 문서를 업데이트하려면 마크다운을 직접 수정하는 대신 [`carla/PythonAPI/docs/`][fileslink] 내의 해당 YAML 파일을 편집하고 [`doc_gen.py`][scriptlink] 또는 `make PythonAPI.docs`를 실행해야 합니다.

이는 `carla/Docs/` 내의 각 마크다운 파일을 다시 생성하며, 이를 `mkdocs`에 제공할 수 있습니다.

[fileslink]: https://github.com/carla-simulator/carla/tree/master/PythonAPI/docs
[scriptlink]: https://github.com/carla-simulator/carla/blob/master/PythonAPI/docs/doc_gen.py

---
## 규칙

*   섹션 사이와 문서 끝에는 항상 빈 줄을 남겨두세요.
*   HTML 관련 콘텐츠, 마크다운 표, 코드 스니펫, 참조 링크를 제외하고 작성은 `100`열을 초과해서는 안 됩니다.
*   인라인 링크가 제한을 초과하는 경우, `[name](https://)`보다는 참조된 `[name][reference_link]` 마크다운 표기법 `[reference_link]: https://`를 사용하세요.
*   줄 끝에 두 개의 공백을 남기는 대신 `<br>`을 사용하여 인라인 줄바꿈을 하세요.
*   새 페이지 시작 시 제목을 만들려면 `<h1>제목</h1>`을, 탐색 바에 **표시되지 않을** 제목을 만들려면 `<hx>제목<hx>`을 사용하세요.
*   탐색 바에 표시할 제목을 만들려면 제목 아래에 `------`를 사용하거나 `#` 계층 구조를 사용하세요.

---
## 예외

  * PythonAPI 레퍼런스와 같이 Python 스크립트를 통해 생성된 문서

유용한 마크다운 [치트시트][cheatlink].

[cheatlink]: https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet