from pathlib import Path
import urllib.request,concurrent.futures,hashlib,time,threading,json
root=Path(r'D:\Codex-3DS\WoW-1.12.1');url='https://downloads.stonetavern.app/clients/classic-1.12.1/all/1.8/Stonetavern-Classic-1.12.1-v1.8.zip';total=5321185674;block=32*1024*1024
p=root/'Stonetavern-Classic-1.12.1-v1.8.zip';part=root/'cliente-paralelo.zip.part';state=root/'descarga-rangos.json';finished=set(json.loads(state.read_text())) if state.exists() else set();lock=threading.Lock();start=time.monotonic();done=sum(min(block,total-i*block) for i in finished);initial=done;last=start;inflight={}
if not part.exists():
 with part.open('wb') as f:f.truncate(total)
def fetch(i):
 global done,last
 if i in finished:return
 begin=i*block;end=min(total,begin+block)-1
 for attempt in range(4):
  try:
   req=urllib.request.Request(url,headers={'User-Agent':'Mozilla/5.0','Range':f'bytes={begin}-{end}'})
   with urllib.request.urlopen(req,timeout=60) as response:
    assert response.status==206 and response.headers.get('Content-Range')==f'bytes {begin}-{end}/{total}'
    pos=begin
    with part.open('r+b') as out:
     out.seek(begin)
     while raw:=response.read(1024*1024):
      out.write(raw);pos+=len(raw)
      with lock:
       inflight[i]=pos-begin;now=time.monotonic();received=done+sum(inflight.values());rate=(received-initial)/max(1,now-start)
       if now-last>20:
        print('Cliente %.1f%%; %.2f MB/s; aprox %d min restantes'%(100*received/total,rate/1e6,(total-received)/max(1,rate)/60),flush=True);last=now
    assert pos==end+1
   with lock:
    inflight.pop(i,None);finished.add(i);done+=end-begin+1;state.write_text(json.dumps(sorted(finished)));now=time.monotonic()
    if now-last>20:print('Cliente %.1f%%; %.2f MB/s'%(100*done/total,(done-initial)/(now-start)/1e6),flush=True);last=now
   return
  except Exception:
   if attempt==3:raise
   time.sleep(1+attempt)
with concurrent.futures.ThreadPoolExecutor(max_workers=16) as pool:list(pool.map(fetch,range((total+block-1)//block)))
print('Descarga completa. Verificando SHA256...',flush=True)
with part.open('rb') as f:assert hashlib.file_digest(f,'sha256').hexdigest()=='0ebb9b5f0386d547e0715fdae57955743dd06e991b507ca5a7729d14af0479fc'
part.rename(p);print('CLIENTE_VERIFICADO',str(p),flush=True)
