from pathlib import Path
import time,json,zipfile,hashlib,subprocess,shutil,ftplib,io,uuid
root=Path(__file__).resolve().parent
work=Path(r'D:\Codex-3DS\WoW-1.12.1')
archive=work/'Stonetavern-Classic-1.12.1-v1.8.zip'
state=root/'estado-preparacion.json'
def status(phase,**fields):
 state.write_text(json.dumps({'phase':phase,'time':time.strftime('%Y-%m-%d %H:%M:%S'),**fields},indent=2));print(phase,fields,flush=True)
status('esperando_descarga',archivo=str(archive))
while not archive.exists():time.sleep(10)
with archive.open('rb') as f:assert hashlib.file_digest(f,'sha256').hexdigest()=='0ebb9b5f0386d547e0715fdae57955743dd06e991b507ca5a7729d14af0479fc'
client=(work/'cliente').resolve();client.mkdir(exist_ok=True)
status('extrayendo_datos')
with zipfile.ZipFile(archive) as z:
 for item in z.infolist():
  if '/data/' not in '/'+item.filename.lower():continue
  dest=(client/item.filename).resolve();assert dest.is_relative_to(client)
  z.extract(item,client)
candidates=list(client.rglob('terrain.MPQ'))+list(client.rglob('terrain.mpq'))
assert candidates,'No se encontro terrain.MPQ'
data=candidates[0].parent
output=work/'convertido';output.mkdir(exist_ok=True)
if shutil.disk_usage(work).free<10*1024**3:raise RuntimeError('Faltan al menos 10 GB libres para la conversion completa')
status('convirtiendo_mundo',data=str(data),salida=str(output))
converter=root/'conversor/w3dconv/w3dconv.exe'
with (root/'conversion-completa.log').open('w',encoding='utf-8') as log:
 subprocess.run([str(converter),'--data',str(data),'--locale','enUS','--all','-o',str(output)],cwd=converter.parent,stdout=log,stderr=subprocess.STDOUT,check=True)
assert (output/'ui.w3dp').exists() and (output/'map0.w3dp').exists()
status('datos_convertidos',salida=str(output))
files=[p for p in output.rglob('*') if p.is_file() and 'preview' not in p.relative_to(output).parts and not p.name.startswith('log_')]
status('transfiriendo_datos',cantidad=len(files))
try:
 with ftplib.FTP() as ftp:
  ftp.connect('192.168.1.123',5000,timeout=30);ftp.login()
  for local in files:
   remote='/3ds/w3dproto/'+local.relative_to(output).as_posix();parts=remote.rsplit('/',1)[0].split('/');walk=''
   for part in parts:
    if not part:continue
    walk+='/'+part
    try:ftp.mkd(walk)
    except (ftplib.error_perm,ftplib.error_temp) as e:
     if not str(e).startswith(('450','550','521')):raise
   assert local.stat().st_size<4*1024**3
   with local.open('rb') as f:ftp.storbinary('STOR '+remote+'.upload',f,blocksize=65536)
   check=hashlib.sha256();ftp.retrbinary('RETR '+remote+'.upload',check.update,blocksize=65536)
   with local.open('rb') as f:assert check.hexdigest()==hashlib.file_digest(f,'sha256').hexdigest()
   try:ftp.rename(remote,remote+'.backup-'+uuid.uuid4().hex[:6])
   except (ftplib.error_perm,ftplib.error_temp) as e:
    if not str(e).startswith(('450','550')):raise
   ftp.rename(remote+'.upload',remote)
   status('transfiriendo_datos',ultimo=local.name)
 status('datos_en_sd_pendiente_cuenta_e_instalacion_fbi')
except Exception as error:
 status('datos_listos_transferencia_pendiente',error=str(error),salida=str(output));raise
