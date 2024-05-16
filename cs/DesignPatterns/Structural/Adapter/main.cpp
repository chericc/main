#include <memory>

#include "xlog.hpp"
#include "text_formatter.hpp"

int main()
{
    std::string text = "John.5th year.Boy.156cm.";
    
    auto tf1 = std::make_shared<NewLineFormatter>();
    auto tf2 = std::make_shared<CsvFormatterAdap>();
    
    xlog_dbg("input: %s", text.c_str());
    xlog_dbg("tf1.format: %s", tf1->formatText(text).c_str());
    xlog_dbg("tf2.format: %s", tf2->formatText(text).c_str());

    return 0;
}