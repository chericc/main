#include "my_nng.hpp"

#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>
#include <array>
#include <vector>
#include <cstring>

#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"
#include "nng/protocol/reqrep0/req.h"
#include <nng/supplemental/util/platform.h>

#include "xlog.h"

using Lock = std::lock_guard<std::mutex>;

namespace {

enum {
    TIMEOUT_MS = 2000,
    MSG_SIZE_LIMIT = 1 * 1024 * 1024,
    MAX_CACHED_SOCK_NUMBER = 5,
};

enum class WORK_STATE
{
     INIT, 
     RECV, 
     WAIT, 
     SEND 
};

struct work {
	WORK_STATE state;
	nng_aio *aio;
	nng_msg *msg;
	nng_ctx  ctx;
    bool ctx_valid;
    my_nm_cb_on_req cb;
};

struct my_nm_server {
    bool sock_valid;
    nng_socket sock;
    std::array<work, 4> workers;
};

struct my_nm_client {
    bool sock_valid;
    nng_socket sock;
};

struct my_nm_context {
    std::mutex mutex_call;

    std::mutex mutex_opened_server;
    std::unordered_map<std::string, std::shared_ptr<my_nm_server>> opened_server;

    std::mutex mutex_opened_client;
    std::unordered_map<std::string, my_nm_client> opened_client;

    std::mutex mutex_msg_client;
    nng_msg *msg_client;

    std::mutex mutex_server_rsp_msg_buf;
    std::vector<uint8_t> server_rsp_msg_buf;
};

struct my_nm_context s_ctx;

void handle_msg(struct work *worker, nng_msg *msg)
{
    do {
        xlog_dbg("handle msg, len=%zd\n", nng_msg_len(msg));

        s_ctx.server_rsp_msg_buf.resize(MSG_SIZE_LIMIT);

        size_t recv_msg_len = nng_msg_len(msg);
        if (worker->cb == nullptr) {
            xlog_dbg("callback is null\n");
            break;
        }
        
        size_t rsp_msg_buf_size = s_ctx.server_rsp_msg_buf.size();
        worker->cb(nng_msg_body(msg), recv_msg_len, 
            s_ctx.server_rsp_msg_buf.data(), &rsp_msg_buf_size);
        if (rsp_msg_buf_size > MSG_SIZE_LIMIT) {
            xlog_err("invalid size returned: %zu\n", rsp_msg_buf_size);
            break;
        }

        nng_msg_clear(msg);
        nng_msg_append(msg, s_ctx.server_rsp_msg_buf.data(), rsp_msg_buf_size);

        // ok
    } while (false);

    xlog_dbg("handle msg over\n");
}

void server_cb(void *arg)
{
	auto *work = reinterpret_cast<struct work *>(arg);
	nng_msg *    msg;
	int          rv;
	// uint32_t     when;

	switch (work->state) {
	case WORK_STATE::INIT: {
		work->state = WORK_STATE::RECV;
		nng_ctx_recv(work->ctx, work->aio);
		break;
    }
	case WORK_STATE::RECV: {
        rv = nng_aio_result(work->aio);
        if (rv != 0) {
            xlog_err("nng_aio_result error\n");
        }
        
		msg = nng_aio_get_msg(work->aio);

        handle_msg(work, msg);

		work->msg   = msg;
		work->state = WORK_STATE::WAIT;
        nng_sleep_aio(0, work->aio);
		break;
    }
	case WORK_STATE::WAIT: {
		// We could add more data to the message here.

		nng_aio_set_msg(work->aio, work->msg);
		work->msg   = NULL;
		work->state = WORK_STATE::SEND;
		nng_ctx_send(work->ctx, work->aio);
		break;
    }
	case WORK_STATE::SEND: {
		rv = nng_aio_result(work->aio);
        if (rv != 0) {
			nng_msg_free(work->msg);
            xlog_err("nng_ctx_send failed\n");
		}
		work->state = WORK_STATE::RECV;
		nng_ctx_recv(work->ctx, work->aio);
		break;
    }
	default: {
        xlog_err("bad state: %d\n", (int)work->state);
		break;
    }
	}

    // return 
}

}

