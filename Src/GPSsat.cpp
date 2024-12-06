/*!
 */

#include "GPSsat.h"

std::map<int, GPSsat> SVs::new_spaceVehicles;
std::map<int, GPSsat> SVs::old_spaceVehicles;
std::vector<Strong> SVs::strongest_sats;

const char *SVs::COLORS[4] = {"WHITE", "YELLOW", "48545", "40137"};

int SVs::indx {0};
char SVs::buf[64];
