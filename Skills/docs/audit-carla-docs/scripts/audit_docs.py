#!/usr/bin/env python3
# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
"""Static documentation-gap audit for a CARLA checkout (Tier A).

Reads only files on disk: no build, no running server. Five signals, each
independent, each reporting what it found rather than acting on it. Deciding
what to document from the report stays a human call.

Signal detail and triage guidance: ../references/signals.md
"""
import argparse
import collections
import datetime
import json
import os
import re
import subprocess
import sys

# --- shared helpers ---------------------------------------------------------

# A fenced code block or an HTML comment is not prose: links inside them are
# examples or deliberately disabled, and counting them produced false positives
# in an earlier hand-run of these checks.
REPORT_NAME = 'audit_carla_docs.md'   # this audit's own output, never audited

FENCE = re.compile(r'```.*?```', re.S)
COMMENT = re.compile(r'<!--.*?-->', re.S)


def strip_noise(text):
    return COMMENT.sub('', FENCE.sub('', text))


def slugify(heading):
    """MkDocs' toc slug: strip inline HTML, lowercase, drop punctuation."""
    h = re.sub(r'<[^>]+>', '', heading)
    h = re.sub(r'[^\w\s-]', '', h.lower())
    return re.sub(r'\s+', '-', h).strip('-')


class Finding(object):
    __slots__ = ('signal', 'where', 'what', 'detail', 'note')

    def __init__(self, signal, where, what, detail='', note=''):
        self.signal, self.where, self.what, self.detail = signal, where, what, detail
        self.note = note


# --- signal 1: CHANGELOG features with no documentation ---------------------

# CARLA's changelog is disciplined about its leading verb. Everything that
# introduces or alters behaviour is a feature and owes documentation; a repair
# of existing documented behaviour does not.
FEATURE_VERBS = ('Added', 'Changed', 'Renamed', 'Optimized', 'Optimised',
                 'Removed', 'Deprecated', 'Exposed', 'Replaced', 'Reworked',
                 'Moved', 'Updated', 'Ported', 'Documented')
FIX_VERBS = ('Fixed', 'Reverted', 'Corrected', 'Restored')

# Anything backticked is a name the entry is telling the reader about: an API
# path, a blueprint id, a cvar, an attribute, a file. Prose in backticks is rare
# enough that a loose match costs less than the dotted-only match it replaced,
# which reported most entries as "nothing to check".
IDENT = re.compile(r'`([^`\n]{2,80})`')
# A checkable name: no spaces, and at least one identifier character class.
CHECKABLE = re.compile(r'^[A-Za-z_][\w./-]*(?:\(\))?$')


def signal_changelog(root, _docs_text=None):
    """List the features in the unreleased section. No documentation check.

    Judging coverage by substring-matching names against Docs/ was too generous
    to trust: a bare mention counted as documentation, one match cleared a whole
    entry, and stale pages still matched. So this lists what shipped and leaves
    the judgement to a human reading the list.
    """
    out = []
    path = os.path.join(root, 'CHANGELOG.md')
    if not os.path.isfile(path):
        return out
    text = open(path, encoding='utf-8', errors='replace').read()
    # Only the unreleased section: released entries went through a different
    # process and re-listing them is noise.
    m = re.search(r'^## LATEST Changes\s*$(.*?)(?=^## )', text, re.S | re.M)
    if not m:
        return out
    for line in m.group(1).splitlines():
        line = line.strip()
        if not line.startswith('* '):
            continue
        entry = line[2:].strip()
        verb = entry.split(' ', 1)[0].strip('`*_')
        if verb in FIX_VERBS:
            continue
        kind = 'feature' if verb in FEATURE_VERBS else 'unclassified'
        # Concise: the first sentence carries what changed; the rest is
        # rationale. Fall back to a hard cap for entries without one.
        cut = re.split(r'(?<=\.)\s+(?=[A-Z`])', entry, maxsplit=1)[0]
        if len(cut) > 220:
            cut = cut[:217].rsplit(' ', 1)[0] + '...'
        out.append(Finding('changelog', 'CHANGELOG.md', cut, kind))
    return out


# --- signal 2: Python bindings absent from the yml reference ----------------

# class_<> and enum_<> both register a Python class; nothing about the name
# says which, so both are matched and the name itself is never used to decide.
CLASS_DEF = re.compile(r'\b(?:class_|enum_)<([^(]{0,200})\(\s*"(\w+)"')


