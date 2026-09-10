/* Demo scene viewer.

   The scene description comes from /api/scenes/:id — ego and actor tracks,
   road geometry, events. The lidar is not shipped: sending a point cloud for
   every frame would be tens of megabytes, so the returns are simulated here,
   in the browser, by casting rays from the ego against the same geometry the
   API sent. That keeps the payload at a few hundred kB and makes the scan
   respond to the sensor model — fog and rain visibly thin it out.

   Rendered with deck.gl in an OrbitView, so coordinates are plain metres on a
   local grid rather than lon/lat. */
window.AtlasScenes = (function () {
  "use strict";

  const SENSOR_HEIGHT_M = 1.8;
  const RINGS = 32;
  /** Elevation sweep of the simulated scanner, radians. */
  const ELEVATION = { from: (-24 * Math.PI) / 180, to: (2.5 * Math.PI) / 180 };

  const COLOR = {
    ego: [42, 120, 214],
    tracked: [27, 175, 122],
    vru: [214, 58, 58],
    missed: [138, 138, 132],
    road: [96, 110, 126],
    verge: [122, 104, 74],
    hit: [235, 176, 60],
    path: [42, 120, 214],
    trail: [235, 104, 52],
  };
  const VRU = { pedestrian: true, cyclist: true, motorcycle: true };

  const state = {
    catalog: [],
    scene: null,
    frame: 0,
    playing: false,
    raf: null,
    last: 0,
    deck: null,
    lidar: true,
    follow: true,
    camera: null,
  };

  const $ = (sel) => document.querySelector(sel);
  const esc = (s) => String(s ?? "").replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));

  /** Deterministic PRNG, so a frame always scans the same way. */
  function mulberry32(seed) {
    let a = seed >>> 0;
    return function () {
      a = (a + 0x6d2b79f5) >>> 0;
      let t = Math.imul(a ^ (a >>> 15), 1 | a);
      t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
      return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
    };
  }

  // ----------------------------------------------------------------- geometry

  /** Bounding sphere of a box, for the cheap rejection test in scan(). */
  const bound = (hl, hw, hh) => {
    const r2 = hl * hl + hw * hw + hh * hh;
    return { r2, radius: Math.sqrt(r2) };
  };

  /** Oriented box for one actor at a frame, in world metres. */
  function actorBox(actor, frame) {
    const pose = actor.track[Math.min(frame, actor.track.length - 1)];
    return {
      x: pose.x, y: pose.y, z: actor.height_m / 2,
      hl: actor.length_m / 2, hw: actor.width_m / 2, hh: actor.height_m / 2,
      heading: pose.heading,
      cos: Math.cos(-pose.heading), sin: Math.sin(-pose.heading),
      ...bound(actor.length_m / 2, actor.width_m / 2, actor.height_m / 2),
      actor,
      pose,
    };
  }

  /** Delineator posts down the outer verge — static lidar targets that make the road read as a road. */
  function posts(scene) {
    const out = [];
    const line = scene.road.centerline;
    const edge = (scene.road.lanes * scene.road.lane_width_m) / 2 + 1.2;
    for (let i = 4; i < line.length - 1; i += 4) {
      const a = line[i];
      const b = line[i + 1];
      const h = Math.atan2(b.y - a.y, b.x - a.x);
      for (const side of [edge, -edge]) {
        out.push({ x: a.x - Math.sin(h) * side, y: a.y + Math.cos(h) * side, z: 0.5, hl: 0.06, hw: 0.06, hh: 0.5, heading: h, cos: Math.cos(-h), sin: Math.sin(-h), ...bound(0.06, 0.06, 0.5), post: true });
      }
    }
    return out;
  }

  /**
   * Slab test: nearest positive hit of a ray against an oriented box, or
   * Infinity. Written out per axis rather than looping over arrays — this runs
   * a few hundred thousand times per frame, and the allocations a loop needs
   * cost more than the arithmetic does.
   */
  function rayBox(ox, oy, oz, dx, dy, dz, box) {
    // Boxes built by actorBox/posts carry their rotation precomputed; a bare
    // box (as in the tests) falls back to computing it here.
    const c = box.cos !== undefined ? box.cos : Math.cos(-box.heading);
    const s = box.sin !== undefined ? box.sin : Math.sin(-box.heading);
    // Into the box's frame.
    const ex = ox - box.x;
    const ey = oy - box.y;
    const px = ex * c - ey * s;
    const py = ex * s + ey * c;
    const pz = oz - box.z;
    const vx = dx * c - dy * s;
    const vy = dx * s + dy * c;

    let near = -Infinity;
    let far = Infinity;
    let t1;
    let t2;
    let swap;

    if (vx > -1e-9 && vx < 1e-9) {
      if (px < -box.hl || px > box.hl) return Infinity;
    } else {
      t1 = (-box.hl - px) / vx;
      t2 = (box.hl - px) / vx;
      if (t1 > t2) { swap = t1; t1 = t2; t2 = swap; }
      if (t1 > near) near = t1;
      if (t2 < far) far = t2;
      if (near > far) return Infinity;
    }

    if (vy > -1e-9 && vy < 1e-9) {
      if (py < -box.hw || py > box.hw) return Infinity;
    } else {
      t1 = (-box.hw - py) / vy;
      t2 = (box.hw - py) / vy;
      if (t1 > t2) { swap = t1; t1 = t2; t2 = swap; }
      if (t1 > near) near = t1;
      if (t2 < far) far = t2;
      if (near > far) return Infinity;
    }

    if (dz > -1e-9 && dz < 1e-9) {
      if (pz < -box.hh || pz > box.hh) return Infinity;
    } else {
      t1 = (-box.hh - pz) / dz;
      t2 = (box.hh - pz) / dz;
      if (t1 > t2) { swap = t1; t1 = t2; t2 = swap; }
      if (t1 > near) near = t1;
      if (t2 < far) far = t2;
      if (near > far) return Infinity;
    }

    return near > 0 ? near : Infinity;
  }

  /**
   * Simulate one scan. Rays sweep elevation in rings and azimuth around the
   * ego, and stop at the nearest of the ground plane, an actor or a post.
   */
  function scan(scene, frame) {
    const ego = scene.ego.track[Math.min(frame, scene.ego.track.length - 1)];
    const range = scene.sensor.lidar_range_m;
    const reach = (range + 12) ** 2;
    const boxes = scene.actors
      .map((a) => actorBox(a, frame))
      .concat(scene.postCache || [])
      .filter((b) => (b.x - ego.x) ** 2 + (b.y - ego.y) ** 2 <= reach);
    const azimuths = Math.max(24, Math.round(scene.sensor.lidar_points_per_frame / RINGS));
    const rnd = mulberry32(scene.seed + frame * 7919);
    const halfRoad = (scene.road.lanes * scene.road.lane_width_m) / 2;

    const positions = [];
    const colors = [];
    const oz = SENSOR_HEIGHT_M;
    for (let r = 0; r < RINGS; r += 1) {
      const elev = ELEVATION.from + ((ELEVATION.to - ELEVATION.from) * r) / (RINGS - 1);
      const dz = Math.sin(elev);
      const horizontal = Math.cos(elev);
      for (let a = 0; a < azimuths; a += 1) {
        // Jitter the azimuth so rings do not moiré into visible spokes.
        const az = ((a + rnd() * 0.6) / azimuths) * Math.PI * 2;
        const dx = Math.cos(az) * horizontal;
        const dy = Math.sin(az) * horizontal;

        let t = dz < -1e-6 ? oz / -dz : Infinity;
        let kind = "ground";
        for (let i = 0; i < boxes.length; i += 1) {
          const b = boxes[i];
          // Reject against the bounding sphere first: a handful of flops rather
          // than the full slab test, and most rays miss most boxes — there are
          // far more verge posts than there are things a given ray can hit.
          const wx = b.x - ego.x;
          const wy = b.y - ego.y;
          const wz = b.z - oz;
          const proj = wx * dx + wy * dy + wz * dz;
          if (proj <= 0 || proj - b.radius > t) continue;
          if (wx * wx + wy * wy + wz * wz - proj * proj > b.r2) continue;
          const hit = rayBox(ego.x, ego.y, oz, dx, dy, dz, b);
          if (hit < t) { t = hit; kind = b.post ? "post" : "actor"; }
        }
        if (!isFinite(t) || t > range) continue;
        // Returns thin out with range, and rain and fog drop them outright.
        if (rnd() > 1 - (t / range) * 0.55) continue;

        const noise = scene.sensor.noise_m * (rnd() + rnd() + rnd() - 1.5);
        const d = t + noise;
        const x = ego.x + dx * d;
        const y = ego.y + dy * d;
        const z = Math.max(0, oz + dz * d);
        positions.push(x, y, z);

        let color = COLOR.hit;
        if (kind === "ground") {
          // Colour the carriageway apart from the verge using distance to the ego's lateral band.
          color = Math.abs(offsetFromRoad(scene, x, y)) <= halfRoad ? COLOR.road : COLOR.verge;
        }
        const shade = 1 - (t / range) * 0.45;
        colors.push(color[0] * shade, color[1] * shade, color[2] * shade);
      }
    }
    return { positions: new Float32Array(positions), colors: new Uint8Array(colors), count: positions.length / 3 };
  }

  const CELL_M = 8;
  const cellKey = (x, y) => Math.floor(x / CELL_M) + ":" + Math.floor(y / CELL_M);

  /**
   * Bucket centreline vertices into an 8 m grid, registering each into its own
   * cell and the eight around it. A return then only has to test the handful of
   * vertices in its own cell — a linear scan of the whole centreline for every
   * one of a few thousand returns is what made this too slow to animate.
   */
  function roadIndex(scene) {
    const index = new Map();
    const line = scene.road.centerline;
    for (let i = 0; i < line.length; i += 1) {
      const cx = Math.floor(line[i].x / CELL_M);
      const cy = Math.floor(line[i].y / CELL_M);
      for (let dx = -1; dx <= 1; dx += 1) {
        for (let dy = -1; dy <= 1; dy += 1) {
          const key = cx + dx + ":" + (cy + dy);
          let bucket = index.get(key);
          if (!bucket) index.set(key, (bucket = []));
          bucket.push(i);
        }
      }
    }
    return index;
  }

  /**
   * Signed distance from the road centreline, using the nearest sampled vertex.
   * Returns Infinity well away from the road, which reads as verge.
   */
  function offsetFromRoad(scene, x, y) {
    const line = scene.road.centerline;
    const bucket = (scene.roadIndex || (scene.roadIndex = roadIndex(scene))).get(cellKey(x, y));
    if (!bucket) return Infinity;
    let best = Infinity;
    let idx = -1;
    for (let k = 0; k < bucket.length; k += 1) {
      const i = bucket[k];
      const d = (line[i].x - x) ** 2 + (line[i].y - y) ** 2;
      if (d < best) { best = d; idx = i; }
    }
    if (idx < 0) return Infinity;
    const a = line[Math.max(0, idx - 1)];
    const b = line[Math.min(line.length - 1, idx + 1)];
    const h = Math.atan2(b.y - a.y, b.x - a.x);
    return -Math.sin(h) * (x - line[idx].x) + Math.cos(h) * (y - line[idx].y);
  }

  /** Corners of an oriented box, for an extruded deck.gl polygon. */
  function footprint(box) {
    const c = Math.cos(box.heading);
    const s = Math.sin(box.heading);
    return [[box.hl, box.hw], [box.hl, -box.hw], [-box.hl, -box.hw], [-box.hl, box.hw]].map(([u, v]) => [box.x + u * c - v * s, box.y + u * s + v * c]);
  }

  /** Left and right kerb lines, offset from the centreline. */
  function roadPolygon(scene) {
    const line = scene.road.centerline;
    const half = (scene.road.lanes * scene.road.lane_width_m) / 2;
    const left = [];
    const right = [];
    for (let i = 0; i < line.length; i += 1) {
      const a = line[Math.max(0, i - 1)];
      const b = line[Math.min(line.length - 1, i + 1)];
      const h = Math.atan2(b.y - a.y, b.x - a.x);
      left.push([line[i].x - Math.sin(h) * half, line[i].y + Math.cos(h) * half]);
      right.push([line[i].x + Math.sin(h) * half, line[i].y - Math.cos(h) * half]);
    }
    return left.concat(right.reverse());
  }

  const range = (scene, actor, frame) => {
    const e = scene.ego.track[Math.min(frame, scene.ego.track.length - 1)];
    const p = actor.track[Math.min(frame, actor.track.length - 1)];
    return Math.hypot(p.x - e.x, p.y - e.y);
  };

  const detected = (scene, actor, frame) => range(scene, actor, frame) <= scene.sensor.lidar_range_m;

  // ------------------------------------------------------------------ layers

  function layers() {
    const scene = state.scene;
    const frame = state.frame;
    const ego = scene.ego.track[Math.min(frame, scene.ego.track.length - 1)];
    const boxes = scene.actors.map((a) => actorBox(a, frame));

    const out = [
      new deck.PolygonLayer({
        id: "carriageway",
        data: [roadPolygon(scene)],
        getPolygon: (d) => d,
        getFillColor: [58, 58, 56, 170],
        getLineColor: [150, 150, 144, 200],
        lineWidthMinPixels: 1,
        stroked: true,
        filled: true,
      }),
      new deck.PathLayer({
        id: "centreline",
        data: [scene.road.centerline.map((p) => [p.x, p.y, 0.02])],
        getPath: (d) => d,
        getColor: [225, 214, 160, 190],
        getWidth: 0.16,
        widthMinPixels: 1,
      }),
      new deck.PathLayer({
        id: "ego-route",
        data: [scene.ego.track.map((p) => [p.x, p.y, 0.05])],
        getPath: (d) => d,
        getColor: COLOR.path.concat(120),
        getWidth: 0.35,
        widthMinPixels: 2,
      }),
      new deck.PathLayer({
        id: "ego-trail",
        data: [scene.ego.track.slice(0, frame + 1).map((p) => [p.x, p.y, 0.08])],
        getPath: (d) => d,
        getColor: COLOR.trail,
        getWidth: 0.6,
        widthMinPixels: 3,
      }),
    ];

    if (state.lidar) {
      const cloud = scan(scene, frame);
      out.push(
        new deck.PointCloudLayer({
          id: "lidar",
          data: { length: cloud.count, attributes: { getPosition: { value: cloud.positions, size: 3 }, getColor: { value: cloud.colors, size: 3 } } },
          pointSize: 1.7,
          opacity: 0.9,
          material: false,
        }),
      );
    }

    out.push(
      new deck.PolygonLayer({
        id: "actors",
        data: boxes,
        getPolygon: footprint,
        extruded: true,
        wireframe: true,
        filled: true,
        getElevation: (b) => b.actor.height_m,
        getFillColor: (b) => {
          if (!detected(scene, b.actor, frame)) return COLOR.missed.concat(70);
          return (VRU[b.actor.class] ? COLOR.vru : COLOR.tracked).concat(150);
        },
        getLineColor: (b) => (detected(scene, b.actor, frame) ? (VRU[b.actor.class] ? COLOR.vru : COLOR.tracked) : COLOR.missed),
        lineWidthMinPixels: 1,
        updateTriggers: { getFillColor: frame, getLineColor: frame },
      }),
      new deck.PolygonLayer({
        id: "ego",
        data: [{ x: ego.x, y: ego.y, z: scene.ego.height_m / 2, hl: scene.ego.length_m / 2, hw: scene.ego.width_m / 2, heading: ego.heading }],
        getPolygon: footprint,
        extruded: true,
        wireframe: true,
        filled: true,
        getElevation: scene.ego.height_m,
        getFillColor: COLOR.ego.concat(190),
        getLineColor: [235, 235, 230],
        lineWidthMinPixels: 1,
      }),
    );

    try {
      out.push(
        new deck.TextLayer({
          id: "actor-labels",
          data: boxes,
          getPosition: (b) => [b.x, b.y, b.actor.height_m + 1.2],
          getText: (b) => (detected(scene, b.actor, frame) ? `${b.actor.class} ${range(scene, b.actor, frame).toFixed(0)} m` : `${b.actor.class} · not detected`),
          getSize: 12,
          sizeUnits: "pixels",
          getColor: (b) => (detected(scene, b.actor, frame) ? [242, 242, 238] : COLOR.missed),
          background: true,
          getBackgroundColor: [20, 20, 20, 170],
          backgroundPadding: [3, 1],
          updateTriggers: { getText: frame, getColor: frame },
        }),
      );
    } catch (_) {
      /* Labels are a nicety; a font-atlas failure must not take the scene down. */
    }
    return out;
  }

  /** Camera the scene opens on: the whole road, from behind and above. */
  function defaultCamera(scene) {
    const line = scene.road.centerline;
    const mid = line[Math.floor(line.length / 2)];
    return { target: [mid.x, mid.y, 0], rotationX: 55, rotationOrbit: 0, zoom: 3, minZoom: 0, maxZoom: 8 };
  }

  /**
   * The camera is deck's to own between renders — otherwise a controlled
   * viewState would throw away the viewer's pan and zoom on the next frame of
   * playback. Follow mode only overrides the target.
   */
  function viewState() {
    const scene = state.scene;
    const camera = state.camera || (state.camera = defaultCamera(scene));
    if (!state.follow) return camera;
    const ego = scene.ego.track[Math.min(state.frame, scene.ego.track.length - 1)];
    return Object.assign({}, camera, { target: [ego.x, ego.y, 0] });
  }

  function render() {
    const container = $("#scene-canvas");
    if (!window.deck) {
      container.innerHTML = '<div class="empty">deck.gl failed to load (offline?). The scene tables below still work.</div>';
      renderHud();
      return;
    }
    const props = {
      layers: layers(),
      views: [new deck.OrbitView({ orbitAxis: "Z", fovy: 50 })],
      viewState: viewState(),
      controller: true,
      onViewStateChange: ({ viewState: next }) => {
        // Keep what the viewer did, but never let a drag fight follow mode.
        state.camera = state.follow ? Object.assign({}, next, { target: state.camera.target }) : next;
        state.deck.setProps({ viewState: viewState() });
      },
    };
    if (!state.deck) {
      state.deck = new deck.DeckGL(Object.assign({ container, parameters: { clearColor: [0.06, 0.06, 0.07, 1] } }, props));
    } else {
      state.deck.setProps(props);
    }
    renderHud();
  }

  // --------------------------------------------------------------------- HUD

  function renderHud() {
    const scene = state.scene;
    const frame = state.frame;
    const ego = scene.ego.track[Math.min(frame, scene.ego.track.length - 1)];
    const t = frame / scene.hz;
    const seen = scene.actors.filter((a) => detected(scene, a, frame)).length;
    $("#scene-time").textContent = t.toFixed(1) + " s";
    $("#scene-slider").value = String(frame);
    $("#scene-hud").innerHTML =
      `<div><span class="muted">speed</span> <b class="mono">${(ego.speed_mps * 3.6).toFixed(0)} km/h</b></div>` +
      `<div><span class="muted">detected</span> <b class="mono">${seen}/${scene.actors.length}</b></div>` +
      `<div><span class="muted">lidar range</span> <b class="mono">${scene.sensor.lidar_range_m} m</b></div>` +
      `<div><span class="muted">returns</span> <b class="mono">${state.lidar ? scene.sensor.lidar_points_per_frame : 0}/frame</b></div>`;

    const rows = scene.actors
      .map((a) => {
        const r = range(scene, a, frame);
        const ok = r <= scene.sensor.lidar_range_m;
        return `<tr class="${ok ? "" : "is-missed"}"><td class="mono">${esc(a.actor_id)}</td><td>${esc(a.class)}</td><td class="mono">${r.toFixed(1)} m</td><td>${ok ? '<span class="pill accepted">tracked</span>' : '<span class="pill rejected">not detected</span>'}</td></tr>`;
      })
      .join("");
    $("#scene-actors").querySelector("tbody").innerHTML = rows;

    // Only the highlight moves per frame; rebuilding the list ten times a
    // second would throw away the viewer's hover and click target.
    $("#scene-events").querySelectorAll("li[data-t]").forEach((li) => {
      li.classList.toggle("is-active", Math.abs(Number(li.dataset.t) - t) < 0.75);
    });
  }

  // ---------------------------------------------------------------- playback

  function step(now) {
    if (!state.playing) return;
    const scene = state.scene;
    if (!state.last) state.last = now;
    const advance = ((now - state.last) / 1000) * scene.hz;
    if (advance >= 1) {
      state.last = now;
      state.frame += Math.floor(advance);
      if (state.frame >= scene.ego.track.length) { state.frame = 0; }
      render();
    }
    state.raf = requestAnimationFrame(step);
  }

  function play() {
    if (!state.scene) return;
    state.playing = true;
    state.last = 0;
    $("#scene-play").textContent = "Pause";
    state.raf = requestAnimationFrame(step);
  }

  function stop() {
    state.playing = false;
    if (state.raf) cancelAnimationFrame(state.raf);
    state.raf = null;
    const btn = $("#scene-play");
    if (btn) btn.textContent = "Play";
  }

  // ------------------------------------------------------------------ loading

  async function fetchJson(path) {
    const response = await fetch(path);
    if (!response.ok) throw new Error(`${response.status}: ${response.statusText}`);
    return response.json();
  }

  function renderEvents(scene) {
    const list = $("#scene-events");
    list.innerHTML = scene.events
      .map((e) => `<li class="scene-event ${esc(e.severity)}" data-t="${e.t}"><span class="mono">${e.t.toFixed(1)} s</span> <b>${esc(e.class.replace(/_/g, " "))}</b><div class="muted">${esc(e.description)}</div></li>`)
      .join("");
    list.querySelectorAll("li[data-t]").forEach((li) => {
      li.addEventListener("click", () => { stop(); state.frame = Math.round(Number(li.dataset.t) * scene.hz); render(); });
    });
  }

  async function select(sceneId) {
    stop();
    const scene = await fetchJson("/api/scenes/" + encodeURIComponent(sceneId));
    scene.postCache = posts(scene);
    state.scene = scene;
    state.frame = 0;
    // A new scene means a new world: drop the deck instance so the view recentres.
    if (state.deck) { state.deck.finalize(); state.deck = null; $("#scene-canvas").innerHTML = ""; }
    state.camera = defaultCamera(scene);
    $("#scene-slider").max = String(scene.ego.track.length - 1);
    $("#scene-description").textContent = scene.description;
    $("#scene-odd").innerHTML = [
      ["visibility", scene.odd.visibility_class],
      ["lighting", scene.odd.lighting_class],
      ["route", scene.odd.route_class],
    ].map(([k, v]) => `<span class="odd-chip"><span class="muted">${k}</span> ${esc(v)}</span>`).join("");
    renderEvents(scene);
    render();
    if (location.hash !== "#scenes/" + sceneId) history.replaceState(null, "", "#scenes/" + sceneId);
  }

  async function load(sceneId) {
    if (!state.catalog.length) {
      const listing = await fetchJson("/api/scenes");
      state.catalog = listing.scenes;
      $("#scene-picker").innerHTML = state.catalog.map((s) => `<option value="${esc(s.scene_id)}">${esc(s.name)}</option>`).join("");
    }
    const wanted = state.catalog.some((s) => s.scene_id === sceneId) ? sceneId : state.catalog[0].scene_id;
    $("#scene-picker").value = wanted;
    if (!state.scene || state.scene.scene_id !== wanted) await select(wanted);
  }

  function wire() {
    $("#scene-picker").addEventListener("change", (ev) => select(ev.target.value));
    $("#scene-play").addEventListener("click", () => (state.playing ? stop() : play()));
    $("#scene-slider").addEventListener("input", (ev) => { stop(); state.frame = Number(ev.target.value); render(); });
    $("#scene-lidar").addEventListener("change", (ev) => { state.lidar = ev.target.checked; render(); });
    $("#scene-follow").addEventListener("change", (ev) => { state.follow = ev.target.checked; render(); });
    $("#scene-top").addEventListener("click", () => {
      if (!state.camera) return;
      state.camera = Object.assign({}, state.camera, { rotationX: state.camera.rotationX >= 89 ? 55 : 90 });
      render();
    });
  }

  return {
    load,
    dispose: stop,
    wire,
    /**
     * Pure helpers, exposed so the lidar simulation can be tested in Node.
     * Nothing in the page should call these.
     */
    __test: { rayBox, scan, posts, footprint, offsetFromRoad, mulberry32 },
  };
})();
