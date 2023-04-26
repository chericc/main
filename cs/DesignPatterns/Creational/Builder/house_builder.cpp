#include "house_builder.hpp"

#include <stdio.h>

void House::setFoundation(const std::string &s)
{
    foundation = s;
}

void House::setStruction(const std::string &s)
{
    structure = s;
}

void House::setRoof(const std::string &s)
{
    roof = s;
}

void House::setFurnished(bool b)
{
    furnished = b;
}

void House::setPainted(bool b)
{
    painted = b;
}

void House::print()
{
    printf ("House:\n"
        "Foundation:%s\n"
        "Structure:%s\n"
        "Roof:%s\n"
        "Furnished:%s"
        "Painted:%s\n\n",
        foundation.c_str(),
        structure.c_str(),
        roof.c_str(),
        furnished?"true":"false",
        painted?"true":"false");
}

HouseBuilder::HouseBuilder()
{
    house = std::make_shared<House>();
}

void HouseBuilder::getHouse(std::shared_ptr<House> &h)
{
    h = house;
    house.reset();
    return;
}

void ConcreteHouseBuilder::buildFoundation()
{
    house->setFoundation("concrete foundation");
}

void ConcreteHouseBuilder::buildStructure()
{
    house->setStruction("concrete structure");
}

void ConcreteHouseBuilder::buildRoof()
{
    house->setRoof("concrete roof");
}

void ConcreteHouseBuilder::paintHouse()
{
    house->setPainted(true);
}

void ConcreteHouseBuilder::furnishHouse()
{
    house->setFurnished(true);
}

void PrefabricatedHouseBuilder::buildFoundation()
{
    house->setFoundation("prefabricated foundation");
}

void PrefabricatedHouseBuilder::buildStructure()
{
    house->setStruction("prefabricated structure");
}

void PrefabricatedHouseBuilder::buildRoof()
{
    house->setRoof("prefabricated roof");
}

void PrefabricatedHouseBuilder::paintHouse()
{
    house->setPainted(true);
}

void PrefabricatedHouseBuilder::furnishHouse()
{
    house->setFurnished(true);
}

ConstructionEngineer::ConstructionEngineer(Type t)
{
    type = t;
}

void ConstructionEngineer::constructHouse()
{
    switch (type)
    {
        case Prefabricated:
        {
            builder = std::make_shared<PrefabricatedHouseBuilder>();
            break;
        }
        case Concrete:
        default:
        {
            builder = std::make_shared<ConcreteHouseBuilder>();
            break;
        }
    }
    
    builder->buildFoundation();
    builder->buildStructure();
    builder->buildRoof();
    builder->paintHouse();
    builder->furnishHouse();
}

void ConstructionEngineer::getHouse(std::shared_ptr<House> &house)
{
    builder->getHouse(house);
    builder.reset();
}