BASES = re.compile(r'bases\s*<([^>]*)>')


def short_cpp_name(cpp_type):
    """cc::Sensor -> Sensor, so a bases<> entry matches a yml class_name."""
    depth, start = 0, 0
    for i, ch in enumerate(cpp_type):
        if ch == '<':
            depth += 1
        elif ch == '>':
            depth -= 1
        elif ch == ':' and depth == 0 and cpp_type[i:i + 2] == '::':
            start = i + 2
    return cpp_type[start:].strip()


def base_names(template_args):
    """The registered base classes of a class_<T, bases<A, B>, ...>."""
    m = BASES.search(template_args)
    return [short_cpp_name(t) for t in m.group(1).split(',')] if m else []


MEMBER_DEF = re.compile(
    r'\.def\s*\(\s*"(\w+)"'
    r'|\.(?:add_property|def_readwrite|def_readonly|add_static_property|value)'
    r'\s*\(\s*"(\w+)"')


def documented_members(yml_dir):
    """Class name -> the def_names and var_names documented under it."""
    doc = {}
    for f in sorted(os.listdir(yml_dir)):
        if not f.endswith('.yml'):
            continue
        text = open(os.path.join(yml_dir, f), encoding='utf-8', errors='replace').read()
        # Walk in order so each member lands under the class it belongs to.
        current = None
        for line in text.splitlines():
            mc = re.match(r'\s*-\s*class_name:\s*([\w.]+)\s*$', line)
            if mc:
                current = mc.group(1).rsplit('.', 1)[-1]
                doc.setdefault(current, set())
                continue
            # 'y' and friends are quoted in the ymls, since bare y is a YAML boolean.
            mm = re.match(r'''\s*-\s*(?:def_name|var_name):\s*['"]?([\w.]+)['"]?\s*$''', line)
            if mm and current is not None:
                doc[current].add(mm.group(1).rsplit('.', 1)[-1])
    return doc


def scan_registrations(src_dir):
    """Every class_<>/enum_<> in the bindings, with its bases and own members."""
    regs = []
    for f in sorted(os.listdir(src_dir)):
        if not f.endswith('.cpp'):
            continue
        src = open(os.path.join(src_dir, f), encoding='utf-8', errors='replace').read()
        # Each registration owns the source up to the next one, which is what
        # ties a member to the class that exposes it.
        marks = [(m.start(), m.group(2), m.group(1)) for m in CLASS_DEF.finditer(src)]
        for i, (pos, cls, template_args) in enumerate(marks):
            end = marks[i + 1][0] if i + 1 < len(marks) else len(src)
            members = {}
            for m in MEMBER_DEF.finditer(src, pos, end):
                # Group 1 is the .def alternative; group 2 is a property, a data
                # member or an enum value.
                members.setdefault(m.group(1) or m.group(2),
                                   '(new function)' if m.group(1) else '(new attribute)')
            regs.append((f, cls, base_names(template_args), members))
    return regs


def inherited_documentation(cls, base_of, doc, seen=None):
    """Every member documented on an ancestor of cls."""
    seen = seen if seen is not None else set()
    out = set()
    for base in base_of.get(cls, ()):
        if base in seen:
            continue
        seen.add(base)
        out |= doc.get(base, set())
        out |= inherited_documentation(base, base_of, doc, seen)
    return out


def signal_bindings(root):
    out = []
    src_dir = os.path.join(root, 'PythonAPI', 'carla', 'src')
    yml_dir = os.path.join(root, 'PythonAPI', 'docs')
    if not os.path.isdir(src_dir) or not os.path.isdir(yml_dir):
        return out
    doc = documented_members(yml_dir)
    regs = scan_registrations(src_dir)
    base_of = {cls: bases for _, cls, bases, _ in regs}
    seen = set()
    for f, cls, _bases, members in regs:
        # STALE_EXEMPT (signal 7) is the same list of bindings-generated dunders;
        # CARLA documents those in bulk, not per class.
        own = set(members) - STALE_EXEMPT
        if cls not in doc:
            # A class that adds nothing of its own is a hierarchy node or a
            # container registration: there is no page to write for it. One
            # whose members are all documented on an ancestor is covered too.
            if not own or own <= inherited_documentation(cls, base_of, doc):
                continue
            if cls not in seen:
                seen.add(cls)
                out.append(Finding('binding', 'PythonAPI/carla/src/' + f, cls,
                                   note='(new class)'))
            continue
        for name in sorted(own - doc[cls]):
            qualified = '%s.%s' % (cls, name)
            if qualified in seen:
                continue
            seen.add(qualified)
            out.append(Finding('binding', 'PythonAPI/carla/src/' + f, qualified,
                               note=members[name]))
    return out


