#include "xconfig_imp.hpp"

#include <stdio.h>
#include <string.h>

#include "xlog.hpp"
#include "xstring.hpp"

XConfigImp::XConfigImp(const std::string &filename, bool readonly)
{
    m_file_loaded__ = false;
    m_filename__ = filename;
    m_readonly__ = readonly;
}

XConfigImp::~XConfigImp()
{
}

int XConfigImp::LoadFile()
{
    std::lock_guard<std::mutex> lock (m_mutex__);

    int ret = 0;

    ret = LoadFromFile__();

    if (0 == ret)
    {
        xlog_dbg("Load file successful\n");
        m_file_loaded__  = true;
    }
    else
    {
        xlog_err("Load file failed\n");
    }

    return m_file_loaded__ ? 0 : -1;
}

int XConfigImp::GetValue(const std::string &section, const std::string &key,std::string &value)
{
    std::lock_guard<std::mutex> lock (m_mutex__);

    if (! m_file_loaded__)
    {
        xlog_err("Not loaded\n");
        return -1;
    }

    if (DataGet__(section, key, value) < 0)
    {
        xlog_err("Get data failed\n");
        return -1;
    }

    return 0;
}

int XConfigImp::SetValue(const std::string &section, const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock (m_mutex__);

    if (! m_file_loaded__)
    {
        xlog_err("Not loaded\n");
        return -1;
    }

    if (m_readonly__)
    {
        xlog_err("Readonly mode\n");
        return -1;
    }

    if (DataSet__(section, key, value) < 0)
    {
        xlog_err("Data set failed\n");
        return -1;
    }

    if (WriteToFile__() < 0)
    {
        xlog_err("Write to file failed\n");
        return -1;
    }

    return 0;
}

bool XConfigImp::Exist(const std::string &section, const std::string &key)
{
    std::lock_guard<std::mutex> lock (m_mutex__);

    if (! m_file_loaded__)
    {
        xlog_err("Not loaded\n");
        return -1;
    }

    if (! DataExist__(section, key))
    {
        return false;
    }

    return true;
}

int XConfigImp::Erase(const std::string &section, const std::string &key)
{
    std::lock_guard<std::mutex> lock (m_mutex__);

    if (! m_file_loaded__)
    {
        xlog_err("Not loaded\n");
        return -1;
    }

    if (m_readonly__)
    {
        xlog_err("Readonly mode\n");
        return -1;
    }

    if (DataErase__(section, key) < 0)
    {
        xlog_err("Erase failed\n");
        return -1;
    }

    if (WriteToFile__() < 0)
    {
        xlog_err("Write to file failed\n");
        return -1;
    }

    return 0;
}

int XConfigImp::WriteToFile__()
{
    FILE *fp = nullptr;

    if (m_readonly__)
    {
        xlog_err("Can't write to file with readonly mode\n");
        return 0;
    }

    if (! m_file_loaded__)
    {
        xlog_dbg("Can't write to file while state not ready\n");
        return -1;
    }

    fp = fopen (m_filename__.c_str(), "wb+");
    if (nullptr == fp)
    {
        xlog_err("Open file to write failed\n");
        return -1;
    }

    for (auto it_map = m_map_data__.cbegin(); it_map != m_map_data__.cend(); ++it_map)
    {
        if (it_map->second.empty())
        {
            xlog_err("Section with no keys in map found\n");
            continue ;
        }

        fprintf (fp, "[%s]\n", it_map->first.c_str());

        for (auto it_item = it_map->second.cbegin(); it_item != it_map->second.cend(); ++it_item)
        {
            fprintf (fp, "%s=%s\n", 
                it_item->first.c_str(), 
                it_item->second.c_str());
        }

        fprintf (fp, "\n");
    }

    fclose (fp);
    fp = nullptr;

    return 0;
}

std::string XConfigImp::LoadFileAsText__()
{
    std::string text;

    FILE* fp = nullptr;
    int ret = 0;

    do
    {
        fp = fopen(m_filename__.c_str(), "r");
        if (!fp)
        {
            break;
        }

        for (;;)
        {
            char buffer[64];

            if (feof(fp) || ferror(fp))
            {
                break;
            }

            ret = fread(buffer, 1, sizeof(buffer) - 1, fp);
            if (ret > 0)
            {
                buffer[ret] = '\0';
                text += std::string(buffer);
            }
        }

    } while (0);

    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }

    return text;
}

int XConfigImp::LoadFromFile__()
{
    std::string text;
    std::list<std::string> list;
    size_t pos_line_section_head = 0;
    size_t pos_line_section_tail = 0;
    size_t pos_line_equal_sign = 0;
    std::string section_name;

    m_map_data__.clear();

    text = LoadFileAsText__();

    list = xstring_split(text, "\n");
    text.clear();

    for (auto it = list.begin(); it != list.end(); ++it)
    {
        pos_line_section_head = it->find("[");
        pos_line_section_tail = it->find("]");
        pos_line_equal_sign = it->find("=");

        /* section */
        if (pos_line_section_head != std::string::npos
            && pos_line_section_tail != std::string::npos
            && pos_line_section_head + 1 < pos_line_section_tail)
        {
            section_name = std::string(*it, pos_line_section_head + 1, pos_line_section_tail - pos_line_section_head);
            continue;
        }

        if (section_name.empty())
        {
            continue;
        }

        /* key */
        if (pos_line_equal_sign != std::string::npos
            && pos_line_equal_sign > 0
            && pos_line_equal_sign + 1 < it->size())
        {
            std::string key(*it, 0, pos_line_equal_sign - 0);
            std::string value(*it, pos_line_equal_sign + 1, it->size() - pos_line_equal_sign - 1);
            key = Trim__(key);
            value = Trim__(value);
            DataSet__(section_name, key, value);
        }
    }

    return 0;
}

