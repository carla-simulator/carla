# Contributing to CARLA Studio

CARLA Studio is community-stewarded under
**AGPL-3.0-or-later**. Anyone willing to put in the work is welcome —
this document codifies how the maintainer and reviewer roles are filled
so the project doesn't bottleneck on any one person.

If you're filing a quick bug fix or doc tweak, you can skip to
[Pull-request checklist](#pull-request-checklist). The role conventions
matter when contributions get sustained.

---

## Author

[Abdul, Hashim](https://github.com/im-hashim) created CARLA Studio. Their
attribution is preserved in the LICENSE, THIRD_PARTY.md, and CITATION.cff
forever — that's how authorship works.

---

## Maintainer model — open by design

There is no appointed maintainer beyond the original author. The role is
**open**: anyone who consistently lands quality contributions, helps
triage issues, and reviews other contributors' PRs is recognised as a
maintainer.

To volunteer:

1. Land at least **3 reviewed-and-merged PRs** in the project.
2. Open an issue titled `Maintainer: <your handle>` proposing yourself,
   linking the merged PRs.
3. Existing maintainers (or, in the bootstrap phase, the author) confirm
   by 👍 reaction. Two confirmations land it.

A maintainer's responsibilities:

- Review PRs in the areas you've worked.
- Triage incoming issues — apply labels, dedupe, ask for repro steps.
- Honour the AGPL-3.0 licence terms; review new dependencies for
  licence compatibility (see [LICENSES/THIRD_PARTY.md](LICENSES/THIRD_PARTY.md)
  for the rule).
- Step away cleanly: if you go inactive for 6 months, you can be removed
  from the maintainer list by a similar two-confirmation issue. No
  drama, no permanent commitment.

The maintainer list lives at the bottom of this file. Edit it in the
same PR as the issue that adds you.

---

## Reviewer model — most-contributed wins

For every PR, the **default reviewer** is the contributor with the most
lines contributed (cumulative authorship — counted as additions + deletions
they've authored historically) in the file(s) the PR modifies, ignoring
the PR's own author.

This is computed mechanically:

```bash
# How to find your default reviewer manually until the action lands:
git log --shortstat --pretty=format:'%an' -- <file1> <file2> ... \
  | awk '/^[A-Za-z]/ {a=$0} /insertions|deletions/ {
           ins=del=0;
           for (i=1; i<=NF; i++) {
             if ($i ~ /insertion/) ins=$(i-1);
             if ($i ~ /deletion/)  del=$(i-1);
           }
           t[a] += ins + del;
         }
         END { for (k in t) printf "%6d  %s\n", t[k], k }' \
  | sort -rn
```

The top non-author contributor is your reviewer. If no prior contributor
exists (greenfield file), request a maintainer manually.

A GitHub Action will land in
`.github/workflows/auto-reviewer.yml` to assign this automatically. Until
then, run the snippet above and `@`-mention the result in your PR
description.

This convention deliberately:

- Routes review to whoever already has the most context for the change.
- Avoids creating a permanent reviewer hierarchy.
- Scales: as new contributors build line-count in different files, they
  naturally become the default reviewers for those files.

If your PR touches widely-shared files (e.g. `src/app/carla_studio.cpp`),
two reviewers are encouraged: the most-changes contributor + a maintainer
in case the first reviewer is slow.

---

## Pull-request checklist

Before opening a PR:

- [ ] Branch from `main`.
- [ ] Commit messages are imperative present-tense, ≤ 72 chars on the
  subject line, with a body if the change isn't self-evident.
- [ ] All new C/C++ source files start with
  `// SPDX-License-Identifier: AGPL-3.0-or-later`.
- [ ] If you added a new third-party dependency, add a row to
  [LICENSES/THIRD_PARTY.md](LICENSES/THIRD_PARTY.md) in the same PR and
  verify the licence against the
  [FSF compatibility list](https://www.gnu.org/licenses/license-list.html).
- [ ] If your change is user-visible, append it to
  [CHANGELOG.md](CHANGELOG.md) under the in-progress version heading.
- [ ] Build cleanly: `cmake --build Build --target carla-studio`.
- [ ] If your change touches tests, run `ctest` from the build dir.
- [ ] Run the offscreen smoke test: `QT_QPA_PLATFORM=offscreen
  ./Build/Apps/CarlaStudio/carla-studio` for at least 5 seconds without
  crashes or warnings.

When you open the PR:

- Describe what changed and why. The "why" lives in the description; the
  "what" lives in the diff.
- Link related issues (`Closes #123`).
- `@`-mention your default reviewer per the section above.

---

## Style

- Prefer editing existing files over creating new ones.
- Match the surrounding code's style. The codebase is monolithic in
  `src/app/carla_studio.cpp`; new modules go under `src/utils/` (cross-
  cutting helpers) or `src/integrations/` (per-external-system glue).
- Don't add comments that explain *what* the code does — only *why*, when
  the why is non-obvious. Names should carry the *what*.
- Don't add error handling for impossible cases. Validate at system
  boundaries; trust internal callers.

---

## Licence and citation

- This project is **AGPL-3.0-or-later**. By submitting a PR you agree
  your contribution is licensed under the same terms.
- If you publish work that uses or extends Studio, please cite via
  [CITATION.cff](CITATION.cff). GitHub renders a "Cite this repository"
  button from it.

---

## Maintainers

| Maintainer | Areas | Active since |
|---|---|---|
| [Abdul, Hashim](https://github.com/im-hashim) | Author / bootstrap maintainer | 2026-04 |
<!-- Add new maintainers below this line in the same PR as the
     "Maintainer: <handle>" issue confirmation. -->
