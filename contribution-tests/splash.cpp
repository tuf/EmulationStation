#include "Window.h"
#include "Settings.h"
#include "Log.h"
#include "utils/FileSystemUtil.h"
#include "renderers/Renderer.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
struct NSVGimage;static int parses;
extern "C" NSVGimage* __real_nsvgParse(char*,const char*,float);
extern "C" NSVGimage* __wrap_nsvgParse(char*data,const char*units,float dpi){++parses;return __real_nsvgParse(data,units,dpi);}
extern "C" int __wrap_main(int argc,char**argv){try{
 if(argc!=3)throw std::runtime_error("home and expected policy required");bool cached=std::string(argv[2])=="cached";
 Utils::FileSystem::setExePath(argv[0]);Utils::FileSystem::setHomePath(argv[1]);Log::init();Log::open();auto s=Settings::getInstance();s->setBool("VSync",false);s->setBool("ShowHelpPrompts",false);s->setInt("ScreenSaverTime",0);
 Window w;if(!w.init())throw std::runtime_error("graphics init failed");
 for(int cycle=0;cycle<3;++cycle){
  int before=parses;auto start=std::chrono::steady_clock::now();for(int i=0;i<12;++i)w.renderLoadingScreen("Fixture loading",i/12.f);
  auto us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();int count=parses-before;
  if((cached && (count<1 || count>2)) || (!cached && count<12))throw std::runtime_error("unexpected SVG parse count: "+std::to_string(count));
  std::cout<<"{\"cycle\":"<<cycle<<",\"frames\":12,\"svg_parses\":"<<count<<",\"microseconds\":"<<us<<"}\n";
  w.render();Renderer::swapBuffers();
 }
 // Teardown while a loading image is held, then recreate the real GL context.
 w.renderLoadingScreen("Unfinished loading",.2f);w.deinit();
 s->setInt("ScreenWidth",1280);s->setInt("ScreenHeight",720);
 if(!w.init())throw std::runtime_error("context recreation failed");int before=parses;w.renderLoadingScreen("New context",.5f);if(parses-before<1 || parses-before>2)throw std::runtime_error("new context did not load splash");
 w.deinit();std::cout<<"{\"passed\":true,\"context_recreated\":true,\"render_size\":\"1280x720\"}\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
