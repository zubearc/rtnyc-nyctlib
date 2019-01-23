#include "../subway/SubwayTrackedTrip.h"

#include <map>

namespace nyctlib {
	class SessInterface {
	public:
		static void commit(std::string nyct_trip_id, int timestamp, bool isDirectionSouth,
			std::map<std::string, std::vector<SubwayTrackedTrip::ConfirmedStop>> &subway_trip_paths);

		static void concatenate();
	};
}