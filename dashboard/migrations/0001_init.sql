-- Bhutan Mobility Atlas: D1 metadata catalog.
-- Raw telemetry and clips live in R2; D1 holds the searchable metadata,
-- quality state, review state and audit trail.

CREATE TABLE IF NOT EXISTS runs (
  run_id TEXT PRIMARY KEY,
  tenant_id TEXT NOT NULL,
  source TEXT NOT NULL,                 -- sim | vehicle
  vehicle_class TEXT NOT NULL,
  route_id TEXT,
  scenario_id TEXT,
  scenario_hash TEXT,
  map_name TEXT,
  odd_zone TEXT,
  started_at REAL,
  ended_at REAL,
  duration_s REAL NOT NULL DEFAULT 0,
  distance_km REAL NOT NULL DEFAULT 0,
  sample_count INTEGER NOT NULL DEFAULT 0,
  event_count INTEGER NOT NULL DEFAULT 0,
  has_video INTEGER NOT NULL DEFAULT 0,
  has_gnss INTEGER NOT NULL DEFAULT 0,
  has_imu INTEGER NOT NULL DEFAULT 0,
  has_can INTEGER NOT NULL DEFAULT 0,
  has_events INTEGER NOT NULL DEFAULT 0,
  quality_status TEXT NOT NULL DEFAULT 'pending',   -- pending | accepted | rejected
  acceptance_rate REAL,
  replay_complete INTEGER NOT NULL DEFAULT 0,
  privacy_status TEXT NOT NULL DEFAULT 'unprocessed', -- unprocessed | redacted | not_required
  consent_ref TEXT,
  notes TEXT,
  bbox_min_lat REAL, bbox_min_lon REAL, bbox_max_lat REAL, bbox_max_lon REAL,
  created_at TEXT NOT NULL DEFAULT (datetime('now')),
  updated_at TEXT
);
CREATE INDEX IF NOT EXISTS runs_tenant_started ON runs(tenant_id, started_at DESC);
CREATE INDEX IF NOT EXISTS runs_tenant_scenario ON runs(tenant_id, scenario_id);

CREATE TABLE IF NOT EXISTS telemetry_chunks (
  chunk_id TEXT PRIMARY KEY,            -- <run_id>:<seq>
  run_id TEXT NOT NULL,
  tenant_id TEXT NOT NULL,
  seq INTEGER NOT NULL,
  r2_key TEXT NOT NULL,
  sample_count INTEGER NOT NULL,
  t_start REAL,
  t_end REAL,
  distance_km REAL NOT NULL DEFAULT 0,
  sha256 TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS chunks_run ON telemetry_chunks(run_id, seq);

CREATE TABLE IF NOT EXISTS events (
  event_id TEXT PRIMARY KEY,
  run_id TEXT NOT NULL,
  tenant_id TEXT NOT NULL,
  t REAL NOT NULL,
  frame INTEGER,
  event_class TEXT NOT NULL,
  severity TEXT NOT NULL DEFAULT 'info',
  lat REAL, lon REAL,
  description TEXT,
  rule_id TEXT,
  data TEXT,                             -- JSON
  review_status TEXT NOT NULL DEFAULT 'unreviewed', -- unreviewed | classified | reviewed
  reviewed_by TEXT,
  reviewed_at TEXT,
  review_notes TEXT,
  created_at TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS events_run_t ON events(run_id, t);
CREATE INDEX IF NOT EXISTS events_tenant_class ON events(tenant_id, event_class);
CREATE INDEX IF NOT EXISTS events_tenant_severity ON events(tenant_id, severity, review_status);

CREATE TABLE IF NOT EXISTS segments (
  segment_id TEXT PRIMARY KEY,           -- <run_id>:<idx>
  run_id TEXT NOT NULL,
  tenant_id TEXT NOT NULL,
  idx INTEGER NOT NULL,
  t_start REAL, t_end REAL,
  sample_count INTEGER NOT NULL DEFAULT 0,
  passed INTEGER NOT NULL DEFAULT 0,
  gates TEXT                             -- JSON
);
CREATE INDEX IF NOT EXISTS segments_tenant ON segments(tenant_id, passed);

CREATE TABLE IF NOT EXISTS scenarios (
  scenario_id TEXT PRIMARY KEY,
  tenant_id TEXT NOT NULL,
  family TEXT NOT NULL,
  group_name TEXT,
  name TEXT NOT NULL,
  description TEXT,
  tags TEXT,                             -- JSON array
  params TEXT NOT NULL,                  -- JSON
  actors TEXT,                           -- JSON array
  expected_events TEXT,                  -- JSON array
  seed INTEGER,
  version TEXT,
  content_hash TEXT NOT NULL,
  route_class TEXT,
  lighting_class TEXT,
  visibility_class TEXT,
  review_status TEXT NOT NULL DEFAULT 'unreviewed', -- unreviewed | reviewed | rejected
  reviewed_by TEXT,
  reviewed_at TEXT,
  review_notes TEXT,
  created_at TEXT NOT NULL DEFAULT (datetime('now')),
  updated_at TEXT
);
CREATE INDEX IF NOT EXISTS scenarios_tenant_family ON scenarios(tenant_id, family);

CREATE TABLE IF NOT EXISTS evaluations (
  evaluation_id TEXT PRIMARY KEY,
  tenant_id TEXT NOT NULL,
  model_id TEXT NOT NULL,
  model_version TEXT,
  run_id TEXT,
  scenario_id TEXT,
  conditions TEXT,                       -- JSON
  overall TEXT NOT NULL,                 -- JSON counts
  by_class TEXT,                         -- JSON
  by_condition TEXT,                     -- JSON
  failure_clusters TEXT,                 -- JSON
  frames_evaluated INTEGER NOT NULL DEFAULT 0,
  inputs TEXT,                           -- JSON of input hashes
  reproducible INTEGER NOT NULL DEFAULT 0,
  replay_verified INTEGER NOT NULL DEFAULT 0,
  created_at TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS evaluations_tenant_model ON evaluations(tenant_id, model_id, created_at DESC);

CREATE TABLE IF NOT EXISTS clips (
  clip_id TEXT PRIMARY KEY,
  tenant_id TEXT NOT NULL,
  run_id TEXT,
  t_start REAL, t_end REAL,
  r2_key TEXT,
  content_type TEXT,
  size_bytes INTEGER NOT NULL DEFAULT 0,
  sha256 TEXT,
  redaction_status TEXT NOT NULL DEFAULT 'pending', -- pending | redacted | failed | not_required
  consent_ref TEXT,
  released INTEGER NOT NULL DEFAULT 0,
  notes TEXT,
  created_at TEXT NOT NULL DEFAULT (datetime('now')),
  updated_at TEXT
);
CREATE INDEX IF NOT EXISTS clips_tenant_run ON clips(tenant_id, run_id);

CREATE TABLE IF NOT EXISTS kpi_snapshots (
  snapshot_id TEXT PRIMARY KEY,
  tenant_id TEXT NOT NULL,
  computed_at TEXT NOT NULL,
  payload TEXT NOT NULL                  -- JSON
);
CREATE INDEX IF NOT EXISTS kpi_tenant_time ON kpi_snapshots(tenant_id, computed_at DESC);

CREATE TABLE IF NOT EXISTS audit_log (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  tenant_id TEXT NOT NULL,
  actor TEXT NOT NULL,
  action TEXT NOT NULL,
  target TEXT,
  detail TEXT,
  created_at TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS audit_tenant_time ON audit_log(tenant_id, created_at DESC);
