# CarlaStudio Application Layer

`Apps/CarlaStudio` is the top-layer CARLA Studio application module.

It is intentionally separate from `Examples/QtClient`:

- `Examples/QtClient`: sample/reference Qt client workflows
- `Apps/CarlaStudio`: full application-layer packaging and orchestration module

The legacy CarlaStudio surface is now the primary `carla-studio` target.
The first-run and dashboard pages have been merged into that same application, so there is no separate shell target anymore.

Current application surface includes:

- The primary legacy CarlaStudio main application
- First-run setup and dashboard pages built directly into the main application
- A broader Qt feature surface that preserves the legacy simulation tooling workflow

For a full LibCarla-backed build, configure from the workspace root with `BUILD_CARLA_CLIENT=ON` and `BUILD_CARLA_STUDIO=ON` so the `carla-client` target is available to the app.

App-layer structure:

- `src/core`: app context and runtime mode
- `src/legacy`: primary CarlaStudio UI entry point (shipped `carla-studio` target)
- `resources`: per-platform branding assets (icons, desktop entries, resource scripts)

## UI surface

The CarlaStudio shell is organized around the following areas:

- Connection and launch controls
- Sensor preview and configuration
- World and actor inspection
- Replay and recording workflow
- Keyboard and joystick control surfaces
- Debug visualization controls
- Scenario and builder tools
- Settings and persisted remote/SSH configuration

## Design direction

The first-run and dashboard pages should stay visually consistent with the earlier CarlaStudio style:

- compact launch-first layout
- clear status and connectivity indicators
- tabbed workspace separation for advanced tools
- light-default controls with strong hierarchy and readable spacing