# --- signal 3: Docs pages unreachable from the mkdocs nav -------------------

def signal_orphans(root):
    out = []
    nav = os.path.join(root, 'mkdocs.yml')
    docs = os.path.join(root, 'Docs')
    if not os.path.isfile(nav) or not os.path.isdir(docs):
        return out
    nav_text = open(nav, encoding='utf-8', errors='replace').read()
    # A commented nav line is deliberately parked, not an orphan.
    live = '\n'.join(l for l in nav_text.splitlines()
                     if not l.lstrip().startswith('#'))
    listed = set(re.findall(r'([A-Za-z0-9_./-]+\.md)', live))
    for f in sorted(os.listdir(docs)):
        if f == REPORT_NAME:
            continue
        if f.endswith('.md') and f not in listed:
            out.append(Finding('orphan', 'Docs/' + f, f,
                               'page exists but is not in the mkdocs.yml nav'))
    return out


# --- signal 4: broken relative links ---------------------------------------

INLINE = re.compile(r'\[[^\]]*\]\(([^)\s]+)\)')
REFDEF = re.compile(r'^\s*\[([^\]]+)\]:\s*(\S+)\s*$', re.M)
NAMED = re.compile(r'<a\s+name="([^"]+)"')
HEAD = re.compile(r'^(#{1,6})\s+(.*?)\s*$', re.M)


def signal_links(root):
    out = []
    docs = os.path.join(root, 'Docs')
    if not os.path.isdir(docs):
        return out
    pages, anchors = set(), {}
    for f in os.listdir(docs):
        if not f.endswith('.md') or f == REPORT_NAME:
            continue
        pages.add(f)
        s = open(os.path.join(docs, f), encoding='utf-8', errors='replace').read()
        a = set(NAMED.findall(s))
        a.update(slugify(h) for _, h in HEAD.findall(s))
        anchors[f] = a
    for f in sorted(pages):
        raw = open(os.path.join(docs, f), encoding='utf-8', errors='replace').read()
        s = strip_noise(raw)
        targets = [m.group(1) for m in INLINE.finditer(s)]
        targets += [m.group(2) for m in REFDEF.finditer(s)]
        for tgt in targets:
            if tgt.startswith(('http://', 'https://', 'mailto:', '#', '<')):
                continue
            path, _, anchor = tgt.partition('#')
            path = path.strip().lstrip('(')
            if not path:
                continue
            if path.endswith('.md'):
                base = os.path.basename(path)
                if base not in pages:
                    out.append(Finding('link', 'Docs/' + f, tgt, 'target page does not exist'))
                elif anchor and anchor not in anchors[base]:
                    out.append(Finding('link', 'Docs/' + f, tgt, 'anchor not found in ' + base))
            elif path.endswith('/') or '.' not in os.path.basename(path):
                # No .md: MkDocs leaves it alone and it resolves below the
                # current page. This is the bug class PR #9927 fixed.
                out.append(Finding('link', 'Docs/' + f, tgt,
                                   'missing .md — resolves relative to the current page'))
    return out


# --- signal 5: sensors missing from the sensor reference --------------------

# Sensor ids are rarely written as literals. They are built inside the
# Make*Definition helpers by
#     FillIdAndTags(Def, TEXT("sensor"), TEXT("<group>"), <name>)
# where <name> is often a parameter supplied at the call site. So resolve each
# helper to its group, then take the literal names its callers pass. Scanning
# only for literal "sensor.x.y" strings found 6 of ~22 and silently missed the
# rest.
SENSOR_ID = re.compile(r'"(sensor\.[a-z0-9_]+\.[a-z0-9_]+)"')
HELPER_DEF = re.compile(
    r'(?:void|FActorDefinition)\s+UActorBlueprintFunctionLibrary::(Make\w*Definition)\s*\(')
FILL_SENSOR = re.compile(
    r'FillIdAndTags\s*\(\s*\w+\s*,\s*TEXT\("sensor"\)\s*,\s*TEXT\("(\w+)"\)\s*,\s*([^;]+?)\)\s*;')
LITERAL_ARG = re.compile(r'TEXT\("([a-z0-9_]+)"\)')


