@echo off
cd /d "%~dp0"
echo Abre FTPD en 192.168.1.123:5000.
py "%~dp0enviar_ftp.py"
pause
