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

struct person {
    string name;
    string address;
    int age;
    std::vector<classmate> classmates;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(classmate, name, address, age)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(person, name, address, age, classmates)

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

    pe.classmates.push_back(part);
    part.name = "Jim";
    pe.classmates.push_back(part);

    json jp = pe;

    xlog_dbg("dump: \n%s\n",jp.dump(4).c_str());

    // from arbitrary ends here

    auto p_de_arbitrary = jp.template get<person>();
    
    xlog_dbg("name: %s, address: %s, age: %d\n", 
        p_de_arbitrary.name.c_str(),
        p_de_arbitrary.address.c_str(),
        p_de_arbitrary.age);
    for (auto const &ref : p_de_arbitrary.classmates) {
        xlog_dbg("classmates: [name:%s,address:%s,age:%d]\n", 
            ref.name.c_str(),
            ref.address.c_str(),
            ref.age);
    }

    return 0;
}