def _sensor_helpers(abfl_src):
    """{helper name: (group, fixed_name or None, suffix)} for sensor builders."""
    helpers = {}
    starts = [(m.group(1), m.start()) for m in HELPER_DEF.finditer(abfl_src)]
    for i, (name, pos) in enumerate(starts):
        end = starts[i + 1][1] if i + 1 < len(starts) else len(abfl_src)
        m = FILL_SENSOR.search(abfl_src, pos, end)
        if not m:
            continue
        group, expr = m.group(1), m.group(2).strip()
        lits = LITERAL_ARG.findall(expr)
        if lits and expr.startswith('TEXT('):
            # Fully literal: sensor.<group>.<lit>, no call site needed.
            helpers[name] = (group, lits[0], '')
        else:
            # Parameterised; a trailing + TEXT("_x") becomes a suffix.
            helpers[name] = (group, None, lits[-1] if lits else '')
    return helpers


def signal_sensors(root):
    out = []
    ref = os.path.join(root, 'Docs', 'ref_sensors.md')
    plugin = os.path.join(root, 'Unreal', 'CarlaUnreal', 'Plugins', 'Carla', 'Source')
    abfl = os.path.join(plugin, 'Carla', 'Actor', 'ActorBlueprintFunctionLibrary.cpp')
    if not os.path.isfile(ref) or not os.path.isfile(abfl):
        return out
    ref_text = open(ref, encoding='utf-8', errors='replace').read()
    abfl_src = open(abfl, encoding='utf-8', errors='replace').read()
    helpers = _sensor_helpers(abfl_src)

    found = {}
    sources = []
    for dirpath, _, files in os.walk(plugin):
        for f in files:
            if f.endswith(('.cpp', '.h')):
                path = os.path.join(dirpath, f)
                sources.append((os.path.relpath(path, root),
                                open(path, encoding='utf-8', errors='replace').read()))

    for rel, src in sources:
        # a) ids written out in full
        for sid in SENSOR_ID.findall(src):
            found.setdefault(sid, rel)
        # b) helpers with a fixed literal name
        for helper, (group, fixed, _suffix) in helpers.items():
            if fixed and re.search(r'\b%s\b' % re.escape(helper), src):
                found.setdefault('sensor.%s.%s' % (group, fixed), rel)
        # c) parameterised helpers: take the literal each caller passes
        for helper, (group, fixed, suffix) in helpers.items():
            if fixed:
                continue
            for m in re.finditer(r'\b%s\s*\(([^;]{0,200}?)\)' % re.escape(helper), src):
                lits = LITERAL_ARG.findall(m.group(1))
                if lits:
                    found.setdefault('sensor.%s.%s%s' % (group, lits[0], suffix), rel)

    for sid in sorted(found):
        tail = sid.split('.', 1)[1]
        if sid not in ref_text and tail not in ref_text:
            out.append(Finding('sensor', found[sid], sid))
    return out


# --- signal 6: catalogue actors absent from the docs ----------------------

# Vehicles, walkers and props are NOT registered in C++: the factories load JSON
# catalogues off disk (VehicleActorFactory::GetDefinitions and siblings), so the
# actor inventory is static data and needs no running server. Attributes still
# do -- they come from FActorVariation at definition time.
#
# Id construction mirrors FillIdAndTags, which joins with "." and lowercases
# (ActorBlueprintFunctionLibrary.cpp:224).
CATALOGUES = (
    # file,                      top key,    id prefix,            fields -> id tail
    ('VehicleParameters.json',   'Vehicles', 'vehicle',            ('Make', 'Model')),
    ('WalkerParameters.json',    'Walkers',  'walker.pedestrian',  ('Id',)),
    ('PropParameters.json',      'Props',    'static.prop',        ('Name',)),
)

# Where each kind is supposed to be documented.
CATALOGUE_DOCS = {
    'vehicle':           ('catalogue_vehicles.md', 'bp_library.md'),
    'walker.pedestrian': ('catalogue_pedestrians.md', 'bp_library.md'),
    'static.prop':       ('catalogue_props.md', 'bp_library.md'),
}


