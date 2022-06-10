#include "json_reader.h"
#include "json_builder.h"

namespace transport_catalogue::store {
	using namespace std::literals;

	void JsonParser::operator()(nullptr_t /*null*/) {}

	void JsonParser::operator()(const bool& value) {
		data_route->is_roundtrip = value;
	}

	void JsonParser::operator()(const double& value) {
		if (key == "latitude") {
			data_stop->coordinates.lat = value;
		}
		if (key == "longitude") {
			data_stop->coordinates.lng = value;
		}
	}

	void JsonParser::operator()(const int& value ) {
		if (key == "latitude") {
			data_stop->coordinates.lat = value;
		}
		if (key == "longitude") {
			data_stop->coordinates.lng = value;
		}
	}

	void JsonParser::operator()(const std::string& line) {
		if (type_data == "Stop") {
			data_stop->stop_name = line;
		}
		if (type_data == "Bus") {
			data_route->bus_name = line;
		}
	}

	void JsonParser::operator()(const json::Array& array) {
		for (auto value : array) {
			data_route->bus_route.push_back(value.AsString());
		}
	}

	void JsonParser::operator()(const json::Dict& dict) {
		for (auto [key_stop, dist] : dict) {
			data_stop->distance_to_stop.insert({ key_stop, dist.AsDouble() });
		}
	}

	void ReadStop(const json::Node& node, DataStop* data_stop) {
		for (auto& [key, value] : node.AsDict()) {
			if (key != "type") {
				std::visit(JsonParser{ key, "Stop", data_stop, nullptr }, value.GetJsonType());
			}
		}
	}

	void ReadBusRoute(const json::Node& node, DataRoute* data_route) {
		for (auto& [key, value] : node.AsDict()) {
			if (key != "type") {
				std::visit(JsonParser{ key, "Bus", nullptr, data_route }, value.GetJsonType());
			}
		}
	}

	void AddBusToCatalogue(const DataRoute& data_route, TransportCatalogue& catalogue) {
		std::vector<Stop*> bus_route;
		for (const std::string& stop : data_route.bus_route) {
			Stop* struct_stop = catalogue.GetAllStops().find(stop)->second;
			bus_route.push_back(struct_stop);
		}
		catalogue.AddBus(data_route.bus_name, bus_route, data_route.is_roundtrip);
	}
	
	json::Node SaveEmptyAnswer(const json::Node& value) {
		json::Node dict_node{ json::Builder{}
								.StartDict()
									.Key("error_message"s).Value("not found"s)
									.Key("request_id"s).Value(value.AsDict().at("id").AsInt())
								.EndDict().Build() };
		return dict_node;
	}

	json::Node RequestStop(const RequestHandler& handler, const json::Node& value) {
		const std::set<std::string_view>* buses = handler.GetBusesByStop(value.AsDict().at("name").AsString());
		if (buses == nullptr) {
			return SaveEmptyAnswer(value);
		}
		json::Array arr;
		for (std::string_view stop : *buses) {
			arr.push_back(std::string(stop));
		}
		json::Node dict_node_stop{ json::Builder{}
									.StartDict()
										.Key("buses"s).Value(arr)
										.Key("request_id"s).Value(value.AsDict().at("id").AsInt())
									.EndDict().Build() };
		return dict_node_stop;
	}

	json::Node RequestBus(const RequestHandler& handler, const json::Node& value) {
		std::optional<BusRouteInfo> route_info = handler.GetBusStat(value.AsDict().at("name").AsString());
		if (route_info == std::nullopt) {
			return SaveEmptyAnswer(value);
		}
		json::Node dict_node_bus{ json::Builder{}
								.StartDict()
									.Key("curvature"s).Value(route_info->curvature)
									.Key("request_id"s).Value(value.AsDict().at("id").AsInt())
									.Key("route_length"s).Value(route_info->lenght)
									.Key("stop_count"s).Value(route_info->stops)
									.Key("unique_stop_count"s).Value(route_info->unique_stops)
								.EndDict().Build() };
		return dict_node_bus;
	}

	json::Node RequestMap(const RequestHandler& handler, const json::Node& value) {
		svg::Document doc = handler.RenderMap();
		std::ostringstream ostr;
		doc.Render(ostr);
		json::Node dict_node_map{ json::Builder{}
									.StartDict()
										.Key("map"s).Value(ostr.str())
										.Key("request_id"s).Value(value.AsDict().at("id"s).AsInt())
									.EndDict().Build() };
		return dict_node_map;
	}

