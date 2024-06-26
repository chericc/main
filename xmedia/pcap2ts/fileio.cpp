#include "fileio.hpp"

FileIO::FileIO(const std::string& url, const std::string& mode) {
    _xio = std::make_shared<XIOFile>(url, mode);
}

FileIO::~FileIO() { _xio.reset(); }

bool FileIO::ok() {
    if (_xio && !_xio->error()) {
        return true;
    }

    return false;
}

void FileIO::setLittleEndian(bool isLittleEndian) { _lendian = isLittleEndian; }

uint8_t FileIO::r8() {
    uint8_t value = 0;
    if (_xio) {
        value = _xio->r8();
    }
    return value;
}

uint16_t FileIO::r16() {
    uint16_t value = 0;
    if (_xio) {
        if (_lendian) {
            value = _xio->rl16();
        } else {
            value = _xio->rb16();
        }
    }
    return value;
}

uint32_t FileIO::r24() {
    uint32_t value = 0;
    if (_xio) {
        if (_lendian) {
            value = _xio->rl24();
        } else {
            value = _xio->rb24();
        }
    }
    return value;
}

uint32_t FileIO::r32() {
    uint32_t value = 0;
    if (_xio) {
        if (_lendian) {
            value = _xio->rl32();
        } else {
            value = _xio->rb32();
        }
    }
    return value;
}

uint64_t FileIO::r64() {
    uint64_t value = 0;
    if (_xio) {
        if (_lendian) {
            value = _xio->rl64();
        } else {
            value = _xio->rb64();
        }
    }
    return value;
}

std::vector<uint8_t> FileIO::read(std::size_t size) {
    if (_xio) {
        return _xio->read(size);
    }
    return std::vector<uint8_t>();
}

int64_t FileIO::tell() { return _xio->tell(); }

int FileIO::seek(int64_t offset, int whence) {
    return _xio->seek(offset, whence);
}

int FileIO::eof() { return _xio->eof(); }