#include "decoration_design.hpp"

#include <stdio.h>

#define xdebug(x...) do {printf("[debug][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)

/**************** main ****************/

void DecorationDesign::Decorate(Style style)
{
    typedef std::shared_ptr<ToolsFactoryInterface> ToolsPtr;

    ToolsPtr ptr_tools;

    switch (style)
    {
        case Style::Modern:
        {
            ptr_tools = std::make_shared<ToolsFactoryModern>();
            break;
        }
        case Style::Classical:
        {
            ptr_tools = std::make_shared<ToolsFactoryClassical>();
            break;
        }
        default:
        {
            xdebug ("Not support\n");
        }
    }

    ptr_tools->CreateColoring()->Color();
    ptr_tools->CreateFloor()->GenerateFloor();
    ptr_tools->CreateFurniture()->GenerateFurniture();

    return ; 
}

/**************** factory ****************/

ColoringPtr ToolsFactoryModern::CreateColoring()
{
    return std::make_shared<ColoringModern>();
}

FurniturePtr ToolsFactoryModern::CreateFurniture()
{
    return std::make_shared<FurnitureModern>();
}

FloorPtr ToolsFactoryModern::CreateFloor()
{
    return std::make_shared<FloorModern>();
}

ColoringPtr ToolsFactoryClassical::CreateColoring()
{
    return std::make_shared<ColoringClassical>();
}

FurniturePtr ToolsFactoryClassical::CreateFurniture()
{
    return std::make_shared<FurnitureClassical>();
}

FloorPtr ToolsFactoryClassical::CreateFloor()
{
    return std::make_shared<FloorClassical>();
}

/**************** coloring ****************/

void ColoringClassical::Color()
{
    xdebug ("Colored with classical style\n");
}

void ColoringModern::Color()
{
    xdebug ("Colored with modern style\n");
}

/**************** furniture ****************/

void FurnitureClassical::GenerateFurniture()
{
    xdebug ("Classical furniture is generated\n");
}

void FurnitureModern::GenerateFurniture()
{
    xdebug ("Modern furniture is generated\n");
}

/**************** floor ****************/

void FloorClassical::GenerateFloor()
{
    xdebug ("Classical floor is generated\n");
}

void FloorModern::GenerateFloor()
{
    xdebug ("Modern floor is generated\n");
}