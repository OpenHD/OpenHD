//
// Created by consti10 on 02.05.22.
//

#include "OHDDiscovery.h"

#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>

#include "DPlatform.h"
//#include "DCameras.h"
//#include "DEthernetCards.h"
#include "DProfile.h"

OHDHardware OHDDiscovery::runOnceOnStartup(bool forceAir,bool forceGround) {
  if(forceAir && forceGround){
	std::cerr << "Cannot force air and ground at the same time\n";
	exit(1);
  }
  OHDHardware discovered_hardware;

  std::cout << "OHDDiscovery::runOnceOnStartup\n";
  try {
	DPlatform platform;
	discovered_hardware.platform=platform.discover();

	/*DCameras cameras(*discovered_hardware.platform);
	cameras.discover();
	cameras.write_manifest();*/

	/*DWifiCards wifi(*discovered_hardware.platform);
	wifi.discover();
	wifi.write_manifest();*/

	/*DEthernetCards ethernet(platform.platform_type(), platform.board_type(), platform.carrier_type());
	ethernet.discover();
	auto ethernet_manifest = ethernet.generate_manifest();
	std::ofstream _t("/tmp/ethernet_manifest");
	_t << ethernet_manifest.dump(4);
	_t.close();*/

	// When we write the profile we need to reason weather this is an air or ground pi.
	//const int camera_count = cameras.getCameraCount();
	const int camera_count=0;
	bool is_air = camera_count > 0;
	if (forceAir) {
	  is_air = true;
	}
	if(forceGround){
	  is_air = false;
	}
	DProfile profile(is_air);
	discovered_hardware.profile=profile.discover();

	// Note: Here stephen wrote all the small sub-manifests into one big manifest.
	// In my opinion, there is an apparent issue with that: The data is suddenly duplicated,
	// and one cannot know what is actually then read by the services running after -
	// The sub-files or the one big file.
	// Since sub-files promote separation, there is only sub-files now.
	std::cout << "OHDDiscovery::runOnceOnStartup::done\n";
  } catch (std::exception &ex) {
	std::cerr << "Error: " << ex.what() << std::endl;
	exit(1);
  } catch (...) {
	std::cerr << "Unknown exception occurred" << std::endl;
	exit(1);
  }

  return discovered_hardware;
}

