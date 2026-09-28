# Editor-side content repair for World Partition maps whose external actors reference
# MaterialInstanceDynamic objects that live outside their own package (typically outered
# to the persistent UWorld, e.g. created with KismetMaterialLibrary.CreateDynamicMaterialInstance
# with the editor world as world-context).  Such MIDs are never saved (nothing inside the map
# package references them), so every external actor that points at one carries a dangling
# import and the cook fails with "Failed import for MaterialInstanceDynamic".
#
# Run headless:
#   UnrealEditor-Cmd CarlaUnreal.uproject -run=pythonscript -script=<this file> -unattended -nosplash -nopause -NullRHI -stdout
# Configuration through environment variables (the pythonscript commandlet has no argv):
#   FIXA_MAP         long package name of the WP map            (default /Game/Carla/Maps/Town15/Town15)
#   FIXA_CANDIDATES  tsv produced by uasset_imports.py (rows with kind 'import' are the packages whose
#                    on-disk import table references a foreign MID).  Optional: without it every actor
#                    whose ActorDesc class is in FIXA_CLASSES is inspected instead.
#   FIXA_CLASSES     comma-separated native class names to walk when no candidate list is given (default DecalActor)
#   FIXA_MESHES      comma-separated static-mesh package paths whose NavCollision.bIsDynamicObstacle is cleared
#                    (SplineMeshComponent::UpdateBounds ensures on dynamic-obstacle nav collision)
#   FIXA_BATCH       actors loaded per batch (default 500)
#   FIXA_DRYRUN      "1": inspect and log only, save nothing
#   FIXA_LOG         result log path
#
# The script has three modes, selected with FIXA_MODE:
#
#   FIXA_MODE=mid-refs          (default) the repair described above.
#
#   FIXA_MODE=dump              read-only.  Dumps, for every selected actor, the properties needed to
#                               decide what material it should carry: package, label, class, current
#                               decal material, DecalComponent decal_size / sort_order / fade_screen_size,
#                               and the actor's world location / rotation / scale.
#                               FIXA_LABELS     comma-separated actor-label prefixes to select (e.g. DecalTemplate_Inst)
#                               FIXA_PACKAGES   file with one long package name per line, added to the selection
#                               FIXA_DUMP_TSV   output tsv (header row + one row per actor)
#
#   FIXA_MODE=assign-materials  assigns a MaterialInstanceConstant (or any material asset) to the
#                               DecalComponent of each listed actor and saves the external packages.
#                               FIXA_ASSIGN_TSV  tsv "<long package name>\t<material object path>"; '#' comments
#                                                and blank lines ignored.  A material path of "None" clears the slot.
#                               Only slots that actually differ are touched; every change is logged.
#                               FIXA_FORCE_VISIBLE=1 additionally forces DecalComponent.bVisible=true.
#                                   The WD_SpawnActors "Add Decals" flow left all 735 of its Town15
#                                   decals with bVisible=false, so they would stay invisible in game
#                                   even with a valid material (every other Town15 decal is visible).
#                               Use FIXA_DRYRUN=1 to see the plan without writing.
import unreal, os, sys, time, traceback

OUT = os.environ.get("FIXA_LOG", "/home/german/Projects/CARLA_SOURCE/.omc/logs/fixA-repair.result")
MAP = os.environ.get("FIXA_MAP", "/Game/Carla/Maps/Town15/Town15")
CAND = os.environ.get("FIXA_CANDIDATES", "")
CLASSES = [c for c in os.environ.get("FIXA_CLASSES", "DecalActor").split(",") if c]
MESHES = [m for m in os.environ.get("FIXA_MESHES", "").split(",") if m]
BATCH = int(os.environ.get("FIXA_BATCH", "500"))
DRY = os.environ.get("FIXA_DRYRUN", "0") == "1"
MODE = os.environ.get("FIXA_MODE", "mid-refs")
LABELS = [l for l in os.environ.get("FIXA_LABELS", "").split(",") if l]
PACKAGES = os.environ.get("FIXA_PACKAGES", "")
DUMP_TSV = os.environ.get("FIXA_DUMP_TSV", "")
ASSIGN_TSV = os.environ.get("FIXA_ASSIGN_TSV", "")
FORCE_VISIBLE = os.environ.get("FIXA_FORCE_VISIBLE", "0") == "1"