	json::Node CreateNodeStop(const route::RouteData& data) {
		json::Node node_stop{ json::Builder{}.StartDict()
											.Key("stop_name"s).Value(std::string(data.stop_name))
											.Key("time"s).Value(data.bus_wait_time)
											.Key("type"s).Value("Wait"s)
										.EndDict().Build() };
		return node_stop;
	}

	json::Node CreateNodeBus(const route::RouteData& data) {
		json::Node node_route{ json::Builder{}.StartDict()
						.Key("bus"s).Value(std::string(data.bus_name))
						.Key("span_count"s).Value(data.span_count)
						.Key("time"s).Value(data.motion_time)
						.Key("type"s).Value("Bus"s)
					 .EndDict().Build() };
		return node_route;
	}
	
	json::Node CreateNodeRoute(const std::vector<route::RouteData>& route_data) {
		json::Node data_route{ json::Builder{}.StartArray().EndArray().Build() };
		for (const route::RouteData& data : route_data) {
			if (data.type == "bus"sv) {
				const_cast<json::Array&>(data_route.AsArray()).push_back(std::move(CreateNodeBus(data)));
			}
			else if (data.type == "stop"sv) {
				const_cast<json::Array&>(data_route.AsArray()).push_back(std::move(CreateNodeStop(data)));
			}
			else if (data.type == "stay_here"sv) {
				return data_route;
			}
		}
		return data_route;
	}

	double CalcTotalTime(const std::vector<route::RouteData>& route_data) {
		double total_time = 0.0;
		for (const route::RouteData& data : route_data) {
			if (data.type == "bus"sv) {
				total_time += data.motion_time;
			}
			else if (data.type == "stop"sv) {
				total_time += data.bus_wait_time;
			}
		}
		return total_time;
	}
	json::Node RequestRoute(const RequestHandler& handler, const json::Node& value) {
		std::optional<std::vector<route::RouteData>> route_data = handler.CreateRoute(value.AsDict().at("from").AsString(), value.AsDict().at("to").AsString());
		if (route_data == std::nullopt) {
			json::Node err_node{ json::Builder{}.StartDict()
													.Key("error_message").Value("not found")
													.Key("request_id").Value(value.AsDict().at("id"s).AsInt())
												.EndDict().Build() };
			return err_node;
		}

		json::Node node_route{json::Builder{}.StartDict()
											.Key("items"s).Value(CreateNodeRoute(route_data.value()).AsArray())
											.Key("request_id"s).Value(value.AsDict().at("id"s).AsInt())
											.Key("total_time").Value(CalcTotalTime(route_data.value()))
											.EndDict().Build() };

		return node_route;
	}

	void StatRequests(const json::Node& node, const RequestHandler& handler) {
		json::Array arr_answer;
		for (const auto& value : node.AsArray()) {
			if (value.AsDict().empty()) {
				continue;
			}
			if (value.AsDict().at("type").AsString() == "Stop") {
				json::Node dict_node_stop = RequestStop(handler, value);
				arr_answer.push_back(std::move(dict_node_stop));
				continue;
			}
			if (value.AsDict().at("type").AsString() == "Bus") {
				json::Node dict_node_bus = RequestBus(handler, value);
				arr_answer.push_back(std::move(dict_node_bus));
				continue;
			}
			if (value.AsDict().at("type").AsString() == "Map") {
				json::Node dict_node_map = RequestMap(handler, value);
				arr_answer.push_back(std::move(dict_node_map));
				continue;
			}
			if (value.AsDict().at("type").AsString() == "Route") {
				json::Node dict_node_route = RequestRoute(handler, value);
				arr_answer.push_back(std::move(dict_node_route));
				continue;
			}
		}
		 json::Print(json::Document{ arr_answer }, std::cout);
	}

	void BaseRequests(const json::Node& node, TransportCatalogue& catalogue) {
		std::vector<DataStop> dist_stop;
		for (const auto& value : node.AsArray()) {	//Step 1 - Add Stop
			if (value.AsDict().empty()) {
				continue;
			}
			if (value.AsDict().at("type").AsString() == "Stop") {
				DataStop data_stop;
				ReadStop(value, &data_stop);
				catalogue.AddStop(data_stop.stop_name, data_stop.coordinates);
				dist_stop.push_back(std::move(data_stop));
			}
		}
		for (DataStop& data_stop : dist_stop) {	//Step 2 - Add Distance
			for (auto [key_stop, dist] : data_stop.distance_to_stop) {	
				catalogue.SetDistance(data_stop.stop_name, key_stop, dist);
			}
		}
		for (const auto& value : node.AsArray()) {	//Step 3 - Add Bus Route
			if (value.AsDict().empty()) {
				continue;			
			}
			if (value.AsDict().at("type").AsString() == "Bus") {
				DataRoute data_route;
				ReadBusRoute(value, &data_route);
				AddBusToCatalogue(data_route, catalogue);
			}
		}
	}

