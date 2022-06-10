#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <deque>
#include <set>
#include <optional>

#include "domain.h"

namespace transport_catalogue {
	namespace store {
		using namespace domain;
		using namespace geo;
		namespace detail {
			struct DistanceHasher {
				size_t operator()(const std::pair<const Stop*, const Stop*>& dist) const {
					return std::hash<const void*>{}(dist.first) + std::hash<const void*>{}(dist.second);
				}
			};
		}
		class TransportCatalogue {
		public:
			void AddBus(const std::string& name, std::vector<Stop*> bus_route, bool is_roundtrip);

			void AddStop(std::string stop_name, geo::Coordinates coordinates);

			const Bus* FindBus(std::string_view name_bus) const;

			const Stop* FindStop(std::string_view name_stop) const;

			void SetDistance(const std::string& first_stop_name, const std::string& second_stop_name, const double& distance);

			const std::unordered_map <std::string_view, Stop*>& GetAllStops() const;

			const std::unordered_map <std::string_view, Bus*>& GetAllBuses() const;

			const std::optional<BusRouteInfo> GetInfoRoute(std::string_view name_bus) const;

			const std::set<std::string_view>* GetInfoStop(std::string_view stop) const;

			const std::optional<double> GetLenght(const Stop* first_stop, const Stop* second_stop) const;

		private:
			std::deque<Bus> deq_buses_;
			std::deque<Stop> deq_stops_;
			std::unordered_map<std::string_view, Stop*> all_stops_;
			std::unordered_map<std::string_view, Bus*> all_buses_;
			std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::DistanceHasher> distance_;
			std::unordered_map<std::string_view, std::set<std::string_view>> stop_buses_;

			template <typename Begin, typename End>
			std::pair<double, double> GetAllLenght(const Begin begin, const End end) const;

			size_t GetUniqueStops(const Bus* bus) const;
		};

		template <typename Begin, typename End>
		inline std::pair<double, double> TransportCatalogue::GetAllLenght(const Begin begin, const End end) const {
			double lenght = 0.0;
			double curvature = 0.0;
			for (auto stop1 = begin, stop2 = stop1 + 1; stop2 != end; ++stop1, ++stop2) {
				std::optional<double> lenght_ = GetLenght(*stop1, *stop2);
				if (lenght_ != std::nullopt) {
					lenght += *lenght_;
				}
				curvature += ComputeDistance((*stop1)->coordinates, (*stop2)->coordinates);
			}
			return { lenght, curvature };
		}
	}
}