lines = []
def log(msg):
    msg = str(msg)
    lines.append(msg)
    unreal.log_warning("[fixA] " + msg)
    with open(OUT, "w") as fh:
        fh.write("\n".join(lines) + "\n")

stats = dict(candidates=0, descs_matched=0, loaded=0, actors_seen=0, components_seen=0,
             slots_dangling_cleared=0, slots_foreign_mid_replaced=0, slots_local_mid_kept=0,
             packages_saved=0, packages_failed=0, meshes_fixed=0,
             rows_dumped=0, slots_assigned=0, slots_already_correct=0, materials_missing=0,
             visibility_fixed=0)

def file_to_package(path):
    # .../Content/<Bundle>/__ExternalActors__/<rest>.uasset  ->  /Game/__ExternalActors__/<rest>
    # (the Carla module mounts /Game/__ExternalActors__/<X>/ onto Content/<Bundle>/__ExternalActors__/<X>/)
    for folder in ("__ExternalActors__", "__ExternalObjects__"):
        k = path.find("/" + folder + "/")
        if k >= 0:
            rest = path[k + len(folder) + 2:]
            if rest.endswith(".uasset"):
                rest = rest[:-7]
            return "/Game/" + folder + "/" + rest
    return None

def read_candidates(tsv):
    cands = {}
    with open(tsv) as fh:
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            if len(parts) < 5 or parts[4] != "import":
                continue
            pkg = file_to_package(parts[0])
            if pkg:
                cands.setdefault(pkg, []).append((parts[2], parts[3]))   # (MID object name, outer chain)
    return cands

def obj_pkg_name(o):
    try:
        return o.get_package().get_name()
    except Exception:
        return o.get_outermost().get_name()

def describe(o):
    if o is None:
        return "None"
    try:
        return "%s '%s' (outer %s, package %s)" % (o.get_class().get_name(), o.get_name(),
                                                     o.get_outer().get_path_name() if o.get_outer() else "None",
                                                     obj_pkg_name(o))
    except Exception:
        return str(o)

def fix_slot(actor, comp, slot_name, mat, actor_pkg_name, dangling, setter):
    """Returns True when the slot was changed."""
    if mat is None:
        if dangling:
            # The on-disk import of the MID failed at load time, leaving the slot null; re-saving the package
            # drops the dangling import.  Set explicitly so the property is recorded as modified.
            log("  %s.%s[%s]: dangling MID import %s -> None (re-save)" % (actor.get_actor_label(), comp.get_name(), slot_name, dangling))
            if not DRY:
                comp.modify()
                setter(None)
            stats["slots_dangling_cleared"] += 1
            return True
        return False
    if isinstance(mat, unreal.MaterialInstanceDynamic):
        if obj_pkg_name(mat) != actor_pkg_name:
            parent = mat.get_editor_property("parent")
            log("  %s.%s[%s]: foreign MID %s -> parent %s" % (actor.get_actor_label(), comp.get_name(), slot_name,
                                                              describe(mat), parent.get_path_name() if parent else "None"))
            if not DRY:
                comp.modify()
                setter(parent)
            stats["slots_foreign_mid_replaced"] += 1
            return True
        stats["slots_local_mid_kept"] += 1
    return False

