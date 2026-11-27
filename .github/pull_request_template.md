<!--

Thanks for sending a pull request! Please make sure you click the link above to
view the contribution guidelines, then fill out the blanks below.
Please, make sure if your contribution is for UE4 version of CARLA you merge against ue4-dev branch. 
if it is for UE5 version of CARLA you merge against ue58-dev branch

Checklist:

  - [ ] Your branch is up-to-date with the `ue4-dev/ue5-dev` branch and tested with latest changes
  - [ ] Extended the README / documentation, if necessary
  - [ ] Code compiles correctly
  - [ ] UE4: all tests passing with `make check` (only Linux)
  - [ ] UE5 (ue58-dev): the Unreal Editor opens on this branch's build and loads the default map without errors, and the LibCarla, Python API unit and smoke tests pass locally (see "Running the tests" in the docs). CI does not run them. Python-only changes outside the build run the changed scripts instead (see the contribution guidelines)
  - [ ] New or modified tests add real value: they assert behaviour and would fail on a regression (no empty tests that only add compute), and were run locally, not only edited
  - [ ] Any new or changed CI workflow was discussed and agreed with the core maintainers beforehand
  - [ ] If relevant, update CHANGELOG.md with your changes

-->

#### Description

<!-- Please explain the changes you made here as detailed as possible. -->

Fixes #  <!-- If fixes an issue, please add here the issue number. -->

#### Where has this been tested?

  * **Platform(s):** ...
  * **Python version(s):** ...
  * **Unreal Engine version(s):** ...
  * **Local checks (commit):** ... <!-- ue58-dev: the Unreal Editor opened and loaded the map; result of the LibCarla, Python unit and smoke tests. "Docs-only" if no code changed; "Python-only" with the server build commit and the scripts run if only skills scripts, examples, util or agents changed. -->

#### Tests

<!-- Which tests did you add or modify, why, and what regression would each one catch? Write "None" if not applicable. -->

#### CI workflows

<!-- If this PR adds or changes anything under .github/workflows/, link the discussion where the core maintainers agreed to it. Otherwise write "None". -->

#### Possible Drawbacks

<!-- What are the possible side-effects or negative impacts of the code change? -->
