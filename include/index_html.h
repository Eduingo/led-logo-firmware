#pragma once
// Ovladaci webova stranka (servirovana z flash). Upravuj primo tady.
// Poradi efektu v poli META musi odpovidat tabulce effects[] v main.cpp.
#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="cs"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0d0f14">
<title>Eduingo logo</title>
<style>
:root{--bg:#0d0f14;--card:#171a22;--card2:#1f2330;--line:#2a2f3d;--fg:#f2f4f8;--mut:#8e96a6;--c:#8b5cf6;--r:18px}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html,body{margin:0;background:var(--bg);color:var(--fg);font:16px/1.4 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
body{background:radial-gradient(120% 60% at 50% -10%,color-mix(in srgb,var(--c) 28%,transparent),transparent 70%) var(--bg);min-height:100vh;transition:background .4s}
main{max-width:520px;margin:0 auto;padding:max(16px,env(safe-area-inset-top)) 16px 32px}
header{display:flex;align-items:center;justify-content:space-between;gap:12px;margin:4px 0 18px}
.brand small{display:block;color:var(--mut);font-size:13px;letter-spacing:.04em}
.logo{font-weight:800;font-size:30px;letter-spacing:.08em;line-height:1.1;background:var(--grad,var(--c));-webkit-background-clip:text;background-clip:text;color:transparent;background-size:300% 100%}
.logo.anim{animation:flow 4s linear infinite}
.logo.pulse{animation:breathe 3s ease-in-out infinite}
.logo.off{background:#3a3f4c;-webkit-background-clip:text;background-clip:text}
@keyframes flow{to{background-position:300% 0}}
@keyframes breathe{50%{opacity:.35}}
.pwr{width:56px;height:56px;border-radius:50%;border:none;background:var(--card2);color:var(--mut);display:grid;place-items:center;flex:none;transition:.25s;box-shadow:0 0 0 1px var(--line) inset}
.pwr.on{background:var(--c);color:#fff;box-shadow:0 6px 24px color-mix(in srgb,var(--c) 55%,transparent)}
.pwr svg{width:26px;height:26px}
section{background:var(--card);border:1px solid var(--line);border-radius:var(--r);padding:16px;margin-bottom:14px}
h2{display:flex;justify-content:space-between;align-items:baseline;font-size:12px;font-weight:600;text-transform:uppercase;letter-spacing:.09em;color:var(--mut);margin:0 0 12px}
h2 b{font-size:15px;color:var(--fg);letter-spacing:0;text-transform:none;font-weight:600}
.fx{display:grid;grid-template-columns:repeat(auto-fill,minmax(150px,1fr));gap:8px}
.fx button{display:grid;grid-template-columns:28px 1fr;column-gap:10px;align-items:center;text-align:left;padding:10px 12px;min-height:54px;border-radius:14px;border:1px solid var(--line);background:var(--card2);color:var(--fg);font:inherit;transition:transform .12s,border-color .2s,background .2s}
.fx button:active{transform:scale(.96)}
.fx .ic{font-size:22px;line-height:1;text-align:center}
.fx .nm{font-weight:600;font-size:15px;line-height:1.2}
.fxd{font-size:13px;color:var(--mut);margin:-4px 0 12px;min-height:18px}
.fx button.on{border-color:var(--c);background:color-mix(in srgb,var(--c) 22%,var(--card2));box-shadow:0 0 0 1px var(--c) inset}
.sl{display:flex;align-items:center;gap:12px;margin:4px 0 14px}
.sl:last-child{margin-bottom:2px}
.sl .i{width:26px;text-align:center;font-size:20px;flex:none}
.sl .v{min-width:52px;text-align:right;font-variant-numeric:tabular-nums;font-weight:600}
.lbl{font-size:14px;color:var(--mut);margin-bottom:2px}
input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:34px;background:transparent;margin:0}
input[type=range]::-webkit-slider-runnable-track{height:10px;border-radius:6px;background:var(--trk)}
input[type=range]::-moz-range-track{height:10px;border-radius:6px;background:var(--trk)}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:28px;height:28px;margin-top:-9px;border-radius:50%;background:#fff;border:3px solid var(--c);box-shadow:0 2px 8px #0008}
input[type=range]::-moz-range-thumb{width:22px;height:22px;border-radius:50%;background:#fff;border:3px solid var(--c);box-shadow:0 2px 8px #0008}
#bri{--trk:linear-gradient(90deg,#000,var(--c))}
#speed{--trk:linear-gradient(90deg,#2a2f3d,var(--c))}
.sws{display:grid;grid-template-columns:repeat(5,1fr);gap:10px}
.sw{aspect-ratio:1;border-radius:50%;border:none;background:var(--s);box-shadow:0 0 0 1px #fff2 inset;transition:transform .12s,box-shadow .2s;position:relative}
.sw:active{transform:scale(.9)}
.sw.on{box-shadow:0 0 0 3px var(--card),0 0 0 5px var(--fg)}
.pick{background:conic-gradient(red,#ff0,lime,cyan,blue,#f0f,red);overflow:hidden}
.pick input{position:absolute;inset:0;opacity:0;width:100%;height:100%;border:0;padding:0}
.pick span{position:absolute;inset:28%;border-radius:50%;background:var(--card);display:grid;place-items:center;font-size:16px;pointer-events:none}
.tg{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-top:16px;padding-top:14px;border-top:1px solid var(--line)}
.tg p{margin:0;font-size:15px}.tg small{display:block;color:var(--mut);font-size:12px}
.swt{position:relative;width:52px;height:30px;flex:none}
.swt input{opacity:0;width:0;height:0;position:absolute}
.swt i{position:absolute;inset:0;border-radius:20px;background:var(--line);transition:.2s}
.swt i:after{content:"";position:absolute;left:3px;top:3px;width:24px;height:24px;border-radius:50%;background:#fff;transition:.2s}
.swt input:checked+i{background:linear-gradient(90deg,#f43f5e,#f59e0b,#22c55e,#3b82f6,#a855f7)}
.swt input:checked+i:after{transform:translateX(22px)}
.dim{opacity:.4;transition:opacity .2s}
.hint{font-size:13px;color:var(--mut);margin:8px 0 0;display:none}
.dimh .hint{display:block}
.dimh .sl,.dimh #colInner{opacity:.4}
#toast{position:fixed;left:50%;bottom:calc(20px + env(safe-area-inset-bottom));transform:translate(-50%,120px);opacity:0;pointer-events:none;background:#e11d48;color:#fff;padding:10px 16px;border-radius:12px;font-size:14px;transition:transform .3s;box-shadow:0 8px 24px #0008}
#toast.show{transform:translate(-50%,0);opacity:1}
.dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#22c55e;margin-right:6px;vertical-align:middle;box-shadow:0 0 8px #22c55e}
.dot.bad{background:#e11d48;box-shadow:0 0 8px #e11d48}
.net{font-size:15px;line-height:1.45}
.net a{color:var(--fg);font-weight:600}
.net small{color:var(--mut);display:block;font-size:13px;margin-top:2px}
.btns{display:flex;gap:8px;margin-top:14px;flex-wrap:wrap}
.btn{flex:1;min-width:130px;padding:12px 14px;border-radius:12px;border:1px solid var(--line);background:var(--card2);color:var(--fg);font:inherit;font-weight:600}
.btn.pri{background:var(--c);border-color:var(--c);color:#fff}
.btn:disabled{opacity:.5}
.btn.ghost{background:none;color:var(--mut)}
.nets{margin-top:12px;border-radius:14px;overflow:hidden;border:1px solid var(--line)}
.nets:empty{display:none}
.nets button{display:flex;align-items:center;gap:12px;width:100%;padding:13px 14px;border:none;border-bottom:1px solid var(--line);background:var(--card2);color:var(--fg);font:inherit;text-align:left}
.nets button:last-child{border-bottom:none}
.nets .n{flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.nets .lk{color:var(--mut);font-size:13px}
.bars{display:inline-flex;align-items:flex-end;gap:2px;height:14px}
.bars i{width:4px;background:var(--line);border-radius:1px}
.bars i.f{background:var(--fg)}
.join{margin-top:12px;padding:14px;border-radius:14px;background:var(--card2);border:1px solid var(--line)}
.join input[type=text],.join input[type=password]{width:100%;padding:12px;border-radius:10px;border:1px solid var(--line);background:var(--bg);color:var(--fg);font:inherit;margin:6px 0 4px}
.join .eye{font-size:13px;color:var(--mut);display:flex;align-items:center;gap:6px;margin-top:4px}
.badge{display:inline-block;padding:2px 8px;border-radius:20px;font-size:12px;font-weight:600;background:var(--line);color:var(--fg);text-transform:none;letter-spacing:0}
.badge.ok{background:#14532d;color:#86efac}.badge.bad{background:#4c0519;color:#fda4af}.badge.wait{background:#422006;color:#fcd34d}
.spin{display:inline-block;width:14px;height:14px;border:2px solid currentColor;border-right-color:transparent;border-radius:50%;animation:rot .8s linear infinite;vertical-align:-2px;margin-right:6px}
@keyframes rot{to{transform:rotate(360deg)}}
.pv{position:relative;margin:0 0 14px;border-radius:var(--r);background:#07080b;border:1px solid var(--line);padding:30px 10px 8px;overflow:hidden}
.pv canvas{display:block;width:100%;height:auto}
.pv .tag{position:absolute;top:10px;left:12px;font-size:11px;letter-spacing:.09em;text-transform:uppercase;color:var(--mut);display:flex;align-items:center;gap:6px}
.pv .tag i{width:7px;height:7px;border-radius:50%;background:#ef4444;box-shadow:0 0 8px #ef4444;animation:breathe 1.6s infinite}
.seg{display:flex;background:var(--card2);border:1px solid var(--line);border-radius:12px;padding:3px;gap:3px}
.seg button{flex:1;padding:9px 2px;border:none;border-radius:9px;background:none;color:var(--mut);font:inherit;font-size:14px;font-weight:600;white-space:nowrap}
.seg button.on{background:var(--c);color:#fff}
.sub{font-size:13px;color:var(--mut);margin:8px 0 16px}
.sub:last-child{margin-bottom:0}
</style></head><body><main>
<header><div class="brand"><div class="logo" id="logo">EDUINGO</div><small><span class="dot" id="dot"></span><span id="conn">Připojeno</span></small></div>
<button class="pwr" id="pwr" aria-label="Zapnout / vypnout"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.4" stroke-linecap="round"><path d="M12 3v8"/><path d="M6.3 7.2a8 8 0 1 0 11.4 0"/></svg></button></header>
<div class="pv" id="pv"><span class="tag"><i></i>Živě</span><canvas id="cv"></canvas></div>

<section><h2>Efekt <b id="fxName"></b></h2><div class="fxd" id="fxDesc"></div><div class="fx" id="fx"></div></section>

<section id="params"><h2>Nastavení</h2>
<div class="lbl">Jas</div>
<div class="sl"><span class="i">🔅</span><input type="range" id="bri" min="1" max="255" aria-label="Jas"><span class="v" id="briV"></span></div>
<div id="spdBox"><div class="lbl">Rychlost</div>
<div class="sl"><span class="i">🐢</span><input type="range" id="speed" min="10" max="500" step="5" aria-label="Rychlost"><span class="v" id="spdV"></span></div>
<div class="hint">Tenhle efekt rychlost nepoužívá.</div></div>
</section>

<section id="colBox"><h2>Barva <b id="colName"></b></h2>
<div id="colInner"><div class="sws" id="sws"></div>
<label class="tg"><div><p>Pestrobarevný režim</p><small>Efekty použijí víc barev. Vypni, ať svítí jen zvolená barva.</small></div>
<span class="swt"><input type="checkbox" id="palette"><i></i></span></label></div>
<div class="hint">Tenhle efekt má vlastní barvy (duha), volba barvy se neprojeví.</div>
</section>
<section id="autoBox"><h2>Automatika</h2>
<div class="lbl">Střídat efekty</div>
<div class="seg" id="autoSeg"></div>
<div class="sub" id="autoSub"></div>
<div class="lbl">Vypnout za</div>
<div class="seg" id="sleepSeg"></div>
<div class="sub" id="sleepSub"></div>
</section>
<section id="netBox"><h2>Domácí WiFi <span class="badge" id="netBadge">…</span></h2>
<div class="net" id="netInfo"></div>
<div class="btns"><button class="btn pri" id="scanBtn">Vyhledat sítě</button><button class="btn ghost" id="forgetBtn" hidden>Zapomenout síť</button></div>
<div class="nets" id="nets"></div>
<form class="join" id="joinF" hidden>
<div class="lbl" id="ssidLbl">Název sítě</div><input type="text" id="ssidIn" autocomplete="off" autocapitalize="off" spellcheck="false" maxlength="32">
<div class="lbl">Heslo</div><input type="password" id="pwIn" autocomplete="current-password" maxlength="63">
<label class="eye"><input type="checkbox" id="pwShow"> Zobrazit heslo</label>
<div class="hint" style="display:block">Během připojování se spojení s logem může na pár sekund přerušit.</div>
<div class="btns"><button class="btn pri" type="submit" id="joinBtn">Připojit</button><button class="btn ghost" type="button" id="joinCancel">Zrušit</button></div>
</form>
</section>
<div id="toast">Logo neodpovídá, zkontroluj připojení k WiFi</div>
</main><script>
const $=id=>document.getElementById(id);
// Poradi musi odpovidat tabulce effects[] ve firmware. c=pouziva barvu, s=pouziva rychlost, p=pouziva paletu, a=animace nahledu
const META=[
 {i:'🎬',d:'Všechny efekty za sebou',c:1,s:1,p:1,a:'anim'},
 {i:'🔠',d:'Písmena jedno po druhém',c:1,s:1,p:1},
 {i:'✨',d:'Náhodné jiskřičky',c:1,s:1,p:1},
 {i:'📶',d:'Sloupce odspodu nahoru',c:1,s:1},
 {i:'🅴',d:'Po skupinách, pak logo',c:1,s:1,p:1},
 {i:'🌈',d:'Plynule běžící duha',s:1,a:'anim',g:1},
 {i:'🎨',d:'Duha přes celý pásek',s:1,a:'anim',g:1},
 {i:'🔤',d:'Každé písmeno jinou barvou',s:1,a:'anim',g:1},
 {i:'🌊',d:'Barva se přelije a zmizí',c:1,s:1},
 {i:'💫',d:'Běžící světla',c:1,s:1},
 {i:'🎠',d:'Běžící světla v duze',s:1,a:'anim',g:1},
 {i:'🫧',d:'Pomalé dýchání barvou',c:1,s:1,a:'pulse'},
 {i:'💡',d:'Svítí jednou barvou',c:1},
 {i:'⏻',d:'Všechny LED zhasnuté',off:1},
 {i:'🌀',d:'Duha jede přes celý nápis',s:1,a:'anim',g:1},
 {i:'☄️',d:'Světlá kometa s ohonem',c:1,s:1,p:1},
 {i:'🌟',d:'Jemné třpytky, které pohasínají',c:1,s:1,p:1},
 {i:'🚨',d:'Střídání levé a pravé půlky',c:1,s:1,p:1}
];
const SW=[['#ff0000','Červená'],['#ff6a00','Oranžová'],['#ffd000','Žlutá'],['#00ff40','Zelená'],['#00ffff','Tyrkysová'],['#0050ff','Modrá'],['#640a82','Fialová'],['#ff00a0','Růžová'],['#ffffff','Bílá']];
const RB='linear-gradient(90deg,#f43f5e,#f59e0b,#22c55e,#06b6d4,#6366f1,#d946ef,#f43f5e)';
let st=null,tm={},lastOn=+(localStorage.lastFx||0),offIdx=13,busy={};
const vib=()=>navigator.vibrate&&navigator.vibrate(8);
function pct(b){return Math.round(b/2.55)+' %'}
function spd(s){return (s/100).toFixed(s%100?(s%10?2:1):0).replace('.',',')+'×'}
function setAccent(c){
 // velmi tmave barvy by v UI nebyly videt -> zesvetlit pro akcent
 const n=parseInt(c.slice(1),16),r=n>>16,g=n>>8&255,b=n&255,m=Math.max(r,g,b)||1,k=m<140?140/m:1;
 document.documentElement.style.setProperty('--c',`rgb(${Math.min(255,r*k)|0},${Math.min(255,g*k)|0},${Math.min(255,b*k)|0})`);
}
function render(){
 offIdx=st.off??st.effects.length-1;
 const m=META[st.effect]||{c:1,s:1};
 if(st.effect!==offIdx)lastOn=st.effect;
 const fx=$('fx');
 if(fx.children.length!==st.effects.length){fx.innerHTML='';const ord=[...st.effects.keys()].filter(i=>i!==offIdx).concat(offIdx);ord.forEach(i=>{const n=st.effects[i];const mt=META[i]||{i:'💠',d:''};
  const b=document.createElement('button');b.innerHTML=`<span class="ic">${mt.i}</span><span class="nm"></span>`;
  b.dataset.i=i;b.querySelector('.nm').textContent=n;b.onclick=()=>{vib();send({effect:i})};fx.appendChild(b);});}
 [...fx.children].forEach(b=>b.classList.toggle('on',+b.dataset.i===st.effect));renderAuto();
 $('fxName').textContent=st.effects[st.effect];$('fxDesc').textContent=m.d||'';
 if(!busy.bri){$('bri').value=st.bri;$('briV').textContent=pct(st.bri)}
 if(!busy.speed){$('speed').value=st.speed;$('spdV').textContent=spd(st.speed)}
 $('palette').checked=st.palette;
 setAccent(st.color);
 [...$('sws').children].forEach(b=>b.classList.toggle('on',b.dataset.c===st.color.toLowerCase()));
 const pk=$('pick');pk.classList.toggle('on',!SW.some(s=>s[0]===st.color.toLowerCase()));$('pickIn').value=st.color;
 const sw=SW.find(s=>s[0]===st.color.toLowerCase());$('colName').textContent=m.c?(m.p&&st.palette?'Pestrobarevně':(sw?sw[1]:st.color.toUpperCase())):'';
 // ktere ovladace dany efekt pouziva
 $('spdBox').classList.toggle('dimh',!m.s&&!m.off);$('colBox').classList.toggle('dimh',!!m.g);
 $('params').classList.toggle('dim',!!m.off);$('colBox').classList.toggle('dim',!!m.off);
 $('pwr').classList.toggle('on',!m.off);
 const lg=$('logo');lg.className='logo'+(m.off?' off':m.a?' '+m.a:'');
 lg.style.setProperty('--grad',m.g||(m.p&&st.palette)?RB:`linear-gradient(90deg,var(--c),color-mix(in srgb,var(--c) 60%,#fff),var(--c))`);
}
function online(ok){$('dot').classList.toggle('bad',!ok);$('conn').textContent=ok?'Připojeno':'Nepřipojeno';$('toast').classList.toggle('show',!ok)}
function send(p,delay){
 const k=Object.keys(p)[0];
 if(st){Object.assign(st,p.color?{color:'#'+p.color}:p.palette!==undefined?{palette:!!p.palette}:p);render();} // okamzita odezva
 clearTimeout(tm[k]);
 tm[k]=setTimeout(()=>fetch('/api/set?'+new URLSearchParams(p)).then(r=>r.json()).then(s=>{st=s;render();online(1);try{localStorage.lastFx=lastOn}catch(e){}})
  .catch(()=>online(0)),delay||0);
}
function slider(id,key,fmt){const el=$(id);
 el.addEventListener('input',()=>{busy[key]=1;$(id==='bri'?'briV':'spdV').textContent=fmt(+el.value);send({[key]:el.value},120)});
 el.addEventListener('change',()=>{setTimeout(()=>busy[key]=0,400)});}
slider('bri','bri',pct);slider('speed','speed',spd);
$('palette').onchange=e=>{vib();send({palette:e.target.checked?1:0})};
$('pwr').onclick=()=>{vib();send({effect:st.effect===offIdx?lastOn:offIdx})};
SW.forEach(([c,n])=>{const b=document.createElement('button');b.className='sw';b.dataset.c=c;b.style.setProperty('--s',c);b.title=n;b.setAttribute('aria-label',n);
 b.onclick=()=>{vib();send({color:c.slice(1)})};$('sws').appendChild(b);});
const pk=document.createElement('label');pk.className='sw pick';pk.id='pick';pk.title='Vlastní barva';
pk.innerHTML='<input type="color" id="pickIn" aria-label="Vlastní barva"><span>＋</span>';$('sws').appendChild(pk);
$('pickIn').addEventListener('input',e=>send({color:e.target.value.slice(1)},150));
function load(){fetch('/api/state').then(r=>r.json()).then(s=>{st=s;render();online(1)}).catch(()=>online(0))}
load();setInterval(()=>{if(!document.hidden&&!Object.values(busy).some(Boolean))load()},5000);
// ---- Automatika ----
const AUTO=[[0,'Vyp'],[10,'10 s'],[30,'30 s'],[60,'1 min'],[300,'5 min']];
const SLEEP=[[0,'Vyp'],[15,'15 min'],[30,'30 min'],[60,'1 h'],[120,'2 h']];
let sleepEnd=0,sleepPick=0;
function segs(el,opts,fn){opts.forEach(([v,t])=>{const b=document.createElement('button');b.textContent=t;b.dataset.v=v;b.onclick=()=>{vib();fn(v)};el.appendChild(b)})}
segs($('autoSeg'),AUTO,v=>send({auto:v}));
segs($('sleepSeg'),SLEEP,v=>{sleepPick=v;sleepEnd=v?Date.now()+v*60000:0;send({sleep:v});renderAuto()});
function fmtT(s){const m=Math.floor(s/60);return m>=60?Math.floor(m/60)+' h '+(m%60)+' min':m>0?m+' min '+(s%60)+' s':s+' s'}
function renderAuto(){
 if(!st)return;
 [...$('autoSeg').children].forEach(b=>b.classList.toggle('on',+b.dataset.v===st.auto));
 $('autoSub').textContent=st.auto?`Každých ${AUTO.find(a=>a[0]===st.auto)?.[1]||st.auto+' s'} se přepne na další efekt.`:'Logo zůstává na zvoleném efektu.';
 if(st.sleep>0&&!sleepEnd)sleepEnd=Date.now()+st.sleep*1000;
 if(!st.sleep&&!sleepPick)sleepEnd=0;
 const left=sleepEnd?Math.max(0,Math.round((sleepEnd-Date.now())/1000)):0;
 [...$('sleepSeg').children].forEach(b=>b.classList.toggle('on',left?+b.dataset.v===sleepPick:+b.dataset.v===0));
 $('sleepSub').textContent=left?'Logo zhasne za '+fmtT(left)+'.':'Časovač neběží.';
}
setInterval(()=>{if(sleepEnd){if(Date.now()>sleepEnd){sleepEnd=0;sleepPick=0;load()}renderAuto()}},1000);

// ---- Zivy nahled ----
let lay=null,ctx=null,pvBusy=false;
const cv=$('cv');
function setupPv(){const W=cv.clientWidth,dpr=window.devicePixelRatio||1,cell=W/(lay.w+1),H=cell*(lay.h+1);
 cv.width=W*dpr;cv.height=H*dpr;cv.style.height=H+'px';ctx=cv.getContext('2d');ctx.setTransform(dpr,0,0,dpr,0,0);lay.cell=cell;}
function draw(hex){if(!ctx)return;const c=lay.cell,r=c*.36;ctx.clearRect(0,0,cv.width,cv.height);
 ctx.globalCompositeOperation='lighter';
 for(const [i,x,y] of lay.leds){const v=hex.substr(i*6,6);const cx=(x+1)*c,cy=(y+1)*c;
  if(v==='000000'||!v){ctx.fillStyle='#1a1d25';ctx.beginPath();ctx.arc(cx,cy,r*.8,0,7);ctx.fill();continue}
  const n=parseInt(v,16),R=n>>16,G=n>>8&255,B=n&255,m=Math.max(R,G,B),k=255/m;
  const col=`rgb(${R*k|0},${G*k|0},${B*k|0})`;
  const g=ctx.createRadialGradient(cx,cy,0,cx,cy,c*.95);g.addColorStop(0,`rgba(${R*k|0},${G*k|0},${B*k|0},${.55*m/255})`);g.addColorStop(1,'rgba(0,0,0,0)');
  ctx.fillStyle=g;ctx.beginPath();ctx.arc(cx,cy,c*.95,0,7);ctx.fill();
  ctx.globalAlpha=.35+.65*m/255;ctx.fillStyle=col;ctx.beginPath();ctx.arc(cx,cy,r,0,7);ctx.fill();ctx.globalAlpha=1;}
 ctx.globalCompositeOperation='source-over';}
function pvLoop(){if(document.hidden||!lay){setTimeout(pvLoop,500);return}
 fetch('/api/leds').then(r=>r.text()).then(draw).catch(()=>{}).finally(()=>setTimeout(pvLoop,120));}
fetch('/api/layout').then(r=>r.json()).then(l=>{const n={};l.leds.forEach(([i,x])=>n[x]=(n[x]||0)+1);l.leds=l.leds.map(([i,x,y])=>[i,x,y+(l.h-n[x])/2]);lay=l;setupPv();pvLoop()}).catch(()=>$('pv').hidden=true);
window.addEventListener('resize',()=>lay&&setupPv());

// ---- Domaci WiFi ----
let wifi=null,scanTimer=null;
const esc=t=>t.replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
function bars(r){const l=r>-55?4:r>-65?3:r>-75?2:1;return '<span class="bars">'+[6,9,12,14].map((h,i)=>`<i style="height:${h}px" class="${i<l?'f':''}"></i>`).join('')+'</span>'}
const ST={none:['Nenastaveno',''],connected:['Připojeno','ok'],connecting:['Připojuji…','wait'],wrongpass:['Špatné heslo','bad'],notfound:['Síť nenalezena','bad']};
function renderWifi(){
 if(!wifi)return;const [t,c]=ST[wifi.status]||ST.none;const b=$('netBadge');b.textContent=t;b.className='badge '+c;
 const n=esc(wifi.ssid);let h;
 if(wifi.status==='connected')h=`Logo je připojené k síti <b>${n}</b>. Na domácí síti ho ovládáš na <a href="http://${wifi.host}/">${wifi.host}</a> nebo <a href="http://${wifi.ip}/">${wifi.ip}</a>.<small>Pokud logo.local nefunguje (některé Androidy), použij IP adresu.</small>`;
 else if(wifi.status==='none')h=`Připoj logo k domácí WiFi a ovládej ho z mobilu nebo počítače bez přepínání sítí, na adrese <b>${wifi.host}</b>.<small>Logo umí jen sítě 2,4 GHz. Vlastní síť ${esc(wifi.ap)} zůstane zapnutá jako záloha.</small>`;
 else if(wifi.status==='connecting')h=`Připojuji se k síti <b>${n}</b>…`;
 else h=`K síti <b>${n}</b> se nepodařilo připojit (${t.toLowerCase()}). Vyhledej sítě a zkus to znovu.`;
 $('netInfo').innerHTML=h;$('forgetBtn').hidden=wifi.status==='none';
 $('scanBtn').textContent=wifi.status==='none'?'Vyhledat sítě':'Změnit síť';
}
function loadWifi(){return fetch('/api/wifi').then(r=>r.json()).then(w=>{wifi=w;renderWifi();return w}).catch(()=>{})}
function scan(start){
 const btn=$('scanBtn');btn.disabled=true;btn.innerHTML='<span class="spin"></span>Hledám sítě…';$('joinF').hidden=true;
 fetch('/api/scan'+(start?'?start=1':'')).then(r=>r.json()).then(d=>{
  if(d.scanning){scanTimer=setTimeout(()=>scan(0),1200);return}
  btn.disabled=false;renderWifi();
  const L=$('nets');L.innerHTML='';
  d.nets.forEach(x=>{const b=document.createElement('button');b.type='button';
   b.innerHTML=`${bars(x.rssi)}<span class="n">${esc(x.ssid)}</span><span class="lk">${x.open?'otevřená':'🔒'}</span>`;
   b.onclick=()=>pick(x.ssid,x.open);L.appendChild(b);});
  const o=document.createElement('button');o.type='button';o.innerHTML='<span class="bars"></span><span class="n">Jiná (skrytá) síť…</span>';o.onclick=()=>pick('',false);L.appendChild(o);
 }).catch(()=>{btn.disabled=false;renderWifi();online(0)});
}
function pick(ssid,open){vib();$('nets').innerHTML='';const f=$('joinF');f.hidden=false;
 $('ssidIn').value=ssid;$('ssidIn').hidden=!!ssid;$('ssidLbl').innerHTML=ssid?'Síť <b>'+esc(ssid)+'</b>':'Název sítě';
 $('pwIn').value='';$('pwIn').placeholder=open?'Síť je bez hesla':'';
 (ssid?$('pwIn'):$('ssidIn')).focus();}
$('scanBtn').onclick=()=>{vib();clearTimeout(scanTimer);scan(1)};
$('joinCancel').onclick=()=>{$('joinF').hidden=true};
$('pwShow').onchange=e=>$('pwIn').type=e.target.checked?'text':'password';
$('forgetBtn').onclick=()=>{if(!confirm('Opravdu odpojit logo od domácí WiFi?'))return;
 fetch('/api/wifi/forget',{method:'POST'}).then(r=>r.json()).then(w=>{wifi=w;renderWifi()})};
$('joinF').onsubmit=e=>{e.preventDefault();const ssid=$('ssidIn').value.trim(),pass=$('pwIn').value;
 if(!ssid)return;if(pass&&pass.length<8){alert('Heslo k WiFi má aspoň 8 znaků.');return}
 const btn=$('joinBtn');btn.disabled=true;btn.innerHTML='<span class="spin"></span>Ukládám…';
 fetch('/api/wifi',{method:'POST',body:new URLSearchParams({ssid,pass})}).then(r=>{if(!r.ok)return r.text().then(t=>{throw t});return r.json()})
  .then(w=>{wifi=w;wifi.status='connecting';renderWifi();$('joinF').hidden=true;
   let n=0;const poll=()=>loadWifi().then(w=>{if(w&&w.status==='connecting'&&++n<15)setTimeout(poll,2000)});setTimeout(poll,2500);})
  .catch(t=>alert(typeof t==='string'?t:'Nepodařilo se uložit, zkus to znovu.')).finally(()=>{btn.disabled=false;btn.textContent='Připojit'});
};
loadWifi();
</script></body></html>
)HTML";
