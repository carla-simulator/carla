#!/usr/bin/env python3
# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
"""Locates where a Python API entry has to be written, and the names it must use.

For each target (a class, or Class.member) it prints the binding that exposes
it -- with the exact arg() names and defaults -- and the yml that governs the
class, with the entry it should be inserted next to. It writes nothing.

Usage:
  python3 yml_target.py --root /path/to/carla Actor.apply_texture World.get_ego_spawn_points
  python3 yml_target.py --root /path/to/carla --from-audit Docs/audit_carla_docs.md
"""
import argparse
import os
import re
import sys

CLASS_DEF = re.compile(r'class_<([^(]{0,200})\(\s*"(\w+)"')
MEMBER_DEF = re.compile(r'\.(def|add_property|def_readwrite|def_readonly)\s*\(\s*"(\w+)"')
ARG = re.compile(r'arg\s*\(\s*"(\w+)"\s*\)(\s*=\s*([^,()]+(?:\([^()]*\))?))?')
# The report groups members under a `### Class` heading, so a line carries only
# the member name; whole-class findings sit under a heading of their own.
AUDIT_HEADING = re.compile(r'^### (\w+)(?: — |$)')
AUDIT_ITEM = re.compile(r'^- (?:`[^`]+` — )?\*\*([\w.]+)\*\*')


def targets_from_audit(section):
    """Every finding in a report section, qualified by its group heading."""
    found, owner = [], None
    for line in section.splitlines():
        heading = AUDIT_HEADING.match(line)
        if heading:
            owner = None if line.startswith('### Classes') else heading.group(1)
            continue
        item = AUDIT_ITEM.match(line)
        if item:
            name = item.group(1)
            found.append(name if owner is None or '.' in name
                         else '%s.%s' % (owner, name))
    return found


def cpp_type_of(template_args):
    """The C++ class a class_<> wraps: its first template argument."""
    text = template_args.strip()
    if text.endswith('>'):
        text = text[:-1]
    depth, out = 0, []
    for ch in text:
        if ch == '<':
            depth += 1
        elif ch == '>':
            depth -= 1
        elif ch == ',' and depth == 0:
            break
        out.append(ch)
    return ' '.join(''.join(out).split())


def call_text(src, start):
    """The full .def(...) call starting at `start`, by balancing parentheses."""
    i = src.find('(', start)
    if i < 0:
        return ''
    depth, j = 0, i
    while j < len(src):
        if src[j] == '(':
            depth += 1
        elif src[j] == ')':
            depth -= 1
            if depth == 0:
                return src[start:j + 1]
        j += 1
    return src[start:start + 400]


def scan_bindings(src_dir):
    """(class, member) -> dict with the file, line, call text and arg list."""
    found = {}
    for f in sorted(os.listdir(src_dir)):
        if not f.endswith('.cpp'):
            continue
        src = open(os.path.join(src_dir, f), encoding='utf-8', errors='replace').read()
        marks = [(m.start(), m.group(2), cpp_type_of(m.group(1)))
                 for m in CLASS_DEF.finditer(src)]
        for i, (pos, cls, cpp) in enumerate(marks):
            end = marks[i + 1][0] if i + 1 < len(marks) else len(src)
            found.setdefault((cls, None), {
                'file': f, 'line': src.count('\n', 0, pos) + 1, 'call': '', 'args': [],
                'cpp': cpp})
            for m in MEMBER_DEF.finditer(src, pos, end):
                text = call_text(src, m.start())
                args = [(a.group(1), (a.group(3) or '').strip()) for a in ARG.finditer(text)]
                found[(cls, m.group(2))] = {
                    'file': f, 'line': src.count('\n', 0, m.start()) + 1,
                    'kind': m.group(1), 'call': ' '.join(text.split()), 'args': args}
    return found


