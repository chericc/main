#include <string>
#include <vector>

#include "json.hpp"
#include "xlog.hpp"

using namespace std;
using namespace nlohmann;

struct classmate {
    string name;
    string address;
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
    int age;
    nlohmann::json attr;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(classmate, name, address, age)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(person_attr, classmates, header)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(person, name, address, age, attr)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(interface_person, name, address, age, attr)

int main()
{
    xlog_dbg("hello\n");

    person pe;
    pe.name = "Tom";
    pe.address = "Wuhan,Hubei";
    pe.age = 17;

    classmate part;
    part.name = "Potter";
    part.address = "Tianjin";
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
        
    return 0;
}