def _catalogue_files(root, filename):
    """Every copy of a catalogue: the base one, per-map ones, and content packs.

    Missing any of these under-reports, and under-reporting is the failure mode
    that matters for an audit.
    """
    found = []
    base = os.path.join(root, 'Unreal', 'CarlaUnreal', 'Content')
    if not os.path.isdir(base):
        return found
    for dirpath, _, files in os.walk(base):
        # Per-map catalogues are named Vehicles.json/Walkers.json/Props.json;
        # the base ones keep the *Parameters.json name.
        short = filename.replace('Parameters', 's').replace('Propss', 'Props')
        for f in files:
            if f == filename or f == short:
                found.append(os.path.join(dirpath, f))
    return found


def signal_catalogue(root):
    out = []
    docs_dir = os.path.join(root, 'Docs')
    if not os.path.isdir(docs_dir):
        return out
    doc_text = {}
    for f in os.listdir(docs_dir):
        if f.endswith('.md') and f != REPORT_NAME:
            doc_text[f] = open(os.path.join(docs_dir, f),
                               encoding='utf-8', errors='replace').read().lower()

    for filename, top_key, prefix, fields in CATALOGUES:
        seen = {}
        for path in _catalogue_files(root, filename):
            try:
                data = json.load(open(path, encoding='utf-8', errors='replace'))
            except ValueError as exc:
                out.append(Finding('catalogue', os.path.relpath(path, root),
                                   os.path.basename(path), 'unparseable JSON: %s' % exc))
                continue
            entries = data.get(top_key, data if isinstance(data, list) else [])
            for e in entries:
                if not isinstance(e, dict):
                    continue
                tail = '.'.join(str(e.get(k, '')).strip() for k in fields)
                if not tail or tail.strip('.') == '':
                    continue
                # Later catalogues override earlier ones by id, as the factory does.
                # FillIdAndTags lowercases, so the real blueprint id is lowercase.
                seen[('%s.%s' % (prefix, tail)).lower()] = os.path.relpath(path, root)

        targets = CATALOGUE_DOCS.get(prefix, ())
        for actor_id, src in sorted(seen.items()):
            aid = actor_id
            where_found = [d for d in targets if d in doc_text and aid in doc_text[d]]
            if where_found:
                continue
            # Fall back to the whole corpus before calling it undocumented.
            anywhere = [d for d, t in doc_text.items() if aid in t]
            note = ('(only in %s)' % ', '.join(sorted(anywhere)[:3])) if anywhere else ''
            out.append(Finding('catalogue', src, actor_id, note=note))
    return out



# --- signal 7: documented API that no longer exists ------------------------

# The mirror of `binding`. That signal finds code with no docs; this finds docs
# with no code -- a reference entry for something renamed or deleted, which is
# worse than a missing entry because it reads as authoritative.
YML_CLASS = re.compile(r'^\s*-\s*class_name:\s*([\w.]+)\s*$', re.M)
YML_DEF = re.compile(r'^\s*-\s*def_name:\s*([\w.]+)\s*$', re.M)
# Everything the bindings can expose a name through.
CPP_EXPOSED = re.compile(
    r'class_<[^(]{0,200}\(\s*"(\w+)"'
    r'|\.def\s*\(\s*"(\w+)"'
    r'|\.add_property\s*\(\s*"(\w+)"'
    r'|\.def_readwrite\s*\(\s*"(\w+)"'
    r'|\.def_readonly\s*\(\s*"(\w+)"'
    r'|\.add_static_property\s*\(\s*"(\w+)"'
    r'|enum_<[^(]{0,200}\(\s*"(\w+)"'
    r'|\.value\s*\(\s*"(\w+)"')

# Documented names with no C++ counterpart by design: Python-side sugar, dunders
# the bindings generate, and names Boost.Python supplies itself.
STALE_EXEMPT = {
    '__init__', '__str__', '__repr__', '__eq__', '__ne__', '__hash__',
    '__len__', '__iter__', '__getitem__', '__setitem__', '__call__',
    '__add__', '__sub__', '__mul__', '__truediv__', '__abs__', '__bool__',
}


