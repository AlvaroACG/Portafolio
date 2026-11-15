from pathlib import Path
import urllib.request,json,time,hashlib,zipfile
r=Path(r'D:\Codex-3DS\WoW-1.12.1');r.mkdir(parents=True,exist_ok=True)
u='https://downloads.stonetavern.app/clients/classic-1.12.1/all/1.8/Stonetavern-Classic-1.12.1-v1.8.zip';p=r/'Stonetavern-Classic-1.12.1-v1.8.zip';part=p.with_suffix('.zip.part');expected='0ebb9b5f0386d547e0715fdae57955743dd06e991b507ca5a7729d14af0479fc'
if not p.exists():
 offset=part.stat().st_size if part.exists() else 0
 req=urllib.request.Request(u,headers={'Range':'bytes='+str(offset)+'-'}) if offset else urllib.request.Request(u)
 with urllib.request.urlopen(req,timeout=60) as f:
  if offset and f.status!=206:offset=0
  size=int(f.headers.get('Content-Length','0'))+offset;start=time.monotonic();mark=start;done=offset
  print('Descarga',size,'bytes; reanudando en',offset,flush=True)
  with part.open('ab' if offset else 'wb') as o:
   while data:=f.read(1024*1024):
    o.write(data);done+=len(data);t=time.monotonic()
    if t-mark>20:print('Cliente: %.1f%% - %.1f MB/s'%(100*done/size,(done-offset)/(t-start)/1e6),flush=True);mark=t
 if size and done!=size:raise RuntimeError('Descarga incompleta')
 sha=hashlib.file_digest(part.open('rb'),'sha256').hexdigest();assert sha==expected,(sha,expected);part.rename(p)
else:assert hashlib.file_digest(p.open('rb'),'sha256').hexdigest()==expected
print('ZIP cliente verificado SHA256.',flush=True)
with zipfile.ZipFile(p) as z:
 target=(r/'cliente').resolve();target.mkdir(exist_ok=True);names=[i for i in z.infolist() if '/data/' in '/'+i.filename.lower() or i.filename.lower().endswith('/realmlist.wtf')]
 total=sum(i.file_size for i in names);print('Extrayendo solo datos necesarios:',len(names),'archivos,',total,'bytes.',flush=True)
 for item in names:
  dest=(target/item.filename).resolve();assert dest.is_relative_to(target);z.extract(item,target)
print('DATOS_PREPARADOS',str(r/'cliente'),flush=True)
