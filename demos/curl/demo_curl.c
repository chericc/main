
#include <curl/curl.h>

#include <stdio.h>

// Custom debug callback function (optional)
static int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr) {
    const char *text;
    switch (type) {
        case CURLINFO_TEXT:
            // text = "== Info";
            break;
        case CURLINFO_HEADER_OUT:
            // text = "=> Send header";
            break;
        case CURLINFO_DATA_OUT:
            // text = "=> Send data";
            break;
        case CURLINFO_SSL_DATA_OUT:
            // text = "=> Send SSL data";
            break;
        case CURLINFO_HEADER_IN:
            // text = "<= Recv header";
            break;
        case CURLINFO_DATA_IN:
            // text = "<= Recv data";
            break;
        case CURLINFO_SSL_DATA_IN:
            // text = "<= Recv SSL data";
            break;
        default:
            return 0;
    }

    // fprintf(stderr, "%s, %lu bytes (0x%lx)\n", text, (unsigned long)size, (unsigned long)size);
    fprintf(stderr, "%s, %lu bytes (0x%lx)\n", "", (unsigned long)size, (unsigned long)size);
    fwrite(data, 1, size, stderr);
    return 0;
}

int main(void) {
    CURL *curl;
    CURLcode res;
    const char *url = "http://114.80.110.25:6001/api/device/binding";
    const char *json_data = 
        "{\n"
        "    \"bindKey\":      \"283f15fae2934c8cadd61174fcf20606\",\n"
        "    \"devId\":        \"791c3a200000a865\"\n"
        "}\";";

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        struct curl_slist *headers = NULL;

        // Set the URL for the POST request
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Specify that this is a POST request
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Set the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

        // Set custom headers
        // headers = curl_slist_append(headers, "Content-Type: application/json");
        // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Enable verbose output
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Set a custom debug callback function (optional)
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);

        headers = curl_slist_append(
            headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
