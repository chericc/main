#include "utf8_enc.hpp"

size_t enc_utf8_length(const std::string &utf8_in)
{
    auto utf32 = enc_utf8_2_utf32(utf8_in);
    return enc_utf32_length(utf32);
    // return enc_utf32_length(enc_utf8_2_utf32(utf8_in));
}

size_t enc_utf32_length(const std::vector<uint32_t> &utf32_in)
{
    int len = 0;
    for (auto const& ref : utf32_in)
    {
        if (!ref)
        {
            break;
        }

        ++len;
    }
    return len;
}

std::string enc_utf8_substr(const std::string &utf8_in, size_t pos, size_t count)
{
    return enc_utf32_2_utf8(
            enc_utf32_substr(
                enc_utf8_2_utf32(utf8_in), pos, count));
}

std::vector<uint32_t> enc_utf32_substr(const std::vector<uint32_t> &utf32_in, size_t pos, size_t count)
{
    std::vector<uint32_t> result;

    size_t in_size = utf32_in.size();

    for (size_t i = pos; 
        (i < pos + count) && (i < in_size); 
        ++i)
    {
        result.push_back(utf32_in.at(i));
    }

    return result;
}

static int enc_get_utf8_size(uint8_t value)
{
    int count = 0;
    for (int i = 7; i >= 0; --i)
    {
        if ((1 << i) & value)
        {
            ++count;
        }
        else 
        {
            break;
        }
    }

    if (1 == count || count > 6)
    {
        // error
        return -1;
    }

    return (0 == count) ? 1 : count;
}


static int enc_unicode_to_utf8_one(uint32_t unic, char *pOutput,
        int outSize)
{ 
    if (outSize < 6)
    {
        return 0;
    }

    if ( unic <= 0x0000007F )
    {
        // * U-00000000 - U-0000007F:  0xxxxxxx
        *pOutput     = (unic & 0x7F);
        return 1;
    }
    else if ( unic >= 0x00000080 && unic <= 0x000007FF )
    {
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
        *(pOutput+1) = (unic & 0x3F) | 0x80;
        *pOutput     = ((unic >> 6) & 0x1F) | 0xC0;
        return 2;
    }
    else if ( unic >= 0x00000800 && unic <= 0x0000FFFF )
    {
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
        *(pOutput+2) = (unic & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 12) & 0x0F) | 0xE0;
        return 3;
    }
    else if ( unic >= 0x00010000 && unic <= 0x001FFFFF )
    {
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput+3) = (unic & 0x3F) | 0x80;
        *(pOutput+2) = ((unic >>  6) & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >> 12) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 18) & 0x07) | 0xF0;
        return 4;
    }
    else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF )
    {
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput+4) = (unic & 0x3F) | 0x80;
        *(pOutput+3) = ((unic >>  6) & 0x3F) | 0x80;
        *(pOutput+2) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >> 18) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 24) & 0x03) | 0xF8;
        return 5;
    }
    else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF )
    {
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput+5) = (unic & 0x3F) | 0x80;
        *(pOutput+4) = ((unic >>  6) & 0x3F) | 0x80;
        *(pOutput+3) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput+2) = ((unic >> 18) & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >> 24) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 30) & 0x01) | 0xFC;
        return 6;
    }
  
    return 0;
}


static int enc_utf8_to_unicode_one(const char* pInput, int utfbytes, uint32_t *Unic)
{ 
    char b1, b2, b3, b4, b5, b6;
  
    *Unic = 0x0;
    // int utfbytes = enc_get_utf8_size(*pInput);
    unsigned char *pOutput = (unsigned char *) Unic;
  
    switch ( utfbytes )
    {
        case 1:
            *pOutput     = *pInput;
            utfbytes    += 1;
            break;
        case 2:
            b1 = *pInput;
            b2 = *(pInput + 1);
            if ( (b2 & 0xE0) != 0x80 )
                return 0;
            *pOutput     = (b1 << 6) + (b2 & 0x3F);
            *(pOutput+1) = (b1 >> 2) & 0x07;
            break;
        case 3:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )
                return 0;
            *pOutput     = (b2 << 6) + (b3 & 0x3F);
            *(pOutput+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);
            break;
        case 4:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) )
                return 0;
            *pOutput     = (b3 << 6) + (b4 & 0x3F);
            *(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
            *(pOutput+2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);
            break;
        case 5:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            b5 = *(pInput + 4);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )
                return 0;
            *pOutput     = (b4 << 6) + (b5 & 0x3F);
            *(pOutput+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);
            *(pOutput+2) = (b2 << 2) + ((b3 >> 4) & 0x03);
            *(pOutput+3) = (b1 << 6);
            break;
        case 6:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            b5 = *(pInput + 4);
            b6 = *(pInput + 5);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)
                    || ((b6 & 0xC0) != 0x80) )
                return 0;
            *pOutput     = (b5 << 6) + (b6 & 0x3F);
            *(pOutput+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);
            *(pOutput+2) = (b3 << 2) + ((b4 >> 4) & 0x03);
            *(pOutput+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
            break;
        default:
            return 0;
            break;
    }
  
    return utfbytes;
}

std::vector<uint32_t> enc_utf8_2_utf32(const std::string &utf8_in)
{
    std::vector<uint32_t> utf32_out;

    int max_len = utf8_in.size();

    int in_index = 0;
    uint32_t utf32_value = 0;

    while (in_index < max_len)
    {
        int utf8bytes = enc_get_utf8_size(utf8_in.at(in_index));
        if (utf8bytes < 0)
        {
            // error
            continue;
        }

        if (in_index + utf8bytes > max_len)
        {
            // error
            break;
        }

        if (enc_utf8_to_unicode_one(utf8_in.data() + in_index, utf8bytes, &utf32_value) > 0)
        {
            utf32_out.push_back(utf32_value);
        }

        in_index += utf8bytes;
    }

    return utf32_out;
}

std::string enc_utf32_2_utf8(const std::vector<uint32_t> &utf32_in)
{
    std::string result;

    for (auto const& utf32_value : utf32_in)
    {
        char utf8_buffer[8]{};
        
        int ret = enc_unicode_to_utf8_one(utf32_value, 
            utf8_buffer, sizeof(utf8_buffer) - 1);
        
        if (ret > 0)
        {
            result += std::string(utf8_buffer);
        }
    }

    return result;
}

