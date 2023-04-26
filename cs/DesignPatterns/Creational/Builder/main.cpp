#include "house_builder.hpp"

int main()
{
    std::shared_ptr<House> house;
    auto type = ConstructionEngineer::Prefabricated;

    ConstructionEngineer e(type);
    e.constructHouse();
    e.getHouse(house);

    house->print();

    return 0;
}