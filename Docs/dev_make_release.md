# How to make a release

> _This document is meant for developers that want to publish a new release._

1. **Make sure content is up-to-date.**<br>See [Updating the content](dev_update_content.md). CI and a fresh setup clone the tip of the `ue58-dev-carla` branch of the content repository, so every content change for the release has to be [merged](dev_update_content.md#merging-a-change) there.

2. **Increase CARLA version where necessary.**<br>Increase version in the following files: _CMakeLists.txt_ (`CARLA_VERSION_MAJOR`, `CARLA_VERSION_MINOR` and `CARLA_VERSION_PATCH`), _DefaultGame.ini_, _Carla.uplugin_ and _Util/Tools/Deploy.sh_ (`PACKAGE_NAME`). Grep for the current version to make sure you don't miss any references.

3. **Clean up CHANGELOG.md.**<br>Make sure the change log is up-to-date, reword and reorganize if necessary; take into account which items can be more important to the users. Rename the `## LATEST Changes` heading to `## CARLA X.X.X`.

4. **Commit changes and add a new tag.**<br>Once all your changes are committed, add a new tag with `git tag -a X.X.X` (replacing `X.X.X` by latest version). Add the changelog of this version as tag message.

5. **Build the package.**<br>Build the package as described in Build a package for [Linux](build_linux_ue5.md#build-a-package-with-carla-ue5) or [Windows](build_windows_ue5.md#build-a-package-with-carla-ue5), after pulling the content checkout in _Unreal/CarlaUnreal/Content/Carla_ to the tip of `ue58-dev-carla`, since setup does not update an existing checkout. Leave `CARLA_BASE_RELEASE` empty when building the package so the release name follows the new version.

6. **Tag content repo.**<br>Add a similar tag to the content repository at the commit the package was built from, recorded as `Content git hash` in the package's _VERSION_ file. See [Releases](dev_update_content.md#releases).

7. **Push changes.**<br>Push all the changes to both repositories, to push tags you may need to use `git push --tags`. Create a Pull Request if necessary.

8. **Publish the packages.**<br>Publish the package of the new version together with the `carla-X.X.X-<platform>-release-metadata.tar.gz` file written next to it in the _Package_ folder of the build directory (_Build/Package_ for the default build); [content packs](content_authoring_packs.md) for this release are cooked against it. __(TODO: document who builds the release packages and where they are uploaded; CI only uploads nightly builds of `ue58-dev`.)__

9. **Edit GitHub release.**<br>Go to [GitHub releases](https://github.com/carla-simulator/carla/releases) and create a new release on top of the newly created tag. Once the packages are published, add the download links to the newly created release.
