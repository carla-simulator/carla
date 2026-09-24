"""Composite MRG layers with NuRec in ACEScg. Never infer source color silently."""
import argparse,json,re
from pathlib import Path
import numpy as np
from PIL import Image
from core import read_exr,write_exr,to_acescg,merge,SRGB_TO_ACESCG,validate_manifest,transfer_receiver_delta


def rgb(channels):return np.stack([channels[x] for x in 'RGB'],axis=-1)

def find_pass(root,layer,frame,utility=False):
    hits=[p for p in root.rglob('*.exr') if layer in p.name and re.search(rf'(?<!\d){frame:04d}(?!\d)',p.name) and ('depth_m' in p.name)==utility]
    if len(hits)!=1:raise ValueError(f'Expected one {layer} pass for {frame}; found {hits}')
    return hits[0]


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('shot',type=Path)
    ap.add_argument('--preview-srgb-assumption',action='store_true',help='Explicitly permit an approximate display-referred NuRec preview; color gate remains unpassed')
    ap.add_argument('--limit',type=int,default=48);ap.add_argument('--depth-tolerance',type=float)
    ap.add_argument('--start-frame',type=int)
    a=ap.parse_args();m=validate_manifest(json.loads((a.shot/'manifest.json').read_text()));space=m['nurec_color_space']
    start=m['frame_start'] if a.start_frame is None else a.start_frame
    if start<m['frame_start'] or not 1<=a.limit<=m['frame_end']-start+1:ap.error('limit exceeds shot range')
    if a.preview_srgb_assumption:space='srgb_display_assumed'
    to_acescg(np.zeros((1,1,3)),space)  # fail before producing partial output
    report={'source_color_space':space,'production_color_gate_passed':space in ('linear_srgb','ACEScg'),'frames':[]}
    for frame in range(start,start+a.limit):
        gs=read_exr(a.shot/f'renders/gs_beauty/{frame}.exr');gz=read_exr(a.shot/f'renders/gs_depth/{frame}.exr')['Z']
        base=a.shot/'renders/ue_review'
        fg=read_exr(find_pass(base,'beauty',frame));full=read_exr(find_pass(base,'integration',frame));clean=read_exr(find_pass(base,'clean',frame))
        utility=read_exr(find_pass(base,'integration',frame,utility=True))
        depth=utility['R']
        plate=to_acescg(rgb(gs),space);beauty=to_acescg(rgb(fg),'linear_srgb');alpha=np.clip(fg['A'],0,1)
        bg=to_acescg(rgb(clean),'linear_srgb');integrated=to_acescg(rgb(full),'linear_srgb')
        if beauty.shape!=plate.shape:raise ValueError('Render resolutions differ')
        if alpha.min()>.99 or ((alpha>.01).sum()<16 and not m.get('allow_empty_foreground',False)):
            raise ValueError('Missing foreground alpha: verify MoviePipeline.AlphaOutputOverride and holdout configuration')
        if not np.allclose(depth,utility['G'],atol=1e-5) or not np.allclose(depth,utility['B'],atol=1e-5):
            raise ValueError('Depth pass is not scalar data; check post-process material blendable location')
        if not np.isfinite(depth).all() or ((alpha>.8).any() and np.median(depth[alpha>.8])<.5):
            raise ValueError('Implausible foreground depth for this shot')
        # Paired proxy renders retain signed light transport (including reflections).
        # Extract only the receiver contribution; exclude near-opaque actor pixels.
        receiver_delta=integrated-beauty-bg*(1-alpha[...,None])
        receiver_delta=np.where((alpha<.99)[...,None],receiver_delta,0)
        # Transfer interaction relative to the receiver's illumination. The
        # proxy's material/exposure differs from the photographed asphalt;
        # adding its absolute radiance difference can erase contact shadows.
        # Preserve signed RGB differences (including positive reflected light),
        # with a bounded luminance scale rather than a multiply-black layer.
        receiver_delta=transfer_receiver_delta(receiver_delta,plate,bg)
        if m.get('match_shadow_color_to_plate',False):
            # Grade the rendered shadow field against local asphalt color;
            # retain positive reflection deltas instead of a black multiplier.
            luma=np.array([.2722287,.6740818,.0536895])
            signed_luma=receiver_delta@luma
            chromaticity=plate/np.maximum((plate@luma)[...,None],.005)
            receiver_delta=np.where((signed_luma<0)[...,None],chromaticity*signed_luma[...,None],receiver_delta)

        receiver=(clean['A']>.5)&np.isfinite(depth)&(depth>0)
        agreement=np.clip(1-np.abs(gz-depth)/.5,0,1)*gs['A']*receiver
        receiver_delta*=agreement[...,None]
        tolerance=a.depth_tolerance if a.depth_tolerance is not None else m.get('depth_tolerance_m',.15)
        result=merge(plate,gs['A'],gz,beauty,alpha,depth,tolerance=tolerance,depth_bias=m.get('depth_bias_m',0.))
        result+=receiver_delta
        meta={'color_space':'ACEScg','frame':frame,'nurec_conversion':space,'integration':'signed paired proxy difference matched to local plate luminance','depth':'optical_z_metres','lens_effects':'none'}
        write_exr(a.shot/f'comp/output/{frame}.exr',dict(zip('RGB',result.transpose(2,0,1))),half=True,metadata=meta)
        write_exr(a.shot/f'renders/ue_utility/{frame}.exr',{'Z':depth,'hero_matte':alpha},metadata=meta)
        display=np.maximum(result@np.linalg.inv(SRGB_TO_ACESCG).T,0)
        display=np.where(display<=.0031308,12.92*display,1.055*display**(1/2.4)-.055)
        path=a.shot/f'comp/preview/{frame}.png';path.parent.mkdir(parents=True,exist_ok=True)
        Image.fromarray(np.clip(display*255,0,255).astype('uint8')).save(path)
        report['frames'].append({'frame':frame,'actor_pixels':int((alpha>.01).sum()),'soft_alpha_pixels':int(((alpha>.01)&(alpha<.99)).sum()),'hdr_max':float(result.max()),'proxy_agreement_pixels':int((agreement>.5).sum())})
    (a.shot/'composite-report.json').write_text(json.dumps(report,indent=2))

if __name__=='__main__':main()
