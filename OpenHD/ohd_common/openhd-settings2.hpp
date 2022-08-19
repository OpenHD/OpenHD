//
// Created by consti10 on 18.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_SETTINGS2_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_SETTINGS2_HPP_

#include <utility>

#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"

namespace openhd::settings{

/**
 * Helper class to persist settings during reboots using json.
 * @tparam T the settings struct to persist, needs to have to and from json(s)
 */
template<class T>
class PersistentSettings{
 public:
  /**
   * @param base_path the directory into which the settings file is then written. (filename: base_path+unique filename).
   */
  explicit PersistentSettings(std::string base_path):_base_path(std::move(base_path)){
	assert(_base_path.back()=='/');
  }
  // delete copy and move constructor
  PersistentSettings(const PersistentSettings&)=delete;
  PersistentSettings(const PersistentSettings&&)=delete;
  // read only, to express the need for calling persist otherwise
  [[nodiscard]] const T& get_settings()const{
	assert(_settings);
	return *_settings;
  }
  // Don't forget to call persist once done
  [[nodiscard]] T& unsafe_get_settings()const{
	assert(_settings);
	return *_settings;
  }
  // save changes by writing them out to the file, and notifying the listener(s)
  void persist()const{
	persist_settings();
	if(_settings_changed_callback){
	  _settings_changed_callback();
	}
  }
  // Persist then new settings, then call the callback to propagate the change
  void update_settings(const T& new_settings){
	std::stringstream ss;
	ss<<"Got new settings in["<<get_unique_filename()<<"]\n";
	std::cout<<ss.str();
	_settings=std::make_unique<T>(new_settings);
	persist_settings();
	if(_settings_changed_callback){
	  _settings_changed_callback();
	}
  }
  typedef std::function<void()> SETTINGS_CHANGED_CALLBACK;
  void register_listener(SETTINGS_CHANGED_CALLBACK callback){
	assert(!_settings_changed_callback);
	_settings_changed_callback=std::move(callback);
  }
 protected:
  [[nodiscard]] virtual std::string get_unique_filename()const=0;
  virtual T create_default()const=0;
  /*
   * looks for a previosly written file (base_path+unique filename).
   * If this file exists, create settings from it - otherwise, create default and persist.
   */
  void init(){
	if(!OHDFilesystemUtil::exists(_base_path.c_str())){
	  OHDFilesystemUtil::create_directory(_base_path);
	}
	const auto last_settings_opt=read_last_settings();
	if(last_settings_opt.has_value()){
	  _settings=std::make_unique<T>(last_settings_opt.value());
	  std::stringstream ss;
	  ss<<"Using settings in ["<<get_file_path()<<"]\n";
	  std::cout<<ss.str();
	}else{
	  std::cout<<"Creating default settings:"<<get_file_path()<<"\n";
	  // create default settings and persist them for the next reboot
	  _settings=std::make_unique<T>(create_default());
	  persist_settings();
	}
  }
 private:
  const std::string _base_path;
  std::unique_ptr<T> _settings;
  SETTINGS_CHANGED_CALLBACK _settings_changed_callback=nullptr;
  [[nodiscard]] std::string get_file_path()const{
	return _base_path+get_unique_filename();
  }
  // write settings locally for persistence
  void persist_settings()const{
	const auto file_path=get_file_path();
	const nlohmann::json tmp=*_settings;
	// and write them locally for persistence
	std::ofstream t(file_path);
	t << tmp.dump(4);
	t.close();
  }
  // read last settings, if they are available
  // Read the last persistent settings for this instance.
  // Return std::nullopt if
  // 1) The file does not exist
  // 2) The json parse encountered an error
  // 3) The json conversion encountered an error
  // In case of 1 this is most likely new hw, and default settings will be created.
  // In case of 2,3 it was most likely a user that modified the json incorrectly
  // Also, default settings will be created in this case.
  [[nodiscard]] std::optional<T> read_last_settings()const{
	const auto file_path=get_file_path();
	if(!OHDFilesystemUtil::exists(file_path.c_str())){
	  return std::nullopt;
	}
	std::ifstream f(file_path);
	try{
	  nlohmann::json j;
	  f >> j;
	  auto tmp=j.get<T>();
	  return tmp;
	}catch(nlohmann::json::exception& ex){
	  std::stringstream ss;
	  ss<<"PersistentSettings::read_last_settings json exception on {"<<file_path<<"} ";
	  ss<<ex.what()<<"\n";
	  std::cerr<<ss.str();
	  std::cout<<"Using default settings\n";
	  // this means the default settings will be created
	  return std::nullopt;
	}
	return  std::nullopt;
  }
};


}

#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_SETTINGS2_HPP_
