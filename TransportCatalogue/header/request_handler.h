#pragma once

#include <optional>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace req_handler {
    using namespace transport_catalogue::store;
    using namespace route;
    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue& catalogue, const renderer::MapRenderer& renderer, const TransportRouter& router);

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusRouteInfo> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через остановку
        const std::set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

        svg::Document RenderMap() const;

        std::optional<std::vector<route::RouteData>> CreateRoute(const std::string_view& from, const std::string_view& to) const;
    private:
        const TransportCatalogue& catalogue_;
        const renderer::MapRenderer& renderer_;
        const route::TransportRouter& router_;
    };
}