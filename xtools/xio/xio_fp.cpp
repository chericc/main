#include "xio_fp.hpp"

#include <cstdio>

#include "xlog.h"
#include "xos_independent.hpp"

XIOFp::Context::~Context() {
}

XIOFp::XIOFp(FILE *fp) {
    do {
        iofctx_.fp = fp;
    } while (0);
}

XIOFp::~XIOFp() {}

int XIOFp::eof() {
    int beof = 0;
    do {
        if (error()) {
            beof = true;
            break;
        }

        if (feof(iofctx_.fp)) {
            beof = true;
            break;
        }
    } while (0);

    return (beof ? true : false);
}

int XIOFp::error() {
    int berror = false;
    do {
        if (!iofctx_.fp) {
            berror = true;
            break;
        }

        if (ferror(iofctx_.fp)) {
            berror = true;
            break;
        }
    } while (0);

    return (berror ? true : false);
}

int XIOFp::seek(int64_t offset, int whence) {
    int berror = 0;

    do {
        if (error()) {
            berror = true;
            break;
        }

        if (x_fseek64(iofctx_.fp, offset, whence) < 0) {
            berror = true;
            break;
        }
    } while (0);

    return (berror ? -1 : 0);
}

int64_t XIOFp::size() {
    int berror = 0;
    int64_t file_size = 0;

    do {
        if (error()) {
            berror = true;
            break;
        }

        long old_offset = x_ftell64(iofctx_.fp);

        if (old_offset < 0) {
            berror = true;
            break;
        }

        if (x_fseek64(iofctx_.fp, 0, SEEK_END) < 0) {
            berror = true;
            break;
        }

        long size = x_ftell64(iofctx_.fp);
        if (size < 0) {
            berror = true;
            break;
        }

        if (x_fseek64(iofctx_.fp, old_offset, SEEK_SET) < 0) {
            berror = true;
            break;
        }

        file_size = size;
    } while (0);

    return (berror ? -1 : file_size);
}

int64_t XIOFp::tell() {
    int berror = 0;
    int64_t pos = 0;

    do {
        if (error()) {
            berror = true;
            break;
        }

        long tell_ret = x_ftell64(iofctx_.fp);
        if (tell_ret < 0) {
            berror = true;
            break;
        }

        pos = tell_ret;
    } while (0);

    return (berror ? -1 : pos);
}

void XIOFp::flush() {
    do {
        if (error()) {
            break;
        }

        fflush(iofctx_.fp);
    } while (0);
}

uint8_t XIOFp::r8() {
    int berror = false;
    uint8_t buf[1] = {0};

    do {
        if (error()) {
            berror = true;
            break;
        }

        if (fread(buf, 1, sizeof(buf), iofctx_.fp) != sizeof(buf)) {
            return 0;
        }
    } while (0);

    if (berror) {
        xlog_err("read with error");
    }

    return buf[0];
}

std::vector<uint8_t> XIOFp::read(std::size_t size) {
    int berror = false;
    std::vector<uint8_t> buf;

    do {
        if (error()) {
            berror = true;
            break;
        }

        buf.resize(size);

        if (fread(buf.data(), 1, size, iofctx_.fp) != size) {
            berror = true;
            break;
        }
    } while (0);

    return (berror ? std::vector<uint8_t>() : buf);
}

void XIOFp::w8(uint8_t b) {
    int berror = false;
    uint8_t buf[1] = {b};

    do {
        if (error()) {
            berror = true;
            break;
        }

        if (fwrite(buf, 1, sizeof(buf), iofctx_.fp) != sizeof(buf)) {
            break;
        }
    } while (0);

    if (berror) {
        xlog_err("write with error");
    }

    return;
}

void XIOFp::write(const std::vector<uint8_t>& buffer) {
    int berror = false;

    do {
        if (error()) {
            berror = true;
            break;
        }

        if (fwrite(buffer.data(), 1, buffer.size(), iofctx_.fp) !=
            buffer.size()) {
            berror = true;
            break;
        }
    } while (0);

    if (berror) {
        xlog_err("write failed");
    }

    return;
}