def scan_ymls(yml_dir):
    """class -> {file, line, sections: {'methods'|'instance_variables': [(name, line)]}}."""
    out = {}
    name_re = re.compile(r'''\s*-\s*(?:def_name|var_name):\s*['"]?([\w.]+)['"]?\s*$''')
    for f in sorted(os.listdir(yml_dir)):
        if not f.endswith('.yml'):
            continue
        cls, section = None, None
        for n, line in enumerate(
                open(os.path.join(yml_dir, f), encoding='utf-8', errors='replace'), 1):
            mc = re.match(r'\s*-\s*class_name:\s*([\w.]+)\s*$', line)
            if mc:
                cls = mc.group(1).rsplit('.', 1)[-1]
                section = None
                out[cls] = {'file': f, 'line': n, 'sections': {}}
                continue
            ms = re.match(r'\s*(methods|instance_variables|dunder_methods|getters|setters):\s*$', line)
            if ms and cls:
                section = ms.group(1)
                out[cls]['sections'].setdefault(section, [])
                continue
            mn = name_re.match(line)
            if mn and cls and section:
                out[cls]['sections'][section].append((mn.group(1), n))
    return out


def neighbours(entries, name):
    """The entries a new `name` sorts between, so the insert keeps the order.

    Dunders are skipped: the ymls list them last, not alphabetically.
    """
    plain = [e for e in entries if not e[0].startswith('__')]
    before = [e for e in plain if e[0] < name]
    after = [e for e in plain if e[0] > name]
    return (max(before) if before else None), (min(after) if after else None)


def report(target, bindings, ymls):
    cls, _, member = target.partition('.')
    member = member or None
    print('=' * 72)
    print(target)
    b = bindings.get((cls, member))
    if b is None:
        print('  binding : NOT FOUND -- check the spelling against the audit')
    else:
        print('  binding : PythonAPI/carla/src/%s:%d' % (b['file'], b['line']))
        if not member and b.get('cpp'):
            # The C++ type behind the registration: a std::vector<> or a *DataIn
            # pair is binding plumbing, not a class the reference should carry.
            print('  wraps   : %s' % b['cpp'])
        if member:
            print('  call    : %s' % b['call'][:300])
            if b['args']:
                print('  params  : ' + ', '.join(
                    n + (' = ' + d if d else '') for n, d in b['args']))
            else:
                print('  params  : none declared with arg(); read the C++ signature')
            print('  kind    : %s -> %s' % (
                b['kind'], 'methods' if b['kind'] == 'def' else 'instance_variables'))
    y = ymls.get(cls)
    if y is None:
        print('  yml     : class %s has NO entry in any yml -- add the class first' % cls)
        return
    print('  yml     : PythonAPI/docs/%s:%d  (class_name: %s)' % (y['file'], y['line'], cls))
    if not member:
        return
    for section, entries in y['sections'].items():
        if member in [e[0] for e in entries]:
            print('  already : documented under %s' % section)
            return
    section = 'instance_variables' if b and b.get('kind') != 'def' else 'methods'
    entries = y['sections'].get(section)
    if entries is None:
        print('  insert  : no `%s:` section yet -- create it under the class' % section)
        return
    prev, nxt = neighbours(entries, member)
    print('  insert  : into `%s:`%s' % (section, '' if nxt else ' at the end, before any dunder'))
    if prev:
        print('            after  %-28s (line %d)' % (prev[0], prev[1]))
    if nxt:
        print('            before %-28s (line %d)' % (nxt[0], nxt[1]))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root', default=os.environ.get('CARLA_ROOT', '.'))
    ap.add_argument('--from-audit', metavar='REPORT',
                    help='read every binding finding out of an audit report')
    ap.add_argument('targets', nargs='*', help='Class or Class.member')
    a = ap.parse_args()

    src_dir = os.path.join(a.root, 'PythonAPI', 'carla', 'src')
    yml_dir = os.path.join(a.root, 'PythonAPI', 'docs')
    for d in (src_dir, yml_dir):
        if not os.path.isdir(d):
            sys.exit('not a CARLA checkout: %s missing' % d)

    targets = list(a.targets)
    if a.from_audit:
        path = a.from_audit if os.path.isabs(a.from_audit) else os.path.join(a.root, a.from_audit)
        text = open(path, encoding='utf-8').read()
        section = text.split('## Python bindings absent')[-1].split('\n## ')[0]
        targets += targets_from_audit(section)
    if not targets:
        sys.exit('nothing to locate: pass targets or --from-audit')

    bindings, ymls = scan_bindings(src_dir), scan_ymls(yml_dir)
    for t in targets:
        report(t, bindings, ymls)
    print('=' * 72)
    print('%d target(s).' % len(targets))


if __name__ == '__main__':
    main()
