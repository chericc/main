#include "decoration_design.hpp"

#include "xlog.hpp"

/**************** main ****************/

void DecorationDesign::Decorate(Style style) {
    typedef std::shared_ptr<ToolsFactoryInterface> ToolsPtr;

    ToolsPtr ptr_tools;

    switch (style) {
        case Style::Modern: {
            ptr_tools = std::make_shared<ToolsFactoryModern>();
            break;
        }
        case Style::Classical: {
            ptr_tools = std::make_shared<ToolsFactoryClassical>();
            break;
        }
        default: {
            xlog_dbg("Not support");
        }
    }

    ptr_tools->CreateColoring()->Color();
    ptr_tools->CreateFloor()->GenerateFloor();
    ptr_tools->CreateFurniture()->GenerateFurniture();

    return;
}

/**************** factory ****************/

ColoringPtr ToolsFactoryModern::CreateColoring() {
    return std::make_shared<ColoringModern>();
}

FurniturePtr ToolsFactoryModern::CreateFurniture() {
    return std::make_shared<FurnitureModern>();
}

FloorPtr ToolsFactoryModern::CreateFloor() {
    return std::make_shared<FloorModern>();
}

ColoringPtr ToolsFactoryClassical::CreateColoring() {
    return std::make_shared<ColoringClassical>();
}

FurniturePtr ToolsFactoryClassical::CreateFurniture() {
    return std::make_shared<FurnitureClassical>();
}

FloorPtr ToolsFactoryClassical::CreateFloor() {
    return std::make_shared<FloorClassical>();
}

/**************** coloring ****************/

void ColoringClassical::Color() { xlog_dbg("Colored with classical style"); }

void ColoringModern::Color() { xlog_dbg("Colored with modern style"); }

/**************** furniture ****************/

void FurnitureClassical::GenerateFurniture() {
    xlog_dbg("Classical furniture is generated");
}

void FurnitureModern::GenerateFurniture() {
    xlog_dbg("Modern furniture is generated");
}

/**************** floor ****************/

void FloorClassical::GenerateFloor() {
    xlog_dbg("Classical floor is generated");
}

void FloorModern::GenerateFloor() { xlog_dbg("Modern floor is generated"); }