	svg::Color ReadTypeColor(const json::Node& node) {
		if (node.IsString()) {
			return node.AsString();
		}
		if (node.IsArray()) {
			if (node.AsArray().size() == 3) {
				svg::Rgb rgb;
				rgb.red = node.AsArray()[0].AsInt();
				rgb.green = node.AsArray()[1].AsInt();
				rgb.blue = node.AsArray()[2].AsInt();
				return rgb;
			}
			if (node.AsArray().size() == 4) {
				svg::Rgba rgba;
				rgba.red = node.AsArray()[0].AsInt();
				rgba.green = node.AsArray()[1].AsInt();
				rgba.blue = node.AsArray()[2].AsInt();
				rgba.opacity = node.AsArray()[3].AsDouble();
				return rgba;
			}
		}
		std::monostate mono;
		return mono;
	}

	void ReadSetting(const json::Node& node, renderer::Settings& settings) {
		for (const auto& [key, value] : node.AsDict()) {
			if (key == "width"sv) {
				settings.width = value.AsDouble();
				continue;
			}
			if (key == "height"sv) {
				settings.height = value.AsDouble();
				continue;
			}
			if (key == "padding"sv) {
				settings.padding = value.AsDouble();
				continue;
			}
			if (key == "line_width"sv) {
				settings.line_width = value.AsDouble();
				continue;
			}
			if (key == "stop_radius"sv) {
				settings.stop_radius = value.AsDouble();
				continue;
			}
			if (key == "bus_label_font_size"sv) {
				settings.bus_label_font_size = value.AsInt();
				continue;
			}
			if (key == "bus_label_offset"sv) {
				settings.bus_label_offset.x = value.AsArray()[0].AsDouble();
				settings.bus_label_offset.y = value.AsArray()[1].AsDouble();
				continue;
			}
			if (key == "stop_label_font_size"sv) {
				settings.stop_label_font_size = value.AsInt();
				continue;
			}
			if (key == "stop_label_offset"sv) {
				settings.stop_label_offset.x = value.AsArray()[0].AsDouble();
				settings.stop_label_offset.y = value.AsArray()[1].AsDouble();
				continue;
			}
			if (key == "underlayer_color"sv) {
				settings.underlayer_color = ReadTypeColor(value);
				continue;
			}
			if (key == "underlayer_width"sv) {
				settings.underlayer_width = value.AsDouble();
				continue;
			}
			if (key == "color_palette"sv) {
				for (const auto& color : value.AsArray()) {
					settings.color_palette.push_back(ReadTypeColor(color));
					continue;
				}
			}
		}
	}		

	void ReadRouteSetting(const json::Node& node, route::RouteSettings& route_settings) {
		for (const auto& [key, value] : node.AsDict()) {
			if (key == "bus_velocity"sv) {
				route_settings.bus_velocity = value.AsInt();
				continue;
			}
			if (key == "bus_wait_time"sv) {
				route_settings.bus_wait_time = value.AsInt();
				continue;
			}
		}
	}

	void ReadRequests(std::istream& input, TransportCatalogue& catalogue ) {
		try {
			json::Document doc = json::Load(input);
			json::Dict json_type = doc.GetRoot().AsDict();

			auto value = json_type.at("base_requests");
			BaseRequests(value, catalogue);
			
			renderer::Settings settings;
			
			value = json_type.at("render_settings");
			ReadSetting(value, settings);
			
			route::RouteSettings route_setting;

			value = json_type.at("routing_settings");
			ReadRouteSetting(value, route_setting);

			route::TransportRouter router(catalogue, route_setting);
			renderer::MapRenderer map_render(settings);
			RequestHandler handler(catalogue, map_render, router);

			value = json_type.at("stat_requests");
			StatRequests(value, handler);
		}
		catch (const std::logic_error& err) {
			std::cerr << "Invalid data format: " << err.what() << std::endl;
		}
		catch (const json::ParsingError& err) {
			std::cerr << "ParsingError: " << err.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknow error" << std::endl;
		}
	}
}
