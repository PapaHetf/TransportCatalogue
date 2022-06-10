#pragma once

#include "json.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace transport_catalogue::store {
    using namespace req_handler;
 
    struct DataStop {
        std::string stop_name;
        geo::Coordinates coordinates;
        std::unordered_map<std::string, double> distance_to_stop = {};
    };

    struct DataRoute {
        std::string bus_name;
        std::vector<std::string> bus_route = {};
        bool is_roundtrip = false;
    };

    struct JsonParser {
        void operator()(const json::Array& array);
        void operator()(const json::Dict& dict);
        void operator()(nullptr_t /*null*/);
        void operator()(const bool& value);
        void operator()(const double& value);
        void operator()(const int& value );
        void operator()(const std::string& line);
        std::string key;
        std::string type_data;
        DataStop* data_stop = nullptr;
        DataRoute* data_route = nullptr;
    };

    void ReadStop(const json::Node& node, DataStop* data_stop);

    void ReadBusRoute(const json::Node& node, DataRoute* data_route);

    void AddBusToCatalogue(const DataRoute& data_route, TransportCatalogue& catalogue);

    json::Node SaveEmptyAnswer(const json::Node& value);
    
    json::Node RequestStop(const RequestHandler& handler, const json::Node& value);
    
    json::Node RequestBus(const RequestHandler& handler, const json::Node& value);

    json::Node RequestMap(const RequestHandler& handler, const json::Node& value);

    json::Node CreateNodeStop(const route::RouteData& data);

    json::Node CreateNodeBus(const route::RouteData& data);

    json::Node CreateNodeRoute(const std::vector<route::RouteData>& route_data);

    double CalcTotalTime(const std::vector<route::RouteData>& route_data);
    
    json::Node RequestRoute(const RequestHandler& handler, const json::Node& value);
    
    void StatRequests(const json::Node& node, const RequestHandler& handler);

    void BaseRequests(const json::Node& node, TransportCatalogue& catalogue);

    svg::Color ReadTypeColor(const json::Node& node);

    void ReadSetting(const json::Node& node, renderer::Settings& settings);

    void ReadRouteSetting(const json::Node& node, route::RouteSettings& route_settings);

    void ReadRequests(std::istream& input, TransportCatalogue& catalogue);
}
