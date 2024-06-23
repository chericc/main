#pragma once

#include <memory>
#include <string>

class TextFormatter {
   public:
    virtual ~TextFormatter() = default;
    virtual std::string formatText(std::string const& text) = 0;
};

class NewLineFormatter : public TextFormatter {
   public:
    std::string formatText(std::string const& text) override;
};

class CsvFormatter {
   public:
    virtual ~CsvFormatter() = default;
    virtual std::string formatText(std::string const& text) = 0;
};

class CsvFormatterImp : public CsvFormatter {
   public:
    std::string formatText(std::string const& text) override;
};

class CsvFormatterAdap : public TextFormatter {
   public:
    std::string formatText(std::string const& text) override;

   private:
    std::shared_ptr<CsvFormatter> csv_formatter;
};