def fix_actor(actor, dangling_list):
    actor_pkg = actor.get_package()
    actor_pkg_name = actor_pkg.get_name()
    dangling = ", ".join(n for n, _ in dangling_list) if dangling_list else ""
    changed = False
    stats["actors_seen"] += 1
    for comp in actor.get_components_by_class(unreal.ActorComponent):
        stats["components_seen"] += 1
        if isinstance(comp, unreal.DecalComponent):
            mat = comp.get_editor_property("decal_material")
            if fix_slot(actor, comp, "DecalMaterial", mat, actor_pkg_name, dangling, lambda m: comp.set_decal_material(m)):
                changed = True
        elif isinstance(comp, unreal.MeshComponent):
            n = comp.get_num_materials()
            for i in range(n):
                mat = comp.get_material(i)
                # a None slot on a mesh component is the mesh default (not an override), so it is never
                # treated as dangling here; dangling imports on mesh components are handled below
                if fix_slot(actor, comp, "Material[%d]" % i, mat, actor_pkg_name, "",
                            (lambda idx: (lambda m: comp.set_material(idx, m)))(i)):
                    changed = True
            if dangling and not changed:
                # A dangling import on a mesh component leaves a null entry in OverrideMaterials; touching the
                # component is enough for the re-save to drop the import.
                log("  %s.%s: dangling MID import %s on a mesh component; re-saving" % (actor.get_actor_label(), comp.get_name(), dangling))
                if not DRY:
                    comp.modify()
                stats["slots_dangling_cleared"] += 1
                changed = True
    if changed and not DRY:
        actor.modify()   # UObject::Modify marks the owning (external) package dirty
    return changed, actor_pkg

def mode_mid_refs():
    t0 = time.time()
    log("map=%s dry=%s batch=%d candidates=%s classes=%s meshes=%s" % (MAP, DRY, BATCH, CAND or "-", CLASSES, MESHES))
    LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    WPBL = unreal.WorldPartitionBlueprintLibrary

    ok = LES.load_level(MAP)
    world = unreal.EditorLevelLibrary.get_editor_world()
    log("load_level=%s world=%s package=%s" % (ok, world.get_path_name(), obj_pkg_name(world)))

    descs = WPBL.get_actor_descs()
    if not descs:
        log("no actor descs (not a World Partition map, or WP not initialized); aborting"); return
    log("actor descs: %d" % len(descs))
    by_pkg = {}
    for d in descs:
        by_pkg[str(d.actor_package)] = d
    sample = next(iter(by_pkg))
    log("sample actor_package name: %s" % sample)

    cands = {}
    if CAND:
        cands = read_candidates(CAND)
        log("candidate packages from %s: %d" % (CAND, len(cands)))
    else:
        for pkg, d in by_pkg.items():
            cls = d.native_class.get_name() if d.native_class else ""
            if cls in CLASSES:
                cands[pkg] = []
        log("candidate packages by class %s: %d" % (CLASSES, len(cands)))
    stats["candidates"] = len(cands)

    todo = []
    missing = []
    for pkg, dang in cands.items():
        d = by_pkg.get(pkg)
        if d is None:
            missing.append(pkg); continue
        todo.append((pkg, d, dang))
    stats["descs_matched"] = len(todo)
    log("matched %d candidates to actor descs, %d without desc%s" % (len(todo), len(missing), (": " + ", ".join(missing[:5])) if missing else ""))

    saved_pkgs = []
    failed_pkgs = []
    class_count = {}
    for b in range(0, len(todo), BATCH):
        batch = todo[b:b + BATCH]
        guids = [d.guid for _, d, _ in batch]
        WPBL.load_actors(guids)
        loaded = {}
        for a in EAS.get_all_level_actors():
            try:
                loaded[a.get_package().get_name()] = a
            except Exception:
                pass
        dirty = []
        for pkg, d, dang in batch:
            a = loaded.get(pkg)
            if a is None:
                log("  NOT LOADED: %s (%s)" % (pkg, d.label)); continue
            stats["loaded"] += 1
            cn = a.get_class().get_name()
            class_count[cn] = class_count.get(cn, 0) + 1
            changed, apkg = fix_actor(a, dang)
            if changed:
                dirty.append(apkg)
        log("batch %d-%d: loaded %d, dirty packages %d" % (b, b + len(batch), len(loaded), len(dirty)))
        if dirty and not DRY:
            res = unreal.EditorLoadingAndSavingUtils.save_packages(dirty, only_dirty=False)
            log("  save_packages -> %s" % res)
            if not res:
                # fall back to per-package save to find the failures
                for p in dirty:
                    r = unreal.EditorLoadingAndSavingUtils.save_packages([p], only_dirty=False)
                    (saved_pkgs if r else failed_pkgs).append(p.get_name())
            else:
                saved_pkgs.extend(p.get_name() for p in dirty)
        WPBL.unload_actors(guids)
    stats["packages_saved"] = len(saved_pkgs)
    stats["packages_failed"] = len(failed_pkgs)
    log("actor classes touched: %s" % class_count)
    if failed_pkgs:
        log("FAILED to save: %s" % failed_pkgs)

    for path in MESHES:
        mesh = unreal.load_asset(path)
        if mesh is None:
            log("mesh %s: not found" % path); continue
        nav = mesh.get_editor_property("nav_collision")
        if nav is None:
            log("mesh %s: no NavCollision" % path); continue
        dyn = nav.get_editor_property("is_dynamic_obstacle")
        log("mesh %s: NavCollision %s is_dynamic_obstacle=%s" % (path, nav.get_class().get_name(), dyn))
        if dyn and not DRY:
            nav.modify(); mesh.modify()
            nav.set_editor_property("is_dynamic_obstacle", False)
            r = unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
            log("  cleared bIsDynamicObstacle, save_loaded_asset -> %s" % r)
            if r: stats["meshes_fixed"] += 1

    log("STATS %s" % stats)
    log("DONE in %.1f s" % (time.time() - t0))



