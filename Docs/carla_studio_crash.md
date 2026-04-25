# CARLA Studio — Crash protection

Studio installs a `std::set_terminate` handler at `main()`'s very
first line so unhandled exceptions thrown from background threads
produce a diagnosable stderr line instead of a silent SIGABRT.

## Why it exists

The most common path the handler guards is a LibCarla **RPC version
mismatch**. When the sim is built from a different commit than the
bundled `libcarla-client`, the client deserialises corrupt size
fields off the wire and throws `std::bad_array_new_length` from a
worker thread. That thread bypasses the outer `try/catch` in
`main()`, so without `set_terminate` the process would just abort
with no context.

| Layer | Behaviour without handler | Behaviour with handler |
|---|---|---|
| Worker thread throw | `std::terminate()` → `abort()` → core dump | `std::terminate()` → handler logs + `_Exit(1)` |
| User-visible signal | SIGABRT, no message | One stderr line + clean exit code |
| Core dump | Yes (large, often unhelpful) | No |

## Why `_Exit(1)` over `abort()`

`abort()` raises SIGABRT, which:

| Concern | `abort()` | `_Exit(1)` |
|---|---|---|
| Triggers core dump | Yes | No |
| Runs `atexit` handlers | No | No |
| Exit code | 134 (signal) | 1 |
| Re-enters faulty libs | Possible (signal handlers) | No |

`_Exit(1)` is a clean, immediate exit that skips destructors of
already-corrupt state — which is the correct posture once
`std::terminate()` has fired. The user gets a deterministic exit
code, no dump dialog from the OS, and the stderr line tells them
exactly what to do next.

## What you'll see

The handler emits a single multi-line block to stderr before the
process exits:

```
CARLA Studio: std::terminate fired
  type:  std::bad_array_new_length
  what:  std::bad_array_new_length
  hint:  LibCarla RPC version mismatch — rebuild libcarla-client
         against the running simulator, or run Health Check →
         "SDK ↔ Sim version" to confirm compatibility.
```

The format is fixed: `type:` (demangled exception type), `what:`
(the exception's `what()` payload), then a short remediation hint
chosen by matching `type` against a known-causes table.

## Pre-flight: Health Check surfaces the same condition

The crash handler is the last line of defence — the **Health Check**
tab catches the same condition before the user clicks anything that
would trip it. The relevant row:

| Row | Probes |
|---|---|
| SDK ↔ Sim version | Reads the simulator's reported version over RPC, compares against the linked `libcarla-client` build tag, flags a red mismatch row when they diverge |

If that row is green, the `std::bad_array_new_length` path
effectively cannot fire from RPC deserialisation. If it's red, the
launcher refuses to START until it's resolved (Cfg → Install /
Update CARLA, or rebuild the client against the sim).

## Other paths the handler covers

The same handler catches anything else that escapes a worker:

| Cause | Typical `type:` |
|---|---|
| RPC version mismatch | `std::bad_array_new_length` |
| Broken pipe from sim | `std::system_error` |
| Plugin throw | Plugin-specific |
| Out-of-memory in client thread | `std::bad_alloc` |

Each maps to its own remediation hint in the same `type/what/hint`
format shown above.

![Crash stderr line](img/cs_crash_stderr.png)
