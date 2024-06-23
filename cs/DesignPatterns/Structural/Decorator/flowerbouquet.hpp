#pragma once

#include <memory>
#include <string>

class FlowerBouquet {
   public:
    std::string getDescription();
    virtual double cost() = 0;

   private:
    std::string description_;
};

class RoseBouquet : public FlowerBouquet {
   public:
    RoseBouquet();
    double cost() override;
};

class OrchidBouquet : public FlowerBouquet {
   public:
    OrchidBouquet();
    double cost() override;
};

class FlowerBouquetDecorator : public FlowerBouquet {};

class Glitter : public FlowerBouquetDecorator {
   public:
   private:
    std::shared_ptr<FlowerBouquet> flowerbouquet_;
};

class PaperWrapper : public FlowerBouquetDecorator {};

class RibbonBow : public FlowerBouquetDecorator {};
