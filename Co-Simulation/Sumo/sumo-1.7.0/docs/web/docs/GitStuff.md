---
title: GitStuff
permalink: /GitStuff/
---

# Miscellaneous Git Topics

## Resolve LF status error

Occasionally, when adding files with different line endings to the
repository, a working copy may enter a state with changes that can not
be reverted (by stash or checkout).

```
warning: CRLF will be replaced by LF in build/wix/sumo.wxs.
The file will have its original line endings in your working directory.
```

Solution
([stackoverflow](https://stackoverflow.com/questions/1967370/git-replacing-lf-with-crlf)):

```
git rm --cached -r .
git reset --hard
```