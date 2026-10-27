"""Link regression probes to the real build objects without installing ES."""
import json,shlex,subprocess,sys
from pathlib import Path
root=Path(sys.argv[1]).resolve();policy=sys.argv[2];tests=sys.argv[3:];source=root/'source';build=root/'build';validation=root/'validation';out=root/'results'/policy;out.mkdir(parents=True,exist_ok=True)
commands=json.loads((build/'compile_commands.json').read_text());cwd=build/'es-app';link=shlex.split((cwd/'CMakeFiles/emulationstation.dir/link.txt').read_text())
for name in tests:
 target='es-app/src/SystemData.cpp' if name=='discovery' else 'es-core/src/Window.cpp'
 entry=next(e for e in commands if e['file'].endswith(target));args=shlex.split(entry['command']);clean=[];skip=False
 for arg in args:
  if skip:skip=False;continue
  if arg=='-o':skip=True;continue
  if arg=='-c' or arg==entry['file']:continue
  clean.append(arg)
 obj=out/(name+'.o');subprocess.check_call(clean+['-c',str(validation/(name+'.cpp')),'-o',str(obj)],cwd=entry['directory'])
 command=link[:];command[command.index('-o')+1]=str(out/name);command.insert(1,str(obj));command+=['-Wl,--wrap=main']
 if name=='splash':command+=['-Wl,--wrap=nsvgParse']
 subprocess.check_call(command,cwd=str(cwd))
 print('Built',policy,name,flush=True)
