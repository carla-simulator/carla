import type { Env, Principal } from "./types";

export async function audit(env: Env, who: Principal, action: string, target: string | null, detail?: unknown): Promise<void> {
  await env.DB.prepare("INSERT INTO audit_log (tenant_id, actor, action, target, detail) VALUES (?1, ?2, ?3, ?4, ?5)")
    .bind(who.tenant, who.actor, action, target, detail === undefined ? null : JSON.stringify(detail).slice(0, 4000))
    .run();
}
