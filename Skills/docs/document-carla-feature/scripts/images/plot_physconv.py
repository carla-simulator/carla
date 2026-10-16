"""plot_physconv.py PHYSCONV.json OUTDIR: speed (and vertical acceleration) curves of physconv.py runs, in the svgkit palette."""
import json, sys
import matplotlib; matplotlib.use('Agg'); import matplotlib.pyplot as plt
D = json.load(open(sys.argv[1])); out = sys.argv[2]
COL = ['#2980b9', '#3c9a5f', '#e69138', '#d9534f']
plt.rcParams.update({'font.family': 'DejaVu Sans', 'font.size': 11, 'axes.edgecolor': '#c9d3da', 'axes.labelcolor': '#2b2b2b',
                     'xtick.color': '#5f6b73', 'ytick.color': '#5f6b73', 'axes.grid': True, 'grid.color': '#e6ecef'})
def fig(keys, labels, idx, ylabel, title, name, xlim=(0, 14), ylim=None):
    f, ax = plt.subplots(figsize=(9, 4.2), dpi=130)
    for k, lab, col in zip(keys, labels, COL):
        r = D[k]; ax.plot([x[0] for x in r], [x[idx] for x in r], color=col, lw=1.8, label=lab)
    ax.set_xlabel('simulation time (s)'); ax.set_ylabel(ylabel); ax.set_xlim(*xlim)
    if ylim: ax.set_ylim(*ylim)
    ax.set_title(title, fontsize=13, color='#2b2b2b', loc='left'); ax.legend(frameon=False)
    for s in ('top', 'right'): ax.spines[s].set_visible(False)
    f.tight_layout(); f.savefig(f'{out}/{name}.png'); plt.close(f)
fig(['0.01_0.01', '0.02_0.01', '0.04_0.01', '0.05_0.01'], ['fixed_delta_seconds 0.01', '0.02', '0.04', '0.05'], 1,
    'speed (m/s)', 'Speed with max_substep_delta_time = 0.01', 'physics_convergence_fixed_pdt')
fig(['0.04_0.005', '0.04_0.01', '0.04_0.02', '0.04_0.04'], ['max_substep_delta_time 0.005', '0.01', '0.02', '0.04'], 1,
    'speed (m/s)', 'Speed with fixed_delta_seconds = 0.04', 'physics_convergence_fixed_dt')
fig(['0.04_0.005', '0.04_0.01', '0.04_0.02', '0.04_0.04'], ['max_substep_delta_time 0.005', '0.01', '0.02', '0.04'], 2,
    'vertical acceleration (m/s²)', 'Vertical acceleration with fixed_delta_seconds = 0.04', 'physics_convergence_z_acceleration', xlim=(0, 14))
