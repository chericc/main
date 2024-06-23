#include "bmp.hpp"

#include <inttypes.h>

#include "xlog.hpp"

#define MKLE16(a, b) ((a) | ((uint16_t)b << 8))

// Align to 4 bytes.
#define WIDTH_PAD_BYTES(width_bytes) (((width_bytes) + 3) & (~0x3))

BmpDecoder::BmpDecoder(const std::string& filename) : _filename(filename) {
    doLoadFile();
}

BmpDecoder::~BmpDecoder() { doCloseFile(); }

bool BmpDecoder::loadSuccessful() {
    if (_load_info) {
        return true;
    }
    return false;
}

int BmpDecoder::width() {
    if (_load_info) {
        return _load_info->width;
    }

    return 0;
}

int BmpDecoder::height() {
    if (_load_info) {
        return _load_info->height;
    }

    return 0;
}

BmpDecoder::PixFmt BmpDecoder::pixfmt() {
    if (_load_info) {
        return _load_info->pixfmt;
    }
    return PIXFMT_NONE;
}

std::shared_ptr<std::vector<uint8_t>> BmpDecoder::getContent(int pos,
                                                             int numPixels) {
    std::shared_ptr<std::vector<uint8_t>> buf;

    do {
        if (!_load_info || !_load_info->xio) {
            xlog_err("null");
            break;
        }

        int depth_bytes = 0;
        int pixel_min = 0;
        int pixel_max = _load_info->width * _load_info->height;

        if (!_load_info->width || !_load_info->height) {
            xlog_err("width or height is 0");
            break;
        }

        if (pos < pixel_min || pos >= pixel_max) {
            xlog_err("pos invalid");
            break;
        }

        if (numPixels < 0) {
            numPixels = pixel_max - pos;
        }

        if (numPixels < pixel_min || pos + numPixels > pixel_max) {
            xlog_err("num pixels invalid");
            break;
        }

        depth_bytes = pixelDepthBytes(_load_info->pixfmt);
        if (depth_bytes <= 0) {
            xlog_err("pixfmt not support");
            break;
        }

        buf = std::make_shared<std::vector<uint8_t>>();

        buf->reserve(depth_bytes * numPixels);

        int width_pad_bytes = WIDTH_PAD_BYTES(_load_info->width * depth_bytes);

        /* TODO: performance improvement. */
        for (int i = pos; i < pos + numPixels; ++i) {
            int width = _load_info->width;
            int x = i % width;
            int y = i / width;
            int offset =
                _load_info->data_offset + y * width_pad_bytes + x * depth_bytes;
            _load_info->xio->seek(offset, SEEK_SET);
            auto pixel_buf = _load_info->xio->read(depth_bytes);
            for (auto const& ref : pixel_buf) {
                buf->push_back(ref);
            }
        }
    } while (0);
    if (buf && buf->empty()) {
        buf->shrink_to_fit();
    }

    return buf;
}

