"""plot_telemetry.py TELEMETRY.json OUT.png: controls, speed, engine RPM and gear, and wheel omega from telemetry_run.py."""
import json, sys
import matplotlib; matplotlib.use('Agg'); import matplotlib.pyplot as plt
R = json.load(open(sys.argv[1])); t = [r['t'] for r in R]
C = ['#2980b9', '#3c9a5f', '#e69138', '#d9534f', '#7e57c2']
plt.rcParams.update({'font.family': 'DejaVu Sans', 'font.size': 10.5, 'axes.edgecolor': '#c9d3da', 'xtick.color': '#5f6b73',
                     'ytick.color': '#5f6b73', 'axes.grid': True, 'grid.color': '#e6ecef'})
f, ax = plt.subplots(4, 1, figsize=(9, 9), dpi=130, sharex=True)
ax[0].plot(t, [r['throttle'] for r in R], color=C[1], label='throttle'); ax[0].plot(t, [r['brake'] for r in R], color=C[3], label='brake')
ax[0].plot(t, [r['steer'] for r in R], color=C[4], label='steer'); ax[0].set_ylabel('control'); ax[0].legend(frameon=False, ncol=3, loc='upper right')
ax[1].plot(t, [r['speed'] for r in R], color=C[0]); ax[1].set_ylabel('speed (m/s)')
a2 = ax[2]; a2.plot(t, [r['rpm'] for r in R], color=C[2]); a2.set_ylabel('engine_rpm')
g = a2.twinx(); g.step(t, [r['gear'] for r in R], color='#5f6b73', lw=1.2, where='post'); g.set_ylabel('gear', color='#5f6b73'); g.grid(False)
for i in range(4): ax[3].plot(t, [r['omega'][i] for r in R], color=C[i], lw=1.3, label=f'wheels[{i}]')
ax[3].set_ylabel('wheel omega'); ax[3].legend(frameon=False, ncol=4, loc='upper left'); ax[3].set_xlabel('simulation time (s)')
for a in ax:
    for s in ('top', 'right'): a.spines[s].set_visible(False)
ax[0].set_title('Vehicle.get_telemetry_data(): launch, lane change, emergency stop', loc='left', fontsize=13, color='#2b2b2b')
f.tight_layout(); f.savefig(sys.argv[2])
