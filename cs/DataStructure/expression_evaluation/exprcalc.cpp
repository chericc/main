
#include "exprcalc.h"

#include <stack>
#include <iostream>
#include <assert.h>

ELETYPE ExprCalc::calc(const std::list<ExprItem> &items) {
    std::list<ExprItem> listSufixItems;
    infix2sufix(items, listSufixItems);

    for (auto it = listSufixItems.cbegin(); it != listSufixItems.cend(); ++it) {
        std::cout << *it;
    }

    std::cout << std::endl;

    return calcsufix(listSufixItems);
}

/**
 * @brief 中缀表达式转换为后缀表达式 
 * 算法描述：
 * 依次扫描中缀表达式，对于遇到的每个元素：
 * 1.如果是操作数，直接输出。
 * 2.如果是左括号，直接压栈；如果是右括号，则出栈并输出，直到遇到左括号，左括号不输出。
 * 3.如果是操作符，如果栈空，则压栈，否则和栈顶元素比较优先级：
 * 3.1 如果优先级高，则压栈。
 * 3.2 如果优先级相等或低，则弹出并输出，直到优先级高或者栈空，最后压栈。
 * 4.如果元素扫描结束，则出栈并输出至栈空。
 */
int ExprCalc::infix2sufix(
const std::list<ExprItem> &infixitems, std::list<ExprItem> &sufixitems) {
    std::stack<ExprItem> stackOp;
    std::list<ExprItem> listSufix;
    for (auto it = infixitems.cbegin(); it != infixitems.cend(); ++it) {
        switch (it->eType) {
        case ExprItem::ItemType::NU: {
            listSufix.push_back(*it);
            break;
        }
        case ExprItem::ItemType::BR: {
            switch (it->data.eBrType) {
            case ExprItem::BrType::LeftBr: {
                stackOp.push(*it);
                break;
            }
            case ExprItem::BrType::RightBr: {
                while (! stackOp.empty()) {
                    auto crefTop = stackOp.top();
                    if (!(crefTop.eType == ExprItem::ItemType::BR &&
                        crefTop.data.eBrType == ExprItem::BrType::LeftBr)) {
                        listSufix.push_back(crefTop);
                        stackOp.pop();
                    }
                    else {
                        stackOp.pop();
                        break;
                    }
                }
                break;
            }
            } // switch 
            break;
        }
        case ExprItem::ItemType::OP: {
            if (stackOp.empty()) {
                stackOp.push(*it);
            }
            else {
                auto crefTop = stackOp.top();
                if (priority(*it, crefTop) > 0) {
                    stackOp.push(*it);
                }
                else {
                    while (! stackOp.empty()) {
                        crefTop = stackOp.top();
                        if (priority(*it, crefTop) <= 0) {
                            listSufix.push_back(crefTop);
                            stackOp.pop();
                        }
                        else {
                            break;
                        }
                    }
                    stackOp.push(*it);
                }
            }
            break;
        }
        } // switch 
    }

    while (!stackOp.empty()) {
        auto crefTop = stackOp.top();
        listSufix.push_back(crefTop);
        stackOp.pop();
    }

    sufixitems = listSufix;
    return 0;
}

/* 表达式元素优先级，值越大，优先级越高 */
int ExprCalc::priority(const ExprItem &itema, const ExprItem &itemb) {
    /**
     * 优先级表：
     * 摘自《数据结构》·严蔚敏 P53
     * 
     *        + - * / ( ) #
     * 
     * -1 0 1 分别表示小于、等于、大于
     * 其它表示未定义
     */

    // static const int map[7][7] = {
    //     1, 1, -1, -1, -1, 1, 1,
    //     1, 1, -1, -1, -1, 1, 1,
    //     1, 1, 1, 1, -1, 1, 1,
    //     1, 1, 1, 1, -1, 1, 1,
    //     -1, -1, -1, -1, -1, 0, 9,
    //     1, 1, 1, 1, 9, 1, 1,
    //     -1, -1, -1, -1, -1, 9, 0,
    // };
    static const int map[7][7] = {
        0, 0, -1, -1, 1, -1, 9,
        0, 0, -1, -1, 1, -1, 9,
        1, 1, 0, 0, 1, -1, 9,
        1, 1, 0, 0, 1, -1, 9,
        -1, -1, -1, -1, 0, -1, 9,
        1, 1, 1, 1, 1, 0, 9,
        9, 9, 9, 9, 9, 9, 9,
    };
    int ida = id(itema);
    int idb = id(itemb);
    assert (ida >= 0 && ida <= 7);
    assert (idb >= 0 && idb <= 7);
    int pri = map[ida][idb];

    assert (pri == -1 || pri == 0 || pri == 1);
    return pri;
}

/* 返回索引，参照priority */
int ExprCalc::id(const ExprItem &item) {
    int id = -1;
    switch (item.eType) {
    case ExprItem::ItemType::BR: {
        switch (item.data.eBrType) {
        case ExprItem::BrType::LeftBr: {
            id = 4;
            break;
        }
        case ExprItem::BrType::RightBr: {
            id = 5;
            break;
        }
        } // switch 
        break;
    } 
    case ExprItem::ItemType::OP: {
        switch (item.data.eOpType) {
        case ExprItem::OpType::PLUS: {
            id = 0;
            break;
        }
        case ExprItem::OpType::MINUS: {
            id = 1;
            break;
        }
        case ExprItem::OpType::TIMES: {
            id = 2;
            break;
        }
        case ExprItem::OpType::DIV: {
            id = 3;
            break;
        }
        }
        break;
    }
    default: {
        assert (false);
    }
    } // switch 
    return id;
}

/* 计算后缀表达式 */
ELETYPE ExprCalc::calcsufix(const std::list<ExprItem> &items) {
    std::stack<ExprItem> stackops;
    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        switch (it->eType) {
        case ExprItem::ItemType::NU: {
            stackops.push(*it);
            break;
        }
        case ExprItem::ItemType::OP: {
            assert (stackops.size() >= 2);

            ExprItem arg2 = stackops.top();
            stackops.pop();
            ExprItem arg1 = stackops.top();
            stackops.pop();
            
            assert (arg1.eType == ExprItem::ItemType::NU);
            assert (arg2.eType == ExprItem::ItemType::NU);

            ExprItem result;

            switch (it->data.eOpType) {
            case ExprItem::OpType::PLUS: {
                result = arg1 + arg2;
                break;
            }
            case ExprItem::OpType::MINUS: {
                result = arg1 - arg2;
                break;
            }
            case ExprItem::OpType::TIMES: {
                result = arg1 * arg2;
                break;
            }
            case ExprItem::OpType::DIV: {
                result = arg1 / arg2;
                break;
            }
            }

            stackops.push(result);

            break;
        }
        default: {
            assert (false);
            break;
        }
        } // switch
    }

    assert (! stackops.empty());
    return stackops.top().data.num;
}