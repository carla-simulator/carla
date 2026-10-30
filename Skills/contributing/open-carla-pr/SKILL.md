---
name: open-carla-pr
description: Takes a contribution to CARLA on UE 5.8 from first contact to merged-ready pull request against ue58-dev — reporting a bug or requesting a feature as a GitHub issue from the repo's templates, opening a GitHub Discussion to agree a CI workflow change or a large feature with the maintainers before coding, then branch naming, syncing with the base, reviewing the diff against the contribution rules (CHANGELOG entry, no unagreed CI workflow changes, no hand-edited generated docs, whitespace, C++ exceptions, pylint, tests that are not weakened), the local build and tests CI does not run, and filling .github/pull_request_template.md before `gh pr create`. Use when the user asks to "open a PR", "open an issue", "report a bug in CARLA", "request a feature", "open a discussion", "propose a CI workflow", "ask the maintainers before coding", "contribute this to CARLA", "prepare my branch for review", "fill the PR template", "is my change ready to merge", or "what does CARLA require in a pull request".
license: MIT
compatibility: Any OS with git, the GitHub CLI (gh, authenticated) and python3, inside a CARLA checkout on branch ue58-dev or a branch made from it. Building and testing additionally needs the build-carla-ue58 environment. UE4 (ue4-dev) contributions are out of scope.
metadata:
  group: contributing
  reference: references/rules.md
---

# Contribute to CARLA: issues, discussions and pull requests (ue58-dev)

> **Paths.** `references/…` below is relative to the directory holding this
> SKILL.md. Every command runs from the root of the CARLA checkout.

A pull request to CARLA is reviewed against written rules —
[`Docs/cont_contribution_guidelines.md`](../../../Docs/cont_contribution_guidelines.md),
[`Docs/cont_coding_standard.md`](../../../Docs/cont_coding_standard.md) and
[`.github/pull_request_template.md`](../../../.github/pull_request_template.md)
and the issue templates under `.github/ISSUE_TEMPLATE/`.
[references/rules.md](references/rules.md) condenses them. This skill walks a
branch through them **before** it is opened, because the PR CI only builds and
runs the LibCarla tests: almost every rule below is checked by a human
reviewer, after a round trip, or not at all.

**Everything that reaches GitHub is public and outward-facing**: an issue, a
discussion, a `git push`, a PR. Draft locally, show the user the title and body
(or the diff summary), and send only after they confirm.

Not every request ends in a PR. Step 0 picks the venue. A bug report or a
question stops at its issue or discussion. A PR whose change needs agreement
first waits for it.

## Instructions

```
Progress:
- [ ] Step 0: Pick the venue — issue, discussion, or straight to a PR
- [ ] Step 1: Confirm the target — ue58-dev tree, GitHub (not content) change
- [ ] Step 2: Branch from an up-to-date ue58-dev, named user/topic
- [ ] Step 3: Review the diff against the rules, fix every finding
- [ ] Step 4: Build and run the tests CI will not run
- [ ] Step 5: Commit and push (after the user confirms)
- [ ] Step 6: Fill the PR template, open the PR (after the user confirms)
- [ ] Step 7: Watch the checks
```

### Step 0: Pick the venue

