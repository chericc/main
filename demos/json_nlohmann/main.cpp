#include "json.hpp"

#include "xlog.h"

namespace {

void test_make_json_str()
{
    nlohmann::json js;
    js["name"] = "Handsome";
    js["sex"] = "Male";
    js["age"] = 32;
    js["temp"] = 36.51;
    js["parents"]["father"] = "HFather";
    js["parents"]["mother"] = "HMother";

    xlog_dbg("js dump: %s\n", js.dump().c_str());
}

void test_parse_json_str()
{
    auto str_json = R"({"name": "Handsome", "sex": "Male", "age": 32, "temp": 36.51})";
    auto js_obj = nlohmann::json::parse(str_json);

    xlog_dbg("js dump: %s\n", js_obj.dump().c_str());
}

struct Parents {
    std::string father;
    std::string mother;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Parents, father, mother);
};

struct Student {
    std::string name;
    std::string sex;
    int age;
    float temp;
    Parents parents;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Student, name, sex, age, temp, parents);
};

void test_serial()
{
    Student student = {};
    student.name = "Handsome";
    student.sex = "Male";
    student.age = 18;
    student.temp = 36.51;
    student.parents.father = "HFather";
    student.parents.mother = "HMother";

    nlohmann::json js = student;
    xlog_dbg("dump: %s\n", js.dump().c_str());
}

void test_unserial()
{
    auto str_json = R"({"age":18,"name":"Handsome","parents":{"father":"HFather","mother":"HMother"},"sex":"Male","temp":36.50})";
    auto js = nlohmann::json::parse(str_json);
    auto stru = js.template get<Student>();
    xlog_dbg("name: %s, temp: %g\n", stru.name.c_str(), stru.temp);

    // what if some element is lost?
    auto str_json_incomplete = R"({"age":18,"name":"Handsome","parents":{"father":"HFather","mother":"HMother"},"sex":"Male"})";
    auto js_incomplete = nlohmann::json::parse(str_json_incomplete);
    auto stru_incomplete = js_incomplete.template get<Student>();
    xlog_dbg("name: %s, temp: %g\n", stru_incomplete.name.c_str(), stru_incomplete.temp);
}

}

int main(int, char *[])
{
    xlog_dbg("in\n");

    // test_make_json_str();
    // test_parse_json_str();

    // test_serial();
    test_unserial();

    return 0;
}