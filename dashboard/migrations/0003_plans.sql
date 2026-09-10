-- Saved budget plans: a named planner input, plus the totals it produced when
-- it was saved so later drift in the reference rates is visible rather than
-- silent. Only `input` is authoritative; `saved_totals` is a snapshot.

CREATE TABLE IF NOT EXISTS plans (
  plan_id TEXT PRIMARY KEY,
  tenant_id TEXT NOT NULL,
  name TEXT NOT NULL,
  notes TEXT,
  input TEXT NOT NULL,                   -- JSON PlannerInput
  saved_totals TEXT NOT NULL,            -- JSON PlanTotals at save time
  created_by TEXT,
  created_at TEXT NOT NULL DEFAULT (datetime('now')),
  updated_at TEXT
);
CREATE INDEX IF NOT EXISTS plans_tenant_created ON plans(tenant_id, created_at DESC);