| The user has… | Venue | Before coding? |
|---|---|---|
| A bug (crash, wrong result, regression) | **Issue**, bug report — [Opening an issue](#opening-an-issue) | Yes. A fix PR then says `Fixes #<n>` |
| An idea for a new feature | **Issue**, feature request — [Opening an issue](#opening-an-issue) | Yes |
| A question, or "is this a bug or am I using it wrong?" | **Discussion** — [Opening a discussion](#opening-a-discussion) | Yes. Issues are for bugs and features, not support |
| A change to `.github/workflows/` (new or modified CI) | **Discussion, required** — [Opening a discussion](#opening-a-discussion) | **Yes, agreed before the PR.** Unagreed workflow PRs are not merged |
| A large feature, an API change, work that may overlap someone else's, or no idea which module it belongs in | **Discussion** (or Discord) to agree the approach | Strongly recommended |
| A small, self-contained fix or improvement | Straight to Step 1 | — |

#### Check for duplicates (mandatory before any issue or discussion)

A duplicate is closed and points elsewhere. The user loses the context, the
maintainers spend triage time, and the original thread loses the +1 that
would have raised its priority. **Never create an issue or a discussion
without running this search and showing the user what it found.**

1. **Search with several phrasings.** Use 2–4 short queries, not one long
   sentence: the API or sensor name (`set_imu_sensor_gravity`,
   `sensor.camera.depth`), the exact error text (quoted, without paths or
   numbers), the map or component, and a synonym of the symptom. Search
   **open and closed** issues, pull requests and discussions:
   ```bash
   gh issue list --repo carla-simulator/carla --state all --limit 20 --search "<terms>"
   gh pr list    --repo carla-simulator/carla --state all --limit 10 --search "<terms>"
   gh api graphql -f query='query($q:String!){search(query:$q,type:DISCUSSION,first:15){nodes{... on Discussion{number title url closed answer{url}}}}}' \
     -f q='repo:carla-simulator/carla <terms>'
   ```
   Without `gh`, use the same queries in the web search at
   `https://github.com/carla-simulator/carla/issues?q=<terms>` and
   `https://github.com/carla-simulator/carla/discussions?discussions_q=<terms>`.
2. **Read the candidates; do not judge by the title.** Open the plausible hits
   (`gh issue view <n> --repo carla-simulator/carla --comments`, or the
   discussion URL). Check the version and branch they were reported on. A
   UE4-era issue may not apply to `ue58-dev`, and vice versa.
3. **Decide, and tell the user which case it is, with the links:**

   | Found | Do instead of creating |
   |---|---|
   | Open issue or discussion about the same thing | Add to it: a comment with the new information (your Setup, a smaller repro, a log) or a 👍 reaction. Do not post "+1" comments. A fix PR then uses its `Fixes #<n>` |
   | Closed issue, fixed | Check whether the fix is in the user's tree (`git log --oneline --grep '#<n>'`, or `git merge-base --is-ancestor <fix commit> HEAD`). If it is not, the answer is to update. If it is and the bug is back, open a **new** issue that links the old one as a regression |
   | Closed as "not a bug" / answered discussion | That answer is the reply. Show it to the user |
   | Open PR already addressing it | Point the user to it (review, test it) instead of a parallel issue or PR |
   | Related but different | Create the new one, and link the related threads in its body (`Related to #<n>`) |
   | Nothing relevant | Create it, and say in the body what you searched for, so a triager does not repeat it |

Report the queries you ran and the result to the user before drafting anything.

### Step 1: Confirm the target

```bash
git rev-parse --abbrev-ref HEAD
ls CMakePresets.json Unreal/CarlaUnreal >/dev/null && echo "UE5 CMake tree"
git remote -v | grep -E 'carla-simulator/carla(\.git)? \(fetch\)'
```

- The tree must be the UE 5.8 one: `CMakePresets.json` and `Unreal/CarlaUnreal`
  exist, no `Makefile`. The base branch is **`ue58-dev`**. A UE4 change
  targets `ue4-dev` with different rules — stop and tell the user this skill
  does not cover it.
- Note which remote points at `carla-simulator/carla`; it is called `UP`
  below. A maintainer's clone often has it as `origin` and pushes branches
  there directly. An external contributor has it as `upstream` and `origin`
  is their fork. Do not assume the name.
- **Assets are not GitHub PRs.** Maps, vehicles, walkers and props live in the
  Bitbucket content repository (`Unreal/CarlaUnreal/Content/Carla`, branch
  `ue58-dev-carla`) and are reviewed by the art team. Only the code, config and
  docs that go with them belong in this PR.

### Step 2: Branch from an up-to-date ue58-dev

```bash
UP=origin          # or upstream — the remote found in Step 1
git fetch "$UP"
git switch -c <github-user>/<short_topic> "$UP/ue58-dev"     # new work
# existing work on a branch:
git rebase "$UP/ue58-dev"
```

- The branch name is **`username/name_of_the_contribution`**. Never commit on
  `ue58-dev` itself. If the work is already there, create the branch from
  the current `HEAD`, then reset local `ue58-dev` to `$UP/ue58-dev`.
- Rebase before the PR is opened. Once reviewers have commented, merge
  `$UP/ue58-dev` into the branch instead, so the commits they reviewed are not
  rewritten under them by a force-push.

### Step 3: Review the diff against the rules

Look at what the PR will actually contain, not only at what you meant to
change:

```bash
BASE=$(git merge-base HEAD "$UP/ue58-dev")
git status --short                 # uncommitted / untracked files
git diff --stat "$BASE"            # what the PR will carry
git diff --name-status "$BASE"
```

Go through every check. Each one cites the rule it enforces in
[references/rules.md](references/rules.md).

1. **One topic, nothing stray.** Every file in `--name-status` belongs to this
   change. Watch for untracked files in the root, such as screenshots, `.log`
   files, `Build*` folders or editor settings. They get committed by `git add -A`.
   Stage paths explicitly.
2. **CHANGELOG.** If the change is visible to a user (anything under
   `LibCarla/`, `PythonAPI/carla/`, `PythonAPI/examples/`, `Unreal/`,
   `Ros2Native/`, `Util/`, `CMake/`, or new skills/docs pages), `CHANGELOG.md` must
   change, with an entry under `## LATEST Changes` in the right
   `### New features` / `### Extensions` / `### Bug fixes` → `####` subsection.
   Match the house format: `* **Bold lead** — what it does, and what the user does with it.`
   ```bash
   git diff "$BASE" -- CHANGELOG.md
   ```
3. **CI workflows.** Any change under `.github/workflows/` needs a maintainers'
   discussion agreed **before** the PR (GitHub Discussions or Discord), linked
   in the PR body. Without one, the change is not merged. If none exists, stop
   and [open a discussion](#opening-a-discussion) first.
   ```bash
   git diff --name-only "$BASE" -- .github/workflows/
   ```
4. **Generated files.** `Docs/python_api.md` is generated from
   `PythonAPI/docs/*.yml` by `PythonAPI/docs/doc_gen.py`. `Docs/bp_library.md`
   comes from `bp_doc_gen.py`, and `Docs/catalogue_*.md` from `catalogue_doc_gen.py`.
   If one of them was edited by hand, revert it, edit the source, and regenerate.
   ```bash
   git diff --name-only "$BASE" -- Docs/python_api.md Docs/bp_library.md 'Docs/catalogue_*.md'
   git diff --name-only "$BASE" -- 'PythonAPI/docs/*.yml'
   ```
   A changed `.yml` with no regenerated `python_api.md` is the opposite mistake:
   regenerate it and commit both.
5. **Whitespace.** Spaces, not tabs; no trailing whitespace.
   ```bash
   git diff --check "$BASE"
   git diff -U0 "$BASE" | grep -nP '^\+(?!\+\+).*\t'
   ```
   Do not "fix" whitespace in lines you did not otherwise touch. It buries the
   real change in the diff.
6. **C++.** `throw` is forbidden; use `carla::throw_exception`. A `try`/`catch`
   must sit inside `#ifndef LIBCARLA_NO_EXCEPTIONS`. Builds must have no warnings
   (`ENABLE_ALL_WARNINGS` is on by default).
   ```bash
   git diff -U0 "$BASE" -- '*.cpp' '*.h' | grep -nE '^\+.*\b(throw|try|catch)\b' | grep -v throw_exception
   ```
7. **Python.** Pylint clean with the repo config. Code ≤ 120 columns,
   comments ≤ 80.
   ```bash
   git diff --name-only --diff-filter=AM "$BASE" -- '*.py' \
     | xargs -r python3 -m pylint --rcfile=PythonAPI/.pylintrc
   ```
   Pre-existing warnings in a file you touched are not yours to fix in this PR,
   but anything you introduce is.
8. **Tests.** A new test must assert behaviour and fail on a regression. A test
   that only checks that something imports or spawns is rejected, as is one
   that duplicates existing coverage. A modified test must not be weakened:
   no removed asserts, no wider tolerances, no new skips. If the expected
   behaviour really changed, the PR body says why.
   ```bash
   git diff "$BASE" -- PythonAPI/test LibCarla/source/test \
     | grep -nE '^-.*(assert|EXPECT_|ASSERT_)|^\+.*(skip|DISABLED_|assertAlmostEqual.*(delta|places))'
   ```
   Every hit needs a justification you can write in the PR's **Tests** section.
9. **Docs.** If behaviour or API changed, the docs page that describes it
   changes too. With docs changes, build them strictly. A missing anchor only
   prints an `INFO` line, so grep for it:
   ```bash
   pip3 install -r Docs/requirements.txt
   python3 -m mkdocs build --strict 2>&1 | grep -E 'WARNING|ERROR|does not contain an anchor'
   ```
   For writing docs, see [[document-carla-feature]].
10. **Docs-only PRs skip CI.** `ue5_pr.yml` has `paths-ignore` for `Docs/**`,
    `**/*.md`, `mkdocs.yml`, `docs_hooks.py` and `PythonAPI/docs/**`. If
    every changed file matches, no build runs, and the mkdocs build above is
    the only check the PR gets.

Report the findings to the user as a short list (rule → file → fix) and fix
them before going on.

### Step 4: Build and run the tests CI will not run

CI on a PR to `ue58-dev` configures the `Release` preset with
`-DENABLE_ROS2=ON -DBUILD_LIBCARLA_TESTS=ON`. It builds the client, the Python
API, the server and the package, and it runs the two LibCarla test binaries.
**It runs no Python tests.** Skip this step only for docs-only PRs.

```bash
cmake --build Build/Release --target carla-client carla-python-api carla-server
cmake --build Build/Release --target libcarla_test_server libcarla_test_client
./Build/Release/LibCarla/libcarla_test_server
./Build/Release/LibCarla/libcarla_test_client
```

If the tree is not configured yet, or a target fails, follow
[[build-carla-ue58]]. The OpenDRIVE tests need `Build/test-content` (see
`Docs/dev_testing.md`).

**Packaging locally is not strictly required, but it is strongly
recommended.** CI builds the package too, and failures that only appear
there cost a full CI round trip each: cook errors, assets missing from the
cook, and code that compiles in the editor but not in a shipping build.
Anything that touches `Unreal/`, content references, cook or config files is
at risk. Build the package with [[package-carla-ue58]] before opening the PR,
and smoke-test against the packaged server rather than the editor. If you
skip it, say so under **Possible Drawbacks**.

For anything touching the server or the Python API, run the smoke tests against
a dedicated server on **port 3654** (see [[run-carla-ue58-server]]):

```bash
cd PythonAPI/test
python3 test_connection.py -p 3654 --timeout=60.0
python3 -m nose2 -v $(cat smoke_test_list.txt)
```

Record what you ran and on what: the PR body needs it, and "tests pass"
without a platform and a list is not an answer.

### Step 5: Commit and push

- Commit messages: an imperative subject line that says what changed
  ("Fix wheel spin when Chaos is not simulating"), not "fixes" or
  "final changes". Follow the repository's commit attribution rules.
- Show the user `git log --oneline "$BASE"..HEAD` and `git diff --stat "$BASE"`,
  and push only after they confirm:
  ```bash
  git push -u origin HEAD        # origin = carla-simulator/carla (maintainer) or the fork
  ```

### Step 6: Fill the PR template and open the PR

Write the body to a scratch file. Keep the template's `####` headings in
order, and drop its leading `<!-- … -->` checklist comment, which renders as
nothing. Fill each section from evidence:

| Section | Where it comes from |
|---|---|
| **Description** | What changed and why, from `git log "$BASE"..HEAD` and the diff. Name the user-visible effect first, then the mechanism. Mention new API, options, and docs pages. |
| **Fixes #** | The issue number, if any (from Step 0's search or [Opening an issue](#opening-an-issue)). `Fixes #<n>` closes it on merge. Check it with `gh issue view <n> --repo carla-simulator/carla`. Otherwise delete the line, never leave `Fixes #` empty. |
| **Where has this been tested?** | Platform: `lsb_release -ds` (or OS name). Python: `python3 --version` of the interpreter that ran the tests. Unreal Engine: `5.8` (from `$CARLA_UNREAL_ENGINE_PATH/Engine/Build/Build.version`). |
| **Tests** | Each new or modified test, what it asserts, and which regression it catches. List what you ran in Step 4. Write `None` only if no tests apply, and say why. |
| **CI workflows** | `None`, unless `.github/workflows/` changed: then the URL of the agreed discussion from [Opening a discussion](#opening-a-discussion). |
| **Possible Drawbacks** | Behaviour changes, performance cost, API breaks, assets or content that must land at the same time, things not tested (e.g. Windows). |

Show the title and body to the user. After they confirm:

```bash
gh pr create --repo carla-simulator/carla --base ue58-dev \
  --head <owner>:<branch> --title "<imperative summary>" --body-file <scratch>/pr_body.md
```

`--head` is `<branch>` when the branch is on `carla-simulator/carla` itself,
and `<fork-owner>:<branch>` from a fork. Add `--draft` if the user wants early
feedback before it is complete. Give the user the PR URL that `gh` prints.

### Step 7: Watch the checks

```bash
gh pr checks <number> --repo carla-simulator/carla --watch
```

The `UE5-PR` build is long: it builds the client, the Python API, the server
and the package. On a red check, read the failing job's log
(`gh run view <run-id> --log-failed`) and fix the cause on the branch. Do not
re-run it hoping for a pass. A docs-only PR shows no `UE5-PR` check at all
(Step 3, item 10).

## Opening an issue

**First run [Check for duplicates](#check-for-duplicates-mandatory-before-any-issue-or-discussion).**
Only continue if it found nothing that covers the report.

The repository has two templates, `.github/ISSUE_TEMPLATE/bug_report.md` and
`feature_request.md`. Reproduce their bold headings exactly and fill each one.
An issue with blank sections is usually answered with a request for them, which
costs a round trip.

**Bug report.** Gather the **Setup** block from the machine. Do not guess it.

| Field | Command |
|---|---|
| CARLA version | `git rev-parse --abbrev-ref HEAD` + `git rev-parse --short HEAD` (branch and commit), or the package's `VERSION`/release name |
| Platform | `lsb_release -ds` (or the OS name and version) |
| Python version | `python3 --version` of the interpreter that ran the client |
| GPU / GPU Drivers | `nvidia-smi --query-gpu=name,driver_version --format=csv,noheader` |
| Unreal Engine | `5.8` (from `$CARLA_UNREAL_ENGINE_PATH/Engine/Build/Build.version`) |

Then fill these:
- **Describe the bug**: what happens, in one or two sentences.
- **Steps to reproduce**: numbered, from a fresh server, including the map,
  the sync/async mode and the command line.
- **Expected behavior**.
- **Logs**: the relevant part of the server log from
  `Unreal/CarlaUnreal/Saved/Logs/` and of the client output, not the whole file.
- **Scripts**: a *minimal* client script that reproduces it. Strip it down until
  removing anything more makes the bug disappear.
- **Screenshots** and **Additional context**, if they help.

Before filing, check it really is a bug: read the docs page and the FAQ
(`Docs/build_faq.md`). Otherwise it is a question, which goes to a
discussion.

**Feature request.** Fill **Is your feature request related to a problem?**,
**Describe the solution you'd like**, **Describe alternatives you've
considered** and **Additional context**. Lead with the problem, not the
solution.

Write the body to a scratch file. Show the user the title and body, then, after
they confirm:

```bash
gh issue create --repo carla-simulator/carla --title "<short, specific title>" --body-file <scratch>/issue.md
```

Titles name the symptom and where it happens ("`sensor.camera.depth` returns
zeros on Town12 after `load_world`"), not "Bug" or "Help". Give the user the
URL that `gh` prints. Labels are set by the maintainers, so do not pass
`--label`.

## Opening a discussion

GitHub Discussions are the venue for questions, for proposals that need the
maintainers' agreement, and for **every CI workflow change before its PR**.
The Discord server (`https://discord.com/invite/8kqACuC`) is the other accepted
venue; a discussion leaves a link the PR can cite.

`gh` has no `discussion` subcommand. Use GraphQL through `gh api graphql`.

1. **Search first. This is mandatory.** Run [Check for duplicates](#check-for-duplicates-mandatory-before-any-issue-or-discussion).
   For a CI proposal, also search for the workflow's subject (`workflow`, `CI`,
   the job or tool name). For a feature proposal, also check the open
   feature-request issues. If a thread already exists, add to it instead of
   opening a second one.
2. **Get the repository id and the categories.** Do not hardcode the category
   names; read them:
   ```bash
   gh api graphql -f query='{repository(owner:"carla-simulator",name:"carla"){id discussionCategories(first:25){nodes{id name description}}}}'
   ```
   Pick the category that fits (a question → the Q&A one; a proposal → the
   ideas/proposals one). Ask the user when none clearly fits.
3. **Write the body** to a scratch file.
   - **CI workflow proposal**: the guidelines require exactly these three points:
     - **What is happening**: the problem the workflow addresses.
     - **Why a workflow is needed** to solve it, and why an existing workflow
       or a local check is not enough.
     - **What it runs, on which triggers, and what it costs** in runners and
       time. Target branch `ue58-dev`. Include a draft of the YAML if one exists.
   - **Feature or design proposal**: the problem, the proposed approach, the
     modules it touches (see `Docs/core_foundations.md`), the API impact, and
     open questions for the maintainers.
   - **Question**: what you are trying to do, what you tried, what happened,
     and the Setup block from [Opening an issue](#opening-an-issue).
4. **Show the title and body to the user. After they confirm**, create it:
   ```bash
   gh api graphql \
     -f query='mutation($r:ID!,$c:ID!,$t:String!,$b:String!){createDiscussion(input:{repositoryId:$r,categoryId:$c,title:$t,body:$b}){discussion{number url}}}' \
     -f r=<repository id> -f c=<category id> -f t='<title>' -F b=@<scratch>/discussion.md
   ```
   Without `gh`, use the web form at
   `https://github.com/carla-simulator/carla/discussions/new/choose`.
5. **Give the user the URL and stop.** A workflow PR waits until a maintainer
   agrees in that thread. Then the URL goes in the PR's **CI workflows**
   section (Step 6).

## Gotchas

- **Pass `--base ue58-dev` explicitly.** Without it, `gh pr create` targets
  the default branch. That branch has changed before (it was `ue4-dev`), and
  an old fork can still point at the previous one.
- **A green CI tick does not mean the Python side works.** CI runs no Python
  tests; a broken smoke test only shows up when a reviewer runs it.
- **Workflow changes without a prior agreement are not merged**, however
  good. Raise the discussion first and put its link in the body.
- **Weakening a test to make it pass hides the regression it guards.** Fix the
  code. If the expected behaviour really changed, say so in **Tests**.
- **Hand-edited `Docs/python_api.md` is overwritten** by the next
  `doc_gen.py` run. Edit the YAML.
- **`git add -A` from the root commits strays.** Stage explicit paths and read
  `git diff --cached --stat` before committing.
- **Rebasing a PR under review** force-pushes over commits reviewers already
  read. Merge the base in instead once review has started.
- **`gh discussion` does not exist.** Discussions are created only through
  `gh api graphql` (or the web form).
- **`-F b=@file` vs `-f b=@file`**: `-F` reads the body from the file; `-f`
  sends the literal string `@file` as the body.
- **Searching once with the user's whole sentence finds nothing.** GitHub search
  matches words, not meaning. Split it into the API name, the error text and
  the symptom, and include closed threads. The fix, or the answer, is often
  in a closed one.
- **Questions are not issues.** Support questions filed as issues get
  redirected to Discussions. File a bug only once you can reproduce it.
- **Never invent the Setup block.** A wrong CARLA commit or GPU driver in a
  bug report sends the triager the wrong way. Read every value from the machine.
- **Asset changes do not go in this PR.** They go to the content repository
  (Bitbucket, `ue58-dev-carla`), through the art team.
