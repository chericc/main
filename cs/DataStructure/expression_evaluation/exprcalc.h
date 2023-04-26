/* 表达式计算 */

#pragma once

#include <list>

#include "public.h"
#include "expritem.h"

/* 表达式计算 */
class ExprCalc 
{
public:
    ELETYPE calc(const std::list<ExprItem> &items);
private:
    int infix2sufix(const std::list<ExprItem> &infixitems, 
            std::list<ExprItem> &sufixitems);
    int priority(const ExprItem &itema, const ExprItem &itemb);
    int id(const ExprItem &item);
    ELETYPE calcsufix(const std::list<ExprItem> &items);
};