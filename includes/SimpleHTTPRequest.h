#pragma once

#include <curl/curl.h>
#include <curl/easy.h>
#include <sstream>
#include <iostream>

extern "C" {
	size_t curl_write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
		const char *cdata = (const char*)ptr;
		std::string data((const char*)ptr, (size_t)size * nmemb);
		*((std::stringstream*)stream) << data << std::endl;
		return size * nmemb;
	}

	size_t curl_write_data_file(void *ptr, size_t size, size_t nmemb, FILE *stream) {
		size_t written = fwrite(ptr, size, nmemb, stream);
		return written;
	};
};

class SimpleHTTPRequest {
	void* curl;

public:
	SimpleHTTPRequest::SimpleHTTPRequest() {
		curl = curl_easy_init();
	}

	bool SimpleHTTPRequest::get_save(const char *url, const char *filename) {
		FILE *file = fopen(filename, "wb");
		if (file == NULL) {
			printf("Failed to open file %s!\n", filename);
			return false;
		}
		curl_easy_setopt(curl, CURLOPT_URL, url);
		/* example.com is redirected, so we tell libcurl to follow redirection */
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data_file);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
		/* Perform the request, res will get the return code */
		CURLcode res = curl_easy_perform(curl);
		fclose(file);
		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			return false;
		}
		return true;
	}

	SimpleHTTPRequest::~SimpleHTTPRequest() {
		curl_easy_cleanup(curl);
	}
};