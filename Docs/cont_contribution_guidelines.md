# Contributing to CARLA

The CARLA team is glad to accept contributions from anybody willing to collaborate. There are different ways to contribute to the project, depending on the capabilities of the contributor. Everyone taking part is expected to follow the [Code of conduct](cont_code_of_conduct.md).

- [__Report bugs__](#report-bugs)
- [__Request features__](#request-features)
- [__Code contributions__](#code-contributions)
- [__Art contributions__](#art-contributions)
- [__Docs contributions__](#docs-contributions)

---
## Report bugs

Issues can be reported in the [issue section][issueslink] on GitHub. Before reporting a new bug, make sure to do some checkups.

__1. Check if the bug has been reported.__ Look it up in that same issue section on GitHub.

__2. Read the docs.__ Make sure that the issue is a bug, not a misunderstanding on how CARLA is supposed to work. Read the pages related to the issue in the [documentation][docslink] and take a look at the [FAQ][faqlink] page.

__3. Say which version you are on.__ The branch and commit you built from, the Unreal Engine version, the platform and the Python version.

[issueslink]: https://github.com/carla-simulator/carla/issues
[docslink]: https://carla-ue5.readthedocs.io
[faqlink]: build_faq.md

---
## Request features

Ideas for new features are also a great way to contribute. Any suggestion that could improve the users' experience can be submitted in the corresponding GitHub section [here][frlink].

[frlink]: https://github.com/carla-simulator/carla/issues?q=is%3Aissue+is%3Aopen+label%3A%22feature+request%22+sort%3Acomments-desc

---
## Code contributions

Before starting hands-on on coding, please check out the [issue board][issueslink] to see what the team is already working on, to avoid overlapping. In case of doubt, or to discuss how to proceed, ask on [GitHub Discussions](https://github.com/carla-simulator/carla/discussions) or contact the team on the [Discord server](https://discord.com/invite/8kqACuC).

To start working, [fork the CARLA repository](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/fork-a-repo) and clone your fork. Remember to [keep your fork in sync](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/syncing-a-fork) with the original repository.

### Before getting started

A basic introduction to C++ programming with Unreal is in Unreal's [C++ programming tutorials][ue5tutorials]. Build CARLA from source first, following [Linux](build_linux_ue5.md) or [Windows](build_windows_ue5.md), and read [Foundations](core_foundations.md) to see which module a new feature belongs in.

[ue5tutorials]: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-cpp-programming-tutorials

### Coding standard

Follow the [coding standard](cont_coding_standard.md) when submitting new code.

### Tests

Every change has to be tested, and the tests have to be sufficient and minimal: enough to show the change works and to catch it breaking later, and nothing more. Each test is code the maintainers keep running and fixing for as long as it exists. [Running the tests](dev_testing.md) lists the suites and how to run them:

| Suite | Location | Needs a server |
| --- | --- | --- |
| LibCarla unit tests | `LibCarla/source/test/` | No |
| Python API unit tests | `PythonAPI/test/unit/` | No |
| Smoke tests | `PythonAPI/test/smoke/`, listed in `PythonAPI/test/smoke_test_list.txt` | Yes, on port 3654 |
| Integration tests | `PythonAPI/test/API/`, listed in `PythonAPI/test/integration_tests_list.txt` | Yes, on port 2000 |

Before adding a test, ask two questions of it: does it do something useful, and would a maintainer merge this kind of test? A useful test exercises real behaviour and fails when that behaviour breaks.

*   __Extend before you add.__ When an existing unit or smoke test already covers the feature, add the case to it rather than creating a new file. A new smoke test module also has to be added to `smoke_test_list.txt`, or the list run skips it.
*   __Not accepted:__ empty tests, tests without assertions, tests that only check that something imports or spawns without checking a result, tests that duplicate existing coverage, tests that pass whatever the code does, flaky tests that depend on timing or on the order they run in, and slow tests where a shorter run would catch the same regression.
*   __Do not weaken a test to make it pass.__ Removing assertions, widening tolerances or skipping a test hides the regression it was there to catch. If the expected behaviour really changed, explain why in the pull request.
*   __Say what each test checks.__ The pull request template has a __Tests__ field: list each new or modified test and the regression it would catch.

CI runs only the LibCarla unit tests ([What CI runs](dev_testing.md#what-ci-runs)). The Python unit, smoke and integration tests have to be run locally: the unit tests after a change to LibCarla or the Python API, and the smoke tests after a change to the server.

### CI workflows

Do not open a pull request that adds or changes a workflow under `.github/workflows/` without discussing it with the core maintainers team first, on [GitHub Discussions](https://github.com/carla-simulator/carla/discussions) or the [Discord server](https://discord.com/invite/8kqACuC). The proposal explains:

*   What is happening: the problem the workflow addresses.
*   Why a workflow is needed to solve it.
*   What the workflow would run, on which triggers, and what it costs in runners and time.

Link that discussion in the pull request. Workflow changes that were not agreed beforehand are not merged.

### Submission

Contributions and new features are merged to the development branch, `ue58-dev`. Its content is in the `ue58-dev-carla` branch of the content repository.

*   Always keep your branch updated with the latest changes in `ue58-dev`.
*   Develop the contribution in a child branch from `ue58-dev` named `username/name_of_the_contribution`.
*   Once the contribution is ready, submit a pull request from your branch to `ue58-dev`. Try to be as descriptive as possible when filling in the template: what changed, where it was tested, and the possible drawbacks.
*   Add an entry to `CHANGELOG.md` under `## LATEST Changes` for every user-visible change.
*   A change to the Python API is documented in `PythonAPI/docs/*.yml`. `Docs/python_api.md` is generated from those files with `python3 PythonAPI/docs/doc_gen.py`; never edit it by hand.
*   Continuous integration builds the client, the Python API, the server and the package on Ubuntu, and builds and runs the LibCarla tests ([What CI runs](dev_testing.md#what-ci-runs)). It does not run the Python tests, so run them locally. A green tick mark appears if the checks pass; if there is a red mark, correct the code accordingly.

### Checklist

Before a pull request to `ue58-dev`:

*   [ ] The branch is up-to-date with `ue58-dev` and tested with the latest changes.
*   [ ] The code compiles: `cmake --build Build --target carla-client carla-python-api carla-server`.
*   [ ] The LibCarla unit tests pass ([LibCarla unit tests](dev_testing.md#libcarla-unit-tests)), and so do the Python API unit tests if the Python API changed ([Python API unit tests](dev_testing.md#python-api-unit-tests)).
*   [ ] The smoke tests pass against a running server on port 3654 ([Smoke tests](dev_testing.md#smoke-tests)). On Linux: `cd PythonAPI/test && python3 -m nose2 -v $(cat smoke_test_list.txt)`.
*   [ ] The change is tested, with the fewest tests that show it works: each one asserts behaviour, would fail on a regression, is not redundant, flaky or slow, and extends an existing test where one fits ([Tests](#tests)).
*   [ ] Any new or changed CI workflow was agreed with the core maintainers beforehand, and the discussion is linked in the pull request ([CI workflows](#ci-workflows)).
*   [ ] `CHANGELOG.md` has an entry under `## LATEST Changes` for any user-visible change.
*   [ ] The documentation was extended, if necessary. Python API changes are in `PythonAPI/docs/*.yml` and `Docs/python_api.md` was regenerated.

---
## Art contributions

Art contributions are vehicles, walkers, maps or any other asset. They live in a separate content repository, so a contributor needs access to it before starting.

__1. Create a Bitbucket account.__ Visit the [Bitbucket page](https://bitbucket.org).

__2. Ask for access to the content repository.__ Join the [Discord server](https://discord.com/invite/8kqACuC), go to the __Contributors__ channel and request access.

__3. Work in your own branch.__ A branch named `contributors/contributor_name` will be created for you; all your contributions go there.

__4. Build CARLA from source__, following [Linux](build_linux_ue5.md) or [Windows](build_windows_ue5.md). The content repository is cloned into `Unreal/CarlaUnreal/Content/Carla` as part of that build.

__5. Keep the branch in sync__ with `ue58-dev-carla`, and push it when the contribution is ready.

__6. Wait for the art team to review it__ before it is merged.

See the [asset tutorials](content_authoring_overview.md) for how to author each kind of asset.

---
## Docs contributions

Missing, vague or imprecise documentation can be reported as any other bug. It can also be fixed directly: the docs are Markdown files under `Docs/`, built with [MkDocs](https://www.mkdocs.org/).

__1. Install MkDocs.__ From the CARLA root:

```sh
pip3 install -r Docs/requirements.txt
```

__2. Preview the docs.__ Run the following and open the link it prints.

```sh
python3 -m mkdocs serve
```

__3. Create a branch__ from `ue58-dev`.

```sh
git checkout -b <contributor_name>/<branch_name>
```

__4. Write.__ Follow the [documentation standard](cont_doc_standard.md). Never edit `Docs/python_api.md` or `Docs/bp_library.md` directly; they are generated (see [Generated pages](cont_doc_standard.md#generated-pages)).

__5. Check the build.__ `python3 -m mkdocs build --strict` fails on a link to a missing page. A link to a missing anchor does not fail it; it only shows as an `INFO` line. Search the build output for `does not contain an anchor` to find those.

__6. Open a pull request__ against `ue58-dev` and add one of the suggested reviewers.
