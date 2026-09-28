#!/usr/bin/env python3
"""Static scan of UE5 external-actor packages: parse FPackageFileSummary name/import/export tables and
report every import of class MaterialInstanceDynamic together with its outer chain, plus the actor class.
Usage: uasset_imports.py <root dir> [--only-mid] [--tsv out]"""
import struct, sys, os, re, json
def rd_fstring(b, o):
    n, = struct.unpack_from('<i', b, o); o += 4
    if n == 0: return '', o
    if n < 0:
        n = -n; s = b[o:o+2*n].decode('utf-16-le', 'replace').rstrip('\0'); return s, o+2*n
    return b[o:o+n].decode('utf-8', 'replace').rstrip('\0'), o+n
def parse(path):
    b = open(path, 'rb').read()
    o = 0
    tag, legacy = struct.unpack_from('<Ii', b, o); o += 8
    assert tag == 0x9E2A83C1, 'bad tag'
    if legacy != -4: o += 4          # LegacyUE3Version
    ue4, = struct.unpack_from('<i', b, o); o += 4
    ue5 = 0
    if legacy <= -8: ue5, = struct.unpack_from('<i', b, o); o += 4
    o += 4                            # licensee
    if ue5 >= 1016:                   # PACKAGE_SAVED_HASH: FIoHash(20) + TotalHeaderSize
        o += 20; hdr, = struct.unpack_from('<i', b, o); o += 4
    if legacy <= -2:
        n, = struct.unpack_from('<i', b, o); o += 4 + 20*n   # custom versions (Optimized format)
    if ue5 < 1016:
        hdr, = struct.unpack_from('<i', b, o); o += 4
    pkgname, o = rd_fstring(b, o)
    flags, = struct.unpack_from('<I', b, o); o += 4
    namecount, nameoff = struct.unpack_from('<ii', b, o); o += 8
    if ue5 >= 1008: o += 8            # soft object paths count/offset
    if ue4 >= 516: _, o = rd_fstring(b, o)   # localization id
    if ue4 >= 459: o += 8             # gatherable text
    expcount, expoff, impcount, impoff = struct.unpack_from('<iiii', b, o)
    # names
    names = []; p = nameoff
    for _ in range(namecount):
        s, p = rd_fstring(b, p)
        if ue4 >= 504: p += 4
        names.append(s)
    def fname(p):
        idx, num = struct.unpack_from('<ii', b, p)
        n = names[idx] if 0 <= idx < len(names) else '?%d' % idx
        return (n + '_' + str(num-1)) if num else n
    imps = []; p = impoff
    for _ in range(impcount):
        cp = fname(p); cn = fname(p+8); outer, = struct.unpack_from('<i', b, p+16); on = fname(p+20)
        imps.append((cp, cn, outer, on)); p += 40 if ue5 else 28
    unversioned = bool(flags & 0x2000)   # PKG_UnversionedProperties
    exps = []; p = expoff
    # FObjectExport (ObjectResource.cpp, operator<<(FStructuredArchive::FSlot, FObjectExport&)):
    # ClassIndex SuperIndex TemplateIndex OuterIndex (int32 each) ObjectName(FName 8) ObjectFlags(4)
    # SerialSize SerialOffset (int64 each) bForcedExport bNotForClient bNotForServer (bool as 4 bytes each)
    # [PackageGuid 16 if ue5 < 1005 REMOVE_OBJECT_EXPORT_PACKAGE_GUID] [bIsInheritedInstance 4 if ue5 >= 1006]
    # PackageFlags(4) bNotAlwaysLoadedForEditorGame(4) bIsAsset(4) [bGeneratePublicHash 4 if ue5 >= 1003]
    # FirstExportDependency + 4 dependency counts (5 x int32)
    # [ScriptSerializationStart/EndOffset 2 x int64 if versioned properties and ue5 >= 1010]
    for _ in range(expcount):
        ci, si, ti, oi = struct.unpack_from('<iiii', b, p); on = fname(p+16)
        flags_, = struct.unpack_from('<I', b, p+24)
        ssize, soff = struct.unpack_from('<qq', b, p+28)
        q = p + 28 + 16 + 12
        if ue5 < 1005: q += 16
        if ue5 >= 1006: q += 4
        q += 4 + 4 + 4
        if ue5 >= 1003: q += 4
        q += 20
        if (not unversioned) and ue5 >= 1010: q += 16
        exps.append(dict(cls=ci, super=si, tmpl=ti, outer=oi, name=on, flags=flags_, size=ssize, off=soff))
        p = q
    return dict(ue4=ue4, ue5=ue5, pkg=pkgname, names=names, imports=imps, exports=exps, exp_count=expcount, exp_off=expoff, raw=b, fname=fname, unversioned=unversioned)
def ref(imps, i):
    # i is packed index: negative -> import (-i-1); positive -> export; 0 -> null
    if i < 0: return imps[-i-1]
    return None
