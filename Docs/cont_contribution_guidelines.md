# Contributing to CARLA

The CARLA team is glad to accept contributions from anybody willing to collaborate. There are different ways to contribute to the project, depending on the capabilities of the contributor. Everyone taking part is expected to follow the [Code of conduct](cont_code_of_conduct.md).

*   [__Report bugs__](#report-bugs)
*   [__Request features__](#request-features)
*   [__Code contributions__](#code-contributions)
	*   [Branches](#branches)
	*   [Before getting started](#before-getting-started)
	*   [Coding standard](#coding-standard)
	*   [Submission](#submission)
	*   [Checklist](#checklist)
*   [__Art contributions__](#art-contributions)
*   [__Docs contributions__](#docs-contributions)

---
## Report bugs

Issues can be reported in the [issue section][issueslink] on GitHub. Before reporting a new bug, make sure to do some checkups.

__1. Check if the bug has been reported.__ Look it up in that same issue section on GitHub.

__2. Read the docs.__ Make sure that the issue is a bug, not a misunderstanding on how CARLA is supposed to work. Read the pages related to the issue in the [documentation][docslink].

__3. Say which version you are on.__ The branch and commit you built from, the Unreal Engine version, the platform and the Python version. A bug on the UE 4.26 line and the same bug on the UE 5.8 line are usually two different bugs.

[issueslink]: https://github.com/carla-simulator/carla/issues
[docslink]: https://carla-ue5.readthedocs.io

---
## Request features

Ideas for new features are also a great way to contribute. Any suggestion that could improve the users' experience can be submitted in the corresponding GitHub section [here][frlink].

[frlink]: https://github.com/carla-simulator/carla/issues?q=is%3Aissue+is%3Aopen+label%3A%22feature+request%22+sort%3Acomments-desc

---
## Code of conduct

Everyone taking part in the project is expected to follow the [code of conduct](cont_code_of_conduct.md).

---
## Code contributions

Before starting hands-on on coding, please check out the [issue board][issueboard] to see what the team is already working on, to avoid overlapping. In case of doubt, or to discuss how to proceed, contact the team on the [Discord server](https://discord.com/invite/8kqACuC).

To start working, [fork the CARLA repository](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/fork-a-repo) and clone your fork. Remember to [keep your fork in sync](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/syncing-a-fork) with the original repository.

[issueboard]: https://github.com/carla-simulator/carla/issues

### Branches

CARLA is developed on two parallel lines, and a contribution goes to the one it was written for.

| Branch | Engine | Content branch |
| --- | --- | --- |
| `ue58-dev` | Unreal Engine 5.8 — the main line | `ue58-dev-carla` |
| `ue4-dev` | Unreal Engine 4.26 — maintenance only | `ue4-dev` |

Branch from the one you target, and open the pull request against that same branch. A pull request opened against the wrong line will not build.

### Before getting started

A basic introduction to C++ programming with Unreal is in Unreal's [C++ programming tutorials][ue5tutorials]. Build CARLA from source first, following [Linux](build_linux_ue5.md) or [Windows](build_windows_ue5.md), and read [Foundations](foundations.md) to see which module a new feature belongs in.

[ue5tutorials]: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-cpp-programming-tutorials

### Coding standard

Follow the [coding standard](cont_coding_standard.md) when submitting new code.

### Submission

*   Develop the contribution in a branch named `username/name_of_the_contribution`, taken from `ue58-dev`.
*   Keep it rebased on the latest `ue58-dev`.
*   Open a pull request against `ue58-dev` and fill in the template: what changed, where it was tested, and the possible drawbacks.
*   Continuous integration builds the client, the Python API, the server and the LibCarla tests. A red mark means the build or the tests failed; fix it before asking for a review.

### Checklist

*   [ ] The branch is up-to-date with `ue58-dev` and tested with the latest changes.
*   [ ] The code compiles: `cmake --build Build/Release --target carla-client carla-python-api carla-server`.
*   [ ] The LibCarla tests pass: build `libcarla_test_client` and `libcarla_test_server` and run them from `Build/Release/LibCarla/`.
*   [ ] The smoke tests pass against a running server on port 3654: `cd PythonAPI/test && python3 -m unittest discover -s smoke`.
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

See the [asset tutorials](custom_assets_tutorials.md) for how to author each kind of asset.

---
## Docs contributions

Missing, vague or imprecise documentation can be reported as any other bug. It can also be fixed directly: the docs are Markdown files under `Docs/`, built with [MkDocs](https://www.mkdocs.org/).

__1. Install MkDocs.__

```sh
pip3 install mkdocs
```

__2. Preview the docs.__ From the CARLA root, run the following and open the link it prints.

```sh
python3 -m mkdocs serve
```

__3. Create a branch__ from `ue58-dev`.

```sh
git checkout -b <contributor_name>/<branch_name>
```

__4. Write.__ Follow the [documentation standard](cont_doc_standard.md). Never edit `Docs/python_api.md` or `Docs/bp_library.md` directly; they are generated.

__5. Check the build.__ `python3 -m mkdocs build --strict` fails on a broken internal link.

__6. Open a pull request__ against `ue58-dev` and add one of the suggested reviewers.
