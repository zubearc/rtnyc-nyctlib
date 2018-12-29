#pragma once

#include "IFeedService.h"

#include <vector>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;


namespace nyctlib {
	template <typename TFeedParser>
	class ReplayFeedService : public IFeedService<TFeedParser> {
	public:
		std::vector<std::string /* File Names */> protobuf_list;
		int current_protobuf_index = 0;
		int playback_limit = INT_MAX;

		ReplayFeedService(std::string search_path, std::string search_prefix) {
			for (fs::recursive_directory_iterator it(search_path), last; it != last; ++it) {
				auto r = it->path();
				auto fname = r.filename().string();
				if (fname.find(search_prefix) != EOF) {
					//std::cout << "File name found is " << fname << std::endl;
					this->protobuf_list.push_back(fname);
				}
			}
		}

		virtual void update() {
			if ((current_protobuf_index + 1) < protobuf_list.size() && (playback_limit > 0))
				current_protobuf_index++;
			playback_limit--;
		};

		void jumpTo(std::string search_query) {
			for (int i = 0; i < this->protobuf_list.size(); i++) {
				auto name = this->protobuf_list[i];
				if (name.find(search_query) != EOF) {
					this->current_protobuf_index = i;
				}
			}
		}

		void setMaximumPlaybacks(int max) {
			playback_limit = max;
		}

		virtual std::shared_ptr<TFeedParser> getLatestFeed() {
			auto parser = std::make_shared<TFeedParser>();
			auto fname = this->protobuf_list[current_protobuf_index];
			if (fname != "")
				parser->loadFile(fname);
			else
				return nullptr;
			return parser;
		}

		virtual std::shared_ptr<TFeedParser> getCurrentFeed() {
			return this->getLatestFeed();
		}
	};
}