def idx_name(P, i):
    # packed index -> readable: import chain, export name, or null
    if i < 0: return chain(P['imports'], i)
    if i > 0:
        e = P['exports'][i-1]
        return '<export %s (%s)>' % (e['name'], idx_name(P, e['cls']) if e['cls'] != i else '?')
    return '<null>'
def chain(imps, i):
    out = []
    while i < 0:
        cp, cn, outer, on = imps[-i-1]; out.append('%s(%s)' % (on, cn)); i = outer
    return '.'.join(reversed(out)) if out else ('<export>' if i > 0 else '<null>')
def main():
    args = sys.argv[1:]; only_mid = '--only-mid' in args; tsv = None
    if '--materials' in args: return materials_main(args)
    if '--tsv' in args: tsv = args[args.index('--tsv')+1]
    root = args[0]
    rows = []
    files = []
    for dp, dn, fn in os.walk(root):
        for f in fn:
            if f.endswith('.uasset'): files.append(os.path.join(dp, f))
    files.sort()
    for f in files:
        try:
            P = parse(f)
        except Exception as e:
            rows.append((f, 'PARSE_ERROR', str(e), '', '')); continue
        if only_mid and 'MaterialInstanceDynamic' not in P['names']: continue
        imps = P['imports']
        # actor class: import whose class is 'Class' and is not a component class; use ActorMetaDataClass name string instead
        names = P['names']
        cls = ''
        try:
            k = names.index('ActorMetaDataClass'); 
        except ValueError: k = -1
        # class via import list: first import with ClassName 'Class' from a /Script or /Game package, excluding component/known engine classes? use all
        classes = [on for cp, cn, outer, on in imps if cn == 'Class' or cn == 'BlueprintGeneratedClass']
        mid_imps = [(cp, cn, outer, on) for cp, cn, outer, on in imps if cn == 'MaterialInstanceDynamic']
        mid_exports = 'MaterialInstanceDynamic' in names and not mid_imps
        for cp, cn, outer, on in mid_imps:
            rows.append((f, ','.join(classes), on, chain(imps, outer), 'import'))
        if not mid_imps:
            rows.append((f, ','.join(classes), '', '', 'export-or-none' if 'MaterialInstanceDynamic' in names else 'none'))
    if tsv:
        with open(tsv, 'w') as fh:
            for r in rows: fh.write('\t'.join(r) + '\n')
    import collections
    c = collections.Counter((r[4], r[3]) for r in rows)
    for k, v in sorted(c.items(), key=lambda x: -x[1]): print(v, k)
    cc = collections.Counter((r[1], r[4]) for r in rows)
    print('--- by class/kind'); 
    for k, v in sorted(cc.items(), key=lambda x: -x[1]): print(v, k)
    print('files scanned:', len(files), 'rows:', len(rows))
def materials_main(args):
    """Per package: actor class, every imported material-like asset (Material / MaterialInstanceConstant / MaterialInstanceDynamic
    with its outer chain) and every exported MaterialInstanceDynamic (name, outer export, and the imports it can only have as
    parent).  Optional --only-class <name> filters by actor class import, --tsv out writes rows."""
    import collections
    root = args[0]; tsv = None
    if '--tsv' in args: tsv = args[args.index('--tsv')+1]
    only_class = args[args.index('--only-class')+1] if '--only-class' in args else None
    files = []
    for dp, dn, fn in os.walk(root):
        for f in fn:
            if f.endswith('.uasset'): files.append(os.path.join(dp, f))
    files.sort()
    rows = []; profiles = collections.Counter(); parse_err = 0
    MATCLS = ('Material', 'MaterialInstanceConstant', 'MaterialInstanceDynamic', 'MaterialInstance')
    for f in files:
        try: P = parse(f)
        except Exception as e:
            parse_err += 1; continue
        imps = P['imports']
        classes = [on for cp, cn, outer, on in imps if cn in ('Class', 'BlueprintGeneratedClass')]
        if only_class and only_class not in classes: continue
        mat_imps = ['%s:%s' % (cn, chain(imps, -(k+1))) for k, (cp, cn, outer, on) in enumerate(imps) if cn in MATCLS]
        mid_exps = []
        for k, e in enumerate(P['exports']):
            if idx_name(P, e['cls']).endswith('MaterialInstanceDynamic(Class)'):
                mid_exps.append('%s@%s' % (e['name'], idx_name(P, e['outer'])))
        # profile key: sorted material import kinds (class + asset name) and export count
        prof = ('mids_exported=%d' % len(mid_exps), '|'.join(sorted(m.split(':')[0] + ':' + m.split('.')[-1].split('(')[0] for m in mat_imps)))
        profiles[prof] += 1
        rows.append((f, ','.join(classes), ';'.join(mat_imps), ';'.join(mid_exps)))
    if tsv:
        with open(tsv, 'w') as fh:
            for r in rows: fh.write('\t'.join(r) + '\n')
    for k, v in sorted(profiles.items(), key=lambda x: -x[1]): print(v, k)
    print('files scanned:', len(files), 'rows:', len(rows), 'parse errors:', parse_err)
if __name__ == '__main__': main()
