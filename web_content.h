#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

const char INDEX_HTML[] = R"rawptr(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
    <title>VexTor AI</title>
    <style>
        * { box-sizing: border-box; }
        body, html { height: 100%; margin: 0; padding: 0; overflow: hidden; font-family: sans-serif; background: #eee; touch-action: manipulation; }
        body { display: flex; flex-direction: column; }
        nav { display: flex; background: #333; justify-content: space-around; padding: 12px; z-index: 100; box-shadow: 0 2px 5px rgba(0,0,0,0.2); }
        nav a { color: #fff; text-decoration: none; font-weight: bold; font-size: 14px; cursor: pointer; padding: 5px 10px; border-radius: 4px; }
        nav a:active { background: #555; }
        .view { display: none !important; flex: 1; flex-direction: column; overflow: hidden; padding: 10px; }
        .view.active { display: flex !important; }
        .logs-view { flex-direction: column; gap: 10px; overflow-y: auto; }
        #chat { height: 100%; }
        .chat-container { flex: 1; display: flex; flex-direction: column; background: #fff; border-radius: 12px; border: 1px solid #ccc; overflow: hidden; position: relative; }
        #msgs { flex: 1; overflow-y: auto; padding: 15px; display: flex; flex-direction: column; gap: 10px; background: #fafafa; }
        .input-area { display: flex; padding: 10px; background: #fff; border-top: 1px solid #ddd; gap: 8px; }
        input[type="text"] { flex: 1; padding: 12px; border: 1px solid #ccc; border-radius: 25px; outline: none; font-size: 16px; transition: border 0.3s; }
        input[type="text"]:focus { border-color: #444; }
        .send-btn { padding: 0 20px; background: #333; color: white; border: none; border-radius: 25px; cursor: pointer; font-weight: bold; }
        .send-btn:active { background: #000; }
        .u { align-self: flex-end; background: #e3f2fd; color: #1565c0; padding: 10px 15px; border-radius: 18px 18px 0 18px; max-width: 85%; font-size: 14px; box-shadow: 0 1px 2px rgba(0,0,0,0.1); }
        .b { align-self: flex-start; background: #f1f8e9; color: #2e7d32; padding: 10px 15px; border-radius: 18px 18px 18px 0; max-width: 85%; font-size: 14px; box-shadow: 0 1px 2px rgba(0,0,0,0.1); }
        .box { background: #fff; padding: 15px; border-radius: 12px; border: 1px solid #ccc; margin-bottom: 10px; box-shadow: 0 2px 4px rgba(0,0,0,0.05); }
        .remote-layout { display: flex; justify-content: space-between; align-items: center; gap: 5px; }
        .ctrl-group { display: flex; flex-direction: column; align-items: center; gap: 8px; }
        .btn-row { display: flex; gap: 10px; }
        .btn-stack { display: flex; flex-direction: column; gap: 10px; }
        .drive-container { display: flex; align-items: center; gap: 15px; }
        .ctrl-btn { width: 65px; height: 65px; font-size: 28px; border-radius: 50%; background: #444; color: white; border: none; display: flex; align-items: center; justify-content: center; box-shadow: 0 5px #222; }
        .ctrl-btn:active { box-shadow: 0 2px #222; transform: translateY(3px); background: #000; }
        .stop-btn { background: #d32f2f !important; width: 55px; height: 55px; font-size: 20px; }
        .auto-btn { background: #2e7d32 !important; width: 55px; height: 55px; font-size: 14px; font-weight: bold; transition: all 0.3s; }
        .auto-btn.active { background: #4caf50 !important; box-shadow: 0 0 15px #4caf50; transform: scale(1.1); border: 2px solid white; }
        .throttle-wrap { display: flex; flex-direction: column; align-items: center; }
        .throttle { -webkit-appearance: slider-vertical; width: 25px; height: 140px; background: #ccc; outline: none; }
        .lbl { font-size: 10px; color: #666; font-weight: bold; text-transform: uppercase; letter-spacing: 1px; }
        .emo-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 6px; }
        .emo-btn { background: #f5f5f5; border: 1px solid #ddd; font-size: 24px; padding: 8px; border-radius: 8px; cursor: pointer; }
        .emo-btn:active { background: #e0e0e0; }
        .logs-view { flex-direction: column; gap: 10px; overflow-y: auto; }
        .log-box { background: #000; color: #0f0; padding: 10px; height: 150px; overflow-y: auto; font-size: 11px; font-family: 'Courier New', monospace; border-radius: 6px; }
        .log-actions { display: flex; gap: 8px; margin-top: 5px; }
        .log-actions button { flex: 1; padding: 8px; border-radius: 4px; border: none; font-weight: bold; cursor: pointer; }
    </style>
</head>
<body>
    <nav>
        <a onclick="shV('chat')">&#128172; Chat</a>
        <a onclick="shV('remote')">&#127918; Control</a>
        <a onclick="shV('logs')">&#128203; Logs</a>
    </nav>
    <div id="chat" class="view active">
        <div class="chat-container">
            <div id="msgs"></div>
            <div class="input-area">
                <input type="text" id="ipt" placeholder="Type a message..." autocomplete="off">
                <button class="send-btn" onclick="snd()">Send</button>
            </div>
        </div>
    </div>
    <div id="remote" class="view" style="overflow-y: auto;">
        <!-- Express Mood at Top -->
        <span class="lbl" style="display:block;margin-bottom:5px">Express Mood</span>
        <div class="box emo-grid" id="emG"></div>

        <div class="box remote-layout">
            <div class="ctrl-group">
                <span class="lbl">Steer</span>
                <div class="btn-row">
                    <button class="ctrl-btn" onmousedown="mv('LEFT')" onmouseup="mv('STOP')" ontouchstart="mv('LEFT')" ontouchend="mv('STOP')">&#8592;</button>
                    <button class="ctrl-btn" onmousedown="mv('RIGHT')" onmouseup="mv('STOP')" ontouchstart="mv('RIGHT')" ontouchend="mv('STOP')">&#8594;</button>
                </div>
            </div>
            
            <div class="ctrl-group">
                <button id="autoBtn" class="ctrl-btn auto-btn" onclick="mv('AUTO')">AUTO</button>
            </div>

            <div class="drive-container">
                <div class="ctrl-group">
                    <span class="lbl">Drive</span>
                    <div class="btn-stack">
                        <button class="ctrl-btn" onmousedown="mv('FORWARD')" onmouseup="mv('STOP')" ontouchstart="mv('FORWARD')" ontouchend="mv('STOP')">&#8593;</button>
                        <button class="ctrl-btn" onmousedown="mv('BACK')" onmouseup="mv('STOP')" ontouchstart="mv('BACK')" ontouchend="mv('STOP')">&#8595;</button>
                    </div>
                </div>
                <div class="throttle-wrap">
                    <span class="lbl">Spd</span>
                    <input type="range" class="throttle" min="100" max="255" value="255" oninput="upS(this.value)">
                    <span id="speedVal" style="font-size:16px">&#128293;</span>
                </div>
            </div>
        </div>
    </div>
    <div id="logs" class="view logs-view">
        <div class="box">
            <span class="lbl">Conversation History</span>
            <div class="log-box" id="lB1">...</div>
            <div class="log-actions">
                <button style="background:#444;color:#fff" onclick="rfL('chat')">Refresh</button>
                <button style="background:#c62828;color:#fff" onclick="clL('chat')">Clear</button>
            </div>
        </div>
        <div class="box">
            <span class="lbl">System & Activity Logs</span>
            <div class="log-box" id="lB2">...</div>
            <div class="log-actions">
                <button style="background:#444;color:#fff" onclick="rfL('loc')">Refresh</button>
                <button style="background:#c62828;color:#fff" onclick="clL('loc')">Clear</button>
            </div>
        </div>
    </div>
    <script>
        const ems = ['&#128528;', '&#128525;', '&#128514;', '&#128564;', '&#128521;', '&#128520;', '&#129324;', '&#128558;', '&#128546;', '&#128128;', '&#128565;', '&#129395;', '&#129320;', '&#128548;', '&#128519;', '&#128557;'];
        function shV(id) {
            document.querySelectorAll('.view').forEach(v => v.classList.remove('active'));
            document.getElementById(id).classList.add('active');
            if (id === 'logs') rfL();
        }
        async function snd() {
            const i = document.getElementById('ipt'), t = i.value;
            if (!t) return; 
            adM('You: ' + t, 'u'); 
            i.value = '';
            try {
                const r = await fetch('/chat?msg=' + encodeURIComponent(t));
                const d = await r.json(); 
                adM('Bot: ' + d.reply, 'b');
                if (d.tts_failed) spk(d.reply); // Fallback to browser voice
            } catch(e) { adM('Error connecting to robot.', 'b'); }
        }
        function spk(t) {
            const u = new SpeechSynthesisUtterance(t);
            u.lang = 'bn-BD'; // Set to Bengali
            window.speechSynthesis.speak(u);
        }
        function adM(t, y) {
            const m = document.getElementById('msgs');
            m.innerHTML += `<div class="${y}">${t}</div>`;
            m.scrollTop = m.scrollHeight;
            localStorage.setItem('vxt_chat', m.innerHTML);
        }
        async function mv(d) { 
            const r = await fetch('/control?dir=' + d);
            const status = await r.text();
            const btn = document.getElementById('autoBtn');
            if (status === 'ON') {
                btn.classList.add('active');
                btn.innerText = 'AUTO: ON';
            } else {
                btn.classList.remove('active');
                btn.innerText = 'AUTO';
            }
        }
        function em(i) { fetch('/emotion?id=' + i); }
        function upS(v) {
            fetch('/speed?val=' + v);
            document.getElementById('speedVal').innerText = v > 200 ? '&#128293;' : '&#128012;';
        }
        async function rfL(t) {
            if (!t) { rfL('chat'); rfL('loc'); return; }
            try {
                const r = await fetch('/' + t + '_logs');
                const txt = await r.text();
                document.getElementById(t === 'chat' ? 'lB1' : 'lB2').innerText = txt;
            } catch(e) {}
        }
        async function clL(t) {
            if (confirm('Clear ' + t + ' logs?')) {
                await fetch('/clear_' + t);
                rfL(t);
            }
        }
        window.onload = () => {
            const m = document.getElementById('msgs');
            m.innerHTML = localStorage.getItem('vxt_chat') || "Bot: System Online.";
            m.scrollTop = m.scrollHeight;
            const g = document.getElementById('emG');
            ems.forEach((e, i) => {
                const b = document.createElement('button');
                b.className = 'emo-btn'; b.innerHTML = e;
                b.onclick = () => em(i); g.appendChild(b);
            });
        }
        document.getElementById('ipt').onkeypress = (e) => { if (e.key === 'Enter') snd(); };
        setInterval(() => { if(document.getElementById('logs').classList.contains('active')) rfL(); }, 10000);
    </script>
</body>
</html>
)rawptr";

#endif
