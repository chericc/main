
#include "expritem.h"

#include <assert.h>

std::unordered_map<std::string, ExprItem> ExprItem::smap;

void ExprItem::initsmap() {
    ExprItem plus;
    ExprItem minus;
    ExprItem times;
    ExprItem div;
    ExprItem lefrbr;
    ExprItem rightbr;
    ExprItem num;

    plus.eType = ItemType::OP;
    minus.eType = ItemType::OP;
    times.eType = ItemType::OP;
    div.eType = ItemType::OP;
    lefrbr.eType = ItemType::BR;
    rightbr.eType = ItemType::BR;
    num.eType = ItemType::NU;

    plus.data.eOpType = OpType::PLUS;
    minus.data.eOpType = OpType::MINUS;
    times.data.eOpType = OpType::TIMES;
    div.data.eOpType = OpType::DIV;

    lefrbr.data.eBrType = BrType::LeftBr;
    rightbr.data.eBrType = BrType::RightBr;

    smap["+"] = plus;
    smap["-"] = minus;
    smap["*"] = times;
    smap["/"] = div;
    smap["("] = lefrbr;
    smap[")"] = rightbr;
}

ExprItem::ExprItem() {
    eType = ItemType::NU;
    data.num = 0.0;
}

ExprItem::ExprItem(const std::string& strItem) {
    if (smap.empty()) {
        initsmap();
    }

    if (smap.find(strItem) != smap.end()) {
        *this = smap.at(strItem);  // default operator =
    } else {
        double dnum = std::stod(strItem);
        eType = ItemType::NU;
        data.num = dnum;
    }
}

std::ostream& operator<<(std::ostream& os, const ExprItem& item) {
    os << "[";
    switch (item.eType) {
        case ExprItem::ItemType::BR: {
            switch (item.data.eBrType) {
                case ExprItem::BrType::LeftBr: {
                    os << "(";
                    break;
                }
                case ExprItem::BrType::RightBr: {
                    os << ")";
                    break;
                }
            }
            break;
        }
        case ExprItem::ItemType::OP: {
            switch (item.data.eOpType) {
                case ExprItem::OpType::PLUS: {
                    os << "+";
                    break;
                }
                case ExprItem::OpType::MINUS: {
                    os << "-";
                    break;
                }
                case ExprItem::OpType::TIMES: {
                    os << "*";
                    break;
                }
                case ExprItem::OpType::DIV: {
                    os << "/";
                    break;
                }
            }
            break;
        }
        case ExprItem::ItemType::NU: {
            os << item.data.num;
            break;
        }
    }
    os << "]";
    return os;
}

ExprItem operator+(const ExprItem& arg1, const ExprItem& arg2) {
    assert(arg1.eType == ExprItem::ItemType::NU);
    assert(arg2.eType == ExprItem::ItemType::NU);
    ExprItem item;
    item.eType = ExprItem::ItemType::NU;
    item.data.num = arg1.data.num + arg2.data.num;
    return item;
}

ExprItem operator-(const ExprItem& arg1, const ExprItem& arg2) {
    assert(arg1.eType == ExprItem::ItemType::NU);
    assert(arg2.eType == ExprItem::ItemType::NU);
    ExprItem item;
    item.eType = ExprItem::ItemType::NU;
    item.data.num = arg1.data.num - arg2.data.num;
    return item;
}

ExprItem operator*(const ExprItem& arg1, const ExprItem& arg2) {
    assert(arg1.eType == ExprItem::ItemType::NU);
    assert(arg2.eType == ExprItem::ItemType::NU);
    ExprItem item;
    item.eType = ExprItem::ItemType::NU;
    item.data.num = arg1.data.num * arg2.data.num;
    return item;
}

ExprItem operator/(const ExprItem& arg1, const ExprItem& arg2) {
    assert(arg1.eType == ExprItem::ItemType::NU);
    assert(arg2.eType == ExprItem::ItemType::NU);
    ExprItem item;
    item.eType = ExprItem::ItemType::NU;
    item.data.num = arg1.data.num / arg2.data.num;
    return item;
}

std::list<ExprItem> ExprItem::parse(const std::string& expr) {
    std::list<ExprItem> explist;

    auto itSeeker = expr.cbegin();
    auto itPos = expr.cbegin();

    while (itPos != expr.cend()) {
        if (std::isdigit(*itPos)) {
            itSeeker = itPos;
            while (itSeeker != expr.cend() && std::isdigit(*itSeeker)) {
                ++itSeeker;
            }

            std::string str(itPos, itSeeker);
            ExprItem item(str);
            explist.push_back(item);

            itPos = itSeeker;
        } else {
            std::string str(itPos, itPos + 1);
            ExprItem item(str);
            explist.push_back(item);

            ++itPos;
        }
    }

    return explist;
}