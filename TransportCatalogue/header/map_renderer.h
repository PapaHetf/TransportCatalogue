#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"
#include <algorithm>
#include <unordered_map>
#include "transport_catalogue.h"

namespace renderer {
    using namespace transport_catalogue::store;
	struct Settings {
		double width = 0.0;			
		double height = 0.0;		
		double padding = 0.0;		
		double line_width = 0.0;	
		double stop_radius = 0.0;	
		uint32_t bus_label_font_size = 0; 
		svg::Point bus_label_offset = { 0.0, 0.0 };	
		uint32_t stop_label_font_size = 0; 
		svg::Point stop_label_offset = { 0.0, 0.0 };	
		svg::Color underlayer_color;
		double underlayer_width = 0.0; 
		std::vector<svg::Color> color_palette;
	};

    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
            double max_height, double padding)
            : padding_(padding) {
            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs.lng < rhs.lng;
                });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            const auto [bottom_it, top_it]
                = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs.lat < rhs.lat;
                    });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const {
            return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

	class MapRenderer {
	public:
		MapRenderer(const Settings& settings);
        const Settings& GetSetting() const;
        std::optional<svg::Polyline> CreateRouteLine(const domain::Bus* bus, const SphereProjector& sphere_proj) const;
        std::vector<std::pair<svg::Text,svg::Text>> CreateRouteName(const domain::Bus* bus, const SphereProjector& sphere_proj) const;
        svg::Circle CreateStopsSymbol(const domain::Stop* stop, const SphereProjector& sphere_proj) const;
        std::pair<svg::Text, svg::Text> CreateStopsName(const domain::Stop* stop, const SphereProjector& sphere_proj) const;
        svg::Document CreateMap(const TransportCatalogue& catalogue) const;
        void ResetColorCount() const;
    private:
        void ChangeCountColor();
        std::pair<svg::Text, svg::Text> FillingText(std::string bus_name , svg::Point point) const;

		const Settings& settings_;
        size_t color_count_ = 0;
        size_t size_palette_ = 0;
	};
}

