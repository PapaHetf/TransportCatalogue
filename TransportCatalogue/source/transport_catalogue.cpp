#include "transport_catalogue.h"

namespace transport_catalogue {
	namespace store {
		void TransportCatalogue::AddBus(const std::string& name, std::vector<Stop*> bus_route, bool is_roundtrip) {
			Bus bus;
			bus.name = name;
			bus.bus_route = move(bus_route);
			bus.is_roundtrip = is_roundtrip;
			deq_buses_.push_back(std::move(bus));
			all_buses_[deq_buses_.back().name] = &deq_buses_.back();
			for (auto stop : deq_buses_.back().bus_route) {
				stop_buses_[stop->name].insert(deq_buses_.back().name);
			}
		}

		void TransportCatalogue::AddStop(std::string stop_name, geo::Coordinates coordinates) {
			if (all_stops_.count(stop_name)) {
				return;
			}
			Stop stop_bus;
			stop_bus.coordinates = std::move(coordinates);
			stop_bus.name = move(stop_name);
			stop_bus.id = all_stops_.size();
			deq_stops_.push_back(std::move(stop_bus));
			all_stops_[deq_stops_.back().name] = &deq_stops_.back();
			stop_buses_[deq_stops_.back().name];
		}

		const Bus* TransportCatalogue::FindBus(std::string_view name_bus) const {
			return (all_buses_.find(name_bus) != all_buses_.end() ? all_buses_.at(name_bus) : nullptr);
		}

		const Stop* TransportCatalogue::FindStop(std::string_view name_stop) const {
			return (all_stops_.find(name_stop) != all_stops_.end() ? all_stops_.at(name_stop) : nullptr);
		}

		void TransportCatalogue::SetDistance(const std::string& first_stop_name, const std::string& second_stop_name, const double& distance) {
			auto first_stop = all_stops_.find(first_stop_name)->second;
			if (second_stop_name.empty()) {
				distance_.insert({ { first_stop, nullptr}, distance });
			}
			else {
				auto second_stop = all_stops_.find(second_stop_name)->second;
				distance_.insert({ { first_stop, second_stop }, distance });
			}
		}

		const std::unordered_map <std::string_view, Stop*>& TransportCatalogue::GetAllStops() const {
			return all_stops_;
		}

		const std::unordered_map <std::string_view, Bus*>& TransportCatalogue::GetAllBuses() const {
			return all_buses_;
		}

		const std::optional<double> TransportCatalogue::GetLenght(const Stop* first_stop, const Stop* second_stop) const {
			auto it_lenght = distance_.find({ first_stop, second_stop });
			if (it_lenght == distance_.end()) {
				it_lenght = distance_.find({ second_stop, first_stop });
				if (it_lenght == distance_.end()) {
					return std::nullopt;
				}
			}
			return it_lenght->second;
		}

		size_t TransportCatalogue::GetUniqueStops(const Bus* bus) const {
			std::set<std::string_view> unique_stops;
			for (auto struct_stop : bus->bus_route) {
				auto stop = all_stops_.find(struct_stop->name)->first;
				unique_stops.insert(stop);
			}
			return unique_stops.size();
		}

		const std::optional<BusRouteInfo> TransportCatalogue::GetInfoRoute(std::string_view name_bus) const {
			const Bus* bus = FindBus(name_bus);
			BusRouteInfo route_info;
			if (bus == nullptr) {
				return std::nullopt;
			}
			route_info.stops = (bus->is_roundtrip == true ? bus->bus_route.size() : bus->bus_route.size() * 2 - 1);
			route_info.unique_stops = std::move(GetUniqueStops(bus));
			if (!bus->bus_route.empty()) {
				auto [lenght, curvature] = GetAllLenght(bus->bus_route.begin(), bus->bus_route.end());
				route_info.lenght = lenght;
				route_info.curvature = curvature;
			}
			if (bus->is_roundtrip == true) {
				route_info.curvature = route_info.lenght / route_info.curvature;
				return route_info;
			}
			if (!bus->bus_route.empty()) {
				auto [lenght, curvature] = GetAllLenght(bus->bus_route.rbegin(), bus->bus_route.rend());
				route_info.lenght += lenght;
				route_info.curvature += curvature;
			}
			route_info.curvature = route_info.lenght / route_info.curvature;
			return route_info;
		}

		const std::set<std::string_view>* TransportCatalogue::GetInfoStop(std::string_view stop) const {
			if (stop_buses_.find(stop) != stop_buses_.end()) {
				return &stop_buses_.at(stop);
			}
			return nullptr;
		}
	}
}