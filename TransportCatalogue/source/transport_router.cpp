#include "transport_router.h"

route::TransportRouter::TransportRouter(const TransportCatalogue& catalogue, const RouteSettings& route_setting) :
	catalogue_(catalogue), route_setting_(route_setting), graph_(catalogue.GetAllStops().size()) {
	BuildGraph();
	router_ = std::make_unique<graph::Router<double>>(graph_);
}

std::optional<std::vector<route::RouteData>> route::TransportRouter::CreatRoute(const std::string_view& from, const std::string_view& to) const {
	std::optional<graph::Router<double>::RouteInfo> route_info = router_->BuildRoute(catalogue_.FindStop(from)->id, catalogue_.FindStop(to)->id);

	if (route_info == std::nullopt) {
		return std::nullopt;
	}
	if (route_info.value().edges.size() == 0) {
		std::vector<RouteData> route_data;
		route_data.push_back(std::move(CreateEmptyAnswer()));
		return route_data;
	}
	return CreateAnswer(route_info);
}

void route::TransportRouter::BuildGraph() const {
	const std::unordered_map <std::string_view, Bus*>& all_buses = catalogue_.GetAllBuses();
	for (const auto& [bus_name, struct_bus] : all_buses) {
		CreateEdgesAlongRoute(struct_bus->bus_route.begin(), struct_bus->bus_route.end(), bus_name);
		if (!struct_bus->is_roundtrip) {
			CreateEdgesAlongRoute(struct_bus->bus_route.rbegin(), struct_bus->bus_route.rend(), bus_name);
		}
	}
}

std::vector<route::RouteData> route::TransportRouter::CreateAnswer(const std::optional<graph::Router<double>::RouteInfo>& route_info) const {
	double total_time = 0.0;
	std::vector<RouteData> route_data;

	for (size_t edge_index : route_info.value().edges) {
		route_data.push_back(std::move(CreateStopAnswer(edge_index)));
		total_time += route_setting_.bus_wait_time;
	
		if (edges_[edge_index].first == edges_[edge_index].second) {
			continue;
		}

		double time = (edges_info_[edge_index].time_weight - route_setting_.bus_wait_time * 60) / 60;
		total_time += time;

		route_data.push_back(std::move(CreateBusAnswer(edge_index, time)));
	}
	return route_data;
}

route::RouteData route::TransportRouter::CreateBusAnswer(size_t edge_index, double time) const {
	RouteData bus_answer;
	bus_answer.type = "bus"sv;
	bus_answer.bus_name = edges_info_[edge_index].bus_name;
	bus_answer.span_count = edges_info_[edge_index].span_count;
	bus_answer.motion_time = time;
	return bus_answer;
}

route::RouteData route::TransportRouter::CreateStopAnswer(size_t edge_index) const {
	RouteData stop_answer;
	stop_answer.type = "stop"sv;
	stop_answer.stop_name = edges_[edge_index].first;
	stop_answer.bus_wait_time = route_setting_.bus_wait_time;
	return stop_answer;
}

route::RouteData route::TransportRouter::CreateEmptyAnswer() const {
	RouteData empty_answer;
	empty_answer.type = "stay_here"sv;
	return empty_answer;
}