#include "text_formatter.hpp"

std::string NewLineFormatter::formatText(std::string const& text) {
    std::string text_copy = text;
    for (auto& it : text_copy) {
        if (it == '.') {
            it = '\n';
        }
    }
    return text_copy;
}

std::string CsvFormatterImp::formatText(std::string const& text) {
    std::string text_copy = text;
    for (auto& it : text_copy) {
        if (it == '.') {
            it = ',';
        }
    }
    return text_copy;
}

std::string CsvFormatterAdap::formatText(std::string const& text) {
    if (!csv_formatter) {
        csv_formatter = std::make_shared<CsvFormatterImp>();
    }

    return csv_formatter->formatText(text);
}