import argparse,os,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--devkitpro',type=Path,required=True);args=p.parse_args()
root=Path(__file__).resolve().parent;dkp=args.devkitpro.resolve();build=root/'build';build.mkdir(exist_ok=True)
cc=dkp/'devkitARM/bin/arm-none-eabi-gcc.exe';tools=dkp/'tools/bin';ctr=dkp/'libctru'
helper=Path(os.environ.get('CTR_TOOLS', str(dkp/'tools/bin')))
env=dict(os.environ);env['PATH']=str(cc.parent)+os.pathsep+str(tools)+os.pathsep+env.get('PATH','')
arch=['-march=armv6k','-mtune=mpcore','-mfloat-abi=hard','-mtp=soft']
def run(command):subprocess.run([str(v) for v in command],cwd=root,env=env,check=True)
objects=[]
for src in sorted((root/'source').glob('*.c')):
 obj=build/(src.stem+'.o');run([cc,*arch,'-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-mword-relocations','-ffunction-sections','-fdata-sections','-D__3DS__','-I'+str(ctr/'include'),'-c',src,'-o',obj]);objects.append(obj)
run([cc,*arch,'-specs=3dsx.specs','-Wl,--gc-sections',*objects,'-L'+str(ctr/'lib'),'-lcitro2d','-lcitro3d','-lctru','-lm','-o',root/'TamaConnectionV2.elf'])
run([tools/'smdhtool.exe','--create','Tama Connection V2','Mascota virtual con sprites a color','Proyecto personal',root/'recursos/icon.png',root/'TamaConnectionV2.smdh'])
run([tools/'3dsxtool.exe',root/'TamaConnectionV2.elf',root/'TamaConnectionV2.3dsx','--smdh='+str(root/'TamaConnectionV2.smdh')])
run([helper/'bannertool.exe','makebanner','-i',root/'recursos/banner.png','-a',root/'recursos/silencio.wav','-o',root/'recursos/banner.bin'])
run([helper/'makerom.exe','-f','cia','-o',root/'TamaConnectionV2.cia','-target','t','-elf',root/'TamaConnectionV2.elf','-rsf',root/'aplicacion.rsf','-exefslogo','-icon',root/'TamaConnectionV2.smdh','-banner',root/'recursos/banner.bin','-major','0','-minor','2','-micro','0'])
print('Compilados: TamaConnectionV2.3dsx y TamaConnectionV2.cia')
