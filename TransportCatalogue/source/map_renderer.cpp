#include "map_renderer.h"

namespace renderer {
	using namespace std::literals;
	MapRenderer::MapRenderer(const Settings& settings) : settings_(settings) {
		size_palette_ = settings.color_palette.size();
	}

	const Settings& MapRenderer::GetSetting() const {
		return settings_;
	}

	std::pair<svg::Text, svg::Text> MapRenderer::FillingText(std::string bus_name, svg::Point point) const {
		svg::Text text_1;
		svg::Text text_2;
		text_1.SetPosition(point).SetOffset(settings_.bus_label_offset).
			SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).
			SetFontWeight("bold"s).SetData(bus_name).
			SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).
			SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).
			SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		text_2.SetPosition(point).SetOffset(settings_.bus_label_offset).
			SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).
			SetFontWeight("bold"s).SetData(bus_name).SetFillColor(settings_.color_palette[color_count_]);
		return { text_1 , text_2 };
	}

	std::optional<svg::Polyline> MapRenderer::CreateRouteLine(const domain::Bus* bus, const SphereProjector& sphere_proj) const {
		svg::Polyline polyline;
		if (bus->bus_route.empty()) {
			return std::nullopt;
		}
		for (domain::Stop* stop : bus->bus_route) {
			svg::Point point = sphere_proj(stop->coordinates);
			polyline.AddPoint(point).SetStrokeColor(settings_.color_palette[color_count_]).SetFillColor(svg::NoneColor).
									 SetStrokeWidth(settings_.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).
									 SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		}
		if (bus->is_roundtrip) {
			const_cast<MapRenderer*>(this)->ChangeCountColor();
			return polyline; 
		}
		for (auto it_back = bus->bus_route.rbegin() + 1; it_back != bus->bus_route.rend(); ++it_back) {
			svg::Point point = sphere_proj((*it_back)->coordinates);
			polyline.AddPoint(point).SetStrokeColor(settings_.color_palette[color_count_]).SetFillColor(svg::NoneColor).
									 SetStrokeWidth(settings_.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).
									 SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		}
		const_cast<MapRenderer*>(this)->ChangeCountColor();
		return polyline;
	}

	std::vector<std::pair<svg::Text, svg::Text>> MapRenderer::CreateRouteName(const domain::Bus* bus, const SphereProjector& sphere_proj) const {
		if (bus->bus_route.empty()) {
			return {};
		}
		svg::Point point = sphere_proj(bus->bus_route.front()->coordinates);
		auto [text_1, text_2] = FillingText(bus->name, point);
		if (bus->is_roundtrip || (bus->bus_route.size() == 1) || (bus->bus_route.front() == bus->bus_route.back())) {
			const_cast<MapRenderer*>(this)->ChangeCountColor();
			return { { text_1, text_2 } };
		}
		std::vector<std::pair<svg::Text, svg::Text>> result;
		result.push_back( {text_1, text_2} );
		point = sphere_proj(bus->bus_route.back()->coordinates);
		auto [text_1_, text_2_] = FillingText(bus->name, point);
		result.push_back({ text_1_, text_2_ });
		const_cast<MapRenderer*>(this)->ChangeCountColor();
		return result;
	}

	svg::Circle MapRenderer::CreateStopsSymbol(const domain::Stop* stop, const SphereProjector& sphere_proj) const {
		svg::Circle circle;
		svg::Point point = sphere_proj(stop->coordinates);
		circle.SetCenter(point).SetRadius(settings_.stop_radius).SetFillColor("white");
		return circle;
	}

	std::pair<svg::Text, svg::Text> MapRenderer::CreateStopsName(const domain::Stop* stop, const SphereProjector& sphere_proj) const {
		svg::Text text_1;
		svg::Text text_2;
		svg::Point point = sphere_proj(stop->coordinates);
		text_1.SetPosition(point).SetOffset(settings_.stop_label_offset).
			SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s).
			SetData(stop->name).SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).
			SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).
			SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		text_2.SetPosition(point).SetOffset(settings_.stop_label_offset).
			SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s).
			SetData(stop->name).SetFillColor("black");
		return {text_1, text_2};
	}

	void MapRenderer::ResetColorCount() const {
		const_cast<MapRenderer*>(this)->color_count_ = 0;
	}

	void MapRenderer::ChangeCountColor(){
		++color_count_;
		if (color_count_ < size_palette_) {
			 return;
		}
		color_count_ = 0;
	}

	svg::Document MapRenderer::CreateMap(const TransportCatalogue& catalogue) const {
		std::set<std::string_view> stops_names;
		std::set<std::string_view> buses_names;
		std::vector<geo::Coordinates> coordinates_stops;
		const std::unordered_map <std::string_view, Bus*>& all_buses = catalogue.GetAllBuses();
		for (const auto& [bus_name, struct_bus] : all_buses) {
			buses_names.insert(bus_name);
			for (Stop* stop : struct_bus->bus_route) {
				auto [_, status] = stops_names.insert(stop->name);
				if (status) {
					coordinates_stops.push_back(stop->coordinates);
				}
			}
		}
		renderer::SphereProjector sphere_proj(coordinates_stops.begin(), coordinates_stops.end(), settings_.width, settings_.height, settings_.padding);
		svg::Document doc;
		for (std::string_view bus : buses_names) {
			std::optional<svg::Polyline> poly = this->CreateRouteLine(all_buses.at(bus), sphere_proj);
			if (poly == std::nullopt) {
				continue;
			}
			doc.Add(*poly);
		}
		this->ResetColorCount();
		for (std::string_view bus : buses_names) {
			auto text = this->CreateRouteName(all_buses.at(bus), sphere_proj);
			if (text.empty()) {
				continue;
			}
			doc.Add(text[0].first);
			doc.Add(text[0].second);
			if (text.size() == 2) {
				doc.Add(text[1].first);
				doc.Add(text[1].second);
			}
		}
		const std::unordered_map <std::string_view, Stop*>& all_stops = catalogue.GetAllStops();
		for (std::string_view stop : stops_names) {
			svg::Circle circle = this->CreateStopsSymbol(all_stops.at(stop), sphere_proj);
			doc.Add(circle);
		}
		for (std::string_view stop : stops_names) {
			auto text = this->CreateStopsName(all_stops.at(stop), sphere_proj);
			doc.Add(text.first);
			doc.Add(text.second);
		}
		return doc;
	}
}