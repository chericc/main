#include "test_comm.hpp"

size_t fileSize(FILE *fp)
{
    size_t size_file = 0;
    size_t size_origin = 0;
    if (fp)
    {
        size_origin = ftell(fp);
        fseek(fp, 0, SEEK_END);
        size_file = ftell(fp);
        fseek(fp, size_origin, SEEK_SET);
    }

    return size_file;
}


std::vector<uint16_t> readFile16(const std::string &filename)
{
    std::vector<uint16_t> buffer;
    FILE *fp = fopen(filename.c_str(), "rb");

    if (fp)
    {
        std::size_t filesize = fileSize(fp);

        buffer.resize(filesize);

        std::size_t ret = fread(buffer.data(), 1, filesize, fp);
        if (ret != filesize)
        {
            buffer.clear();
        }

        fclose(fp);
        fp = nullptr;
    }

    return buffer;
}

std::vector<uint8_t> readFile8(const std::string &filename)
{
    std::vector<uint8_t> buffer;
    FILE *fp = fopen(filename.c_str(), "rb");

    if (fp)
    {
        size_t filesize = fileSize(fp);

        
        buffer.resize(filesize);

        std::size_t ret = fread(buffer.data(), 1, filesize, fp);
        if (ret != filesize)
        {
            buffer.clear();
        }

        fclose(fp);
        fp = nullptr;
    }

    return buffer;
}

std::vector<uint8_t> readFile(const std::string &filename)
{
    return readFile8(filename);
}

void saveFile(const std::string &filename, std::vector<uint8_t> data)
{
    FILE *fp = fopen(filename.c_str(), "wb");
    if (fp)
    {
        std::size_t ret = fwrite(data.data(), 1, data.size(), fp);
        if (ret != data.size())
        {
            perror("Write failed");
        }

        if (fp)
        {
            fclose(fp);
            fp = nullptr;
        }
    }
}