int my_nng_start(const char *url, my_nm_start_param const* param)
{
    Lock lock(s_ctx.mutex_call);

    bool error_flag = false;

    // 为什么用指针？
    // 因为 nng_aio_alloc 回调用了地址，不能让指针失效
    auto serv = std::make_shared<my_nm_server>();
    
    do {
        int ret = 0;

        {
            Lock lock(s_ctx.mutex_opened_server);

            if (s_ctx.opened_server.find(url) != s_ctx.opened_server.end()) {
                xlog_err("url already started\n");
                break;
            }

            if (s_ctx.opened_server.size() > 4) {
                xlog_err("server number over limit: %zd\n", s_ctx.opened_server.size());
            }
        }
        
        ret = nng_rep0_open(&serv->sock);
        if (ret != 0) {
            xlog_err("nng_rep0_open failed\n");
            error_flag = true;
            break;
        }

        serv->sock_valid = true;
        
        for (size_t i = 0; i < serv->workers.size(); ++i) {
            serv->workers[i].cb = param->cb_on_req; // pass to workers

            ret = nng_aio_alloc(&serv->workers[i].aio, server_cb, &serv->workers[i]);
            if (ret != 0) {
                xlog_err("nng_aio_alloc failed\n");
                error_flag = true;
                break;
            }

            ret = nng_ctx_open(&serv->workers[i].ctx, serv->sock);
            if (ret != 0) {
                xlog_err("nng_ctx_open failed\n");
                error_flag = true;
                break;
            }
            serv->workers[i].ctx_valid = true;
            serv->workers[i].state = WORK_STATE::INIT;
        }

        ret = nng_listen(serv->sock, url, nullptr, 0);

        for (size_t i = 0; i < serv->workers.size(); ++i) {
            server_cb(&serv->workers[i]);
        }

        s_ctx.opened_server[url] = serv;

        xlog_dbg("nm start ok: %s\n", url);
    } while (false);

    if (error_flag) {
        for (size_t i = 0; i < serv->workers.size(); ++i) {
            if (serv->workers[i].aio != nullptr) {
                nng_aio_free(serv->workers[i].aio);
                serv->workers[i].aio = nullptr;
            }
            if (serv->workers[i].ctx_valid) {
                nng_ctx_close(serv->workers[i].ctx);
                serv->workers[i].ctx_valid = false;
            }
        }

        if (serv->sock_valid) {
            nng_close(serv->sock);
            serv->sock_valid = false;
        }
    }

    return error_flag ? -1 : 0;
}

int my_nng_req(const char *url, my_nm_req_param *req_param)
{
    Lock lock(s_ctx.mutex_call);

    bool error_flag = false;
    my_nm_client client = {};

    do {
        Lock lock(s_ctx.mutex_opened_client);
        auto client_it = s_ctx.opened_client.end();
        int ret = 0;

        if (req_param->req_size > MSG_SIZE_LIMIT) {
            xlog_err("req size over limit: %zd > %d\n", req_param->req_size, (int)MSG_SIZE_LIMIT);
            error_flag = true;
            break;
        }

        client_it = s_ctx.opened_client.find(url);
        if (client_it == s_ctx.opened_client.end()) {
            if (s_ctx.opened_client.size() >= MAX_CACHED_SOCK_NUMBER) {
                xlog_err("over max cached client number: %zu\n", s_ctx.opened_client.size());
                error_flag = true;
                break;
            }

            ret = nng_req0_open(&client.sock);
            if (ret != 0) {
                xlog_err("nng_req0_open failed\n");
                error_flag = true;
                break;
            }
            client.sock_valid = true;

            ret = nng_dial(client.sock, url, nullptr, 0);
            if (ret != 0) {
                xlog_err("nng_dial failed\n");
                error_flag = true;
                break;
            }

            client_it = s_ctx.opened_client.insert(std::make_pair(url, client)).first;
            client.sock_valid = false;
        }

        if (client_it == s_ctx.opened_client.end()) {
            xlog_err("unexpected happendd\n");
            error_flag = true;
            break;
        }

        Lock lock_msg(s_ctx.mutex_msg_client);
        if (s_ctx.msg_client == nullptr) {
            ret = nng_msg_alloc(&s_ctx.msg_client, 0);
            if (ret != 0) {
                xlog_err("nng_msg_alloc failed\n");
                error_flag = true;
                break;
            }
            ret = nng_msg_reserve(s_ctx.msg_client, MSG_SIZE_LIMIT);
            if (ret != 0) {
                xlog_err("nng_msg_reserve failed\n");
                error_flag = true;
                break;
            }
        }

        nng_msg_clear(s_ctx.msg_client);
        nng_msg_append(s_ctx.msg_client, req_param->req, req_param->req_size);

        ret = nng_sendmsg(client_it->second.sock, s_ctx.msg_client, 0);
        if (ret != 0) {
            xlog_err("nng_sendmsg failed\n");
            error_flag = true;
            break;
        }

        ret = nng_recvmsg(client_it->second.sock, &s_ctx.msg_client, 0);
        if (ret != 0) {
            xlog_err("nng_recvmsg failed\n");
            error_flag = true;
            break;
        }

        if ((req_param->rsp == nullptr) || (req_param->rsp_size == nullptr)) {
            break;
        }
        
        size_t rsp_size_cache = *req_param->rsp_size;
        size_t msg_len = nng_msg_len(s_ctx.msg_client);
        if (msg_len > rsp_size_cache) {
            xlog_err("rsp size over buf len, deserted\n");
            error_flag = true;
            break;
        }

        memcpy(req_param->rsp, nng_msg_body(s_ctx.msg_client), msg_len);
        *req_param->rsp_size = msg_len;

        // success
    } while (false);
    
    if (error_flag) {
        if (client.sock_valid) {
            nng_close(client.sock);
            client.sock_valid = false;
        }
    }

    return error_flag ? -1 : 0;
}

int my_nng_stop(const char *url)
{
    Lock lock(s_ctx.mutex_call);
    
    xlog_dbg("not implemented yet\n");
    return 0;
}
