# Category: python-api

Closes the audit's `binding` findings (exposed to Python, not in the
reference) and `stale` findings (documented, not exposed any more).

**Never edit `Docs/python_api.md`.** It is generated from
`PythonAPI/docs/*.yml` by `PythonAPI/docs/doc_gen.py`. Edit the yml, then
regenerate.

## 1. Read the findings

```bash
sed -n '/^## Python bindings absent/,/^## Sensors absent/p' "${CARLA_ROOT:?}/Docs/audit_carla_docs.md"
```

Both sections are grouped by owning class, so **one `###` heading is one yml
block to edit**. Take a heading at a time and the file never has to be reopened.

| group | meaning | work |
|---|---|---|
| `### Classes absent from the reference` | no yml entry at all | add the class, then its members |
| `### Actor — \`…/Actor.cpp\`` | the class is documented, these members are not | add one entry each, into that one class |

Each line ends with what it is — `(new class)`, `(new function)`,
`(new attribute)` — which decides where it goes: a function belongs under
`methods:`, an attribute under `instance_variables:`, and an attribute on an
enum class is one of its values.

Do the `stale` section first — a wrong name in the reference is a call that
raises, which is worse than a missing entry. See step 6.

## 2. Triage: what not to document

The audit reports every name the bindings register. Some exist only to make
Boost.Python work and must never reach the reference:

- **Container registrations** are already dropped by the audit — a
  `class_<std::vector<...>>` exists so Boost.Python has a type for the
  container, and every API converts to a list before returning. You will not
  see `VectorOfWheels` in the findings. **Do not re-derive this from names**:
  the locator's `wraps:` line is the test, and `BoneTransformDataIn` wraps a
  real type despite having shipped as `bone_transform`.
- **Output twins** — anything ending in `_out` (`bone_transform_out`), the
  by-reference variant of a type that is already documented.
- **Hierarchy-only base classes** — registered with `no_init` and no members
  of their own, there so `bases<>` resolves: `ClientSideSensor`,
  `ServerSideSensor`.
- **Deprecated aliases** kept for compatibility (`old_location`,
  `frame_number`). The replacement is documented; the alias is not.

Everything else is a real entry. When a name is genuinely ambiguous, leave it
and say so in the report rather than guessing.

## 3. Locate the target

```bash
python3 scripts/yml_target.py --root "${CARLA_ROOT:?}" Actor.apply_texture
python3 scripts/yml_target.py --root "${CARLA_ROOT:?}" --from-audit Docs/audit_carla_docs.md
```

For each target it prints the binding site, the **exact** `arg()` names and
defaults, whether the member belongs in `methods:` or `instance_variables:`,
and the two entries the new one sorts between. Insert alphabetically; dunders
stay last, where the ymls already keep them.

Then open the binding site and read the C++. The locator cannot tell you what
a function returns, and `CALL_RETURNING_LIST(cc::World, GetEgoSpawnPoints)`
declares no `arg()` at all — the signature is in `LibCarla`.

## 4. Write the entry

Follow [style.md](style.md) for the sentence. The shape:

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

- Keep the `# --------------------------------------` separator between
  methods; the ymls use it throughout.
- `return:` carries the type for anything that returns a value, with
  `return_units:` when it has units.
- An `instance_variables` entry is `var_name` / `type` / `doc`, plus
  `var_units` where it applies.
- A whole new class needs `class_name`, `doc`, then its sections. Give it the
  `parent:` key if the binding declares `bases<>`.

## 5. Remove what no longer exists

The `stale` section lists documented API the bindings no longer have. **Most of
it is renamed, not gone** — in the current report 9 of 10 are renames. So for
each entry, look for the same member under a corrected spelling:

1. Search the `binding` section for it: `axel_type` -> `axle_type`,
   `intertia_tensor_scale` -> `inertia_tensor_scale`, `port` -> `tm_port`,
   `None` -> `NONE`, `CAMData` -> `CAMMessage`.
2. Dunders never appear there, since the audit exempts them. Grep the binding
   for the class and read the names it registers: `__get_item__` is documented
   where the code exposes `__getitem__`.
3. Found one? It is a **rename**: change the `def_name`/`var_name`/`class_name`
   in place and keep the doc text. Both findings close at once.
4. Found nothing? It is a **removal**: delete the entry from the yml and stop
   there. No deprecation note, no "removed in" marker, no tombstone — the
   reference describes what exists now.

One caveat on renames: a member can be replaced by several. `CityObjectLabel.Vehicles`
is gone and `Car`, `Bus`, `Truck`, `Motorcycle`, `Bicycle`, `Rider` and `Train`
took its place, so that one is a delete plus seven additions, not a rename.

## 6. Verify

```bash
python3 "${CARLA_ROOT:?}/PythonAPI/docs/doc_gen.py"
git -C "${CARLA_ROOT:?}" diff --stat Docs/python_api.md
python3 ../audit-carla-docs/scripts/audit_docs.py --root "${CARLA_ROOT:?}" --signal binding --signal stale
```

`doc_gen.py` prints the offending file and key for a malformed entry and
leaves `Docs/python_api.md` untouched, so a silent run plus a non-empty diff
is the check that the yml parsed. The audit count must drop by exactly the
number of entries written; anything else means a name does not match the
binding.
