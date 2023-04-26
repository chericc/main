#pragma once

#include <memory>

class ToolsFactoryInterface;
class ToolsFactoryModern;
class ToolsFactoryClassical;

class ColoringInterface;
class ColoringClassical;
class ColoringModern;

class FurnitureInterface;
class FurnitureClassical;
class FurnitureModern;

class FloorInterface;
class FloorClassical;
class FloorModern;

/**************** main ****************/

class DecorationDesign
{
public:
    enum class Style
    {
        Modern,
        Classical,
    };

    void Decorate(Style style);
};

/**************** factory ****************/

typedef std::shared_ptr<ColoringInterface> ColoringPtr;
typedef std::shared_ptr<FurnitureInterface> FurniturePtr;
typedef std::shared_ptr<FloorInterface> FloorPtr;

class ToolsFactoryInterface
{
public:
    virtual ~ToolsFactoryInterface() = default;
    virtual ColoringPtr CreateColoring() = 0;
    virtual FurniturePtr CreateFurniture() = 0;
    virtual FloorPtr CreateFloor() = 0;
};

class ToolsFactoryModern : public ToolsFactoryInterface
{
public:
    ColoringPtr CreateColoring() override;
    FurniturePtr CreateFurniture() override;
    FloorPtr CreateFloor() override;
};

class ToolsFactoryClassical : public ToolsFactoryInterface
{
public:
    ColoringPtr CreateColoring() override;
    FurniturePtr CreateFurniture() override;
    FloorPtr CreateFloor() override;
};

/**************** coloring ****************/

class ColoringInterface
{
public:
    virtual ~ColoringInterface() = default;
    virtual void Color() = 0;
};

class ColoringClassical : public ColoringInterface
{
public:
    void Color() override;
};

class ColoringModern : public ColoringInterface
{
public:
    void Color() override;
};

/**************** furniture ****************/

class FurnitureInterface
{
public:
    virtual ~FurnitureInterface() = default;
    virtual void GenerateFurniture() = 0;
};

class FurnitureClassical : public FurnitureInterface
{
public:
    void GenerateFurniture() override;
};

class FurnitureModern : public FurnitureInterface
{
public:
    void GenerateFurniture() override;
};

/**************** floor ****************/

class FloorInterface
{
public:
    virtual ~FloorInterface() = default;
    virtual void GenerateFloor() = 0;
};

class FloorClassical : public FloorInterface
{
public:
    void GenerateFloor() override;
};

class FloorModern : public FloorInterface
{
public:
    void GenerateFloor() override;
};