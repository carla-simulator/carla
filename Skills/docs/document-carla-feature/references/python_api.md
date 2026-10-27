# Format: Python API

The Python API reference, `Docs/python_api.md`, is generated from `PythonAPI/docs/*.yml` by `PythonAPI/docs/doc_gen.py`. **Never edit `Docs/python_api.md`.** Edit the YAML, then regenerate.

## 1. Find what to document

The target names the classes or members: a new binding, a renamed one, one that was removed. The bindings in `PythonAPI/carla/src/*.cpp` decide what exists; the YAML and the generated page only say what was documented.

The [`audit-carla-docs`](../../audit-carla-docs/SKILL.md) report lists both directions at once: bindings missing from the reference (`binding`) and documented API the bindings no longer have (`stale`). Do the stale ones first: a wrong name in the reference is a call that raises, which is worse than a missing entry.

## 2. What not to document

Some registered names exist only to make Boost.Python work and must never reach the reference:

- **Container registrations.** A `class_<std::vector<...>>` exists so Boost.Python has a type for the container; every API converts to a list before returning. Do not decide this from the name: `yml_target.py` prints a `wraps:` line, and that is the test (`BoneTransformDataIn` wraps a real type despite having shipped as `bone_transform`).
- **Output twins**: anything ending in `_out` (`bone_transform_out`), the by-reference variant of a type that is already documented.
- **Hierarchy-only base classes**, registered with `no_init` and no members of their own, there so `bases<>` resolves: `ClientSideSensor`, `ServerSideSensor`.
- **Deprecated aliases** kept for compatibility (`old_location`, `frame_number`). The replacement is documented; the alias is not.

When a name is genuinely ambiguous, leave it and say so in the report rather than guessing.

## 3. Locate the target

```bash
python3 scripts/yml_target.py --root "${CARLA_ROOT:?}" Actor.apply_texture
python3 scripts/yml_target.py --root "${CARLA_ROOT:?}" --from-audit Docs/audit_carla_docs.md
```

For each target it prints the binding site, the **exact** `arg()` names and defaults, whether the member belongs in `methods:` or `instance_variables:`, and the two entries the new one sorts between. Insert alphabetically; dunders stay last, where the YAML files already keep them.

Then open the binding site and read the C++. The locator cannot tell you what a function returns, and `CALL_RETURNING_LIST(cc::World, GetEgoSpawnPoints)` declares no `arg()` at all: the signature is in `LibCarla`.

## 4. Write the entry

The sentence follows [style.md](style.md#reference-entries). The shape:

```yaml
    - def_name: apply_texture
      params:
      - param_name: material_parameter
        type: carla.MaterialParameter
      - param_name: texture
        type: carla.TextureColor
      doc: >
        Applies a texture to the actor's material parameter.
    # --------------------------------------
```

- Keep the `# --------------------------------------` separator between methods; the YAML files use it throughout.
- `return:` carries the type for anything that returns a value, with `return_units:` when it has units.
- An `instance_variables` entry is `var_name` / `type` / `doc`, plus `var_units` where it applies.
- A whole new class needs `class_name`, `doc`, then its sections. Give it the `parent:` key if the binding declares `bases<>`.
- A function goes under `methods:`, an attribute under `instance_variables:`, and an attribute on an enum class is one of its values.

## 5. Renamed or removed

A documented name the bindings no longer register is **usually renamed, not gone**. Look for the same member under a corrected spelling: `axel_type` -> `axle_type`, `intertia_tensor_scale` -> `inertia_tensor_scale`, `port` -> `tm_port`, `None` -> `NONE`, `CAMData` -> `CAMMessage`. For dunders, grep the binding for the class and read the names it registers: `__get_item__` was documented where the code exposes `__getitem__`.

- **Renamed**: change the `def_name` / `var_name` / `class_name` in place and keep the doc text.
- **Removed**: delete the entry. No deprecation note, no "removed in" marker: the reference describes what exists now.
- **Replaced by several**: `CityObjectLabel.Vehicles` is gone and `Car`, `Bus`, `Truck`, `Motorcycle`, `Bicycle`, `Rider` and `Train` took its place: one delete and seven additions.

Then grep the rest of `Docs/` for the old name; those pages are part of the target.

## 6. Verify

```bash
python3 "${CARLA_ROOT:?}/PythonAPI/docs/doc_gen.py"
git -C "${CARLA_ROOT:?}" diff --stat Docs/python_api.md
```

`doc_gen.py` prints the offending file and key for a malformed entry and leaves `Docs/python_api.md` untouched, so a silent run plus a non-empty diff is the check that the YAML parsed. When the target came from the audit, re-run it with `--signal binding --signal stale`: the count must drop by exactly the number of entries written, and anything else means a name does not match the binding.