# ---------------------------------------------------------------------------
# Shared helpers for the dump / assign-materials modes
# ---------------------------------------------------------------------------

def open_map():
    """Loads the WP map and returns (by_pkg, WPBL, EAS).  by_pkg maps long package name -> actor desc."""
    LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    WPBL = unreal.WorldPartitionBlueprintLibrary
    ok = LES.load_level(MAP)
    world = unreal.EditorLevelLibrary.get_editor_world()
    log("load_level=%s world=%s package=%s" % (ok, world.get_path_name(), obj_pkg_name(world)))
    descs = WPBL.get_actor_descs()
    if not descs:
        log("no actor descs (not a World Partition map, or WP not initialized); aborting")
        return None, WPBL, EAS
    log("actor descs: %d" % len(descs))
    by_pkg = {}
    for d in descs:
        by_pkg[str(d.actor_package)] = d
    return by_pkg, WPBL, EAS

def for_each_actor(todo, WPBL, EAS, handler):
    """todo is a list of (package name, actor desc).  Loads them in batches, calls
    handler(package, desc, actor) -> package to save (or None), saves and unloads."""
    saved, failed = [], []
    for b in range(0, len(todo), BATCH):
        batch = todo[b:b + BATCH]
        guids = [d.guid for _, d in batch]
        WPBL.load_actors(guids)
        loaded = {}
        for a in EAS.get_all_level_actors():
            try:
                loaded[a.get_package().get_name()] = a
            except Exception:
                pass
        dirty = []
        for pkg, d in batch:
            a = loaded.get(pkg)
            if a is None:
                log("  NOT LOADED: %s (%s)" % (pkg, d.label)); continue
            stats["loaded"] += 1
            p = handler(pkg, d, a)
            if p is not None:
                dirty.append(p)
        log("batch %d-%d: loaded %d, dirty packages %d" % (b, b + len(batch), len(loaded), len(dirty)))
        if dirty and not DRY:
            res = unreal.EditorLoadingAndSavingUtils.save_packages(dirty, only_dirty=False)
            log("  save_packages -> %s" % res)
            if res:
                saved.extend(x.get_name() for x in dirty)
            else:
                for x in dirty:
                    r = unreal.EditorLoadingAndSavingUtils.save_packages([x], only_dirty=False)
                    (saved if r else failed).append(x.get_name())
        WPBL.unload_actors(guids)
    stats["packages_saved"] = len(saved)
    stats["packages_failed"] = len(failed)
    if failed:
        log("FAILED to save: %s" % failed[:20])
    return saved, failed