def signal_stale(root):
    out = []
    src_dir = os.path.join(root, 'PythonAPI', 'carla', 'src')
    yml_dir = os.path.join(root, 'PythonAPI', 'docs')
    if not os.path.isdir(src_dir) or not os.path.isdir(yml_dir):
        return out

    exposed = set()
    for f in sorted(os.listdir(src_dir)):
        if not f.endswith('.cpp'):
            continue
        src = open(os.path.join(src_dir, f), encoding='utf-8', errors='replace').read()
        for groups in CPP_EXPOSED.findall(src):
            exposed.update(g for g in groups if g)

    for f in sorted(os.listdir(yml_dir)):
        if not f.endswith('.yml'):
            continue
        text = open(os.path.join(yml_dir, f), encoding='utf-8', errors='replace').read()
        # Walk the file in order so each method carries its owning class: the
        # same def_name appears under several classes (two __get_item__ entries
        # in sensor_data.yml), and reporting the bare name makes them look like
        # one duplicated finding instead of two distinct ones.
        current = '?'
        seen = set()
        for line in text.splitlines():
            mc = re.match(r'\s*-\s*class_name:\s*([\w.]+)\s*$', line)
            if mc:
                current = mc.group(1)
                bare = current.rsplit('.', 1)[-1]
                if bare not in exposed and current not in seen:
                    seen.add(current)
                    out.append(Finding('stale', 'PythonAPI/docs/' + f, current))
                continue
            md = re.match(r'''\s*-\s*(?:def_name|var_name):\s*['"]?([\w.]+)['"]?\s*$''', line)
            if not md:
                continue
            name = md.group(1)
            bare = name.rsplit('.', 1)[-1]
            if bare in STALE_EXEMPT or bare in exposed:
                continue
            qualified = '%s.%s' % (current, name)
            if qualified in seen:
                continue
            seen.add(qualified)
            out.append(Finding('stale', 'PythonAPI/docs/' + f, qualified))
    return out


# The two Python API signals are grouped by owning class, because the work is
# per class: one heading is one yml block to edit.
GROUP_BY_CLASS = {
    'binding': 'Classes absent from the reference',
    'stale': 'Classes that no longer exist',
}

# Grouped by the file they come from, which is the catalogue they belong to.
GROUP_BY_FILE = {'catalogue', 'catalogue-stale'}


def group_by_file(items):
    """One subsection per source file, so the path is not on every line."""
    groups = collections.OrderedDict()
    for f in items:
        groups.setdefault(f.where, []).append(f)
    lines = []
    for where in sorted(groups):
        lines += ['### %s — `%s`' % (os.path.basename(where).split('.')[0], where), '']
        for f in groups[where]:
            lines.append('- **%s**%s' % (f.what, ' %s' % f.note if f.note else ''))
        lines.append('')
    return lines


def group_by_class(items, whole_class_title):
    """Whole-class findings first, then one subsection per class with members."""
    whole = [f for f in items if '.' not in f.what]
    members = collections.OrderedDict()
    for f in items:
        if '.' in f.what:
            members.setdefault(f.what.split('.', 1)[0], []).append(f)

    def line(f, what):
        return '- `%s` — **%s**%s' % (f.where, what, ' %s' % f.note if f.note else '')

    lines = []
    if whole:
        lines += ['### %s' % whole_class_title, '']
        lines += [line(f, f.what) for f in whole]
        lines.append('')
    for cls in sorted(members):
        group = members[cls]
        where = {f.where for f in group}
        heading = '### %s' % cls if len(where) > 1 else '### %s — `%s`' % (cls, group[0].where)
        lines += [heading, '']
        for f in group:
            what = f.what.split('.', 1)[1]
            lines.append('- **%s**%s' % (what, ' %s' % f.note if f.note else '')
                         if len(where) == 1 else line(f, f.what))
        lines.append('')
    return lines


# --- provenance -------------------------------------------------------------

# The content repo is a separate checkout inside the code one, and the two move
# independently -- a report is only meaningful against a known pair, as a cloud
# investigation on this repo found the hard way.
CONTENT_SUBDIR = os.path.join('Unreal', 'CarlaUnreal', 'Content', 'Carla')


def _git(repo, *args):
    """A git value, or None when git is missing, the dir is not a repo, etc."""
    try:
        out = subprocess.check_output(('git', '-C', repo) + args,
                                      stderr=subprocess.DEVNULL)
    except (OSError, subprocess.CalledProcessError):
        return None
    return out.decode('utf-8', 'replace').strip() or None


def describe_repo(repo):
    """'branch @ sha', 'detached @ sha', or 'not a git checkout'.

    No dirty marker: this report is written into Docs/, so generating it always
    dirties the code checkout and the flag would be on in every single report.
    The commit is what identifies the state worth recording.
    """
    if not os.path.isdir(repo):
        return 'absent'
    sha = _git(repo, 'rev-parse', '--short', 'HEAD')
    if sha is None:
        return 'not a git checkout'
    branch = _git(repo, 'rev-parse', '--abbrev-ref', 'HEAD')
    if branch in (None, 'HEAD'):
        branch = 'detached'
    return '`%s` @ `%s`' % (branch, sha)


