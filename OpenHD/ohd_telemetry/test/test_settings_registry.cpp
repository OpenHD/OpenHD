#include "mavsdk_temporary/XMavlinkParamProvider.h"
#include "openhd_settings_registry.h"
#include "openhd_sysutil_settings_adapter.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

const char* getVideoPath() { return "/tmp/"; }
void require(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }

std::string download(XMavlinkParamProvider& provider,const std::string& path) {
  openhd::settings::MetadataFtpClient client; client.start(path);
  while(client.active) {
    auto request=client.poll(); require(bool(request),"Expected next FTP block");
    MavlinkMessage message;
    mavlink_msg_file_transfer_protocol_pack(255,190,&message.m,0,101,100,request->data());
    bool handled=false;
    for(const auto& reply:provider.process_mavlink_messages({message})) {
      if(reply.m.msgid!=MAVLINK_MSG_ID_FILE_TRANSFER_PROTOCOL) continue;
      mavlink_file_transfer_protocol_t ftp{}; mavlink_msg_file_transfer_protocol_decode(&reply.m,&ftp);
      openhd::settings::FtpPayload payload{}; std::copy_n(ftp.payload,251,payload.begin());
      handled=client.handle(payload);
    }
    require(handled,"No FTP response");
  }
  require(client.success,"FTP transfer failed"); return client.content;
}
int main(int argc,char** argv) {
  nlohmann::json debug={{"debug",false}}, display={{"supported",true},{"enabled",false},{"width",1920},{"height",1080},{"refresh_hz",60},{"connector","HDMI-A-1"}};
  bool owner_online=true;
  auto owned=openhd::settings::make_sysutil_settings([&](const nlohmann::json& q)->std::optional<nlohmann::json>{
    if(!owner_online) return std::nullopt;
    const auto type=q.at("type").get<std::string>();
    if(type=="sysutil.debug.request") return debug;
    if(type=="sysutil.display.request") return display;
    if(type=="sysutil.debug.update") {debug["debug"]=q.at("debug");return nlohmann::json{{"ok",true}};}
    if(type=="sysutil.display.update") {display.update(q);return nlohmann::json{{"ok",true}};}
    return std::nullopt;
  },false);
  require(owned.size()==3,"Absent owners exposed settings");
  for(auto& s:owned) {
    if(s.id=="SYS_DEBUG") {require(std::get<openhd::IntSetting>(s.setting).change_callback(s.id,1),"Owner debug write failed");require(debug.at("debug")==true,"Boolean conversion failed");}
    if(s.id=="SYS_DISP_MODE") {
      require(std::get<openhd::StringSetting>(s.setting).change_callback(s.id,"1280x720@50"),"Display tuple write failed");
      require(display.at("width")==1280 && display.at("connector")=="HDMI-A-1" && display.at("enabled")==true,"Display fields not preserved");
      require(!std::get<openhd::StringSetting>(s.setting).change_callback(s.id,"bad"),"Invalid tuple accepted");
    }
  }
  owner_online=false;
  require(openhd::settings::make_sysutil_settings([](const nlohmann::json&)->std::optional<nlohmann::json>{return std::nullopt;},false).empty(),"Unavailable owner exposed controls");
  auto role=openhd::settings::make_sysutil_settings([](const nlohmann::json& q)->std::optional<nlohmann::json>{
    if(q.at("type")=="sysutil.settings.request") return nlohmann::json{{"has_run_mode",true},{"run_mode","air"},{"run_mode_choices",{"air"}}};
    return std::nullopt;
  },false);
  require(role.size()==1 && !openhd::settings::valid(role.front(),"ground") && openhd::settings::valid(role.front(),"air"),"Unsupported role choice exposed");
  openhd::Setting bounded{"BOUNDED",openhd::IntSetting{0}};
  require(!openhd::settings::valid(bounded,nlohmann::json::parse("18446744073709551615")),"Unsigned JSON overflow accepted");
  require(!openhd::settings::valid(bounded,int64_t(INT32_MAX)+1),"Out-of-range int32 accepted");
  const std::string initial_path="/tmp/openhd-initial-test-"+std::to_string(::getpid())+".json";
  std::ofstream(initial_path)<<R"({"role":"air","parameter_defaults":{"version":1,"components":{"100":{"SEED_NEW":2,"SEED_EXIST":2}}}})";
  setenv("OPENHD_INITIAL_SETTINGS_FILE",initial_path.c_str(),1);
  {
    int fresh=0,existing=1;
    XMavlinkParamProvider seed_provider(101,100);
    seed_provider.add_param({"SEED_NEW",openhd::IntSetting{0,[&](std::string,int n){fresh=n;return true;}},nullptr,false,true});
    seed_provider.add_param({"SEED_EXIST",openhd::IntSetting{1,[&](std::string,int n){existing=n;return true;}},nullptr,false,false});
    seed_provider.set_ready();
    require(fresh==2 && existing==1,"Initial provisioning overwrote existing settings or failed to seed new settings");
  }
  std::remove(initial_path.c_str()); unsetenv("OPENHD_INITIAL_SETTINGS_FILE");
  XMavlinkParamProvider provider(101,100);
  int current=0,writes=0;
  bool capability=true;
  provider.add_param({"NEW_TEST",openhd::IntSetting{0,[&](std::string,int n){current=n;++writes;return true;},[&]{return current;}},
    [&]{return nlohmann::json{{"available",capability},{"enforce",true},{"minimum",0},{"maximum",2}};}});
  provider.add_param(openhd::create_read_only_int("READ_ONLY",7));
  provider.add_param({"TEXT_TEST",openhd::StringSetting{"initial",[](std::string,std::string){return true;}}});
  provider.set_ready();
  const auto initial=nlohmann::json::parse(download(provider,"/openhd/settings.json"));
  require(initial.at("settings").size()==3,"Unexpected parameter inventory");
  require(initial.at("systemId")==101 && initial.at("componentId")==100,"Wrong target");
  const auto standard=download(provider,"/component_metadata/parameters.json");
  require(nlohmann::json::parse(standard).at("parameters").size()==2,"String leaked into numeric upstream schema");
  const auto general=download(provider,"/component_metadata/general.json");
  if(argc>1) {
    std::ofstream(std::string(argv[1])+"/registry.json")<<initial.dump(2);
    std::ofstream(std::string(argv[1])+"/parameters.json")<<standard;
    std::ofstream(std::string(argv[1])+"/general.json")<<general;
  }
  const auto set=[&](const char* id,int value) {
    MavlinkMessage message; char data[128]{}; std::memcpy(data,&value,4);
    mavlink_msg_param_ext_set_pack(255,190,&message.m,101,100,id,data,MAV_PARAM_EXT_TYPE_INT32);
    int ack=-1;
    for(const auto& reply:provider.process_mavlink_messages({message})) if(reply.m.msgid==MAVLINK_MSG_ID_PARAM_EXT_ACK) {
      mavlink_param_ext_ack_t a{}; mavlink_msg_param_ext_ack_decode(&reply.m,&a); ack=a.param_result;
    }
    return ack;
  };
  require(set("NEW_TEST",2)==PARAM_ACK_ACCEPTED && current==2 && writes==1,"Valid write failed");
  require(set("NEW_TEST",3)!=PARAM_ACK_ACCEPTED && writes==1,"Invalid write reached backend");
  require(set("READ_ONLY",1)!=PARAM_ACK_ACCEPTED,"Readonly write accepted");
  capability=false;
  require(nlohmann::json::parse(download(provider,"/openhd/settings.json")).at("settings").size()==2,"Unsupported setting exposed");
  require(set("NEW_TEST",1)!=PARAM_ACK_ACCEPTED && writes==1,"Capability bypassed");
  require(set("NEW_TEST",2)!=PARAM_ACK_ACCEPTED && writes==1,"Same-value write bypassed capability");
  require(set("READ_ONLY",7)!=PARAM_ACK_ACCEPTED,"Same-value readonly write accepted");
  require(set("ABSENT",1)==PARAM_ACK_VALUE_UNSUPPORTED,"Missing parameter write did not receive rejection");
  capability=true;
  const std::string path="/tmp/openhd-registry-test-"+std::to_string(::getpid())+".json";
  setenv("OPENHD_RUNTIME_SETTINGS_FILE",path.c_str(),1);
  std::ofstream(path)<<R"({"version":1,"systemId":101,"components":{"100":{"NEW_TEST":1,"READ_ONLY":0,"ABSENT":5}}})";
  provider.generate_mavlink_messages();
  require(current==1 && writes==2,"File write did not use setter");
  provider.generate_mavlink_messages(); require(writes==2,"File reapplied repeatedly");
  const auto report=nlohmann::json::parse(download(provider,"/openhd/file-results.json"));
  require(report["results"]["NEW_TEST"]["accepted"]==true && report["results"]["READ_ONLY"]["accepted"]==false && report["results"]["ABSENT"]["accepted"]==false,"Wrong file results");
  std::remove(path.c_str()); unsetenv("OPENHD_RUNTIME_SETTINGS_FILE");
  std::cout<<"SETTINGS REGISTRY MAVLINK AND FILE EDIT PASSED\n";
}
