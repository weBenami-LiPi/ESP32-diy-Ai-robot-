#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

const char INDEX_HTML[] = R"rawptr(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Vex AI</title><style>
:root{--bg:#0b0b0f;--card:#15151e;--accent:#00e5ff;--danger:#ff3d00;--glass:rgba(255,255,255,0.05)}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent;outline:none;user-select:none}
body{margin:0;font:14px 'Segoe UI',sans-serif;background:var(--bg);color:#eee;height:100vh;height:100dvh;display:flex;flex-direction:column;overflow:hidden}
nav{display:flex;background:#1a1a24;border-bottom:1px solid var(--glass);flex-shrink:0}
nav a{flex:1;padding:12px;text-align:center;cursor:pointer;font-size:22px;transition:.3s;opacity:.4}
nav a.active-nav{opacity:1;border-bottom:3px solid var(--accent);background:var(--glass)}
.v{display:none;flex:1;flex-direction:column;padding:8px;overflow:hidden;height:100%}.active{display:flex}
#m,#lc{flex:1;background:var(--card);border-radius:12px;padding:12px;overflow-y:auto;margin-bottom:8px;border:1px solid var(--glass);display:flex;flex-direction:column;gap:6px}
.u,.b{padding:8px 12px;border-radius:12px;max-width:85%;line-height:1.4}
.u{align-self:flex-end;background:linear-gradient(45deg,#00b0ff,#00e5ff);color:#000;font-weight:600}
.b{align-self:flex-start;background:var(--glass);border:1px solid rgba(255,255,255,0.1)}
.r{display:flex;gap:6px;flex-shrink:0}input,textarea,button{padding:10px;border:none;border-radius:10px;background:var(--card);color:#fff;border:1px solid var(--glass);width:100%}
button{cursor:pointer;font-weight:700;transition:.2s;background:var(--glass)}button:active{transform:scale(0.92)}
.g{display:grid;grid-template-columns:repeat(6,1fr);grid-template-rows:repeat(3,1fr);gap:6px;padding:10px;background:rgba(0,0,0,0.2);border-radius:12px;width:100%;flex:1;margin:0;min-height:0;max-height:40vh}
.g button{width:100%;height:100%;min-height:50px;font-size:26px;padding:8px;background:var(--card);border:1px solid var(--glass);border-radius:10px;display:flex;justify-content:center;align-items:center;transition:.2s}
.g button:active{transform:scale(0.9);background:var(--accent);border-color:var(--accent)}
.joy-layout{flex:1;display:flex;flex-direction:column;justify-content:space-between;align-items:center;padding:10px 0}
.split-wrap{display:flex;width:100%;justify-content:space-between;align-items:center;padding:0 5px;position:relative;flex:1}
.joy-stick{position:relative;width:35vw;height:35vw;max-width:160px;max-height:160px;background:rgba(255,255,255,0.02);border-radius:50%;border:1px solid var(--glass);display:flex;justify-content:center;align-items:center}
.dbtn{width:13vw;height:13vw;max-width:60px;max-height:60px;border-radius:50%;background:var(--card);border:1px solid var(--glass);font-size:18px;display:flex;justify-content:center;align-items:center;box-shadow:0 5px 15px rgba(0,0,0,0.5);transition:.1s;position:absolute}
.dbtn:active{background:var(--accent);color:#000;box-shadow:0 0 15px var(--accent);z-index:5}
.btn-f{top:-8px}.btn-b{bottom:-8px}.btn-l{left:-8px}.btn-r{right:-8px}
.btn-stop{width:16vw;height:16vw;max-width:75px;max-height:75px;background:rgba(255,61,0,0.15);border:2px solid var(--danger);color:var(--danger);font-size:12px;font-weight:900;border-radius:50%;z-index:10;display:flex;justify-content:center;align-items:center;padding:0}
.auto-pill{width:80%;max-width:250px;background:rgba(0,229,255,0.05);color:var(--accent);border:1px solid var(--accent);padding:14px;border-radius:30px;font-size:14px;font-weight:700;letter-spacing:1px;margin-top:10px;flex-shrink:0}
.auto-pill.on{background:var(--accent);color:#000;box-shadow:0 0 20px var(--accent)}
.l{background:#000;color:#0f0;font:11px monospace;padding:10px;flex:1;overflow:auto;border-radius:10px;border:1px solid var(--glass);margin:5px 0}
@media(min-width:600px){.joy-stick{width:150px;height:150px}.dbtn{width:65px;height:65px}.btn-stop{width:85px;height:85px}}
</style></head><body>
<nav><a onclick="v('c')" class="active-nav">💬</a><a onclick="v('rc')">🕹️</a><a onclick="v('st')">⚙️</a><a onclick="v('lg')">📜</a></nav>
<div style="background:rgba(0,229,255,0.05);padding:8px 15px;font-size:11px;display:flex;justify-content:space-between;border-bottom:1px solid var(--glass);flex-shrink:0">
<span>SYSTEM: ACTIVE</span><span style="color:var(--accent)">🔋 100%</span></div>
<div id="c" class="v active"><div id="m"></div><div class="r"><input id="i" placeholder="Ask Vextor..."><button onclick="sd()" style="width:80px">SEND</button></div></div>
<div id="rc" class="v">
<div class="g" id="e"></div>
<div class="joy-layout">
<div class="split-wrap">
  <div class="joy-stick">
    <button class="dbtn btn-l" onmousedown="mv('LEFT')" onmouseup="mv('STOP')" ontouchstart="mv('LEFT')" ontouchend="mv('STOP')">◀</button>
    <button class="dbtn btn-r" onmousedown="mv('RIGHT')" onmouseup="mv('STOP')" ontouchstart="mv('RIGHT')" ontouchend="mv('STOP')">▶</button>
    <div style="color:var(--accent);font-size:10px;font-weight:900;opacity:0.2">STEER</div>
  </div>
  <button class="btn-stop" onclick="mv('STOP')">STOP</button>
  <div class="joy-stick">
    <button class="dbtn btn-f" onmousedown="mv('FORWARD')" onmouseup="mv('STOP')" ontouchstart="mv('FORWARD')" ontouchend="mv('STOP')">▲</button>
    <button class="dbtn btn-b" onmousedown="mv('BACK')" onmouseup="mv('STOP')" ontouchstart="mv('BACK')" ontouchend="mv('STOP')">▼</button>
    <div style="color:var(--accent);font-size:10px;font-weight:900;opacity:0.2">DRIVE</div>
  </div>
</div>
<button id="at" class="auto-pill" onclick="ta()">AUTO PILOT</button>
</div></div>
<div id="st" class="v">
<label>Gemini API Key</label><input id="sk" placeholder="Gemini Key">
<label>ElevenLabs Key</label><input id="se" placeholder="ElevenLabs">
<label>Voice ID</label><input id="sv" placeholder="Voice ID">
<label>Persona (Prompt)</label><textarea id="sp" rows="6"></textarea>
<button onclick="sc()" style="background:var(--accent);color:#000;margin-top:10px">SAVE SETTINGS</button></div>
<div id="lg" class="v"><h4 style="margin:5px 0">CHAT HISTORY</h4><div id="lc"></div><h4 style="margin:5px 0">LOCATION LOGS</h4><div class="l" id="ls"></div>
<button onclick="rl()" style="width:100%;margin-bottom:5px">REFRESH HISTORY</button>
<button onclick="cl()" style="width:100%;background:rgba(255,0,0,0.1);color:#ff5555">CLEAR HISTORY</button></div>
<script>
const em=['😐','😍','😂','😴','😉','😈','😤','😮','😢','💀','😵','🥳','🧐','😠','😇','😭','😱','😫'], f=u=>fetch(u);
const v=n=>{document.querySelectorAll('.v').forEach(x=>x.classList.remove('active'));document.querySelectorAll('nav a').forEach(x=>x.classList.remove('active-nav'));document.getElementById(n).classList.add('active');const idx={c:0,rc:1,st:2,lg:3}[n];document.querySelectorAll('nav a')[idx].classList.add('active-nav');if(n=='lg')rl();if(n=='st')ld()};
const ad=(t,s)=>{const g=document.getElementById('m');g.innerHTML+=`<div class="${s}">${t}</div>`;g.scrollTop=g.scrollHeight};
const sd=async()=>{const i=document.getElementById('i'),t=i.value;if(!t)return;ad(t,'u');i.value='';const r=await(await f('/chat?msg='+encodeURIComponent(t))).json();ad(r.clean_reply||r.reply,'b')};
const mv=d=>f('/control?dir='+d), sp=v=>f('/speed?val='+v);
const ta=async()=>{const r=await(await f('/control?dir=AUTO')).text();const b=document.getElementById('at');if(r.includes('ON')){b.classList.add('on');b.innerText='AUTO: ACTIVE'}else{b.classList.remove('on');b.innerText='AUTO PILOT'}};
const rl=async()=>{
  const chat = await(await f('/chat_logs')).text();
  const loc = await(await f('/loc_logs')).text();
  document.getElementById('ls').innerText = loc;
  const lc = document.getElementById('lc'); lc.innerHTML = '';
  chat.split('\n').filter(l=>l.trim()).forEach(l=>{
    const s = l.startsWith('User:')?'u':'b';
    lc.innerHTML += `<div class="${s}">${l.replace(/^(User:|Vextor:)/,'')}</div>`;
  });
  lc.scrollTop = lc.scrollHeight;
};
const cl=async()=>{await f('/clear_chat');await f('/clear_loc');rl();};
const ld=async()=>{const c=await(await f('/get_config')).json();document.getElementById('sk').value=c.webui_gemini_key;document.getElementById('se').value=c.webui_elevenlabs_key;document.getElementById('sv').value=c.voice_id;document.getElementById('sp').value=c.system_prompt;};
const sc=async()=>{const d={webui_gemini_key:document.getElementById('sk').value,webui_elevenlabs_key:document.getElementById('se').value,voice_id:document.getElementById('sv').value,system_prompt:document.getElementById('sp').value};await fetch('/save_config',{method:'POST',body:JSON.stringify(d)});alert('Saved')};
window.onload=()=>{
  const e=document.getElementById('e');
  em.forEach((x,i)=>{const b=document.createElement('button');b.innerHTML=x;b.style.padding='8px';b.onclick=()=>f('/emotion?id='+i);e.appendChild(b)});
  rl(); // Initial load of history
};
document.getElementById('i').onkeypress=e=>{if(e.key=='Enter')sd()};
</script></body></html>
)rawptr";
#endif
