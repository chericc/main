
#pragma once

#include <memory>
#include <string>

class House {
   public:
    void setFoundation(const std::string& s);
    void setStruction(const std::string& s);
    void setRoof(const std::string& s);
    void setFurnished(bool b);
    void setPainted(bool b);
    void print();

   private:
    std::string foundation;
    std::string structure;
    std::string roof;
    bool furnished{false};
    bool painted{false};
};

class HouseBuilder {
   public:
    HouseBuilder();
    virtual ~HouseBuilder() = default;
    virtual void buildFoundation() = 0;
    virtual void buildStructure() = 0;
    virtual void buildRoof() = 0;
    virtual void paintHouse() = 0;
    virtual void furnishHouse() = 0;
    void getHouse(std::shared_ptr<House>& house);

   protected:
    std::shared_ptr<House> house;
};

class ConcreteHouseBuilder : public HouseBuilder {
   public:
    void buildFoundation() override;
    void buildStructure() override;
    void buildRoof() override;
    void paintHouse() override;
    void furnishHouse() override;
};

class PrefabricatedHouseBuilder : public HouseBuilder {
   public:
    void buildFoundation() override;
    void buildStructure() override;
    void buildRoof() override;
    void paintHouse() override;
    void furnishHouse() override;
};

class ConstructionEngineer {
   public:
    enum Type {
        Concrete,
        Prefabricated,
    };
    ConstructionEngineer(Type t);
    void constructHouse();
    void getHouse(std::shared_ptr<House>& house);

   private:
    std::shared_ptr<HouseBuilder> builder;
    Type type{Concrete};
};