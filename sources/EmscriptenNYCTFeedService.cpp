#include "EmscriptenNYCTFeedService.h"

#include <emscripten.h>

#define API_SAMPLE_ENDPOINT_WITH_API_KEY_DONT_SHARE "http://192.168.1.10/api/nyctgtfsproxy/servelatestfeed.php"

namespace nyctlib {
    std::shared_ptr<NYCTFeedParser> EmscriptenNYCTFeedService::getLatestFeed() {
        char **buffer = nullptr;
        int length = 0;
        int status = -1;
        emscripten_wget_data(API_SAMPLE_ENDPOINT_WITH_API_KEY_DONT_SHARE, (void**)buffer, &length, &status);
        if (status != 0) {
            printf("Failed to open URL! Status code: %d\n", status);
            free(buffer);
            return nullptr;
        }

        printf("Read %d bytes OK.\n", length);
        
        //std::string sbuffer(*buffer, length);
        //printf("buffer: '%s'\n", sbuffer.c_str());
        auto f = std::make_shared<NYCTFeedParser>();
        bool loadbuf = f->loadBuffer(*buffer, length);

        printf("Loaded feed parser, loadBuffer(): %d\n", loadbuf);
        free(buffer);
        return f;
    }
}