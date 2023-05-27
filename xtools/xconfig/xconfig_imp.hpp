/**
 * @file xconfig_imp.hpp
 * @author xxx
 * @brief 
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <stdio.h>

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <stdint.h>

class XConfigImp
{
public:
    XConfigImp(const std::string &filename, bool readonly);
    ~XConfigImp();
    
    /**
     * @brief Load file into memory.
     * 
     * @return int Return 0 if successful or -1 if failed.
     */
    int LoadFile ();

    /**
     * @brief Get the Value object
     * 
     * @param section 
     * @param key 
     * @param default_value 
     * @param value 
     * @return int Return 0 if successful or -1 if failed.
     */
    int GetValue(const std::string &section, const std::string &key,std::string &value);

    /**
     * @brief Set the Value object
     * 
     * @param section 
     * @param key 
     * @param value 
     * @return int Return 0 if successful or -1 if failed.
     */
    int SetValue (const std::string &section, const std::string &key, const std::string &value);

    /**
     * @brief Test if key exist.
     * 
     * @param section 
     * @param key 
     * @return true Exist.
     * @return false Not Exist.
     */
    bool Exist(const std::string &section, const std::string &key);

    /**
     * @brief Remove a key from config. If the section is empty after this key is 
     * removed, then the section will also be removed.
     * 
     * @param section 
     * @param key 
     * @return Return 0 if successful or -1 if failed.
     */
    int Erase(const std::string &section, const std::string &key);
private:
    int WriteToFile__ ();
    int LoadFromFile__ ();
    std::string LoadFileAsText__();

    int DataGet__ (const std::string &section, const std::string &key, std::string &value);
    int DataSet__ (const std::string &section, const std::string &key, const std::string &value);
    bool DataExist__ (const std::string &section, const std::string &key);
    int DataErase__ (const std::string &section, const std::string &key);

    static std::string Trim__(const std::string &str);

private:
    const XConfigImp & operator= (const XConfigImp &) = delete;
    XConfigImp & operator= (XConfigImp &) = delete;
    XConfigImp (const XConfigImp &) = delete;
private:
    std::string m_filename__;
    bool m_readonly__;

    bool m_file_loaded__;  // Indicate file load successfully.
    // bool m_modify_state__;  // Indicate content changed but hasn't write to file yet.
    std::map<std::string, std::map<std::string,std::string> > m_map_data__;
    std::mutex m_mutex__;
};