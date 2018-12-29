#pragma once

namespace nyctlib {

	enum class EventType {
		SubwayTripEvent,
		BusTripEvent
	};

	struct BaseEvent {
		virtual EventType getType() = 0;
	};
}