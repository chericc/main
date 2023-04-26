

#pragma once

#include <string>
#include <unordered_map>
#include <ostream>
#include <list>

#include "public.h"

/* 表达式基本 */
class ExprItem
{
public:
    enum class ItemType
    {
        OP, /* 运算 */
        BR, /* 括号 */
        NU  /* 数 */
    };
    enum class OpType
    {
        PLUS,   /* + */
        MINUS,  /* - */
        TIMES,  /* * */
        DIV,    /* / */
    };
    enum class BrType
    {
        LeftBr,     /* 左括号 */
        RightBr,    /* 右括号 */
    };
    union Data
    {
        OpType eOpType;
        BrType eBrType;
        ELETYPE num;
    };
    
    ItemType eType;
    Data data;

    ExprItem();
    ExprItem(const std::string &strItem);
    friend std::ostream & operator<<(std::ostream &os, const ExprItem &item);
    friend ExprItem operator+(const ExprItem &arg1, const ExprItem &arg2);
    friend ExprItem operator-(const ExprItem &arg1, const ExprItem &arg2);
    friend ExprItem operator/(const ExprItem &arg1, const ExprItem &arg2);
    friend ExprItem operator*(const ExprItem &arg1, const ExprItem &arg2);

    static std::unordered_map<std::string, ExprItem> smap;
    static void initsmap();
    static std::list<ExprItem> parse(const std::string &expr);
};
