#pragma once

#include "blockingconcurrentqueue.h"

namespace nyctlib {
	template<typename EventT>
	struct BlockingEventHolder {
		moodycamel::BlockingConcurrentQueue<EventT> queue;
	};
}