int BmpDecoder::saveBmp(const BmpInfo& info) {
    int berror = false;

    do {
        BMP_FileHeader fileheader{};
        BMP_BitmapHeader bitmapheader{};

        uint16_t bitspersample = 0;

        std::shared_ptr<XIO> xio;

        if (!info.data || info.data->empty()) {
            xlog_err("data is empty");
            berror = true;
            break;
        }

        if (info.width <= 0 || info.height <= 0) {
            xlog_err("width or height invalid[%d,%d]", info.width, info.height);
            berror = true;
            break;
        }

        switch (info.pixfmt) {
            case PIXFMT_BGRA: {
                bitspersample = 32;
                break;
            }
            case PIXFMT_BGR24: {
                bitspersample = 24;
                break;
            }
            case PIXFMT_RGB555: {
                bitspersample = 16;
                break;
            }
            default: {
                xlog_err("pixfmt not support(%#x)", (int)info.pixfmt);
                berror = true;
                break;
            }
        }

        if (berror) {
            break;
        }

        int bytespersample = bitspersample / 8;

        if (info.data->size() !=
            (std::size_t)(info.width * info.height * bytespersample)) {
            xlog_err("data.size != width * height * depth");
            berror = true;
            break;
        }

        int width_pad_bytes = WIDTH_PAD_BYTES(info.width * bytespersample);

        int pad_bytes = width_pad_bytes - (info.width * bytespersample);

        fileheader.filetype = MKLE16('B', 'M');
        fileheader.filesize = 14 + 40 + width_pad_bytes * info.height;
        fileheader.reserved1 = 0;
        fileheader.reserved2 = 0;
        fileheader.bitmapoffset = 14 + 40;

        bitmapheader.size = 40;
        bitmapheader.width_v3 = info.width;
        bitmapheader.height_v3 = info.height;
        bitmapheader.planes = 1;
        bitmapheader.bitsperpixel = bitspersample;

        bitmapheader.compression = 0;
        bitmapheader.sizeofbitmap = width_pad_bytes * info.height;
        bitmapheader.horzresolution = 0;
        bitmapheader.vertresolution = 0;
        bitmapheader.colorsimportant = 0;

        xio = std::make_shared<XIOFile>(info.file, "wb");
        if (xio->error()) {
            xlog_err("open file failed");
            berror = true;
            break;
        }

        // fileheader
        xio->wl16(fileheader.filetype);
        xio->wl32(fileheader.filesize);
        xio->wl16(fileheader.reserved1);
        xio->wl16(fileheader.reserved2);
        xio->wl32(fileheader.bitmapoffset);

        // bitmapheader
        xio->wl32(bitmapheader.size);
        xio->wl32(bitmapheader.width_v3);
        xio->wl32(bitmapheader.height_v3);
        xio->wl16(bitmapheader.planes);
        xio->wl16(bitmapheader.bitsperpixel);

        xio->wl32(bitmapheader.compression);
        xio->wl32(bitmapheader.sizeofbitmap);
        xio->wl32(bitmapheader.horzresolution);
        xio->wl32(bitmapheader.vertresolution);
        xio->wl32(bitmapheader.colorsused);
        xio->wl32(bitmapheader.colorsimportant);

        // data
        std::vector<uint8_t> buf_pixel;
        buf_pixel.resize(bytespersample);

        auto lambda_write_line = [&](int y) {
            for (int x = 0; x < info.width; ++x) {
                std::size_t offset = (y * info.width + x) * bytespersample;
                auto const& data = *info.data;
                for (std::size_t i = 0; i < (std::size_t)bytespersample; ++i) {
                    buf_pixel[i] = data[i + offset];
                }
                xio->write(buf_pixel);
            }

            for (int i = 0; i < pad_bytes; ++i) {
                xio->w8(0x0);
            }
        };

        if (!info.invert_y) {
            for (int y = info.height - 1; y >= 0; --y) {
                lambda_write_line(y);
            }
        } else {
            for (int y = 0; y < info.height; ++y) {
                lambda_write_line(y);
            }
        }

        if (xio->error()) {
            xlog_err("xio failed");
            berror = true;
            break;
        }

        xio.reset();
    } while (0);

    return berror ? -1 : 0;
}

int BmpDecoder::doLoadFile() {
    int berror = false;
    std::shared_ptr<LoadInfo> info;

    info = std::make_shared<LoadInfo>();

    do {
        if (!info->xio) {
            info->xio = std::make_shared<XIOFile>(_filename, "rb");
        }

        if (!info->xio) {
            xlog_err("open failed");
            berror = true;
            break;
        }

        if (readHeader(info) < 0) {
            xlog_err("read header failed");
            berror = true;
            break;
        }
    } while (0);

    if (berror) {
        info.reset();
        return -1;
    }

    _load_info = info;

    return 0;
}

int BmpDecoder::doCloseFile() {
    _load_info.reset();
    return 0;
}

