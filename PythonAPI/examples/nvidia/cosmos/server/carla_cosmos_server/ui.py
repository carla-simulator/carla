"""Minimal HTML status page at ``/ui`` (token kept in the browser's localStorage)."""

PAGE = r"""<!doctype html>
<html lang="en"><head><meta charset="utf-8"><title>carla-cosmos</title>
<style>
:root{--bg:#f6f7f4;--fg:#1e2320;--mute:#6a736c;--line:#d7dcd5;--acc:#2f6f5e;--bad:#a23b3b;--ok:#2f6f5e;--warn:#9a6b1f}
@media(prefers-color-scheme:dark){:root{--bg:#161a17;--fg:#e6eae4;--mute:#97a09a;--line:#2c332e;--acc:#6fc3a8;--bad:#e07a7a;--ok:#6fc3a8;--warn:#e0b25c}}
body{margin:0;background:var(--bg);color:var(--fg);font:14px/1.45 ui-sans-serif,system-ui,sans-serif}
header{display:flex;gap:1rem;align-items:center;padding:.8rem 1.2rem;border-bottom:1px solid var(--line)}
h1{font-size:1rem;margin:0;font-weight:600}h2{font-size:.85rem;text-transform:uppercase;letter-spacing:.06em;color:var(--mute);margin:1.6rem 0 .6rem}
main{padding:0 1.2rem 2rem;max-width:1100px}
input{background:transparent;color:var(--fg);border:1px solid var(--line);border-radius:4px;padding:.3rem .5rem;min-width:22rem;font-family:ui-monospace,monospace}
button{background:var(--acc);color:#fff;border:0;border-radius:4px;padding:.35rem .7rem;cursor:pointer}
table{border-collapse:collapse;width:100%;font-variant-numeric:tabular-nums}
th,td{text-align:left;padding:.35rem .5rem;border-bottom:1px solid var(--line);vertical-align:top}
th{color:var(--mute);font-weight:500}
.pill{display:inline-block;padding:0 .5rem;border-radius:999px;border:1px solid var(--line);font-size:.8rem}
.done,.ready{color:var(--ok)}.failed,.error,.dead{color:var(--bad)}.running,.busy,.preparing{color:var(--warn)}
.mute{color:var(--mute)}code{font-family:ui-monospace,monospace;font-size:.85em}
progress{width:8rem;height:.6rem}
#err{color:var(--bad)}
</style></head><body>
<header><h1>carla-cosmos</h1><span id="ready" class="pill">…</span>
<input id="tok" placeholder="bearer token" autocomplete="off"><button onclick="saveTok()">use token</button>
<span id="err"></span></header>
<main>
<h2>Workers</h2><table id="workers"><thead><tr><th>name</th><th>type</th><th>backends</th><th>GPUs</th><th>state</th><th>smoke</th><th>job</th></tr></thead><tbody></tbody></table>
<h2>Backends</h2><table id="models"><thead><tr><th>id</th><th>controls</th><th>views</th><th>fps</th><th>available</th><th>queued</th></tr></thead><tbody></tbody></table>
<h2>Jobs</h2><table id="jobs"><thead><tr><th>id</th><th>backend</th><th>priority</th><th>status</th><th>progress</th><th>created</th><th>worker</th><th>result</th></tr></thead><tbody></tbody></table>
</main>
<script>
const $=s=>document.querySelector(s);
let tok=localStorage.getItem('cosmos_token')||'';$('#tok').value=tok;
function saveTok(){tok=$('#tok').value.trim();try{localStorage.setItem('cosmos_token',tok)}catch(e){}refresh()}
async function api(p){const r=await fetch(p,{headers:{Authorization:'Bearer '+tok}});if(!r.ok)throw new Error(p+' → '+r.status);return r.json()}
function esc(s){return String(s??'').replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]))}
async function refresh(){
 try{const h=await fetch('/v1/health/ready');const j=await h.json();$('#ready').textContent=h.ok?'ready':'not ready';$('#ready').className='pill '+(h.ok?'ready':'error');
  $('#workers tbody').innerHTML=(j.workers||[]).map(w=>`<tr><td>${esc(w.name)}</td><td>${esc(w.type)}</td><td>${esc(w.backends.join(', '))}</td><td>${esc(w.gpus.join(','))||'–'}</td><td class="${esc(w.state)}">${esc(w.state)}${w.error?` <span class="mute">${esc(w.error)}</span>`:''}</td><td>${w.smoke_ok===null?'–':w.smoke_ok?'ok':'failed'}</td><td><code>${esc(w.current_job||'')}</code></td></tr>`).join('');
 }catch(e){$('#err').textContent=e.message}
 if(!tok){$('#err').textContent='enter a token to see backends and jobs';return}
 try{
  const m=await api('/v1/models');
  $('#models tbody').innerHTML=Object.values(m).map(x=>`<tr><td><code>${esc(x.contract.id)}</code></td><td>${esc(x.contract.controls.map(c=>c.name+(c.required?'*':'')).join(', '))}</td><td>${x.contract.max_views}</td><td>${esc(x.contract.fps.source.join('/'))}→${esc(x.contract.fps.model??'same')}</td><td class="${x.available?'ready':'error'}">${x.available?'yes':'no'}</td><td>${x.queued}</td></tr>`).join('');
  const jobs=await api('/v1/jobs?limit=50');
  $('#jobs tbody').innerHTML=jobs.map(j=>`<tr><td><code>${esc(j.id)}</code></td><td>${esc(j.backend)}</td><td>${esc(j.priority)}</td><td class="${esc(j.status)}">${esc(j.status)}${j.error?` <span class="mute">${esc(j.error)}</span>`:''}</td><td><progress value="${j.progress}" max="1"></progress> <span class="mute">${esc(j.message)}</span></td><td class="mute">${esc(j.created.replace('T',' ').slice(0,19))}</td><td>${esc(j.worker||'')}</td><td>${j.status==='done'?`<a href="#" onclick="showResult('${esc(j.id)}');return false">files</a>`:''}</td></tr>`).join('');
  $('#err').textContent='';
 }catch(e){$('#err').textContent=e.message}
}
async function showResult(id){const r=await api('/v1/jobs/'+id+'/result');alert(r.files.map(f=>f.name+'  '+f.size+' B').join('\n')+'\n\nGET /v1/jobs/'+id+'/result/<name> with the bearer token')}
refresh();setInterval(refresh,3000);
</script></body></html>
"""
