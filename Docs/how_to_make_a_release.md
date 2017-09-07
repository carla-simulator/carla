How to make a release
=====================

Increase version
----------------

  * CarlaUE4/Plugins/Carla
    - Increase version in `Plugins/Carla/Carla.uplugin`.
    - Update `Plugins/Carla/CHANGELOG.md`.
    - Commit changes.
    - Add tag, `git tag -a X.X.X`, this opens a text editor, fill in the latest adds in CHANGELOG.md.
    - `git push && git push --tags`

  * CarlaUE4
    - Increase version in `Config/DefaultGame.ini` (try to keep same version as in plugin).
    - Update `CHANGELOG.md`.
    - Commit changes, including the latest commit to the Carla plugin.
    - Add tag, `git tag -a X.X.X`, this opens a text editor, fill in the latest adds in CHANGELOG.md.
    - `git push && git push --tags`

Build and package latest version
--------------------------------

  * Pull latest version of CarlaUE4 AND Plugins/Carla (probably should be on branch dev)
    - `git pull && (cd Plugins/Carla/ && git pull)`
  * Run `./Rebuild.sh`, this deletes all intermediate data and rebuilds CarlaServer and CarlaUE4 (including plugins). It will launch the editor when it's done.
  * For each level that is going to be packaged
    - Open the level, select the road generator
    - Check "Save Road Map to Disk"
    - Press "Trigger Road Map Generation"
  * On the top-left menu select "File > Package Project > Linux", select the folder in which Unreal will generate the build. This will take a while.
  * Go to `<path-you-select-for-packaging>/LinuxNoEditor`, copy there the following files
    - `Plugins/Carla/Docs/release_readme.md` --> `README.md`
    - `Plugins/Carla/Docs/Example.CarlaSettings.ini` --> `CarlaSettings.ini`
    - `Saved/<level-name>.png` --> `<level-name>.png`
    - `Saved/<level-name>.txt` --> `<level-name>.txt`
  * Generate the archive `CARLAUE4_X.X.X.tag.gz` with all the files in the folder (where X.X.X is the latest version number).
  * Copy it to
    - `//NASBACKUP/synthia-bck/CARLA/Releases/CARLAUE4_X.X.X.tag.gz`
    - `//NASBACKUP/synthia-www/CARLA/CARLAUE4_X.X.X.tag.gz`
  * Done :)

