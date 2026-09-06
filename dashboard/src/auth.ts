import type { Env, Principal, Role } from "./types";
import { HttpError, sha256 } from "./util";

const ROLE_RANK: Record<Role, number> = { reader: 1, writer: 2, admin: 3 };

interface TokenEntry {
  tenant: string;
  role: Role;
}

/** Parse the API_TOKENS secret: "token=tenant:role;token2=tenant2:reader". */
export function parseTokens(raw: string | undefined): Map<string, TokenEntry> {
  const map = new Map<string, TokenEntry>();
  if (!raw) return map;
  for (const part of raw.split(";")) {
    const trimmed = part.trim();
    if (!trimmed) continue;
    const eq = trimmed.indexOf("=");
    if (eq <= 0) continue;
    const token = trimmed.slice(0, eq).trim();
    const [tenant, role] = trimmed.slice(eq + 1).split(":").map((s) => s.trim());
    if (!token || !tenant) continue;
    const r = (role || "reader") as Role;
    if (!(r in ROLE_RANK)) continue;
    map.set(token, { tenant, role: r });
  }
  return map;
}

function constantTimeEqual(a: string, b: string): boolean {
  if (a.length !== b.length) return false;
  let diff = 0;
  for (let i = 0; i < a.length; i++) diff |= a.charCodeAt(i) ^ b.charCodeAt(i);
  return diff === 0;
}

export async function authenticate(request: Request, env: Env, minimum: Role): Promise<Principal> {
  const header = request.headers.get("authorization") || "";
  const token = header.startsWith("Bearer ") ? header.slice(7).trim() : new URL(request.url).searchParams.get("token") || "";
  if (!token) throw new HttpError(401, "missing bearer token");
  const tokens = parseTokens(env.API_TOKENS);
  if (tokens.size === 0) throw new HttpError(503, "API_TOKENS secret is not configured");
  let entry: TokenEntry | undefined;
  for (const [candidate, value] of tokens) {
    if (constantTimeEqual(candidate, token)) {
      entry = value;
      break;
    }
  }
  if (!entry) throw new HttpError(401, "invalid token");
  if (ROLE_RANK[entry.role] < ROLE_RANK[minimum]) throw new HttpError(403, `${minimum} role required`);
  // A token may be scoped to a tenant; the X-Tenant header is only honoured for admins.
  const requested = request.headers.get("x-tenant");
  const tenant = entry.role === "admin" && requested ? requested : entry.tenant;
  const actor = (await sha256(token)).slice(7, 13);
  return { tenant, role: entry.role, actor };
}
