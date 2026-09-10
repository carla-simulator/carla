import type { RouteContext } from "../router";
import { json } from "../util";
import { buildScene, listScenes } from "../scenes";

/**
 * The demo scenes carry no tenant data — they are generated from a fixed seed —
 * so they are served without authentication and cached. That is deliberate: the
 * scene viewer is the one tab that works on a fresh deployment, before a token
 * exists or a single run has been ingested.
 */
const CACHE = { "cache-control": "public, max-age=3600" };

/** GET /api/scenes — the demo scene catalog, without the per-frame tracks. */
export function getScenes(_c: RouteContext): Response {
  return json({ scenes: listScenes() }, 200, CACHE);
}

/** GET /api/scenes/:id — one scene with ego, actor tracks, road and events. */
export function getScene(c: RouteContext): Response {
  const scene = buildScene(c.params.id);
  if (!scene) return json({ error: `unknown scene: ${c.params.id}` }, 404);
  return json(scene, 200, CACHE);
}
