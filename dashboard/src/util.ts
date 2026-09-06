export class HttpError extends Error {
  constructor(public status: number, message: string, public detail?: unknown) {
    super(message);
  }
}

export function json(data: unknown, status = 200, headers: Record<string, string> = {}): Response {
  return new Response(JSON.stringify(data), {
    status,
    headers: { "content-type": "application/json; charset=utf-8", "cache-control": "no-store", ...headers },
  });
}

export async function readJson<T>(request: Request, maxBytes = 50 * 1024 * 1024): Promise<T> {
  const length = Number(request.headers.get("content-length") || 0);
  if (length > maxBytes) throw new HttpError(413, `body exceeds ${maxBytes} bytes`);
  const text = await request.text();
  if (text.length > maxBytes) throw new HttpError(413, `body exceeds ${maxBytes} bytes`);
  try {
    return JSON.parse(text) as T;
  } catch {
    throw new HttpError(400, "body is not valid JSON");
  }
}

export function requireString(value: unknown, name: string, maxLen = 200): string {
  if (typeof value !== "string" || !value.trim()) throw new HttpError(400, `${name} is required`);
  if (value.length > maxLen) throw new HttpError(400, `${name} is longer than ${maxLen} characters`);
  return value;
}

export function optionalString(value: unknown, maxLen = 2000): string | null {
  if (value === undefined || value === null) return null;
  if (typeof value !== "string") return String(value).slice(0, maxLen);
  return value.slice(0, maxLen);
}

export function optionalNumber(value: unknown): number | null {
  if (value === undefined || value === null || value === "") return null;
  const n = Number(value);
  return Number.isFinite(n) ? n : null;
}

const SAFE_ID = /^[A-Za-z0-9][A-Za-z0-9._:-]{0,199}$/;
export function requireId(value: unknown, name: string): string {
  const s = requireString(value, name);
  if (!SAFE_ID.test(s)) throw new HttpError(400, `${name} contains unsupported characters`);
  return s;
}

export function nowIso(): string {
  return new Date().toISOString();
}

export function canonicalJson(value: unknown): string {
  return JSON.stringify(sortKeys(value));
}

function sortKeys(value: unknown): unknown {
  if (Array.isArray(value)) return value.map(sortKeys);
  if (value && typeof value === "object") {
    const out: Record<string, unknown> = {};
    for (const key of Object.keys(value as Record<string, unknown>).sort()) {
      out[key] = sortKeys((value as Record<string, unknown>)[key]);
    }
    return out;
  }
  return value;
}

function hex(buffer: ArrayBuffer): string {
  return Array.from(new Uint8Array(buffer))
    .map((b) => b.toString(16).padStart(2, "0"))
    .join("");
}

export async function sha256(data: string | ArrayBuffer): Promise<string> {
  const bytes = typeof data === "string" ? new TextEncoder().encode(data) : data;
  return "sha256:" + hex(await crypto.subtle.digest("SHA-256", bytes));
}

export async function hmacSign(key: string, message: string): Promise<string> {
  const cryptoKey = await crypto.subtle.importKey("raw", new TextEncoder().encode(key), { name: "HMAC", hash: "SHA-256" }, false, ["sign"]);
  return "hmac-sha256:" + hex(await crypto.subtle.sign("HMAC", cryptoKey, new TextEncoder().encode(message)));
}

export function haversineKm(lat1: number, lon1: number, lat2: number, lon2: number): number {
  const r = 6371.0088;
  const p1 = (lat1 * Math.PI) / 180;
  const p2 = (lat2 * Math.PI) / 180;
  const dphi = p2 - p1;
  const dl = ((lon2 - lon1) * Math.PI) / 180;
  const a = Math.sin(dphi / 2) ** 2 + Math.cos(p1) * Math.cos(p2) * Math.sin(dl / 2) ** 2;
  return 2 * r * Math.asin(Math.sqrt(a));
}

export function parseJsonColumn<T>(value: unknown, fallback: T): T {
  if (typeof value !== "string" || !value) return fallback;
  try {
    return JSON.parse(value) as T;
  } catch {
    return fallback;
  }
}

export function uuid(): string {
  return crypto.randomUUID();
}