# --- report -----------------------------------------------------------------

# A blueprint id always has a make and a model after its prefix. Without that
# depth the pattern also matches `vehicle.apply_control(...)` in a code snippet.
CATALOGUE_ID = re.compile(
    r'\b(?:vehicle\.[a-z0-9_-]+|walker\.pedestrian|static\.prop)\.[a-z0-9_-]+(?:\.[a-z0-9_-]+)*')


def live_blueprint_ids(host, port):
    """Actor ids from a running server, or None when there is none to ask.

    The rest of the audit is static; this is the only signal that gains from a
    server, because the blueprint library is the authoritative list.
    """
    try:
        import carla
    except ImportError:
        return None, 'the carla module is not importable'
    try:
        client = carla.Client(host, port)
        client.set_timeout(5.0)
        library = client.get_world().get_blueprint_library()
    except Exception as e:
        return None, '%s on %s:%d (%s)' % (type(e).__name__, host, port, str(e)[:60])
    keep = tuple(CATALOGUE_DOCS)
    return {bp.id for bp in library if bp.id.startswith(keep)}, None


def signal_catalogue_stale(root, live):
    """Blueprint ids a catalogue page names that nothing knows about.

    The live library is per map -- Mine_01's vehicles are not registered on
    Town10 -- so it is added to the JSON catalogues rather than replacing them.
    An entry is only stale when neither source has it.
    """
    out = []
    known = set(live) if live else set()
    if True:
        for filename, top_key, prefix, fields in CATALOGUES:
            for path in _catalogue_files(root, filename):
                try:
                    data = json.load(open(path, encoding='utf-8', errors='replace'))
                except ValueError:
                    continue
                entries = data.get(top_key, data if isinstance(data, list) else [])
                for e in entries:
                    if not isinstance(e, dict):
                        continue
                    tail = '.'.join(str(e.get(k, '')).strip() for k in fields)
                    if tail.strip('.'):
                        known.add(('%s.%s' % (prefix, tail)).lower())
    if not known:
        return out
    for prefix, targets in CATALOGUE_DOCS.items():
        page = os.path.join(root, 'Docs', targets[0])
        if not os.path.isfile(page):
            continue
        text = open(page, encoding='utf-8', errors='replace').read()
        for actor_id in sorted(set(CATALOGUE_ID.findall(text))):
            if actor_id.startswith(prefix) and actor_id not in known:
                out.append(Finding('catalogue-stale', 'Docs/' + targets[0], actor_id))
    return out


SIGNAL_TITLE = {
    'changelog': 'Unreleased features (check each against the docs)',
    'binding':   'Python bindings absent from the API reference',
    'orphan':    'Docs pages not reachable from the nav',
    'link':      'Broken relative links',
    'sensor':    'Sensors absent from ref_sensors.md',
    'catalogue': 'Catalogue actors absent from the catalogue pages',
    'catalogue-stale': 'Catalogue entries for actors that no longer exist',
    'stale':     'Documented API that no longer exists in the bindings',
}
ORDER = ['changelog', 'binding', 'stale', 'sensor', 'catalogue',
         'catalogue-stale', 'link', 'orphan']

