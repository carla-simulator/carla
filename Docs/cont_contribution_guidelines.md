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

### Submission

Contributions and new features are merged to the development branch, `ue58-dev`. Its content is in the `ue58-dev-carla` branch of the content repository.

*   Always keep your branch updated with the latest changes in `ue58-dev`.
*   Develop the contribution in a child branch from `ue58-dev` named `username/name_of_the_contribution`.
*   Once the contribution is ready, submit a pull request from your branch to `ue58-dev`. Try to be as descriptive as possible when filling in the template: what changed, where it was tested, and the possible drawbacks.
*   Continuous integration builds the client, the Python API, the server and the package on Ubuntu, and builds and runs the LibCarla tests ([What CI runs](dev_testing.md#what-ci-runs)). It does not run the Python tests, so run them locally. A green tick mark appears if the checks pass; if there is a red mark, correct the code accordingly.

### Checklist

Before a pull request to `ue58-dev`:

*   [ ] The branch is up-to-date with `ue58-dev` and tested with the latest changes.
*   [ ] The code compiles: `cmake --build Build --target carla-client carla-python-api carla-server`.
*   [ ] The LibCarla unit tests pass ([LibCarla unit tests](dev_testing.md#libcarla-unit-tests)).
*   [ ] The smoke tests pass against a running server on port 3654 ([Smoke tests](dev_testing.md#smoke-tests)). On Linux: `cd PythonAPI/test && python3 -m nose2 -v $(cat smoke_test_list.txt)`.
*   [ ] `CHANGELOG.md` has an entry under `## LATEST Changes` for any user-visible change.
*   [ ] The documentation was extended, if necessary.

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

__4. Write.__ Follow the [documentation standard](cont_doc_standard.md). Never edit `Docs/python_api.md` or `Docs/bp_library.md` directly; they are generated.

__5. Check the build.__ `python3 -m mkdocs build --strict` fails on a link to a missing page. A link to a missing anchor does not fail it; it only shows as an `INFO` line. Search the build output for `does not contain an anchor` to find those.

__6. Open a pull request__ against `ue58-dev` and add one of the suggested reviewers.
