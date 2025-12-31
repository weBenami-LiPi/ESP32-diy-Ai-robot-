#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

const char INDEX_HTML[] = R"rawptr(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no"><title>VexTor AI</title>
<style>*{box-sizing:border-box}body,html{height:100%;margin:0;padding:0;overflow:hidden;font-family:sans-serif;background:#eee;touch-action:manipulation}
body{display:flex;flex-direction:column}nav{display:flex;background:#333;justify-content:space-around;padding:12px;z-index:100;box-shadow:0 2px 5px rgba(0,0,0,.2)}
nav a{color:#fff;text-decoration:none;font-weight:700;font-size:14px;cursor:pointer;padding:5px 10px;border-radius:4px}.view{display:none!important;flex:1;flex-direction:column;overflow:hidden;padding:10px}
.view.active{display:flex!important}#chat{height:100%}.chat-container{flex:1;display:flex;flex-direction:column;background:#fff;border-radius:12px;border:1px solid #ccc;overflow:hidden;position:relative}
#msgs{flex:1;overflow-y:auto;padding:15px;display:flex;flex-direction:column;gap:10px;background:#fafafa}.input-area{display:flex;padding:10px;background:#fff;border-top:1px solid #ddd;gap:8px}
input[type=text],textarea{flex:1;padding:12px;border:1px solid #ccc;border-radius:8px;outline:0;font-size:16px}.send-btn{padding:0 20px;background:#333;color:#fff;border:none;border-radius:25px;cursor:pointer;font-weight:700}
.u{align-self:flex-end;background:#e3f2fd;color:#1565c0;padding:10px 15px;border-radius:18px 18px 0 18px;max-width:85%;font-size:14px}.b{align-self:flex-start;background:#f1f8e9;color:#2e7d32;padding:10px 15px;border-radius:18px 18px 18px 0;max-width:85%;font-size:14px}
.box{background:#fff;padding:15px;border-radius:12px;border:1px solid #ccc;margin-bottom:10px}.remote-layout{display:flex;justify-content:space-between;align-items:center;gap:5px}
.ctrl-group{display:flex;flex-direction:column;align-items:center;gap:8px}.ctrl-btn{width:65px;height:65px;font-size:28px;border-radius:50%;background:#444;color:#fff;border:none;display:flex;align-items:center;justify-content:center;box-shadow:0 5px #222}
.ctrl-btn:active{box-shadow:0 2px #222;transform:translateY(3px);background:#000}.auto-btn{background:#2e7d32!important;width:55px;height:55px;font-size:14px;font-weight:700}.auto-btn.active{background:#4caf50!important;box-shadow:0 0 15px #4caf50;transform:scale(1.1);border:2px solid #fff}
.throttle{-webkit-appearance:slider-vertical;width:25px;height:140px;background:#ccc;outline:0}.lbl{font-size:10px;color:#666;font-weight:700;text-transform:uppercase;letter-spacing:1px}.emo-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:6px}
.emo-btn{background:#f5f5f5;border:1px solid #ddd;font-size:24px;padding:8px;border-radius:8px;cursor:pointer}.log-box{background:#000;color:#0f0;padding:10px;height:120px;overflow-y:auto;font-size:11px;font-family:monospace;border-radius:6px}
.log-actions{display:flex;gap:8px;margin-top:5px}.log-actions button{flex:1;padding:8px;border-radius:4px;border:none;font-weight:700;cursor:pointer}
.set-group{display:flex;flex-direction:column;gap:5px;margin-bottom:15px}.set-group label{font-size:12px;font-weight:700;color:#555}
.save-btn{width:100%;padding:12px;background:#2e7d32;color:#fff;border:none;border-radius:8px;font-weight:700;cursor:pointer;margin-top:10px}</style></head>
<body><nav><a onclick="shV('chat')">&#128172; Chat</a><a onclick="shV('remote')">&#127918; Control</a><a onclick="shV('settings')">&#9881; Settings</a><a onclick="shV('logs')">&#128203; Logs</a></nav>
<div id="chat" class="view active"><div class="chat-container"><div id="msgs"></div><div class="input-area"><input type="text" id="ipt" placeholder="Type..." autocomplete="off"><button class="send-btn" onclick="snd()">Send</button></div></div></div>
<div id="remote" class="view" style="overflow-y:auto;"><span class="lbl">Mood</span><div class="box emo-grid" id="emG"></div><div class="box remote-layout"><div class="ctrl-group"><span class="lbl">Steer</span><div style="display:flex;gap:10px">
<button class="ctrl-btn" onmousedown="mv('LEFT')" onmouseup="mv('STOP')" ontouchstart="mv('LEFT')" ontouchend="mv('STOP')">&#8592;</button><button class="ctrl-btn" onmousedown="mv('RIGHT')" onmouseup="mv('STOP')" ontouchstart="mv('RIGHT')" ontouchend="mv('STOP')">&#8594;</button></div></div><button id="autoBtn" class="ctrl-btn auto-btn" onclick="mv('AUTO')">AUTO</button>
<div style="display:flex;align-items:center;gap:15px"><div class="ctrl-group"><span class="lbl">Drive</span><div style="display:flex;flex-direction:column;gap:10px"><button class="ctrl-btn" onmousedown="mv('FORWARD')" onmouseup="mv('STOP')" ontouchstart="mv('FORWARD')" ontouchend="mv('STOP')">&#8593;</button>
<button class="ctrl-btn" onmousedown="mv('BACK')" onmouseup="mv('STOP')" ontouchstart="mv('BACK')" ontouchend="mv('STOP')">&#8595;</button></div></div><div class="ctrl-group"><input type="range" class="throttle" min="100" max="255" value="255" oninput="upS(this.value)"><span id="speedVal">&#128293;</span></div></div></div></div>
<div id="settings" class="view" style="overflow-y:auto;"><div class="box">
<div class="set-group"><label>OpenAI Model</label><input type="text" id="cfg_model" placeholder="gpt-4o-mini"></div>
<div class="set-group"><label>Speaking Style (Persona)</label><textarea id="cfg_prompt" rows="5" placeholder="Enter robot's personality description..."></textarea></div>
<div class="set-group"><label>OpenAI UI Key (Optional)</label><input type="text" id="cfg_okey" placeholder="sk-..."></div>
<div class="set-group"><label>ElevenLabs UI Key (Optional)</label><input type="text" id="cfg_ekey" placeholder="ElevenLabs Key"></div>
<button class="save-btn" onclick="svC()">Save Configuration</button></div></div>
<div id="logs" class="view"><div class="box"><span class="lbl">Chat</span><div class="log-box" id="lB1"></div><div class="log-actions"><button onclick="rfL('chat')">Refresh</button><button onclick="clL('chat')">Clear</button></div></div>
<div class="box"><span class="lbl">System</span><div class="log-box" id="lB2"></div><div class="log-actions"><button onclick="rfL('loc')">Refresh</button><button onclick="clL('loc')">Clear</button></div></div></div>
<script>
const ems=['&#128528;','&#128525;','&#128514;','&#128564;','&#128521;','&#128520;','&#129324;','&#128558;','&#128546;','&#128128;','&#128565;','&#129395;','&#129320;','&#128548;','&#128519;','&#128557;'];
function shV(e){document.querySelectorAll(".view").forEach(v=>v.classList.remove("active")),document.getElementById(e).classList.add("active"),"logs"===e&&rfL(),"settings"===e&&ldC()}
async function snd(){const e=document.getElementById("ipt"),n=e.value;if(!n)return;adM("You: "+n,"u"),e.value="";try{const e=await fetch("/chat?msg="+encodeURIComponent(n)),r=await e.json();adM("Bot: "+(r.clean_reply||r.reply),"b"),r.tts_failed&&spk(r.clean_reply||r.reply)}catch(e){adM("Error.","b")}}
function spk(e){const n=new SpeechSynthesisUtterance(e);n.lang="bn-BD",n.onend=()=>fetch("/stop_talk"),window.speechSynthesis.speak(n)}function adM(e,n){const r=document.getElementById("msgs");r.innerHTML+=`<div class="${n}">${e}</div>`,r.scrollTop=r.scrollHeight,localStorage.setItem("vxt_chat",r.innerHTML)}
async function mv(e){const n=await(await fetch("/control?dir="+e)).text(),r=document.getElementById("autoBtn");"ON"===n?(r.classList.add("active"),r.innerText="AUTO: ON"):(r.classList.remove("active"),r.innerText="AUTO")}
function em(e){fetch("/emotion?id="+e)}function upS(e){fetch("/speed?val="+e),document.getElementById("speedVal").innerHTML=e>200?"&#128293;":"&#128012;"}
async function rfL(e){if(!e)return rfL("chat"),void rfL("loc");try{const n=await(await fetch("/"+e+"_logs")).text();document.getElementById("chat"===e?"lB1":"lB2").innerText=n}catch(e){}}
async function clL(e){confirm("Clear "+e+"?")&&(await fetch("/clear_"+e),rfL(e))}
async function ldC(){try{const e=await(await fetch("/get_config")).json();document.getElementById("cfg_prompt").value=e.system_prompt||"",document.getElementById("cfg_okey").value=e.webui_openai_key||"",document.getElementById("cfg_ekey").value=e.webui_elevenlabs_key||"",document.getElementById("cfg_model").value=e.openai_model||"gpt-4o-mini"}catch(e){}}
async function svC(){const e={system_prompt:document.getElementById("cfg_prompt").value,webui_openai_key:document.getElementById("cfg_okey").value,webui_elevenlabs_key:document.getElementById("cfg_ekey").value,openai_model:document.getElementById("cfg_model").value};try{const n=await fetch("/save_config",{method:"POST",body:JSON.stringify(e)});alert(await n.text())}catch(e){alert("Save failed")}}
window.onload=()=>{const e=document.getElementById("msgs");e.innerHTML=localStorage.getItem("vxt_chat")||"Bot: System Online.",e.scrollTop=e.scrollHeight;const n=document.getElementById("emG");ems.forEach((e,r)=>{const t=document.createElement("button");t.className="emo-btn",t.innerHTML=e,t.onclick=()=>em(r),n.appendChild(t)})};
document.getElementById("ipt").onkeypress=e=>{"Enter"===e.key&&snd()};setInterval(()=>{document.getElementById("logs").classList.contains("active")&&rfL()},1e4);
</script></body></html>
)rawptr";
#endif