TIER_B_WARNING = """\
## Still owed: a runtime audit (Tier B)

This report is **static only**. It cannot see anything that exists solely at
runtime, because those facts come from a live server rather than from files:

- per-blueprint **attributes** and their recommended values — these come from
  `FActorVariation` at definition time, not from the catalogues
- the available maps (`client.get_available_maps()`)
- which content packs a given build actually ships

The actor *inventory* (vehicles, walkers, props) is covered statically by the
`catalogue` signal, which reads the same JSON the factories read.

Run the runtime audit against a built server, on the same code **and content**
this report was generated from, before treating the documentation as complete.
`PythonAPI/docs/bp_doc_gen.py` already regenerates `Docs/bp_library.md` from a
live server; diff its output rather than letting it overwrite the page.
"""


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--root', default=os.environ.get('CARLA_ROOT') or os.getcwd(),
                    help='carla checkout to audit (default: $CARLA_ROOT or $PWD)')
    ap.add_argument('--out', nargs='?', const='', default=None,
                    help='write the report (default path: <root>/Docs/' + REPORT_NAME +
                         '); omit the flag entirely for stdout')
    ap.add_argument('--host', default='127.0.0.1',
                    help='server to ask for the live blueprint library (catalogue signals)')
    ap.add_argument('--port', type=int, default=2000,
                    help='RPC port of that server')
    ap.add_argument('--signal', action='append', choices=ORDER,
                    help='run only these signals (repeatable)')
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    if not os.path.isfile(os.path.join(root, 'CHANGELOG.md')):
        sys.exit('not a carla checkout (no CHANGELOG.md): %s' % root)

    docs_dir = os.path.join(root, 'Docs')
    docs_text = ''
    if os.path.isdir(docs_dir):
        for f in sorted(os.listdir(docs_dir)):
            if f.endswith('.md'):
                docs_text += open(os.path.join(docs_dir, f),
                                  encoding='utf-8', errors='replace').read()

    wanted = args.signal or ORDER
    findings = []
    if 'changelog' in wanted:
        findings += signal_changelog(root, docs_text)
    if 'binding' in wanted:
        findings += signal_bindings(root)
    if 'orphan' in wanted:
        findings += signal_orphans(root)
    if 'link' in wanted:
        findings += signal_links(root)
    if 'sensor' in wanted:
        findings += signal_sensors(root)
    live, live_why = (None, 'not needed')
    if 'catalogue' in wanted or 'catalogue-stale' in wanted:
        live, live_why = live_blueprint_ids(args.host, args.port)
    if 'catalogue' in wanted:
        findings += signal_catalogue(root)
    if 'catalogue-stale' in wanted:
        findings += signal_catalogue_stale(root, live)
    if 'stale' in wanted:
        findings += signal_stale(root)

    by_signal = collections.defaultdict(list)
    for f in findings:
        by_signal[f.signal].append(f)

    now = datetime.datetime.now().astimezone()
    code = describe_repo(root)
    content = describe_repo(os.path.join(root, CONTENT_SUBDIR))

    lines = [
        '# CARLA documentation audit (static)',
        '',
        'Static check of the documentation. Created at **%s** using the code '
        'branch %s and content %s.' % (now.strftime('%Y-%m-%d %H:%M %Z'), code, content),
        '',
        'Generated by the `audit-carla-docs` skill. It reads only files on disk: '
        'no build, no running server. Findings are candidates for a human to '
        'triage, not verdicts — and the list is incomplete until the runtime '
        'audit at the end of this file has also been run.',
        '',
        'Checkout: `%s`' % root,
        '',
        '| signal | findings |', '|---|---|']
    for s in ORDER:
        if s in wanted:
            lines.append('| %s | %d |' % (SIGNAL_TITLE[s], len(by_signal[s])))
    lines += ['| **total** | **%d** |' % len(findings), '']

    for s in ORDER:
        if s not in wanted:
            continue
        items = by_signal[s]
        lines += ['## %s (%d)' % (SIGNAL_TITLE[s], len(items)), '']
        if not items:
            lines += ['None.', '']
            continue
        if s == 'changelog':
            for f in items:
                mark = '' if f.detail == 'feature' else '  _(verb not recognised — check)_'
                lines.append('- [ ] %s%s' % (f.what, mark))
        elif s in GROUP_BY_CLASS:
            lines += group_by_class(items, GROUP_BY_CLASS[s])
        elif s in GROUP_BY_FILE:
            if s == 'catalogue-stale':
                lines += [(('Checked against the JSON catalogues and the blueprint library '
                            'of the server on %s:%d.' % (args.host, args.port))
                           if live is not None else
                           ('Checked against the JSON catalogues only: no server to ask (%s). '
                            'Start one and re-run to also check what actually registers.'
                            % live_why)), '']
            lines += group_by_file(items)
        else:
            for f in items:
                lines.append('- `%s` — **%s**%s'
                             % (f.where, f.what, ' %s' % f.note if f.note else ''))
                if f.detail:
                    lines.append('  - %s' % f.detail)
        lines.append('')

    lines.append(TIER_B_WARNING)
    report = '\n'.join(lines)

    if args.out is not None:
        dest = args.out or os.path.join(root, 'Docs', REPORT_NAME)
        args.out = dest
        with open(args.out, 'w', encoding='utf-8') as fh:
            fh.write(report)
        print('wrote %s (%d findings)' % (args.out, len(findings)))
        print('NOTE: static audit only — a runtime (Tier B) audit is still owed.')
    else:
        print(report)


main()
