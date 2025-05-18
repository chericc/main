#include <string>
#include <vector>

#include "xlog.h"

#define JSON_DIAGNOSTICS 1
#include "json.hpp"

using namespace std;
using namespace nlohmann;

struct classmate {
    string name;
    char address[64];
    int age;
};

struct person_attr {
    std::vector<classmate> classmates;
    classmate header;
};

struct person {
    string name;
    string address;
    int age;
    person_attr attr;
};

struct interface_person {
    string name;
    string address;
    int age = 20;
    nlohmann::json attr;
};

struct interface_person_default {
    string name;
    string address;
    int age;
    std::string attr;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(classmate, name, address, age)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(person_attr, classmates, header)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(person, name, address, age, attr)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(interface_person, name, address, age, attr)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(interface_person_default, name, address, age, attr)
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(interface_person_default, age)

int main()
{
    xlog_dbg("hello\n");

    {
        person pe;
        pe.name = "Tom";
        pe.address = "Wuhan,Hubei";
        pe.age = 17;
    
        classmate part;
        part.name = "Potter";
        snprintf(part.address, sizeof(part.address), "Tianjin");
        part.age = 19;
    
        pe.attr.classmates.push_back(part);
        part.name = "Jim";
        pe.attr.classmates.push_back(part);
    
        pe.attr.header = part;
    
        json jp = pe;
    
        xlog_dbg("dump: \n%s\n",jp.dump(4).c_str());
    
        // from arbitrary ends here
    
        auto p_de_arbitrary = jp.template get<interface_person>();
        
        xlog_dbg("name: %s, address: %s, age: %d, attr: %s\n", 
            p_de_arbitrary.name.c_str(),
            p_de_arbitrary.address.c_str(),
            p_de_arbitrary.age,
            p_de_arbitrary.attr.dump().c_str());
    
    }

    {
        std::string json_str = "{ \"address\": \"Tianjin\" }";
        json jp = json::parse(json_str);
        auto p_de = jp.template get<interface_person_default>();
        xlog_dbg("name: %s, address: %s, age: %d, attr: %s\n", 
            p_de.name.c_str(),
            p_de.address.c_str(),
            p_de.age,
            p_de.attr.c_str());
    }
    


    return 0;
}