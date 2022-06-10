#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include <memory>

namespace route {
	using namespace transport_catalogue::store;
	using namespace std::literals;

	struct RouteSettings {
		int bus_wait_time = 0;
		int bus_velocity = 0;
	};

	struct EdgeData {
		double time_weight = 0.0;
		int span_count = 0;
		std::string_view bus_name = {};
	};

	struct RouteData {
		std::string_view type = {};
		std::string_view bus_name = {};
		std::string_view stop_name = {};
		double motion_time = 0.0;
		int bus_wait_time = 0;
		int span_count = 0;
	};

	class TransportRouter {
	public:
		using Graph = graph::DirectedWeightedGraph<double>;

		using EdgesList = std::vector<std::pair<std::string_view, std::string_view>>;
		using EdgesInfo = std::vector<EdgeData>;

		TransportRouter(const TransportCatalogue& catalogue, const RouteSettings& route_setting);

		std::optional<std::vector<RouteData>> CreatRoute(const std::string_view& from, const std::string_view& to) const;

	private:
		const TransportCatalogue& catalogue_;
		const RouteSettings& route_setting_;
		mutable Graph graph_;
		std::unique_ptr<graph::Router<double>> router_ = nullptr;

		mutable EdgesList edges_;
		mutable EdgesInfo edges_info_;

		void BuildGraph() const;

		std::vector<RouteData> CreateAnswer(const std::optional<graph::Router<double>::RouteInfo>& route_info) const;

		RouteData CreateBusAnswer(size_t edge_index, double time) const;

		RouteData CreateStopAnswer(size_t edge_index) const;

		RouteData CreateEmptyAnswer() const;

		template <typename Begin, typename End>
		void CreateEdgesAlongRoute(const Begin begin, const End end, std::string_view bus_name) const;

	};

	template <typename Begin, typename End>
	void TransportRouter::CreateEdgesAlongRoute(const Begin begin, const End end, std::string_view bus_name) const {
		const double wait = route_setting_.bus_wait_time * 60 * 1.0;
		const double bus_speed = route_setting_.bus_velocity * 1.0 / 3.6;

		for (auto it_stop = begin; it_stop < end; ++it_stop) {
			int span_count = 1;
			double current_lenght = 0.0;
			for (auto from_stop = it_stop, to_stop = from_stop + 1; to_stop != end; ++from_stop, ++to_stop) {
				std::optional<double> lenght = catalogue_.GetLenght(*from_stop, *to_stop);
				current_lenght += lenght.value();
				double time_weight = current_lenght / bus_speed + wait;

				graph_.AddEdge({ (*it_stop)->id, (*to_stop)->id, time_weight });

				edges_.push_back({ (*it_stop)->name, (*to_stop)->name });
				edges_info_.push_back({ time_weight, span_count, bus_name });
				++span_count;
			}
		}
	}
}