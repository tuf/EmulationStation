import json,subprocess,tempfile,sys,xml.etree.ElementTree as E
from pathlib import Path
root=Path(sys.argv[1]).resolve();policy=sys.argv[2];mode=sys.argv[3];out=root/'results'/policy;reports=[]
valid='<theme><formatVersion>4</formatVersion><view name="system"><text name="sentinel"><text>loaded</text></text></view></theme>'
with tempfile.TemporaryDirectory(prefix='es-upstream-fixtures-') as temp:
 base=Path(temp);home=base/'home';cfg=home/'.emulationstation';cfg.mkdir(parents=True);theme=cfg/'themes/probe';theme.mkdir(parents=True);(theme/'theme.xml').write_text(valid)
 if mode=='axis':
  p=subprocess.run([str(out/'axis'),str(home),'patched' if policy=='axis' else 'baseline'],stdout=subprocess.PIPE,stderr=subprocess.PIPE,universal_newlines=True);assert p.returncode==0,(p.stdout,p.stderr);reports.append(json.loads(p.stdout.strip().splitlines()[-1]))
 else:
  roms=base/'roms';roms.mkdir()
  def check(label,count,name='nes',extension='.nes',listonly=False,poison=False):
   for threaded in (False,True):
    tree=E.Element('systemList');node=E.SubElement(tree,'system')
    for k,v in [('name',name),('fullname','Fixture'),('path',str(roms)),('extension',extension),('command','/bin/true'),('theme',name)]:E.SubElement(node,k).text=v
    E.ElementTree(tree).write(str(cfg/'es_systems.cfg'))
    p=subprocess.run([str(out/'discovery'),str(home),str(roms),name,extension,str(count),'patched' if policy=='empty' else 'baseline',str(threaded).lower(),str(listonly).lower(),str(poison).lower()],stdout=subprocess.PIPE,stderr=subprocess.PIPE,universal_newlines=True,timeout=30)
    assert p.returncode==0,(label,p.stdout,p.stderr,(cfg/'es_log.txt').read_text());report=json.loads(p.stdout.strip().splitlines()[-1]);report['case']=label
    errors=[l for l in (cfg/'es_log.txt').read_text().splitlines() if 'lvl0:' in l];assert bool(errors)==(poison and policy!='empty'),(label,errors);report['theme_errors']=len(errors);reports.append(report)
  check('empty directory with valid fallback theme',0)
  (roms/'theme.xml').write_text('<invalid');check('discarded system with malformed theme',0,poison=True);(roms/'theme.xml').unlink()
  (roms/'readme.txt').write_text('fixture');(roms/'.hidden.nes').write_text('not a ROM');check('irrelevant and hidden files',0)
  sub=roms/'nested';sub.mkdir();game=sub/'new.nes';game.write_text('fixture, never emulated');check('new ROM in nested directory',1);game.unlink();check('last ROM removed',0)
  (roms/'gamelist.xml').write_text('<gameList><game><path>./missing.nes</path><name>Fixture</name></game></gameList>');check('stale gamelist normal discovery',0);check('gamelist-only mode',1,listonly=True);(roms/'gamelist.xml').unlink()
  (roms/'menu.sh').write_text('#!/bin/sh\nexit 0\n');check('RetroPie menu',1,'retropie','.sh')
report=dict(passed=True,policy=policy,mode=mode,results=reports);(out/(mode+'-verification.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
