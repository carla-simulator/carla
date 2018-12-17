<h2>Decorative Blueprints</h2>

This is a small collection of assets not primordial to building the world but that could be of decorative use:

<h4>DroppingAsset</h4>

This blueprint is composed of a mesh that when hit by any car will drop another mesh assigned to it the number of times required.

  - PropGenerator: The position relative to the root from whitch the spawning props will spawn
  - PropToDispense: The prop that will be dispensed once the vehicle hits the item.
  - PropWeight: Physical weight of the dispensed Items
  - EjectionMultiplyer: The force with Whitch the items will be ejected once spawned
  - MaximumItemsDispensed: Maximum number of items dispensed
  - MinimumItemsDispensed: Minimum number of items dispensed

<h4>Bp_Terrain</h4>

This blueprint simulates a far mountain region procedurally and very cheaply it's composed of an enormous cilinder with a material simulating a far mountain line.

  - GradientHeightZ: The lenght of the fog gradient.
  - GradientZ: The Z position of the fog effect.
  - Seed: Seed for the mountain line.
  - Random: Gives you a random seed.
  - Height: Vertical position of the mountains.
  - Amplitude: Vertical distance from peaks to valleys. 
  - Frequency: Horizontal Distance from peaks to valleys.
  - TextureUv: Scales the mountain texture.
  - CilinderScale: Scales the cilinder horizontally.
