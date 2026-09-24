"""CineCamera optics using the existing CARLA per-view path-tracer lens API."""
import json
import math
import unreal as u


def configure_camera(camera, manifest, frames, root):
    w,h=manifest['resolution'];cc=camera.get_cine_camera_component()
    film=cc.get_editor_property('filmback');film.sensor_width=36;film.sensor_height=36*h/w
    cc.set_editor_property('filmback',film)
    lens=cc.get_editor_property('lens_settings');lens.min_focal_length=1.;lens.max_focal_length=1000.
    cc.set_editor_property('lens_settings',lens)
    cc.set_editor_property('current_focal_length',manifest['K'][0][0]*36/w)
    pp=cc.get_editor_property('post_process_settings')
    params={'path_tracing_lens_model':0}
    if manifest['camera_model']=='ftheta':
        f=manifest['ftheta'];poly=f['pixeldist_to_angle_poly']+[0.]*(8-len(f['pixeldist_to_angle_poly']))
        params.update(path_tracing_lens_model=8,
                      path_tracing_lens_focal=u.Vector4(1/w,1/h,f['principal_point_x']/w,f['principal_point_y']/h),
                      path_tracing_lens_coeffs01=u.Vector4(*poly[:4]),
                      path_tracing_lens_coeffs23=u.Vector4(*poly[4:]),
                      path_tracing_lens_theta_max=f['max_angle'])
        # FVector2f's components are not reflected in this engine's Python API.
        # Preserve the camera's native achromatic (1,1) default.
    for name,value in params.items():
        pp.set_editor_property('override_'+name,True);pp.set_editor_property(name,value)
    cc.set_editor_property('post_process_settings',pp)
    pose=frames[0]['camera'];p,y,r=pose['rotation']
    camera.set_actor_location(u.Vector(*[v*100 for v in pose['location']]),False,False)
    camera.set_actor_rotation(u.Rotator(pitch=p,yaw=y,roll=r),False)
    # Validate actual serialized per-camera state, not global lens CVars.
    readback=cc.get_editor_property('post_process_settings')
    report={'camera_model':manifest['camera_model'],'backend':'CARLA RTLensEngineAdapter / Engine LensModels.ush',
            'settings':{name:str(readback.get_editor_property(name)) for name in params},
            'passed':all(readback.get_editor_property('override_'+name) for name in params),
            'scope':'per-view lens configuration readback; rendered-marker gate separate'}
    if readback.path_tracing_lens_model!=params['path_tracing_lens_model']:raise RuntimeError('Lens mode readback differs')
    (root/'camera-configuration-validation.json').write_text(json.dumps(report,indent=2))
