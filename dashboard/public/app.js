/* Bhutan Mobility Atlas front end. Plain JS, no build step.
   Talks to the Worker API on the same origin; deck.gl is loaded from a CDN for route playback. */
(function () {
  "use strict";

  const state = {
    token: localStorage.getItem("atlas.token") || "",
    tenant: localStorage.getItem("atlas.tenant") || "",
    view: "overview",
    runs: [],
    run: null,          // selected run detail
    samples: [],
    events: [],
    scenarios: [],
    families: [],
    evaluations: [],
    playback: { t: 0, playing: false, raf: null, t0: 0, t1: 0 },
    deck: null,
    history: [],
    fleet: { devices: [], device: null, track: [], deck: null, timer: null },
    planner: { options: null, plan: null },
  };

  const $ = (sel) => document.querySelector(sel);
  const fmt = {
    num: (v, d = 1) => (v === null || v === undefined || Number.isNaN(v) ? "–" : Number(v).toFixed(d)),
    pct: (v) => (v === null || v === undefined ? "–" : Math.round(v * 1000) / 10 + " %"),
    time: (s) => (s ? new Date(s * 1000).toLocaleString() : "–"),
    dur: (s) => (s ? (s >= 3600 ? (s / 3600).toFixed(1) + " h" : (s / 60).toFixed(1) + " min") : "–"),
    bytes: (b) => (b >= 1e9 ? (b / 1e9).toFixed(2) + " GB" : b >= 1e6 ? (b / 1e6).toFixed(1) + " MB" : b >= 1e3 ? (b / 1e3).toFixed(0) + " kB" : b + " B"),
  };
  const esc = (s) => String(s ?? "").replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
  const pill = (status) => `<span class="pill ${esc(status)}">${esc(String(status).replace(/_/g, " "))}</span>`;

  function toast(message, ms = 3000) {
    const el = $("#toast");
    el.textContent = message;
    el.hidden = false;
    clearTimeout(toast.timer);
    toast.timer = setTimeout(() => (el.hidden = true), ms);
  }

  async function api(path, options = {}) {
    const headers = Object.assign({ authorization: "Bearer " + state.token }, options.headers || {});
    if (state.tenant) headers["x-tenant"] = state.tenant;
    if (options.body && typeof options.body !== "string") {
      headers["content-type"] = "application/json";
      options.body = JSON.stringify(options.body);
    }
    const response = await fetch(path, Object.assign({}, options, { headers }));
    if (!response.ok) {
      let detail = response.statusText;
      try { detail = (await response.json()).error || detail; } catch (_) { /* ignore */ }
      throw new Error(`${response.status}: ${detail}`);
    }
    if (options.raw) return response;
    return response.json();
  }

  /** Fetch an authenticated export and hand it to the browser as a download. */
  async function download(path, filename) {
    const response = await api(path, { raw: true });
    const blob = await response.blob();
    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = filename;
    a.click();
    setTimeout(() => URL.revokeObjectURL(a.href), 10000);
    return blob;
  }

  function tokenisedUrl(path) {
    const url = new URL(path, location.origin);
    url.searchParams.set("token", state.token);
    return url.toString();
  }

  // ------------------------------------------------------------------ charts
  const SERIES = ["var(--series-1)", "var(--series-2)", "var(--series-3)"];
  const SEVERITY_COLOR = { info: "var(--text-muted)", warning: "var(--status-warning)", critical: "var(--status-critical)" };
  const SEVERITY_RGB = { info: [122, 121, 115], warning: [250, 178, 25], critical: [208, 59, 59] };

  function svgEl(name, attrs = {}) {
    const el = document.createElementNS("http://www.w3.org/2000/svg", name);
    for (const [k, v] of Object.entries(attrs)) el.setAttribute(k, v);
    return el;
  }

  function niceTicks(min, max, count = 4) {
    if (!(max > min)) return [min];
    const span = max - min;
    const step0 = span / count;
    const mag = Math.pow(10, Math.floor(Math.log10(step0)));
    const step = [1, 2, 5, 10].map((m) => m * mag).find((s) => span / s <= count + 1) || mag;
    const ticks = [];
    for (let v = Math.ceil(min / step) * step; v <= max + 1e-9; v += step) ticks.push(+v.toFixed(6));
    return ticks;
  }

  /** Multi-series line chart with crosshair tooltip. series: [{name, color, values:[[x,y]]}] */
  function lineChart(container, { title, series, xLabel, yUnit, onHover }) {
    container.innerHTML = "";
    const width = Math.max(320, container.clientWidth || 600);
    const height = 150;
    const m = { top: 24, right: 84, bottom: 26, left: 44 };
    const all = series.flatMap((s) => s.values);
    if (!all.length) { container.innerHTML = '<div class="empty">no data</div>'; return; }
    const xMin = Math.min(...all.map((p) => p[0])), xMax = Math.max(...all.map((p) => p[0]));
    let yMin = Math.min(0, ...all.map((p) => p[1])), yMax = Math.max(...all.map((p) => p[1]));
    if (yMax === yMin) yMax = yMin + 1;
    const sx = (x) => m.left + ((x - xMin) / (xMax - xMin || 1)) * (width - m.left - m.right);
    const sy = (y) => height - m.bottom - ((y - yMin) / (yMax - yMin)) * (height - m.top - m.bottom);
    const svg = svgEl("svg", { viewBox: `0 0 ${width} ${height}`, role: "img", "aria-label": title });
    svg.appendChild(svgEl("text", { x: m.left, y: 14, class: "title" })).textContent = title + (yUnit ? ` (${yUnit})` : "");
    const grid = svgEl("g", { class: "grid" });
    const axis = svgEl("g", { class: "axis" });
    for (const t of niceTicks(yMin, yMax, 3)) {
      grid.appendChild(svgEl("line", { x1: m.left, x2: width - m.right, y1: sy(t), y2: sy(t) }));
      axis.appendChild(svgEl("text", { x: m.left - 6, y: sy(t) + 4, "text-anchor": "end" })).textContent = t;
    }
    for (const t of niceTicks(xMin, xMax, 6)) {
      axis.appendChild(svgEl("text", { x: sx(t), y: height - 8, "text-anchor": "middle" })).textContent = Math.round(t) + (xLabel || "");
    }
    svg.appendChild(grid);
    svg.appendChild(axis);
    const g = svgEl("g", { class: "series" });
    series.forEach((s, i) => {
      const d = s.values.map((p, k) => (k ? "L" : "M") + sx(p[0]).toFixed(1) + " " + sy(p[1]).toFixed(1)).join("");
      g.appendChild(svgEl("path", { d, stroke: s.color || SERIES[i] }));
      const last = s.values[s.values.length - 1];
      const label = svgEl("text", { x: sx(last[0]) + 4, y: sy(last[1]) + 4, class: "label" });
      label.textContent = s.name;
      g.appendChild(label);
    });
    svg.appendChild(g);
    const cross = svgEl("line", { class: "crosshair", y1: m.top, y2: height - m.bottom, x1: -10, x2: -10 });
    svg.appendChild(cross);
    const markers = series.map((s, i) => svg.appendChild(svgEl("circle", { class: "marker", r: 4, stroke: s.color || SERIES[i], cx: -10, cy: -10, visibility: "hidden" })));
    container.appendChild(svg);
    const tip = document.createElement("div");
    tip.className = "tooltip";
    tip.hidden = true;
    container.appendChild(tip);

    function showAt(x) {
      cross.setAttribute("x1", sx(x));
      cross.setAttribute("x2", sx(x));
      const rows = series.map((s, i) => {
        const p = nearest(s.values, x);
        markers[i].setAttribute("cx", sx(p[0]));
        markers[i].setAttribute("cy", sy(p[1]));
        markers[i].setAttribute("visibility", "visible");
        return `<div class="row"><span><i class="swatch" style="background:${s.color || SERIES[i]}"></i>${esc(s.name)}</span><b>${fmt.num(p[1], 2)}</b></div>`;
      });
      tip.innerHTML = `<div class="row"><span>t</span><b>${fmt.num(x, 1)} s</b></div>` + rows.join("");
      tip.hidden = false;
      const rect = container.getBoundingClientRect();
      const px = (sx(x) / width) * rect.width;
      tip.style.left = Math.min(rect.width - tip.offsetWidth - 4, px + 10) + "px";
      tip.style.top = "22px";
    }
    svg.addEventListener("mousemove", (ev) => {
      const rect = svg.getBoundingClientRect();
      const x = xMin + ((ev.clientX - rect.left) / rect.width * width - m.left) / (width - m.left - m.right) * (xMax - xMin);
      if (x < xMin || x > xMax) return;
      showAt(x);
      if (onHover) onHover(x);
    });
    svg.addEventListener("mouseleave", () => { tip.hidden = true; });
    // Keep end-of-line labels inside the plot when two series end close together.
    const labels = Array.from(g.querySelectorAll("text.label")).sort((a, b) => Number(a.getAttribute("y")) - Number(b.getAttribute("y")));
    for (let i = 1; i < labels.length; i++) {
      const prev = Number(labels[i - 1].getAttribute("y")), cur = Number(labels[i].getAttribute("y"));
      if (cur - prev < 12) labels[i].setAttribute("y", prev + 12);
    }
    container._showAt = showAt;
  }

  function nearest(values, x) {
    let lo = 0, hi = values.length - 1;
    while (lo < hi) { const mid = (lo + hi) >> 1; if (values[mid][0] < x) lo = mid + 1; else hi = mid; }
    return values[lo];
  }

  /** Horizontal grouped bar chart. rows: [{label, values:[..]}], series: [{name,color}].
      `format` overrides how a value is written on the bar and in the tooltip. */
  function barChart(container, { rows, series, unit, max, format }) {
    const label = format || ((v) => fmt.num(v, unit === "%" ? 0 : 1) + (unit || ""));
    container.innerHTML = "";
    if (!rows.length) { container.innerHTML = '<div class="empty">no data</div>'; return; }
    const width = Math.max(320, container.clientWidth || 600);
    const barH = 12, gap = 2, groupGap = 10, labelW = 130;
    const groupH = series.length * (barH + gap) + groupGap;
    const height = rows.length * groupH + 30;
    const vmax = max || Math.max(1e-9, ...rows.flatMap((r) => r.values));
    const sx = (v) => labelW + (v / vmax) * (width - labelW - 60);
    const svg = svgEl("svg", { viewBox: `0 0 ${width} ${height}`, role: "img", "aria-label": "bar chart" });
    const tip = document.createElement("div");
    tip.className = "tooltip"; tip.hidden = true;
    rows.forEach((row, ri) => {
      const y0 = 10 + ri * groupH;
      svg.appendChild(svgEl("text", { x: labelW - 8, y: y0 + (series.length * (barH + gap)) / 2 + 3, "text-anchor": "end", class: "label" })).textContent = row.label;
      series.forEach((s, si) => {
        const v = row.values[si] || 0;
        const y = y0 + si * (barH + gap);
        const bar = svgEl("rect", { class: "bar", x: labelW, y, width: Math.max(0, sx(v) - labelW), height: barH, fill: s.color || SERIES[si] });
        bar.addEventListener("mousemove", (ev) => {
          tip.innerHTML = `<b>${esc(row.label)}</b><div class="row"><span>${esc(s.name)}</span><b>${esc(label(v))}</b></div>`;
          tip.hidden = false;
          const rect = container.getBoundingClientRect();
          tip.style.left = Math.min(rect.width - 140, ev.clientX - rect.left + 12) + "px";
          tip.style.top = ev.clientY - rect.top - 30 + "px";
        });
        bar.addEventListener("mouseleave", () => (tip.hidden = true));
        svg.appendChild(bar);
        svg.appendChild(svgEl("text", { x: sx(v) + 4, y: y + barH - 2, class: "bar-label" })).textContent = label(v);
      });
    });
    container.appendChild(svg);
    container.appendChild(tip);
    if (series.length > 1) {
      const legend = document.createElement("div");
      legend.className = "legend";
      legend.innerHTML = series.map((s, i) => `<span style="--c:${s.color || SERIES[i]}">${esc(s.name)}</span>`).join("");
      container.appendChild(legend);
    }
  }

  // ---------------------------------------------------------------- overview
  function renderTrend() {
    const sel = $("#trend-kpi");
    const id = sel.value;
    const points = [];
    for (const snap of state.history) {
      const kpi = (snap.kpis || []).find((k) => k.id === id);
      if (kpi && kpi.value !== null && kpi.value !== undefined) points.push([Date.parse(snap.computed_at) / 86400000, kpi.value]);
    }
    const container = $("#trend-chart");
    if (points.length < 2) { container.innerHTML = '<div class="empty">need at least two nightly snapshots — POST /api/kpis/snapshot forces one</div>'; return; }
    const label = sel.options[sel.selectedIndex].textContent;
    const day0 = Math.floor(points[0][0]);
    lineChart(container, { title: label, xLabel: " d", series: [{ name: label, values: points.map((p) => [p[0] - day0, p[1]]) }] });
  }

  async function loadOverview() {
    const [report, history] = await Promise.all([api("/api/kpis"), api("/api/kpis/history?limit=120")]);
    state.history = history.snapshots || [];
    const sel = $("#trend-kpi");
    const keep = sel.value;
    sel.innerHTML = report.kpis.map((k) => `<option value="${esc(k.id)}" ${k.id === keep ? "selected" : ""}>${esc(k.label)}</option>`).join("");
    $("#metrics-link").href = tokenisedUrl("/api/metrics");
    renderTrend();
    $("#kpi-grid").innerHTML = report.kpis.map((k) => `
      <div class="kpi">
        <div class="kpi-label">${esc(k.label)}</div>
        <div class="kpi-value">${k.value === null ? "–" : esc(fmt.num(k.value, Number.isInteger(k.value) ? 0 : 1))}<small>${esc(k.unit)}</small></div>
        ${pill(k.status)}
        <div class="kpi-target">Target: ${esc(k.target)}</div>
        <div class="kpi-why">${esc(k.why)}</div>
      </div>`).join("");
    barChart($("#edge-chart"), {
      rows: report.edge_case_rates.slice(0, 12).map((r) => ({ label: r.event_class.replace(/_/g, " "), values: [r.per_100km] })),
      series: [{ name: "events per 100 km" }],
    });
    const models = report.perception.models;
    if (!models.length) { $("#perception-table").innerHTML = '<div class="empty">no evaluations yet</div>'; return; }
    const dims = ["weather", "lighting", "route_class"];
    let html = "";
    for (const m of models.slice(0, 3)) {
      html += `<h3>${esc(m.model_id)} v${esc(m.model_version || "?")} · ${m.evaluations} evaluations · recall ${fmt.pct(m.overall.recall)} · precision ${fmt.pct(m.overall.precision)}</h3>`;
      html += '<table class="data-table"><thead><tr><th>Dimension</th><th>Condition</th><th class="num">Precision</th><th class="num">Recall</th><th class="num">F1</th><th class="num">GT objects</th></tr></thead><tbody>';
      for (const dim of dims) {
        for (const [val, c] of Object.entries(m.by_condition[dim] || {})) {
          html += `<tr><td>${esc(dim)}</td><td>${esc(val)}</td><td class="num">${fmt.pct(c.precision)}</td><td class="num">${fmt.pct(c.recall)}</td><td class="num">${fmt.pct(c.f1)}</td><td class="num">${c.tp + c.fn}</td></tr>`;
        }
      }
      html += "</tbody></table>";
    }
    $("#perception-table").innerHTML = html;
  }

  // -------------------------------------------------------------------- runs
  async function loadRuns() {
    const params = new URLSearchParams();
    if ($("#run-source").value) params.set("source", $("#run-source").value);
    if ($("#run-quality").value) params.set("quality_status", $("#run-quality").value);
    const data = await api("/api/runs?" + params.toString());
    state.runs = data.runs;
    const body = $("#runs-table tbody");
    if (!state.runs.length) { body.innerHTML = '<tr><td colspan="10" class="empty">no runs yet — upload one with toolkit/scripts/upload_run.py</td></tr>'; return; }
    body.innerHTML = state.runs.map((r) => `
      <tr class="is-clickable" data-run="${esc(r.run_id)}">
        <td class="mono">${esc(r.run_id)}</td><td>${esc(r.source)}</td><td class="mono">${esc(r.scenario_id || "–")}</td>
        <td>${esc(fmt.time(r.started_at))}</td><td class="num">${esc(fmt.dur(r.duration_s))}</td><td class="num">${fmt.num(r.distance_km, 2)} km</td>
        <td class="num">${r.event_count}</td><td class="num">${r.driving_score === null || r.driving_score === undefined ? "–" : fmt.num(r.driving_score, 1)}</td>
        <td>${pill(r.quality_status)}</td><td>${pill(r.replay_complete ? "yes" : "no")}</td>
      </tr>`).join("");
    body.querySelectorAll("tr[data-run]").forEach((tr) => tr.addEventListener("click", () => selectRun(tr.dataset.run)));
    const wanted = state.pendingRun;
    state.pendingRun = null;
    if (wanted && state.runs.some((r) => r.run_id === wanted)) selectRun(wanted);
  }

  function renderScore(run) {
    const strip = $("#score-strip");
    if (run.driving_score === null || run.driving_score === undefined) { strip.hidden = true; return; }
    const infractions = (run.infractions && run.infractions.infractions) || [];
    strip.hidden = false;
    strip.innerHTML = `<span>Driving score <b>${fmt.num(run.driving_score, 1)}</b></span>` +
      `<span>Route completion <b>${fmt.num(run.route_completion, 0)}%</b> <span class="muted">${esc((run.infractions && run.infractions.basis || "").replace(/_/g, " "))}</span></span>` +
      `<span>Infraction penalty <b>×${fmt.num(run.infraction_penalty, 3)}</b></span>` +
      (infractions.length ? `<span class="infraction">${infractions.map((i) => `${esc(i.kind.replace(/_/g, " "))} ×${i.count} (${i.penalty})`).join(" · ")}</span>` : '<span class="infraction">no infractions</span>');
  }

  async function selectRun(runId) {
    setHash("runs", runId);
    document.querySelectorAll("#runs-table tr").forEach((tr) => tr.classList.toggle("is-selected", tr.dataset.run === runId));
    const [detail, telemetry, events] = await Promise.all([
      api(`/api/runs/${encodeURIComponent(runId)}`),
      api(`/api/runs/${encodeURIComponent(runId)}/telemetry?max=4000`),
      api(`/api/runs/${encodeURIComponent(runId)}/events`),
    ]);
    state.run = detail.run;
    state.samples = telemetry.samples;
    state.events = events.events;
    $("#run-detail").hidden = false;
    $("#detail-title").textContent = `${detail.run.run_id} · ${detail.run.scenario_id || detail.run.route_id || detail.run.source}`;
    renderScore(detail.run);
    renderTimeline();
    renderMap();
    renderEvents();
    stopPlayback();
    const t0 = state.samples.length ? state.samples[0].t : 0;
    state.playback = Object.assign(state.playback, { t: t0, t0, t1: state.samples.length ? state.samples[state.samples.length - 1].t : t0 });
    updatePlayback(t0);
  }

  function renderTimeline() {
    const s = state.samples;
    const t0 = s.length ? s[0].t : 0;
    const rel = (p) => p.t - t0;
    const hover = (x) => updatePlayback(t0 + x, true);
    lineChart($("#chart-speed"), { title: "Speed", yUnit: "km/h", xLabel: "s", onHover: hover, series: [{ name: "speed", values: s.map((p) => [rel(p), (p.speed_mps || 0) * 3.6]) }] });
    lineChart($("#chart-accel"), { title: "Acceleration", yUnit: "m/s²", xLabel: "s", onHover: hover, series: [
      { name: "longitudinal", values: s.map((p) => [rel(p), p.accel_x || 0]) },
      { name: "lateral", values: s.map((p) => [rel(p), p.accel_y || 0]) },
    ] });
    lineChart($("#chart-grade"), { title: "Grade and brake", yUnit: "% / ×100", xLabel: "s", onHover: hover, series: [
      { name: "grade %", values: s.map((p) => [rel(p), p.grade_pct || 0]) },
      { name: "brake ×100", values: s.map((p) => [rel(p), (p.brake || 0) * 100]) },
    ] });
  }

  function renderEvents() {
    const body = $("#events-table tbody");
    const t0 = state.samples.length ? state.samples[0].t : 0;
    if (!state.events.length) { body.innerHTML = '<tr><td colspan="6" class="empty">no events</td></tr>'; return; }
    body.innerHTML = state.events.map((e) => `
      <tr class="is-clickable" data-t="${e.t}">
        <td class="num mono">${fmt.num(e.t - t0, 1)}</td><td>${esc(e.event_class.replace(/_/g, " "))}</td><td>${pill(e.severity)}</td>
        <td class="mono">${esc(e.rule_id || "")}</td><td>${esc(e.description || "")}</td>
        <td>${pill(e.review_status)} ${e.severity === "critical" && e.review_status !== "reviewed" ? `<button class="small" data-review="${esc(e.event_id)}">Mark reviewed</button>` : ""}</td>
      </tr>`).join("");
    body.querySelectorAll("tr[data-t]").forEach((tr) => tr.addEventListener("click", (ev) => {
      if (ev.target.tagName === "BUTTON") return;
      updatePlayback(Number(tr.dataset.t));
    }));
    body.querySelectorAll("button[data-review]").forEach((b) => b.addEventListener("click", async () => {
      try {
        await api(`/api/events/${encodeURIComponent(b.dataset.review)}/review`, { method: "POST", body: { review_status: "reviewed" } });
        toast("event marked reviewed");
        selectRun(state.run.run_id);
      } catch (err) { toast(err.message); }
    }));
  }

  function renderMap() {
    const container = $("#map");
    if (!window.deck) { container.innerHTML = '<div class="empty">deck.gl failed to load (offline?). Timeline and tables still work.</div>'; return; }
    const path = state.samples.map((p) => [p.lon, p.lat]);
    if (!path.length) { container.innerHTML = '<div class="empty">no telemetry</div>'; return; }
    const lats = path.map((p) => p[1]), lons = path.map((p) => p[0]);
    const center = [(Math.min(...lons) + Math.max(...lons)) / 2, (Math.min(...lats) + Math.max(...lats)) / 2];
    const span = Math.max(Math.max(...lats) - Math.min(...lats), (Math.max(...lons) - Math.min(...lons)) * Math.cos(center[1] * Math.PI / 180), 1e-4);
    const zoom = Math.min(18, Math.max(8, Math.log2(360 / span) - 1.2));
    if (!state.deck) {
      container.innerHTML = "";
      state.deck = new deck.DeckGL({ container, initialViewState: { longitude: center[0], latitude: center[1], zoom, pitch: 0 }, controller: true, layers: [],
        getTooltip: ({ object }) => object && object.event_class ? { text: `${object.event_class} (${object.severity})\n${object.description || ""}` } : null });
    } else {
      state.deck.setProps({ initialViewState: { longitude: center[0], latitude: center[1], zoom, pitch: 0, transitionDuration: 500 } });
    }
    $("#map-legend").innerHTML = `<span class="line" style="--c:var(--series-1)">route</span><span class="line" style="--c:var(--series-2)">played trail</span>` +
      Object.entries(SEVERITY_COLOR).map(([k, c]) => `<span style="--c:${c}">${k} event</span>`).join("") + `<span style="--c:var(--text-primary)">vehicle</span>`;
    updateMapLayers(state.playback.t || state.samples[0].t);
  }

  function updateMapLayers(t) {
    if (!state.deck || !window.deck) return;
    const tileUrl = window.BHUTAN_TILE_URL || "https://tile.openstreetmap.org/{z}/{x}/{y}.png";
    const s = state.samples;
    const t0 = s[0].t;
    const path = s.map((p) => [p.lon, p.lat]);
    const trail = { path, timestamps: s.map((p) => p.t - t0) };
    const pos = nearest(s.map((p) => [p.t, p]), t)[1];
    const layers = [
      new deck.TileLayer({ id: "basemap", data: tileUrl, minZoom: 0, maxZoom: 19, tileSize: 256,
        renderSubLayers: (props) => { const { boundingBox } = props.tile; return new deck.BitmapLayer(props, { data: null, image: props.data, bounds: [boundingBox[0][0], boundingBox[0][1], boundingBox[1][0], boundingBox[1][1]] }); } }),
      new deck.PathLayer({ id: "route", data: [{ path }], getPath: (d) => d.path, getColor: [42, 120, 214, 200], widthMinPixels: 3, widthUnits: "pixels", getWidth: 3 }),
      new deck.TripsLayer({ id: "trail", data: [trail], getPath: (d) => d.path, getTimestamps: (d) => d.timestamps, getColor: [235, 104, 52], widthMinPixels: 5, trailLength: 30, currentTime: t - t0, fadeTrail: true }),
      new deck.ScatterplotLayer({ id: "events", data: state.events.filter((e) => e.lat != null), getPosition: (e) => [e.lon, e.lat], getFillColor: (e) => SEVERITY_RGB[e.severity] || SEVERITY_RGB.info,
        getLineColor: [252, 252, 251], lineWidthMinPixels: 2, stroked: true, radiusMinPixels: 6, radiusMaxPixels: 12, pickable: true }),
      new deck.ScatterplotLayer({ id: "vehicle", data: [pos], getPosition: (p) => [p.lon, p.lat], getFillColor: [11, 11, 11], getLineColor: [252, 252, 251], lineWidthMinPixels: 2, stroked: true, radiusMinPixels: 8, radiusMaxPixels: 8 }),
    ];
    state.deck.setProps({ layers });
  }

  function updatePlayback(t, fromHover) {
    const pb = state.playback;
    pb.t = Math.min(pb.t1, Math.max(pb.t0, t));
    const span = pb.t1 - pb.t0 || 1;
    $("#time-slider").value = Math.round(((pb.t - pb.t0) / span) * 1000);
    $("#time-label").textContent = fmt.num(pb.t - pb.t0, 1) + " s";
    updateMapLayers(pb.t);
    if (!fromHover) for (const id of ["#chart-speed", "#chart-accel", "#chart-grade"]) { const el = $(id); if (el._showAt) el._showAt(pb.t - pb.t0); }
  }

  function stopPlayback() {
    state.playback.playing = false;
    if (state.playback.raf) cancelAnimationFrame(state.playback.raf);
    $("#play-btn").textContent = "Play";
  }

  function togglePlayback() {
    const pb = state.playback;
    if (pb.playing) return stopPlayback();
    pb.playing = true;
    $("#play-btn").textContent = "Pause";
    let last = performance.now();
    const step = (now) => {
      if (!pb.playing) return;
      const dt = (now - last) / 1000;
      last = now;
      let t = pb.t + dt * 4;                    // 4x real time
      if (t >= pb.t1) { t = pb.t0; }
      updatePlayback(t);
      pb.raf = requestAnimationFrame(step);
    };
    pb.raf = requestAnimationFrame(step);
  }

  async function exportEvidence() {
    if (!state.run) return;
    try {
      const pack = await api(`/api/runs/${encodeURIComponent(state.run.run_id)}/evidence`);
      const blob = new Blob([JSON.stringify(pack, null, 2)], { type: "application/json" });
      const a = document.createElement("a");
      a.href = URL.createObjectURL(blob);
      a.download = `evidence-${state.run.run_id}.json`;
      a.click();
      toast(pack.signed ? "signed evidence pack downloaded" : "evidence pack downloaded (unsigned: MANIFEST_SIGNING_KEY not set)");
    } catch (err) { toast(err.message); }
  }

  async function exportRun(format) {
    if (!state.run) return;
    const id = state.run.run_id;
    try {
      toast(`preparing ${format} export…`, 8000);
      const blob = await download(`/api/runs/${encodeURIComponent(id)}/export/${format}`, `${id}.${format}`);
      toast(`${format} export downloaded (${fmt.bytes(blob.size)})`);
    } catch (err) { toast(err.message); }
  }

  function openInFoxglove() {
    if (!state.run) return;
    // Foxglove Studio opens remote MCAP files by URL; the reader token travels in the URL, so share with care.
    const mcap = tokenisedUrl(`/api/runs/${encodeURIComponent(state.run.run_id)}/export/mcap`);
    const url = "https://app.foxglove.dev/~/view?ds=remote-file&ds.url=" + encodeURIComponent(mcap);
    window.open(url, "_blank", "noopener");
    toast("opened Foxglove Studio with a tokenised MCAP URL");
  }

  // --------------------------------------------------------------- scenarios
  async function loadScenarios() {
    const params = new URLSearchParams();
    for (const [key, id] of [["group", "#scenario-group"], ["family", "#scenario-family"], ["review_status", "#scenario-review"]]) {
      if ($(id).value) params.set(key, $(id).value);
    }
    const data = await api("/api/scenarios?" + params.toString());
    state.scenarios = data.scenarios;
    state.families = data.families;
    const groups = [...new Set(state.families.map((f) => f.group_name).filter(Boolean))];
    const groupSel = $("#scenario-group"), famSel = $("#scenario-family");
    const keepG = groupSel.value, keepF = famSel.value;
    groupSel.innerHTML = '<option value="">all</option>' + groups.map((g) => `<option ${g === keepG ? "selected" : ""}>${esc(g)}</option>`).join("");
    famSel.innerHTML = '<option value="">all</option>' + state.families.map((f) => `<option ${f.family === keepF ? "selected" : ""}>${esc(f.family)}</option>`).join("");
    $("#family-grid").innerHTML = state.families.map((f) => `
      <div class="family ${f.family === keepF ? "is-selected" : ""}" data-family="${esc(f.family)}">
        <div class="name">${esc(f.family.replace(/_/g, " "))}</div>
        <div class="meta">${esc(f.group_name || "")} · ${f.variants} variants · ${f.reviewed} reviewed</div>
        <div class="bar"><i style="width:${f.variants ? (f.reviewed / f.variants) * 100 : 0}%"></i></div>
      </div>`).join("");
    $("#family-grid").querySelectorAll(".family").forEach((el) => el.addEventListener("click", () => { famSel.value = famSel.value === el.dataset.family ? "" : el.dataset.family; loadScenarios(); }));
    const body = $("#scenarios-table tbody");
    setHash("scenarios", famSel.value || "");
    if (!state.scenarios.length) { body.innerHTML = '<tr><td colspan="10" class="empty">no scenarios — import with toolkit/scripts/generate_library.py --dashboard</td></tr>'; return; }
    body.innerHTML = state.scenarios.map((s) => {
      const p = s.params || {}, d = p.sensor_degradation || {};
      const deg = [d.camera_blur ? "blur " + d.camera_blur : "", d.gnss_noise_m ? "gnss ±" + d.gnss_noise_m + " m" : "", d.dropout_probability ? "dropout " + Math.round(d.dropout_probability * 100) + "%" : ""].filter(Boolean).join(", ") || "none";
      return `<tr>
        <td class="mono" title="${esc(s.content_hash)}">${esc(s.scenario_id)}</td><td>${esc(s.family)}</td><td>${esc(p.weather_preset)}</td><td>${esc(p.time_of_day)}</td>
        <td>${esc(s.route_class || "")}</td><td class="num">${fmt.num(p.traffic_density, 2)}</td><td class="num">${fmt.num(p.lane_quality, 2)}</td><td>${esc(deg)}</td>
        <td>${pill(s.review_status)} ${s.review_status !== "reviewed" ? `<button class="small" data-approve="${esc(s.scenario_id)}">Mark reviewed</button>` : ""}</td>
        <td><button class="small" data-xosc="${esc(s.scenario_id)}" title="ASAM OpenSCENARIO 1.2">.xosc</button></td>
      </tr>`;
    }).join("");
    body.querySelectorAll("button[data-xosc]").forEach((b) => b.addEventListener("click", async () => {
      try { await download(`/api/scenarios/${encodeURIComponent(b.dataset.xosc)}/export/xosc`, `${b.dataset.xosc}.xosc`); toast("OpenSCENARIO downloaded — run with scenario_runner.py --openscenario or esmini"); } catch (err) { toast(err.message); }
    }));
    body.querySelectorAll("button[data-approve]").forEach((b) => b.addEventListener("click", async () => {
      try {
        await api(`/api/scenarios/${encodeURIComponent(b.dataset.approve)}/review`, { method: "POST", body: { review_status: "reviewed" } });
        toast("scenario marked reviewed");
        loadScenarios();
      } catch (err) { toast(err.message); }
    }));
  }

  // ------------------------------------------------------------- evaluations
  async function loadEvaluations() {
    const data = await api("/api/evaluations");
    state.evaluations = data.evaluations;
    const body = $("#evaluations-table tbody");
    if (!state.evaluations.length) { body.innerHTML = '<tr><td colspan="10" class="empty">no evaluations — run scripts/evaluate_model.py --upload</td></tr>'; return; }
    body.innerHTML = state.evaluations.map((e) => `
      <tr class="is-clickable" data-eval="${esc(e.evaluation_id)}">
        <td class="mono">${esc(e.evaluation_id)}</td><td>${esc(e.model_id)} v${esc(e.model_version || "?")}</td><td class="mono">${esc(e.run_id || "")}</td>
        <td>${esc(e.conditions.weather || "")}</td><td>${esc(e.conditions.lighting || "")}</td>
        <td class="num">${fmt.pct(e.overall.precision)}</td><td class="num">${fmt.pct(e.overall.recall)}</td><td class="num">${fmt.pct(e.overall.f1)}</td>
        <td>${pill(e.reproducible ? "yes" : "no")}</td><td>${pill(e.replay_verified ? "yes" : "pending")}</td>
      </tr>`).join("");
    body.querySelectorAll("tr[data-eval]").forEach((tr) => tr.addEventListener("click", () => selectEvaluation(tr.dataset.eval)));
  }

  function selectEvaluation(id) {
    const e = state.evaluations.find((x) => x.evaluation_id === id);
    if (!e) return;
    setHash("evaluations", id);
    document.querySelectorAll("#evaluations-table tr").forEach((tr) => tr.classList.toggle("is-selected", tr.dataset.eval === id));
    $("#evaluation-detail").hidden = false;
    $("#evaluation-title").textContent = `${e.model_id} v${e.model_version || "?"} on ${e.run_id || "?"} · ${e.frames_evaluated} frames`;
    barChart($("#eval-class-chart"), {
      rows: Object.entries(e.by_class).map(([cls, c]) => ({ label: cls, values: [(c.precision || 0) * 100, (c.recall || 0) * 100] })),
      series: [{ name: "precision" }, { name: "recall" }], unit: "%", max: 100,
    });
    $("#eval-clusters tbody").innerHTML = (e.failure_clusters || []).slice(0, 12).map((c) =>
      `<tr><td>${esc(c.cls)}</td><td>${esc(c.weather)}</td><td>${esc(c.lighting)}</td><td>${esc(c.range_band)}</td><td class="num">${c.missed}</td></tr>`).join("") || '<tr><td colspan="5" class="empty">no misses</td></tr>';
  }

  // -------------------------------------------------------------- governance
  async function loadGovernance() {
    const [critical, clips, audit] = await Promise.all([api("/api/events/critical"), api("/api/clips"), api("/api/audit?limit=100")]);
    const cb = $("#critical-table tbody");
    cb.innerHTML = critical.events.map((e) => `
      <tr><td class="mono">${esc(e.run_id)}</td><td class="num mono">${fmt.num(e.t, 1)}</td><td class="mono">${esc(e.rule_id || "")}</td><td>${esc(e.event_class)}</td>
      <td class="num">${e.data && e.data.value !== undefined ? fmt.num(e.data.value, 2) : ""}</td><td>${pill(e.review_status)}</td>
      <td>${e.review_status !== "reviewed" ? `<button class="small" data-review="${esc(e.event_id)}">Mark reviewed</button>` : ""}</td></tr>`).join("") || '<tr><td colspan="7" class="empty">no critical violations logged</td></tr>';
    cb.querySelectorAll("button[data-review]").forEach((b) => b.addEventListener("click", async () => {
      try { await api(`/api/events/${encodeURIComponent(b.dataset.review)}/review`, { method: "POST", body: { review_status: "reviewed" } }); toast("reviewed"); loadGovernance(); } catch (err) { toast(err.message); }
    }));
    $("#clips-table tbody").innerHTML = clips.clips.map((c) => `
      <tr><td class="mono">${esc(c.clip_id)}</td><td class="mono">${esc(c.run_id || "")}</td><td class="num">${fmt.bytes(c.size_bytes || 0)}</td>
      <td>${pill(c.redaction_status)}</td><td>${esc(c.consent_ref || "–")}</td><td>${pill(c.released ? "yes" : "no")}</td>
      <td>${!c.released ? `<button class="small" data-release="${esc(c.clip_id)}">Release</button>` : ""}</td></tr>`).join("") || '<tr><td colspan="7" class="empty">no clips registered</td></tr>';
    $("#clips-table").querySelectorAll("button[data-release]").forEach((b) => b.addEventListener("click", async () => {
      try { await api(`/api/clips/${encodeURIComponent(b.dataset.release)}/governance`, { method: "POST", body: { released: true } }); toast("released"); loadGovernance(); } catch (err) { toast(err.message); }
    }));
    $("#audit-table tbody").innerHTML = audit.audit.map((a) => `
      <tr><td class="mono">${esc(a.created_at)}</td><td class="mono">${esc(a.actor)}</td><td>${esc(a.action)}</td><td class="mono">${esc(a.target || "")}</td><td class="mono">${esc(a.detail ? JSON.stringify(a.detail) : "")}</td></tr>`).join("") || '<tr><td colspan="5" class="empty">empty</td></tr>';
  }

  // ------------------------------------------------------------------- fleet
  const FLEET_RGB = { online: [27, 175, 122], stale: [250, 178, 25], offline: [122, 121, 115] };
  const deviceStatus = (d) => (d.age_s === null ? "offline" : d.age_s < 300 ? "online" : d.age_s < 3600 ? "stale" : "offline");

  async function loadFleet() {
    const data = await api("/api/fleet/live");
    state.fleet.devices = data.devices;
    const body = $("#fleet-table tbody");
    if (!data.devices.length) {
      body.innerHTML = '<tr><td colspan="8" class="empty">no devices yet — point Traccar forwarding at /api/ingest/traccar, a phone at /api/ingest/osmand, or post to /api/fleet/positions</td></tr>';
    } else {
      body.innerHTML = data.devices.map((d) => `
        <tr class="is-clickable ${d.device_id === state.fleet.device ? "is-selected" : ""}" data-device="${esc(d.device_id)}">
          <td class="mono">${esc(d.name || d.device_id)}${d.name ? `<div class="muted">${esc(d.device_id)}</div>` : ""}</td><td>${esc(d.vehicle_class)}</td><td>${esc(d.source)}</td>
          <td>${d.age_s === null ? "–" : d.age_s < 60 ? Math.round(d.age_s) + " s ago" : d.age_s < 3600 ? Math.round(d.age_s / 60) + " min ago" : fmt.time(d.last_t)}</td>
          <td class="num">${d.last_speed_mps === null ? "–" : fmt.num(d.last_speed_mps * 3.6, 0) + " km/h"}</td><td class="num">${d.position_count}</td>
          <td>${pill(deviceStatus(d))}</td><td><button class="small" data-track="${esc(d.device_id)}">Track</button></td>
        </tr>`).join("");
      body.querySelectorAll("tr[data-device]").forEach((tr) => tr.addEventListener("click", () => selectDevice(tr.dataset.device)));
    }
    if (state.fleet.device) await selectDevice(state.fleet.device, true);
    else renderFleetMap();
    scheduleFleetRefresh();
  }

  function scheduleFleetRefresh() {
    clearTimeout(state.fleet.timer);
    if (state.view === "fleet" && $("#fleet-auto").checked) state.fleet.timer = setTimeout(() => loadFleet().catch((err) => toast(err.message)), 15000);
  }

  async function selectDevice(deviceId, silent) {
    state.fleet.device = deviceId;
    setHash("fleet", deviceId);
    document.querySelectorAll("#fleet-table tr").forEach((tr) => tr.classList.toggle("is-selected", tr.dataset.device === deviceId));
    const since = Date.now() / 1000 - Number($("#fleet-window").value);
    const data = await api(`/api/fleet/devices/${encodeURIComponent(deviceId)}/track?since=${since}&limit=5000`);
    state.fleet.track = data.positions;
    $("#fleet-title").textContent = `${deviceId} · ${data.positions.length} positions`;
    $("#fleet-materialize").hidden = data.positions.length < 2;
    renderFleetMap();
    if (!silent && !data.positions.length) toast("no positions in the selected window");
  }

  function renderFleetMap() {
    const container = $("#fleet-map");
    if (!window.deck) { container.innerHTML = '<div class="empty">deck.gl failed to load (offline?)</div>'; return; }
    const devices = state.fleet.devices.filter((d) => d.last_lat !== null && d.last_lon !== null);
    const track = state.fleet.track;
    const pts = track.length ? track.map((p) => [p.lon, p.lat]) : devices.map((d) => [d.last_lon, d.last_lat]);
    if (!pts.length) { container.innerHTML = '<div class="empty">no positions</div>'; state.fleet.deck = null; return; }
    const lons = pts.map((p) => p[0]), lats = pts.map((p) => p[1]);
    const center = [(Math.min(...lons) + Math.max(...lons)) / 2, (Math.min(...lats) + Math.max(...lats)) / 2];
    const span = Math.max(Math.max(...lats) - Math.min(...lats), (Math.max(...lons) - Math.min(...lons)) * Math.cos(center[1] * Math.PI / 180), 1e-3);
    const zoom = Math.min(17, Math.max(6, Math.log2(360 / span) - 1.2));
    const tileUrl = window.BHUTAN_TILE_URL || "https://tile.openstreetmap.org/{z}/{x}/{y}.png";
    const layers = [
      new deck.TileLayer({ id: "fleet-basemap", data: tileUrl, minZoom: 0, maxZoom: 19, tileSize: 256,
        renderSubLayers: (props) => { const { boundingBox } = props.tile; return new deck.BitmapLayer(props, { data: null, image: props.data, bounds: [boundingBox[0][0], boundingBox[0][1], boundingBox[1][0], boundingBox[1][1]] }); } }),
      new deck.PathLayer({ id: "fleet-track", data: track.length ? [{ path: track.map((p) => [p.lon, p.lat]) }] : [], getPath: (d) => d.path, getColor: [42, 120, 214, 200], widthMinPixels: 3 }),
      new deck.ScatterplotLayer({ id: "fleet-devices", data: devices, getPosition: (d) => [d.last_lon, d.last_lat], getFillColor: (d) => FLEET_RGB[deviceStatus(d)],
        getLineColor: [252, 252, 251], lineWidthMinPixels: 2, stroked: true, radiusMinPixels: 8, radiusMaxPixels: 10, pickable: true }),
    ];
    if (!state.fleet.deck) {
      container.innerHTML = "";
      state.fleet.deck = new deck.DeckGL({ container, initialViewState: { longitude: center[0], latitude: center[1], zoom, pitch: 0 }, controller: true, layers,
        getTooltip: ({ object }) => object && object.device_id ? { text: `${object.name || object.device_id}\n${object.last_speed_mps === null ? "" : Math.round(object.last_speed_mps * 3.6) + " km/h · "}${deviceStatus(object)}` } : null,
        onClick: ({ object }) => { if (object && object.device_id) selectDevice(object.device_id); } });
    } else {
      state.fleet.deck.setProps({ layers, initialViewState: track.length ? { longitude: center[0], latitude: center[1], zoom, pitch: 0, transitionDuration: 400 } : undefined });
    }
    $("#fleet-legend").innerHTML = Object.entries(FLEET_RGB).map(([k, c]) => `<span style="--c:rgb(${c.join(",")})">${k}</span>`).join("") + `<span class="line" style="--c:var(--series-1)">selected track</span>`;
  }

  async function materializeTrack() {
    const id = state.fleet.device;
    if (!id || !state.fleet.track.length) return;
    const since = state.fleet.track[0].t, until = state.fleet.track[state.fleet.track.length - 1].t;
    const consent = window.prompt("Consent reference for this real-world run (leave empty if not yet recorded):", "") || "";
    try {
      const result = await api(`/api/fleet/devices/${encodeURIComponent(id)}/materialize`, { method: "POST", body: { since, until, consent_ref: consent || undefined } });
      toast(`run ${result.run_id} created with ${result.samples} samples — finish it with a quality report to count toward coverage`, 6000);
    } catch (err) { toast(err.message); }
  }

  // ----------------------------------------------------------------- planner
  const PLAN_FIELDS = {
    scenes: "#plan-scenes", clip_seconds: "#plan-clip", fps: "#plan-fps", cameras: "#plan-cameras", resolution: "#plan-resolution",
    program: "#plan-program", gpu: "#plan-gpu", gpus: "#plan-gpus", usd_per_gpu_hour: "#plan-price", usd_per_gb_month: "#plan-storage-price", retention_months: "#plan-retention",
  };
  const usd = (n) => "$" + Math.round(n).toLocaleString("en-US");

  /** Fill the selects from /api/planner/options so the form can never drift from the API. */
  async function loadPlannerOptions() {
    if (state.planner.options) return state.planner.options;
    const options = await api("/api/planner/options");
    state.planner.options = options;
    const fill = (sel, items, label) => {
      const el = $(sel);
      el.innerHTML = items.map((i) => `<option value="${esc(i.id)}">${esc(label(i))}</option>`).join("");
    };
    fill("#plan-resolution", options.resolutions, (r) => `${r.id} · ${r.mbps} Mbit/s`);
    fill("#plan-program", options.programs, (p) => p.label);
    fill("#plan-gpu", options.gpus, (g) => `${g.label} · $${g.usd_per_hour.toFixed(2)}/h`);
    for (const [key, sel] of Object.entries(PLAN_FIELDS)) {
      const value = options.defaults[key];
      if (value !== null && value !== undefined && $(sel).value === "") $(sel).value = value;
      else if (value !== null && value !== undefined && $(sel).tagName === "SELECT") $(sel).value = value;
    }
    $("#plan-interruptible").checked = !!options.defaults.interruptible;
    return options;
  }

  function plannerQuery() {
    const params = new URLSearchParams();
    for (const [key, sel] of Object.entries(PLAN_FIELDS)) {
      const value = $(sel).value.trim();
      if (value !== "") params.set(key, value);
    }
    if ($("#plan-interruptible").checked) params.set("interruptible", "true");
    return params;
  }

  async function loadPlanner() {
    await loadPlannerOptions();
    const params = plannerQuery();
    const plan = await api("/api/planner?" + params.toString());
    state.planner.plan = plan;
    setHash("planner", String(plan.input.scenes));
    renderPlan(plan);
  }

  function renderPlan(plan) {
    const d = plan.dataset, c = plan.compute;
    const band = (b, f) => `${f(b.low)} – ${f(b.high)}`;
    $("#plan-summary").innerHTML = `
      <div class="tier-banner">
        <b>${esc(plan.tier.label)}</b><span class="muted">${esc(plan.tier.range)}</span>
        <span>${esc(plan.tier.note)}</span>
        ${plan.tier.next_tier_at ? `<span class="muted">next tier at ${plan.tier.next_tier_at.toLocaleString("en-US")} scenes</span>` : ""}
      </div>
      <div class="plan-tiles">
        <div class="plan-tile"><div class="k">Frames</div><div class="v">${(d.frames / 1e6).toFixed(1)}<small>M</small></div><div class="r">${d.video_hours.toLocaleString("en-US")} h of video</div></div>
        <div class="plan-tile"><div class="k">Storage</div><div class="v">${d.total_gb >= 1000 ? (d.total_gb / 1000).toFixed(2) + "<small>TB</small>" : d.total_gb + "<small>GB</small>"}</div><div class="r">${d.raw_gb} GB raw · ${d.per_scene_mb} MB/scene</div></div>
        <div class="plan-tile"><div class="k">GPU-hours</div><div class="v">${Math.round(c.gpu_hours.expected).toLocaleString("en-US")}</div><div class="r">${band(c.gpu_hours, (n) => Math.round(n).toLocaleString("en-US"))}</div></div>
        <div class="plan-tile"><div class="k">Wall clock</div><div class="v">${fmt.num(c.wall_clock_days.expected, 1)}<small>d</small></div><div class="r">${c.gpus} × ${esc(c.gpu)} · ${Math.round(c.parallel_efficiency * 100)} % scaling</div></div>
        <div class="plan-tile"><div class="k">Compute cost</div><div class="v">${usd(c.cost_usd.expected)}</div><div class="r">${band(c.cost_usd, usd)} at $${c.usd_per_gpu_hour}/h${c.interruptible ? " interruptible" : ""}</div></div>
        <div class="plan-tile"><div class="k">Storage cost</div><div class="v">${usd(plan.storage.cost_usd)}</div><div class="r">${plan.storage.months} month(s) at $${plan.storage.usd_per_gb_month}/GB</div></div>
        <div class="plan-tile"><div class="k">Total</div><div class="v">${usd(plan.total_usd.expected)}</div><div class="r">${band(plan.total_usd, usd)}</div></div>
      </div>`;
    barChart($("#plan-gpu-chart"), {
      rows: plan.compare.by_gpu.map((g) => ({ label: `${g.label} · ${Math.round(g.gpu_hours).toLocaleString("en-US")} h`, values: [g.cost_usd] })),
      series: [{ name: "expected cost" }], format: usd,
    });
    barChart($("#plan-program-chart"), {
      rows: plan.compare.by_program.map((p) => ({ label: p.label, values: [p.cost_usd] })),
      series: [{ name: "expected cost" }], format: usd,
    });
    const cov = plan.coverage;
    $("#plan-coverage").innerHTML = `
      <div class="coverage-bar"><i style="width:${Math.min(100, cov.progress_pct)}%"></i></div>
      <div class="muted">${cov.equivalent_scenes.toLocaleString("en-US")} of ${cov.target_scenes.toLocaleString("en-US")} scenes (${fmt.num(cov.progress_pct, 1)} %) — accepted collection time cut into ${fmt.num(plan.input.clip_seconds, 0)} s clips.</div>
      <table class="data-table"><tbody>
        <tr><td>Runs in the catalog</td><td class="num">${cov.runs}</td><td>${cov.vehicle_hours} h vehicle · ${cov.sim_hours} h sim</td></tr>
        <tr><td>Accepted collection time</td><td class="num">${cov.accepted_hours} h</td><td>counts toward the target</td></tr>
        <tr><td>Scenario library</td><td class="num">${cov.scenario_variants}</td><td>${cov.scenario_families} families available for synthetic generation</td></tr>
        <tr><td>Registered clips</td><td class="num">${cov.clips}</td><td>${cov.clip_gb} GB stored</td></tr>
        <tr><td>Still to collect or generate</td><td class="num">${cov.scenes_remaining.toLocaleString("en-US")}</td><td>${cov.collection_hours_remaining.toLocaleString("en-US")} h of driving at this clip length</td></tr>
      </tbody></table>`;
    $("#plan-assumptions").innerHTML = plan.assumptions.map((a) => `<li>${esc(a)}</li>`).join("");
  }

  async function copyPlan() {
    if (!state.planner.plan) return toast("run a plan first");
    const text = JSON.stringify(state.planner.plan, null, 2);
    try {
      await navigator.clipboard.writeText(text);
      toast("plan JSON copied to the clipboard");
    } catch (_) {
      toast("clipboard blocked — the same JSON is at /api/planner?" + plannerQuery().toString(), 6000);
    }
  }

  // ------------------------------------------------------------------ wiring
  const loaders = { overview: loadOverview, runs: loadRuns, scenarios: loadScenarios, evaluations: loadEvaluations, governance: loadGovernance, fleet: loadFleet, planner: loadPlanner };

  function setHash(view, id) {
    const next = "#" + view + (id ? "/" + encodeURIComponent(id) : "");
    if (location.hash !== next) history.replaceState(null, "", next);
  }

  function parseHash() {
    const m = /^#([a-z]+)(?:\/(.+))?$/.exec(location.hash || "");
    return m && loaders[m[1]] ? { view: m[1], id: m[2] ? decodeURIComponent(m[2]) : null } : null;
  }

  async function showView(view, id) {
    state.view = view;
    setHash(view, id);
    document.querySelectorAll(".tab").forEach((t) => t.classList.toggle("is-active", t.dataset.view === view));
    document.querySelectorAll(".view").forEach((v) => v.classList.toggle("is-active", v.id === "view-" + view));
    if (view !== "fleet") clearTimeout(state.fleet.timer);
    if (!state.token) return;
    if (id && view === "runs") state.pendingRun = id;
    if (id && view === "scenarios") { const sel = $("#scenario-family"); if (![...sel.options].some((o) => o.value === id)) sel.add(new Option(id, id)); sel.value = id; }
    if (id && view === "fleet") state.fleet.device = id;
    if (id && view === "planner" && Number(id) > 0) $("#plan-scenes").value = Number(id);
    try {
      await loaders[view]();
      if (id && view === "evaluations") selectEvaluation(id);
    } catch (err) { toast(err.message, 5000); }
  }

  async function connect() {
    state.token = $("#auth-token").value.trim();
    state.tenant = $("#auth-tenant").value.trim();
    localStorage.setItem("atlas.token", state.token);
    localStorage.setItem("atlas.tenant", state.tenant);
    try {
      await api("/api/kpis");
      $("#auth-status").textContent = "connected" + (state.tenant ? " · " + state.tenant : "");
      $("#auth-status").classList.add("ok");
      const target = parseHash();
      showView(target ? target.view : state.view, target ? target.id : null);
    } catch (err) {
      $("#auth-status").textContent = err.message;
      $("#auth-status").classList.remove("ok");
    }
  }

  document.addEventListener("DOMContentLoaded", () => {
    document.querySelectorAll(".tab").forEach((t) => t.addEventListener("click", () => showView(t.dataset.view)));
    $("#auth-form").addEventListener("submit", (ev) => { ev.preventDefault(); connect(); });
    $("#auth-token").value = state.token;
    $("#auth-tenant").value = state.tenant;
    $("#run-refresh").addEventListener("click", loadRuns);
    $("#run-source").addEventListener("change", loadRuns);
    $("#run-quality").addEventListener("change", loadRuns);
    ["#scenario-group", "#scenario-family", "#scenario-review"].forEach((id) => $(id).addEventListener("change", loadScenarios));
    $("#play-btn").addEventListener("click", togglePlayback);
    $("#time-slider").addEventListener("input", (ev) => { stopPlayback(); const pb = state.playback; updatePlayback(pb.t0 + (Number(ev.target.value) / 1000) * (pb.t1 - pb.t0)); });
    $("#evidence-btn").addEventListener("click", exportEvidence);
    const menu = $("#export-menu"), menuBtn = $("#export-btn");
    menuBtn.addEventListener("click", (ev) => { ev.stopPropagation(); menu.hidden = !menu.hidden; menuBtn.setAttribute("aria-expanded", String(!menu.hidden)); });
    document.addEventListener("click", () => { menu.hidden = true; menuBtn.setAttribute("aria-expanded", "false"); });
    menu.querySelectorAll("button[data-export]").forEach((b) => b.addEventListener("click", () => exportRun(b.dataset.export)));
    menu.querySelector("button[data-foxglove]").addEventListener("click", openInFoxglove);
    $("#trend-kpi").addEventListener("change", renderTrend);
    $("#fleet-refresh").addEventListener("click", () => loadFleet().catch((err) => toast(err.message)));
    $("#fleet-auto").addEventListener("change", scheduleFleetRefresh);
    $("#fleet-window").addEventListener("change", () => { if (state.fleet.device) selectDevice(state.fleet.device).catch((err) => toast(err.message)); });
    $("#fleet-materialize").addEventListener("click", materializeTrack);
    $("#planner-form").addEventListener("submit", (ev) => { ev.preventDefault(); loadPlanner().catch((err) => toast(err.message)); });
    $("#plan-copy").addEventListener("click", copyPlan);
    $("#plan-program").addEventListener("change", () => loadPlanner().catch((err) => toast(err.message)));
    $("#plan-gpu").addEventListener("change", () => loadPlanner().catch((err) => toast(err.message)));
    $("#plan-interruptible").addEventListener("change", () => loadPlanner().catch((err) => toast(err.message)));
    $("#fleet-table").addEventListener("click", (ev) => { const b = ev.target.closest("button[data-track]"); if (b) { ev.stopPropagation(); selectDevice(b.dataset.track).catch((err) => toast(err.message)); } });
    window.addEventListener("hashchange", () => { const target = parseHash(); if (target && state.token && (target.view !== state.view)) showView(target.view, target.id); });
    window.addEventListener("resize", () => { if (state.view === "runs" && state.samples.length) renderTimeline(); if (state.view === "overview") renderTrend(); if (state.view === "planner" && state.planner.plan) renderPlan(state.planner.plan); });
    const initial = parseHash();
    if (initial) state.view = initial.view;
    if (state.token) connect(); else showView(state.view, initial ? initial.id : null);
  });
})();
