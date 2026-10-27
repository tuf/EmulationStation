#include "HelpPrompt.h"
#include "InputConfig.h"
#include "Settings.h"
#include <memory>
#include <vector>
#define private public
#include "Window.h"
#undef private
#include "GuiComponent.h"
#include "Log.h"
#include "utils/FileSystemUtil.h"
#include <iostream>
#include <stdexcept>
static int checks;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);++checks;}
struct Gui:GuiComponent{int calls=0;Gui(Window*w):GuiComponent(w){} bool input(InputConfig*,Input)override{++calls;return true;}};
struct Saver:Window::ScreenSaver{
 bool active=false;int stops=0;void startScreenSaver(SystemData*)override{active=true;} void stopScreenSaver(bool)override{active=false;++stops;}
 void renderScreenSaver()override{} bool allowSleep()override{return true;} void update(int)override{} bool isScreenSaverActive()override{return active;}
 FileData*getCurrentGame()override{return nullptr;}void selectGame(bool)override{}bool inputDuringScreensaver(InputConfig*,Input)override{return false;}
};
extern "C" int __wrap_main(int argc,char**argv){try{
 require(argc==3,"home and expected policy required");bool patched=std::string(argv[2])=="patched";
 Utils::FileSystem::setExePath(argv[0]);Utils::FileSystem::setHomePath(argv[1]);Log::init();Log::open();auto settings=Settings::getInstance();
 if(patched)require(!settings->getBool("IgnoreUnmappedJoystickAxes"),"opt-in must default off");
 settings->setBool("ScreenSaverControls",false);Window w;auto gui=new Gui(&w);w.pushGui(gui);Saver saver;w.setScreenSaver(&saver);
 InputConfig pad(0,"Fixture pad","fixture");pad.mapInput("left",Input(0,TYPE_AXIS,0,-1,true));pad.mapInput("a",Input(0,TYPE_BUTTON,1,1,true));
 auto reset=[&](bool on,bool sleep){w.mTimeSinceLastInput=1234;w.mRenderScreenSaver=on;w.mSleeping=sleep;saver.active=on;saver.stops=0;};
 settings->setBool("IgnoreUnmappedJoystickAxes",false);reset(true,false);w.input(&pad,Input(0,TYPE_AXIS,2,1,false));require(w.mTimeSinceLastInput==0 && !w.mRenderScreenSaver,"legacy default wake changed");
 settings->setBool("IgnoreUnmappedJoystickAxes",true);
 for(bool controls:{false,true})for(int value:{-1,0,1}){
  settings->setBool("ScreenSaverControls",controls);reset(true,false);w.input(&pad,Input(0,TYPE_AXIS,2,value,false));
  if(patched)require(w.mTimeSinceLastInput==1234 && w.mRenderScreenSaver && saver.stops==0,"unused axis affected screensaver");
  else require(w.mTimeSinceLastInput==0,"baseline should reset inactivity");
 }
 if(patched){
  reset(false,false);int n=gui->calls;w.input(&pad,Input(0,TYPE_AXIS,2,-1,false));require(gui->calls==n+1 && w.mTimeSinceLastInput==1234,"configuration dialog did not receive unused axis");
  reset(false,true);n=gui->calls;w.input(&pad,Input(0,TYPE_AXIS,2,1,false));require(w.isSleeping() && w.mTimeSinceLastInput==1234 && gui->calls==n,"unused axis woke sleeping UI");
 }
 for(int value:{-1,0,1}){reset(false,false);int n=gui->calls;w.input(&pad,Input(0,TYPE_AXIS,0,value,false));require(w.mTimeSinceLastInput==0 && gui->calls==n+1,"mapped axis or release was filtered");}
 for(Input input:{Input(0,TYPE_BUTTON,8,1,false),Input(DEVICE_KEYBOARD,TYPE_KEY,SDLK_SPACE,1,false),Input(0,TYPE_HAT,0,SDL_HAT_UP,false)}){reset(true,false);w.input(&pad,input);require(w.mTimeSinceLastInput==0 && !w.mRenderScreenSaver,"non-axis wake changed");}
 InputConfig fresh(1,"Unconfigured pad","new");reset(false,false);int n=gui->calls;w.input(&fresh,Input(1,TYPE_AXIS,2,1,false));require(gui->calls==n+1 && w.mTimeSinceLastInput==0,"unconfigured pad filtered");
 reset(false,true);w.input(&pad,Input(0,TYPE_BUTTON,1,1,false));require(!w.isSleeping(),"mapped button failed to wake");
 w.setScreenSaver(nullptr);std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"policy\":\""<<(patched?"patched":"baseline")<<"\"}\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
