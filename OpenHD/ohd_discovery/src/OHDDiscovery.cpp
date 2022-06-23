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


  return discovered_hardware;
}