int BmpDecoder::readHeader(std::shared_ptr<LoadInfo> info) {
    int berror = false;
    std::vector<uint8_t> buf;

    BiCompression comp = BMP_RGB;

    if (!info || !info->xio) {
        xlog_err("null");
        return -1;
    }

    do {
        info->xio->seek(0, SEEK_SET);

        if (info->xio->size() < 14) {
            berror = true;
            xlog_err("bmp size too small");
            break;
        }

        info->fileheader.filetype = info->xio->rl16();
        if (info->fileheader.filetype != MKLE16('B', 'M')) {
            berror = true;
            xlog_err("bmp magic number error");
            break;
        }

        info->fileheader.filesize = info->xio->rl32();
        if (info->xio->size() < info->fileheader.filesize) {
            berror = true;
            xlog_err("bmp size check failed");
            break;
        }

        info->fileheader.reserved1 = info->xio->rl16();
        info->fileheader.reserved2 = info->xio->rl16();

        info->fileheader.bitmapoffset = info->xio->rl32();

        xlog_trc("filesize=%d,dataoffset=%d", (int)info->fileheader.filesize,
                 (int)info->fileheader.bitmapoffset);

        info->bitmapheader.size = info->xio->rl32();

        xlog_trc("filesize=%d,headersize=%d", (int)info->fileheader.filesize,
                 (int)info->bitmapheader.size);

        switch (info->bitmapheader.size) {
            case 12:  // OS/2 v1
            {
                info->bitmapheader.width_v2 = info->xio->rl16();
                info->bitmapheader.height_v2 = info->xio->rl16();

                info->width = info->bitmapheader.width_v2;
                info->height = info->bitmapheader.height_v2;
                break;
            }
            case 40:  // windib
            // case  56: // windib v3
            // case  64: // OS/2 v2
            case 108:  // windib v4
                // case 124: // windib v5
                {
                    info->bitmapheader.width_v3 = info->xio->rl32();
                    info->bitmapheader.height_v3 = info->xio->rl32();

                    info->width = info->bitmapheader.width_v3;
                    info->height = info->bitmapheader.height_v3;
                    break;
                }
            default: {
                berror = true;
                break;
            }
        }  // end of switch

        if (info->height < 0) {
            xlog_trc("reverse height");
            info->height = (0 - info->height);
        }

        xlog_trc("width=%d,height=%d", (int)info->width, (int)info->height);

        if (berror) {
            xlog_err("header not support(size=%d)",
                     (int)info->bitmapheader.size);
            break;
        }

        /* planes */
        if (info->xio->rl16() != 1) {
            xlog_err("invalid BMP header");
            berror = true;
            break;
        }

        info->bitmapheader.bitsperpixel = info->xio->rl16();

        if (info->bitmapheader.bitsperpixel != 16 &&
            info->bitmapheader.bitsperpixel != 24 &&
            info->bitmapheader.bitsperpixel != 32) {
            xlog_err("bmp depth not support(%d)",
                     (int)info->bitmapheader.bitsperpixel);
            berror = true;
            break;
        }

        xlog_trc("bmp bit depth=%d", (int)info->bitmapheader.bitsperpixel);

        if (info->bitmapheader.size >= 40) {
            comp = (BiCompression)info->xio->rl32();
            info->bitmapheader.compression = comp;
            info->bitmapheader.sizeofbitmap = info->xio->rl32();
            info->bitmapheader.horzresolution = info->xio->rl32();
            info->bitmapheader.vertresolution = info->xio->rl32();
            info->bitmapheader.colorsused = info->xio->rl32();
            info->bitmapheader.colorsimportant = info->xio->rl32();

            xlog_trc(
                "bmp bitmap header: [comp=%d,sizeofbitmap=%d,horres=%d,]"
                "verres=%d,colorused=%d,colorimpor=%d]",
                info->bitmapheader.compression, info->bitmapheader.sizeofbitmap,
                info->bitmapheader.horzresolution,
                info->bitmapheader.vertresolution,
                info->bitmapheader.colorsused,
                info->bitmapheader.colorsimportant);
        } else {
            comp = BMP_RGB;
        }

        if (comp != BMP_RGB && comp != BMP_BITFIELDS && comp != BMP_RLE4 &&
            comp != BMP_RLE8) {
            xlog_err("bmp compression not support(%d)", (int)comp);
            berror = true;
            break;
        }

        /* TODO. */
        if (comp != BMP_RGB) {
            xlog_err("bmp not support compression");
            berror = true;
            break;
        }

        switch (info->bitmapheader.bitsperpixel) {
            case 32: {
                info->pixfmt = PIXFMT_BGRA;
                break;
            }
            case 24: {
                info->pixfmt = PIXFMT_BGR24;
                break;
            }
            case 16: {
                info->pixfmt = PIXFMT_RGB555;
                break;
            }
            default: {
                xlog_err("bmp pixfmt not support");
                berror = true;
                break;
            }
        }

        if (berror) {
            break;
        }

        info->data_offset = info->fileheader.bitmapoffset;
        info->data_size =
            info->width * info->height * (info->bitmapheader.bitsperpixel / 8);

        xlog_trc("dataoffset=%d,datasize=%d,filesize=%" PRId64,
                 (int)info->data_offset, (int)info->data_size,
                 info->xio->size());

        /* TODO. */
        if (info->fileheader.bitmapoffset != 14 + info->bitmapheader.size) {
            xlog_err("bmp decode not support palette section");
            berror = true;
            break;
        }
    } while (0);

    if (berror) {
        return -1;
    }

    return 0;
}

int BmpDecoder::pixelDepthBytes(PixFmt pixfmt) {
    int bytes = -1;

    if (PIXFMT_BGRA == pixfmt) {
        bytes = 4;
    } else if (PIXFMT_BGR24 == pixfmt) {
        bytes = 3;
    } else if (PIXFMT_RGB555 == pixfmt) {
        bytes = 2;
    }

    return bytes;
}