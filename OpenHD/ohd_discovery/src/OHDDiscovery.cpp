//
// Created by consti10 on 02.05.22.
//

#include "OHDDiscovery.h"

#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>

#include "DPlatform.h"
#include "DCameras.h"
//#include "DEthernetCards.h"
#include "DWifiCards.h"
#include "DProfile.h"

void OHDDiscovery::runOnceOnStartup(bool forceAir) {
  std::cout << "OHDDiscovery::runOnceOnStartup\n";
  try {
	DPlatform platform;
	platform.discover();
	platform.write_manifest();

	DCameras cameras(platform.platform_type(), platform.board_type(), platform.carrier_type());
	cameras.discover();
	cameras.write_manifest();

	DWifiCards wifi(platform.platform_type(), platform.board_type(), platform.carrier_type());
	wifi.discover();
	wifi.write_manifest();

	/*DEthernetCards ethernet(platform.platform_type(), platform.board_type(), platform.carrier_type());
	ethernet.discover();
	auto ethernet_manifest = ethernet.generate_manifest();
	std::ofstream _t("/tmp/ethernet_manifest");
	_t << ethernet_manifest.dump(4);
	_t.close();*/

	// When we write the profile we need to reason weather this is an air or ground pi.
	const int camera_count = cameras.count();
	bool is_air = camera_count > 0;
	if (forceAir) {
	  is_air = true;
	}
	DProfile profile(is_air);
	profile.discover();
	profile.write_manifest();

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
}
