-- Leaderboard-style driving score per run and live fleet positions
-- (Traccar / OsmAnd / generic position posts).

ALTER TABLE runs ADD COLUMN driving_score REAL;
ALTER TABLE runs ADD COLUMN route_completion REAL;
ALTER TABLE runs ADD COLUMN infraction_penalty REAL;
ALTER TABLE runs ADD COLUMN infractions TEXT;          -- JSON [{event_class, kind, count, penalty}]

CREATE TABLE IF NOT EXISTS fleet_devices (
  tenant_id TEXT NOT NULL,
  device_id TEXT NOT NULL,
  name TEXT,
  vehicle_class TEXT NOT NULL DEFAULT 'truck',
  source TEXT NOT NULL DEFAULT 'generic',       -- traccar | osmand | generic
  last_t REAL,
  last_lat REAL, last_lon REAL, last_alt REAL,
  last_speed_mps REAL,
  last_heading_deg REAL,
  position_count INTEGER NOT NULL DEFAULT 0,
  attributes TEXT,                               -- JSON, last device attributes
  created_at TEXT NOT NULL DEFAULT (datetime('now')),
  updated_at TEXT,
  PRIMARY KEY (tenant_id, device_id)
);

CREATE TABLE IF NOT EXISTS fleet_positions (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  tenant_id TEXT NOT NULL,
  device_id TEXT NOT NULL,
  t REAL NOT NULL,
  lat REAL NOT NULL,
  lon REAL NOT NULL,
  alt REAL,
  speed_mps REAL,
  heading_deg REAL,
  accuracy_m REAL,
  source TEXT NOT NULL DEFAULT 'generic',
  attributes TEXT,                               -- JSON
  created_at TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS fleet_positions_device_t ON fleet_positions(tenant_id, device_id, t DESC);
CREATE UNIQUE INDEX IF NOT EXISTS fleet_positions_unique ON fleet_positions(tenant_id, device_id, t);
