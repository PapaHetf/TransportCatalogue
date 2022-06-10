#pragma once

#include <string>
#include <vector>
#include "geo.h"

namespace domain {
	struct Bus;

	struct Stop {
		std::string name;
		geo::Coordinates coordinates = {};
		size_t id = 0;
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> bus_route;
		bool is_roundtrip = false;
	};

	struct BusRouteInfo {
		double lenght = 0;
		double curvature = 0;
		int stops = 0;
		int unique_stops = 0;
	};
}