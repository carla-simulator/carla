# House style for CARLA documentation

Shared by every category of `document-carla-feature`. Read it before writing.

## The sentence

A doc string says **what the thing is or does**, and stops.

- One or two sentences. Third person, present tense.
- A getter starts with "Returns".
- No reason, no history, no "this is useful when", no "note that".
- No caveats, failure modes or known issues. If a caveat genuinely has to be
  recorded, it goes in a `note:` or `warning:` key, never in `doc:`.
- No cross-reference unless the type is only usable through another call.

```yaml
# good
doc: >
  Returns the actor's transform.

# bad - explains itself, warns, and speculates
doc: >
  Returns the actor's transform. This is useful when you need to know where an
  actor is, for example to place a sensor. Note that in asynchronous mode the
  value may be one tick old, so you should call world.tick() first.
```

## Values and ranges

The only detail worth adding is **the range a value can take**, and what its
ends mean. Put units in `param_units` / `return_units` / `var_units`, not in
the sentence.

```yaml
# good
- def_name: ignore_lights_percentage
  params:
  - param_name: actor
    type: carla.Actor
  - param_name: percentage
    type: float
  doc: >
    Chance for a vehicle to ignore a traffic light, being 0 always detecting it
    and 100 a full ignore.
```

An enum-valued parameter lists its values the same way. A parameter whose
range is the obvious one for its type gets nothing.

## Names are API

Parameter names are **keyword arguments** — `actor.set_collisions(enabled=False)`
works because the binding declares `arg("enabled")`. A name invented for the
docs is a documented call that raises `TypeError`.

- Copy `param_name` from the binding's `arg("…")`, in order, spelling included.
- Copy `default:` from the binding's `arg("…") = value`, written Python-side
  (`True`, not `true`).
- `def_name` and `var_name` are the string the binding registers, not the C++
  method name.

## Types

`type:` is what a user writes or receives in Python: `int`, `float`, `str`,
`bool`, `list(int)`, `dict`, or a qualified CARLA class (`carla.Vector3D`).
Not the C++ type.
