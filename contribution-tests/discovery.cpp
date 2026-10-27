#include "SystemData.h"
#include "FileData.h"
#include "CollectionSystemManager.h"
#include "ThemeData.h"
#include "Window.h"
#include "views/ViewController.h"
#include "Settings.h"
#include "Log.h"
#include "utils/FileSystemUtil.h"
#include <iostream>
#include <stdexcept>
static void require(bool b,const char*w){if(!b)throw std::runtime_error(w);}
extern "C" int __wrap_main(int argc,char**argv){try{
 require(argc==10,"Expected home roms name extension count patched threaded gamelist-only");std::string home=argv[1],roms=argv[2],name=argv[3],ext=argv[4];unsigned expected=std::stoul(argv[5]);bool patched=std::string(argv[6])=="patched",threaded=std::string(argv[7])=="true",listonly=std::string(argv[8])=="true";
 Utils::FileSystem::setExePath(argv[0]);Utils::FileSystem::setHomePath(home);Log::init();Log::open();auto s=Settings::getInstance();s->setBool("ParseGamelistOnly",listonly);s->setBool("IgnoreGamelist",false);s->setBool("ShowHiddenFiles",false);s->setBool("ThreadedLoading",threaded);s->setString("SaveGamelistsMode","never");s->setString("ThemeSet","probe");s->setString("CollectionSystemsAuto","");s->setString("CollectionSystemsCustom","");
 SystemEnvironmentData env;env.mStartPath=roms;env.mSearchExtensions={ext};env.mLaunchCommand="/bin/true";
 {SystemData sys(name,"Fixture",&env,name);require(sys.getRootFolder()->getFilesRecursive(GAME).size()==expected,"discovery count changed");require(bool(sys.getTheme()),"getTheme must stay non-null even for empty systems");bool loaded=sys.getTheme()->getElement("system","sentinel","text")!=nullptr;require(loaded==((!patched || expected>0) && std::string(argv[9])!="true"), "theme load policy incorrect");require(sys.isGameSystem()==(name!="retropie"),"menu classification changed");}
 // Use an independent fixture path for virtual collections.
 env.mStartPath=home+"/collection";
 {SystemData collection("favorites","Favorites",&env,"auto-favorites",true);require(collection.getRootFolder()->getChildren().empty(),"collection should start empty");require(bool(collection.getTheme()) && collection.getTheme()->getElement("system","sentinel","text")!=nullptr,"virtual collection lost theme");}
 Window window;ViewController::init(&window);CollectionSystemManager::init(&window);require(SystemData::loadConfig(nullptr),"loadConfig failed");require(SystemData::sSystemVector.size()==(expected?1u:0u),"system visibility changed");
 if(expected)require(SystemData::sSystemVector[0]->getGameCount()==expected,"loaded count changed");
 std::cout<<"{\"passed\":true,\"games\":"<<expected<<",\"threaded\":"<<(threaded?"true":"false")<<",\"gamelist_only\":"<<(listonly?"true":"false")<<"}\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
