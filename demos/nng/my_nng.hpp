#pragma once

#include <cstddef>

using my_nm_cb_on_req = void(*)(void const *req, size_t req_size, void *rsp, size_t *rsp_size);
struct my_nm_start_param {
    my_nm_cb_on_req cb_on_req;
};
struct my_nm_req_param {
    void const *req;
    size_t req_size;
    void *rsp;
    size_t *rsp_size;
};

int my_nng_start(const char *url, my_nm_start_param const* param);
int my_nng_req(const char *url, my_nm_req_param *req_param);
int my_nng_stop(const char *url);
