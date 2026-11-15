from pathlib import Path
import ftplib,io,hashlib,uuid
root=Path(__file__).resolve().parent
files=[(root/'TamaConnectionV2.3dsx','/3ds/TamaConnectionV2/TamaConnectionV2.3dsx'),(root/'TamaConnectionV2.smdh','/3ds/TamaConnectionV2/TamaConnectionV2.smdh'),(root/'recursos/sprites.bin','/3ds/TamaConnectionV2/sprites.bin'),(root/'TamaConnectionV2.cia','/cias/TamaConnectionV2.cia')]
with ftplib.FTP() as ftp:
 ftp.connect('192.168.1.123',5000,timeout=15);ftp.login()
 try:ftp.mkd('/3ds/TamaConnectionV2')
 except (ftplib.error_perm,ftplib.error_temp) as e:
  if not str(e).startswith(('450','550','521')):raise
 for local,remote in files:
  raw=local.read_bytes();old=io.BytesIO()
  try:ftp.retrbinary('RETR '+remote,old.write)
  except (ftplib.error_perm,ftplib.error_temp) as e:
   if not str(e).startswith(('450','550')):raise
  ftp.storbinary('STOR '+remote+'.upload',io.BytesIO(raw))
  check=io.BytesIO();ftp.retrbinary('RETR '+remote+'.upload',check.write)
  assert hashlib.sha256(check.getvalue()).digest()==hashlib.sha256(raw).digest()
  if old.getvalue():ftp.rename(remote,remote+'.backup-'+uuid.uuid4().hex[:6])
  ftp.rename(remote+'.upload',remote)
  print('Copiado y verificado:',remote,len(raw),'bytes',flush=True)
print('Aplicacion nueva transferida sin modificar partidas ni la app de IA.')
