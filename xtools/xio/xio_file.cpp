#include "xio_file.hpp"

#include <memory>
#include <stdio.h>

#include "xlog.h"
#include "xio_fp.hpp"

XIOFile::IOFileContenxt::~IOFileContenxt() {
    xiofp.reset();
    if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
}

XIOFile::XIOFile(const std::string& url, const std::string& mode) {
    int berror = false;

    do {

        iofctx_.fp = fopen(url.c_str(), mode.c_str());
        if (nullptr == iofctx_.fp) {
            xlog_err("open file failed({})", url.c_str());
            berror = true;
            break;
        }

        iofctx_.xiofp = std::make_shared<XIOFp>(iofctx_.fp);

        xlog_trc("open file successful");
    } while (0);

    if (berror) {
        xlog_err("init failed");
    }
}

XIOFile::~XIOFile() {
}

int XIOFile::eof() {
    int beof = 0;
    do {
        if (nullptr == iofctx_.xiofp) {
            xlog_err("null\n");
            break;
        }

        beof = iofctx_.xiofp->eof();
    } while (0);

    return (beof ? true : false);
}

int XIOFile::error() {
    int berror = false;
    do {
        if (nullptr == iofctx_.xiofp) {
            xlog_err("null\n");
            berror = true;
            break;
        }

        berror = iofctx_.xiofp->error();
    } while (0);

    return (berror ? true : false);
}

int XIOFile::seek(int64_t offset, int whence) {
    int berror = 0;

    do {
        if (error()) {
            berror = true;
            break;
        }

        int ret = iofctx_.xiofp->seek(offset, whence);
        if (ret < 0) {
            berror = true;
        }
    } while (0);

    return (berror ? -1 : 0);
}

int64_t XIOFile::size() {
    int berror = 0;
    int64_t file_size = 0;

    do {
        if (error()) {
            berror = true;
            break;
        }

        file_size = iofctx_.xiofp->size();
    } while (0);

    return (berror ? -1 : file_size);
}

int64_t XIOFile::tell() {
    int berror = 0;
    int64_t pos = 0;

    do {
        if (error()) {
            berror = true;
            break;
        }

        pos = iofctx_.xiofp->tell();
    } while (0);

    return (berror ? -1 : pos);
}

void XIOFile::flush() {
    do {
        if (error()) {
            break;
        }

        iofctx_.xiofp->flush();
    } while (0);
}

uint8_t XIOFile::r8() {
    int berror = false;
    uint8_t buf[1] = {0};

    do {
        if (error()) {
            berror = true;
            break;
        }

        buf[0] = iofctx_.xiofp->r8();
    } while (0);

    if (berror) {
        xlog_err("read with error");
    }

    return buf[0];
}

std::vector<uint8_t> XIOFile::read(std::size_t size) {
    int berror = false;
    std::vector<uint8_t> buf;

    do {
        if (error()) {
            berror = true;
            break;
        }
        
        buf = iofctx_.xiofp->read(size);
    } while (0);

    return (berror ? std::vector<uint8_t>() : buf);
}

void XIOFile::w8(uint8_t b) {
    int berror = false;

    do {
        if (error()) {
            berror = true;
            break;
        }

        iofctx_.xiofp->w8(b);
    } while (0);

    if (berror) {
        xlog_err("write with error");
    }

    return;
}

void XIOFile::write(const std::vector<uint8_t>& buffer) {
    int berror = false;

    do {
        if (error()) {
            berror = true;
            break;
        }

        iofctx_.xiofp->write(buffer);
    } while (0);

    if (berror) {
        xlog_err("write failed");
    }

    return;
}