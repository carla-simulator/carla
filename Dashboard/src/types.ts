export interface Env {
  DB: D1Database;
  STORAGE: R2Bucket;
  ASSETS: Fetcher;
  APP_NAME: string;
  DEFAULT_TENANT: string;
  MAX_TELEMETRY_SAMPLES_PER_CHUNK: string;
  MAX_CLIP_BYTES: string;
  MANIFEST_KEY_ID: string;
  /** Secret: "token=tenant:role;token2=tenant2:reader". */
  API_TOKENS?: string;
  /** Secret used to HMAC-sign evidence manifests. */
  MANIFEST_SIGNING_KEY?: string;
}

export type Role = "reader" | "writer" | "admin";

export interface Principal {
  tenant: string;
  role: Role;
  /** Short, non-secret identifier for audit rows (first 6 chars of token hash). */
  actor: string;
}

export interface Sample {
  t: number;
  frame: number;
  lat: number;
  lon: number;
  alt?: number;
  speed_mps?: number;
  heading_deg?: number;
  accel_x?: number;
  accel_y?: number;
  accel_z?: number;
  yaw_rate?: number;
  throttle?: number;
  brake?: number;
  steer?: number;
  grade_pct?: number;
  lead_distance_m?: number | null;
  lead_rel_speed_mps?: number | null;
  source?: string;
}

export interface EventIn {
  event_id?: string;
  t: number;
  frame?: number;
  event_class: string;
  severity?: string;
  lat?: number | null;
  lon?: number | null;
  description?: string;
  rule_id?: string | null;
  data?: unknown;
  review_status?: string;
}

export interface RunIn {
  run_id: string;
  source: string;
  vehicle_class: string;
  route_id?: string;
  scenario_id?: string | null;
  scenario_hash?: string | null;
  map_name?: string;
  odd_zone?: string;
  started_at?: number;
  ended_at?: number;
  streams?: Record<string, boolean>;
  consent_ref?: string | null;
  notes?: string;
  privacy_status?: string;
}

export interface QualityIn {
  duration_s?: number;
  distance_km?: number;
  acceptance_rate?: number;
  replay_complete?: boolean;
  passed?: boolean;
  privacy_status?: string;
  streams?: Record<string, boolean>;
  segments?: Array<{ index: number; t_start: number; t_end: number; sample_count: number; passed: boolean; gates: unknown }>;
}

export interface RunRow {
  run_id: string;
  tenant_id: string;
  source: string;
  vehicle_class: string;
  route_id: string | null;
  scenario_id: string | null;
  scenario_hash: string | null;
  map_name: string | null;
  odd_zone: string | null;
  started_at: number | null;
  ended_at: number | null;
  duration_s: number;
  distance_km: number;
  sample_count: number;
  event_count: number;
  has_video: number;
  has_gnss: number;
  has_imu: number;
  has_can: number;
  has_events: number;
  quality_status: string;
  acceptance_rate: number | null;
  replay_complete: number;
  privacy_status: string;
  consent_ref: string | null;
  notes: string | null;
  bbox_min_lat: number | null;
  bbox_min_lon: number | null;
  bbox_max_lat: number | null;
  bbox_max_lon: number | null;
  driving_score: number | null;
  route_completion: number | null;
  infraction_penalty: number | null;
  infractions: string | null;
  created_at: string;
  updated_at: string | null;
}

export interface ChunkRow {
  chunk_id: string;
  run_id: string;
  seq: number;
  r2_key: string;
  sample_count: number;
  t_start: number | null;
  t_end: number | null;
  distance_km: number;
  sha256: string;
}

export interface Counts {
  tp: number;
  fp: number;
  fn: number;
  precision?: number;
  recall?: number;
  f1?: number;
}
