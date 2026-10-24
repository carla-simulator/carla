# CARLA Map Maker — End-to-End (`e2e_tests`) Verification Suite

This directory contains self-contained, reproducible end-to-end (E2E) verification tests and input fixtures for every milestone in the **Editor-Free Map Creation & Packaging** plan ([`.idea/editor_free_map_creation_plan.md`](../.idea/editor_free_map_creation_plan.md) & [`Docs/map-maker/README.md`](../Docs/map-maker/README.md)).

Every milestone adds its test suite and input artifacts here, and `run_all.py` re-runs all previous milestone suites to ensure zero regressions as new features are added.

---

## Directory Layout

```text
e2e_tests/
├── README.md                              # E2E suite documentation
├── run_all.py                             # Master regression runner across all milestones
├── m1_osm2odr/                            # Milestone 1: OSM2ODR Build & Python Wheel
│   ├── fixtures/
│   │   └── sample_network.osm             # Multi-road + signalized junction OSM fixture
│   └── test_m1_osm2odr.py                 # Verifies DoD 1.1 through DoD 1.5
├── m2_custom_mesh/                        # Milestone 2: Custom Mesh, Material & Semantic Engine
│   ├── fixtures/
│   │   └── simple_road.xodr               # Straight OpenDRIVE road fixture for hybrid mode
│   └── test_m2_custom_mesh.py             # Verifies DoD 2.1 through DoD 2.5
├── m3_carlamap_loader/                    # Milestone 3: Server-Side .carlamap Bundle Discovery & Loader
│   ├── fixtures/
│   │   ├── sample_bundle/                 # Sample .carlamap directory with .cmesh, .xodr, .bin
│   │   └── sample_bundle.carlamap.tar.gz  # Compressed archive for live remote bundle load
│   └── test_m3_carlamap_loader.py         # Verifies DoD 3.1 through DoD 3.7
├── m4_carla_map_cli/                      # Milestone 4: Offline carla-map CLI & OSM Buildings
│   ├── fixtures/
│   │   ├── sample_city.osm                # OSM network with buildings & traffic signals
│   │   ├── custom_town.obj                # Multi-submesh 3D model (Roads, Sidewalks, Buildings)
│   │   └── custom_town.xodr               # OpenDRIVE driving road fixture
│   └── test_m4_carla_map_cli.py           # Verifies DoD 4.1 through DoD 4.7
├── m5_packaged_e2e/                      # Milestone 5: Shipped Package Integration & Scenarios
│   ├── fixtures/
│   │   ├── sample_city.osm                # OSM network for Scenario A test
│   │   ├── custom_town.obj                # Custom 3D meshes for Scenario B test
│   │   └── custom_town.xodr               # OpenDRIVE network for Scenario B test
│   └── test_m5_packaged_e2e.py            # Verifies DoD 5.1 through DoD 5.5
├── m6_terrain/                           # Milestone 6: Multi-Category Urban Landuse & Ground Sub-Base
│   └── test_m6_terrain.py                 # Verifies DoD 6.1 through DoD 6.6
├── m7_clutter_parking/                   # Milestone 7: Roadside Clutter, Vegetation & Parked Cars
│   └── test_m7_clutter_parking.py         # Verifies DoD 7.1 through DoD 7.7
├── m8_lighting/                          # Milestone 8: Authentic Street Lighting & Nocturnal Radiance
│   └── test_m8_lighting.py                # Verifies DoD 8.1 through DoD 8.7
├── m9_traffic_lights/                    # Milestone 9: Dynamic Traffic Lights & OpenDRIVE Signals
│   └── test_m9_traffic_lights.py          # Verifies DoD 9.1 through DoD 9.8
├── m10_photorealism/                     # Milestone 10: Photorealistic Architectural & Material Suite
│   └── test_m10_photorealism.py           # Verifies DoD 10.1 through DoD 10.9
├── m11_crosswalks/                       # Milestone 11: Pedestrian Crosswalks, Zebra Markings & Tactile Paving
│   └── test_m11_crosswalks.py             # Verifies DoD 11.1 through DoD 11.7
├── m12_satellite/                        # Milestone 12: High-Resolution Satellite & Aerial Ground Infill
│   └── test_m12_satellite.py              # Verifies DoD 12.1 through DoD 12.7
├── m13_cycleways/                        # Milestone 13: Protected Cycleways, Green Lanes & Bike Infrastructure
│   └── test_m13_cycleways.py              # Verifies DoD 13.1 through DoD 13.8
└── m14_barriers/                         # Milestone 14: Continuous Barriers, Perimeter Walls, Fences & Guard Rails
    ├── fixtures/
    │   └── sample_barriers.osm            # Test OSM network with connected barriers
    └── test_m14_barriers.py               # Verifies DoD 14.1 through DoD 14.8
```

---

## Running the E2E Tests

### 1. Run All Milestone E2E Tests (Full Regression Suite)

If a CARLA server is already listening on `127.0.0.1:2000`, `run_all.py` connects to it. Otherwise, it automatically starts an offscreen CARLA server (`UnrealEditor ... OpenDriveMap -game -RenderOffScreen -notrace -nosound`), runs all milestone test suites, and stops the server cleanly when finished:

```bash
python3.10 e2e_tests/run_all.py
```

### 2. Run Offline Tests Only (Fast Build/Linkage/Conversion Check)

To skip live simulator tests and run only offline build, static linkage, and API binding tests:

```bash
python3.10 e2e_tests/run_all.py --offline-only
```

### 3. Run a Specific Milestone Suite

```bash
# Via master runner filter:
python3.10 e2e_tests/run_all.py --milestone m1
python3.10 e2e_tests/run_all.py --milestone m2
python3.10 e2e_tests/run_all.py --milestone m3
python3.10 e2e_tests/run_all.py --milestone m4

# Or directly:
python3.10 e2e_tests/m1_osm2odr/test_m1_osm2odr.py
python3.10 e2e_tests/m2_custom_mesh/test_m2_custom_mesh.py
python3.10 e2e_tests/m3_carlamap_loader/test_m3_carlamap_loader.py
python3.10 e2e_tests/m4_carla_map_cli/test_m4_carla_map_cli.py
```