def select_actors(by_pkg):
    """Selection for the dump mode: label prefixes (FIXA_LABELS) plus an explicit package list."""
    sel = {}
    if LABELS:
        for pkg, d in by_pkg.items():
            label = str(d.label)
            for pref in LABELS:
                if label.startswith(pref):
                    sel[pkg] = d
                    break
    if PACKAGES:
        want = set()
        with open(PACKAGES) as fh:
            for line in fh:
                line = line.strip()
                if line and not line.startswith("#"):
                    want.add(line)
        hit = 0
        for pkg in want:
            d = by_pkg.get(pkg)
            if d is not None:
                sel[pkg] = d; hit += 1
        log("package list %s: %d names, %d matched a desc" % (PACKAGES, len(want), hit))
    return sel

DUMP_COLUMNS = ["package", "label", "class", "folder", "material", "material_class",
                "size_x", "size_y", "size_z", "loc_x", "loc_y", "loc_z",
                "rot_pitch", "rot_yaw", "rot_roll", "scale_x", "scale_y", "scale_z",
                "sort_order", "fade_screen_size", "tags"]

def mode_dump():
    t0 = time.time()
    if not DUMP_TSV:
        log("FIXA_DUMP_TSV is required for FIXA_MODE=dump"); return
    log("mode=dump map=%s labels=%s packages=%s out=%s" % (MAP, LABELS, PACKAGES or "-", DUMP_TSV))
    by_pkg, WPBL, EAS = open_map()
    if by_pkg is None:
        return
    sel = select_actors(by_pkg)
    stats["candidates"] = len(sel)
    log("selected %d actors" % len(sel))
    todo = sorted(sel.items())
    fh = open(DUMP_TSV, "w")
    fh.write("\t".join(DUMP_COLUMNS) + "\n")

    def handler(pkg, d, a):
        stats["actors_seen"] += 1
        decal = None
        for comp in a.get_components_by_class(unreal.ActorComponent):
            stats["components_seen"] += 1
            if isinstance(comp, unreal.DecalComponent):
                decal = comp
                break
        mat = decal.get_editor_property("decal_material") if decal else None
        size = decal.get_editor_property("decal_size") if decal else unreal.Vector(0, 0, 0)
        sort_order = decal.get_editor_property("sort_order") if decal else 0
        fade = decal.get_editor_property("fade_screen_size") if decal else 0.0
        loc = a.get_actor_location(); rot = a.get_actor_rotation(); scale = a.get_actor_scale3d()
        try:
            folder = str(a.get_folder_path())
        except Exception:
            folder = ""
        try:
            tags = ",".join(str(t) for t in a.get_editor_property("tags"))
        except Exception:
            tags = ""
        row = [pkg, a.get_actor_label(), a.get_class().get_name(), folder,
               mat.get_path_name() if mat else "",
               mat.get_class().get_name() if mat else "",
               "%.4f" % size.x, "%.4f" % size.y, "%.4f" % size.z,
               "%.3f" % loc.x, "%.3f" % loc.y, "%.3f" % loc.z,
               "%.3f" % rot.pitch, "%.3f" % rot.yaw, "%.3f" % rot.roll,
               "%.5f" % scale.x, "%.5f" % scale.y, "%.5f" % scale.z,
               str(sort_order), "%.5f" % fade, tags]
        fh.write("\t".join(row) + "\n")
        stats["rows_dumped"] += 1
        return None       # read-only: never dirty a package

    for_each_actor(todo, WPBL, EAS, handler)
    fh.close()
    log("STATS %s" % stats)
    log("DONE in %.1f s -> %s" % (time.time() - t0, DUMP_TSV))

