CARLA Settings
==============

The CARLA plugin reads its settings from a "CarlaSettings.ini" file. This file
controls most aspects of the game, and it is loaded every time a new episode is
started (every time the level is loaded).

Settings are loaded following the next hierarchy, with values later in the
hierarchy overriding earlier values.

  1. `{ProjectFolder}/Config/CarlaSettings.ini`
  2. File provided by command-line argument `-carla-settings=<path-to-ini-file>`
  3. Other command-line arguments as `-world-port`, or `-carla-no-networking`
  4. Settings file sent by the client on every new episode.

Take a look at the [CARLA Settings example](Example.CarlaSettings.ini).

**Important:** If you are in editor, you most probably want to disable
networking. Otherwise the game will hang until a client connects. Copy
"Example.CarlaSettings.ini" to "CARLAUE4/Config/CarlaSettings.ini" and set
`UseNetworking=false`.
