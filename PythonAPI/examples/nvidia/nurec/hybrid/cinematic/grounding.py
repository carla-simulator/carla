"""Create a reviewable mesh-height correction from a completed raw render.

Requires an uncorrected render. This is a cinematic derivative, not simulation
replay truth or foot IK. Review the rerender before accepting foot contact.
"""
import argparse
import json
from pathlib import Path


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('shot',type=Path)
    args=ap.parse_args();shot=args.shot
    if (shot/'grounding.json').exists():raise ValueError('Existing grounding is protected; do not derive offsets from an already corrected render')
    report=json.loads((shot/'ue-pass-validation.json').read_text())
    manifest=json.loads((shot/'manifest.json').read_text())
    if not report['artifact_passes_valid'] or len(report['frames'])!=48:
        raise ValueError('A validated 48-frame raw render is required')
    offsets={str(f['frame']):min(0.,.005-f['minimum_visible_surface_above_road_m']) for f in report['frames']}
    if any(abs(x)>.1 for x in offsets.values()):raise ValueError('Correction exceeds 10 cm; inspect geometry instead')
    data={'method':'render-derived vertical mesh alignment to fitted road; original simulation capture unchanged',
          'source_capture_sha256':manifest['capture_sha256'],'target_visible_clearance_m':.005,
          'offset_z_m_by_frame':offsets,'contact_gate':'pending rerender, not foot IK'}
    (shot/'grounding.json').write_text(json.dumps(data,indent=2))


if __name__=='__main__':main()
