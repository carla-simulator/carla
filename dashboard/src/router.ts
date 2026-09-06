import type { Env } from "./types";

export interface RouteContext {
  request: Request;
  env: Env;
  ctx: ExecutionContext;
  params: Record<string, string>;
  url: URL;
}

export type Handler = (c: RouteContext) => Promise<Response> | Response;

interface Route {
  method: string;
  pattern: RegExp;
  keys: string[];
  handler: Handler;
}

export class Router {
  private routes: Route[] = [];

  add(method: string, path: string, handler: Handler): this {
    const keys: string[] = [];
    const source = path
      .replace(/\//g, "\\/")
      .replace(/:([A-Za-z_]+)/g, (_m, key: string) => {
        keys.push(key);
        return "([^\\/]+)";
      });
    this.routes.push({ method, pattern: new RegExp(`^${source}\\/?$`), keys, handler });
    return this;
  }

  get(path: string, handler: Handler): this { return this.add("GET", path, handler); }
  post(path: string, handler: Handler): this { return this.add("POST", path, handler); }
  put(path: string, handler: Handler): this { return this.add("PUT", path, handler); }
  delete(path: string, handler: Handler): this { return this.add("DELETE", path, handler); }

  match(method: string, pathname: string): { handler: Handler; params: Record<string, string> } | null {
    for (const route of this.routes) {
      if (route.method !== method && !(route.method === "GET" && method === "HEAD")) continue;
      const m = route.pattern.exec(pathname);
      if (!m) continue;
      const params: Record<string, string> = {};
      route.keys.forEach((key, i) => (params[key] = decodeURIComponent(m[i + 1])));
      return { handler: route.handler, params };
    }
    return null;
  }
}
