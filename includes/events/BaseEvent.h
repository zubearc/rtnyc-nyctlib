#pragma once

namespace nyctlib {

	enum class EventType {
		SubwayTripEvent
	};

	struct BaseEvent {
		virtual EventType getType() = 0;
	};
}