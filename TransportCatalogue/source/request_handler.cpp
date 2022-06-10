#include "request_handler.h"

namespace req_handler {
	RequestHandler::RequestHandler(const TransportCatalogue& catalogue, const renderer::MapRenderer& renderer, const TransportRouter& router)
		: catalogue_(catalogue), renderer_(renderer), router_(router) {}

	std::optional<BusRouteInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
		return catalogue_.GetInfoRoute(bus_name);
	}

	const std::set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
		return catalogue_.GetInfoStop(stop_name);;
	}

	svg::Document RequestHandler::RenderMap() const {
		return renderer_.CreateMap(catalogue_);
	}

	std::optional<std::vector<route::RouteData>> RequestHandler::CreateRoute(const std::string_view& from, const std::string_view& to) const {
		return router_.CreatRoute(from, to);
	}
}