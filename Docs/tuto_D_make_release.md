# How to make a release

> _This document is meant for developers that want to publish a new release._

1. **Make sure content is up-to-date.**<br>
   See [Upgrade the content](tuto_D_contribute_assets.md).

2. **Increase CARLA version where necessary.**<br>
   Increase version in the following files: _DefaultGame.ini_, _Carla.uplugin_,
   _setup.py_, _ContentVersions.txt_. Grep for the current version to make sure
   you don't miss any references.

3. **Clean up CHANGELOG.md.**<br>
   Make sure the change log is up-to-date, reword and reorganize if necessary;
   take into account which items can be more important to the users.

4. **Commit changes and add a new tag.**<br>
   Once all your changes are committed, add a new tag with `git tag -a X.X.X`
   (replacing `X.X.X` by latest version). Add the changelog of this version as
   tag message.

5. **Tag content repo.**<br>
   Add a similar tag to the content repository at the exact commit as in
   _ContentVersions.txt_.

6. **Push changes.**<br>
   Push all the changes to both repositories, to push tags you may need to use
   `git push --tags`. Create a Pull Request if necessary.

7. **Edit GitHub release.**<br>
   Go to [GitHub releases](https://github.com/carla-simulator/carla/releases)
   and create a new release on top of the newly created tag. Wait until Jenkins
   has finished publishing the builds with the latest version and add the
   download links to the newly created release.