#if 0
int XConfigImp::LoadFromFile__()
{
    size_t size = 0;
    ssize_t ret_ssize = 0;
    std::string cur_section;
    FILE *fp = nullptr;

    char *pc_line = nullptr;

    char *pc_line_section_head = nullptr;
    char *pc_line_section_tail = nullptr;
    char *pc_line_equal_sign = nullptr;
    char *pc_line_end = nullptr;

    fp = fopen (m_filename__.c_str(), "rb");
    if (nullptr == fp)
    {
        xlog_dbg("Open file failed trying loading file(%s)\n",
            m_filename__.c_str());
        return -1;
    }

    xlog_dbg("Data clear\n");
    m_map_data__.clear();

    /* Parse file line by line. */
    while ( (ret_ssize = getline (& pc_line, & size, fp)) >= 0)
    {
        if (ret_ssize == 0)
        {
            continue ;
        }

        pc_line_section_head = strstr (pc_line, "[");
        pc_line_section_tail = strstr (pc_line, "]");
        pc_line_equal_sign = strstr(pc_line, "=");
        pc_line_end = pc_line + strlen (pc_line);
        
        /* section */
        if (pc_line_section_head != nullptr
            && pc_line_section_tail != nullptr
            && pc_line_section_head + 1 < pc_line_section_tail)
        {
            cur_section = std::string (pc_line_section_head + 1, pc_line_section_tail - pc_line_section_head - 1);
            continue ;
        }

        if (cur_section.empty())
        {
            xlog_err("Line without section found\n");
            continue;
        }

        /* key */
        if (pc_line_equal_sign != nullptr
            && pc_line < pc_line_equal_sign
            && pc_line_equal_sign + 1 < pc_line_end)
        {
            std::string key_tmp(pc_line, pc_line_equal_sign - pc_line);
            std::string value_tmp (pc_line_equal_sign + 1, pc_line_end - pc_line_equal_sign - 1);

            key_tmp = Trim__(key_tmp);
            value_tmp = Trim__(value_tmp);

            DataSet__(cur_section, key_tmp, value_tmp);
        }
    }

    if (nullptr != pc_line)
    {
        free (pc_line);
        pc_line = nullptr;
    }

    if (nullptr != fp)
    {
        fclose (fp);
        fp = nullptr;
    }

    return 0;
}
#endif 

int XConfigImp::DataGet__ (const std::string &section, const std::string &key, std::string &value)
{
    auto it_section = m_map_data__.find (section);
    if (it_section == m_map_data__.cend())
    {
        xlog_err("Get [%s.%s]: section not exist\n",
            section.c_str(), key.c_str());
        return -1;
    }

    auto it_item = it_section->second.find (key);
    if (it_item == it_section->second.cend())
    {
        xlog_err("Get [%s.%s]: key not exist\n",
            section.c_str(), key.c_str());
        return -1;
    }

    if (it_item->second.empty())
    {
        xlog_inf("Value is empty\n");
    }

    value = it_item->second;

    return 0;
}

int XConfigImp::DataSet__ (const std::string &section, const std::string &key, const std::string &value)
{
    auto it_section = m_map_data__.find (section);
    if (it_section == m_map_data__.cend())
    {
        std::map<std::string, std::string> itemsMapTmp = {{key, value}};
        m_map_data__[section] = itemsMapTmp;
    }
    else
    {
        auto it_item = it_section->second.find (key);
        it_section->second[key] = value;
    }

    return 0;
}

bool XConfigImp::DataExist__ (const std::string &section, const std::string &key)
{
    auto it_section = m_map_data__.find (section);
    if (it_section == m_map_data__.cend())
    {
        xlog_dbg("Section not exist\n");
        return false;
    }

    auto it_item = it_section->second.find (key);
    if (it_item == it_section->second.cend())
    {
        xlog_dbg("Key not exist\n");
        return false;
    }

    return true;
}

int XConfigImp::DataErase__ (const std::string &section, const std::string &key)
{
    auto it_section = m_map_data__.find (section);
    if (it_section == m_map_data__.cend())
    {
        xlog_dbg("Section not exist\n");
        return 0;
    }

    auto it_item = it_section->second.find (key);
    if (it_item == it_section->second.cend())
    {
        xlog_dbg("Key not exist\n");
        return 0;
    }

    it_section->second.erase (it_item);
    if (it_section->second.empty())
    {
        m_map_data__.erase (it_section);
    }

    return 0;
}

std::string XConfigImp::Trim__(const std::string &str)
{
    const char *pc_head = str.c_str();
    const char *pc_tail = str.c_str() + str.length();
    const char *pc_iterator_head = pc_head;
    const char *pc_iterator_tail = pc_tail;

    while (std::isblank(*pc_iterator_head) && pc_iterator_head < pc_tail)
    {
        ++pc_iterator_head;
    }
    while (std::isblank(*pc_iterator_tail) && pc_iterator_head > pc_head)
    {
        --pc_iterator_tail;
    }

    if (pc_iterator_head >= pc_iterator_tail)
    {
        return std::string();
    }
    else
    {
        return std::string(pc_iterator_head, pc_iterator_tail);
    }
}