def read_assignment(tsv):
    """tsv rows: <long package name>\t<material object path>.  'None'/'' clears the slot."""
    plan = {}
    with open(tsv) as fh:
        for line in fh:
            line = line.rstrip("\n")
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 2:
                continue
            pkg, mat = parts[0].strip(), parts[1].strip()
            plan[pkg] = None if mat in ("", "None") else mat
    return plan

def mode_assign_materials():
    t0 = time.time()
    if not ASSIGN_TSV:
        log("FIXA_ASSIGN_TSV is required for FIXA_MODE=assign-materials"); return
    log("mode=assign-materials map=%s dry=%s force_visible=%s plan=%s" % (MAP, DRY, FORCE_VISIBLE, ASSIGN_TSV))
    plan = read_assignment(ASSIGN_TSV)
    stats["candidates"] = len(plan)
    log("assignment rows: %d, distinct materials: %d" % (len(plan), len(set(plan.values()))))

    # Load every material once up front so a typo fails loudly before anything is written.
    cache = {}
    for path in sorted(set(v for v in plan.values() if v)):
        m = unreal.load_asset(path)
        if m is None:
            log("MATERIAL NOT FOUND: %s" % path)
            stats["materials_missing"] += 1
        else:
            cache[path] = m
            log("material ok: %s (%s)" % (path, m.get_class().get_name()))
    if stats["materials_missing"]:
        log("aborting: %d material(s) could not be loaded" % stats["materials_missing"]); return

    by_pkg, WPBL, EAS = open_map()
    if by_pkg is None:
        return
    todo, missing = [], []
    for pkg in plan:
        d = by_pkg.get(pkg)
        if d is None:
            missing.append(pkg); continue
        todo.append((pkg, d))
    todo.sort()
    stats["descs_matched"] = len(todo)
    log("matched %d packages to actor descs, %d without desc%s"
        % (len(todo), len(missing), (": " + ", ".join(missing[:5])) if missing else ""))

    def handler(pkg, d, a):
        stats["actors_seen"] += 1
        want_path = plan.get(pkg)
        want = cache.get(want_path) if want_path else None
        changed = False
        for comp in a.get_components_by_class(unreal.ActorComponent):
            if not isinstance(comp, unreal.DecalComponent):
                continue
            stats["components_seen"] += 1
            if FORCE_VISIBLE and not comp.get_editor_property("visible"):
                # A DecalComponent with bVisible=false never renders, material or not.
                log("  %s [%s]: DecalComponent.bVisible false -> true"
                    % (a.get_actor_label(), pkg.rsplit("/", 1)[-1]))
                if not DRY:
                    comp.modify()
                    comp.set_editor_property("visible", True)
                stats["visibility_fixed"] += 1
                changed = True
            cur = comp.get_editor_property("decal_material")
            if cur is want:
                stats["slots_already_correct"] += 1
                continue
            log("  %s [%s]: DecalMaterial %s -> %s"
                % (a.get_actor_label(), pkg.rsplit("/", 1)[-1],
                   cur.get_path_name() if cur else "None",
                   want.get_path_name() if want else "None"))
            if not DRY:
                comp.modify()
                comp.set_decal_material(want)
            stats["slots_assigned"] += 1
            changed = True
        if changed and not DRY:
            a.modify()          # UObject::Modify marks the owning (external) package dirty
            return a.get_package()
        return None

    for_each_actor(todo, WPBL, EAS, handler)
    log("STATS %s" % stats)
    log("DONE in %.1f s" % (time.time() - t0))

MODES = {"mid-refs": mode_mid_refs, "dump": mode_dump, "assign-materials": mode_assign_materials}

try:
    if MODE not in MODES:
        log("unknown FIXA_MODE=%s (known: %s)" % (MODE, ", ".join(sorted(MODES))))
    else:
        MODES[MODE]()
except Exception:
    log("EXC " + traceback.format_exc())
