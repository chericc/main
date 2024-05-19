#include "matrix_order.hpp"

MatrixOrder::MatrixOrder()
{
    
}

MatrixOrder::MatrixOrder(int w, int h)
{
    w_ = w;
    h_ = h;
}

MatrixOrder::MatrixOrder(const MatrixOrder &order)
{
    *this = order;
}

int MatrixOrder::Width() const
{
    return w_;
}

int MatrixOrder::Height() const
{
    return h_;
}

int MatrixOrder::Set(int w, int h)
{
    *this = MatrixOrder(w, h);
    return 0;
}

int MatrixOrder::Size() const
{
    int ret = w_ * h_;
    return ret;
}

bool MatrixOrder::Joinable(const MatrixOrder &order) const
{
    bool ret = (w_ == order.h_);
    return ret;
}

MatrixOrder MatrixOrder::MakeJoined(const MatrixOrder &order) const
{
    MatrixOrder order_tmp(order.w_, h_);
    return order_tmp;
}

MatrixOrder MatrixOrder::MakeTranspose() const
{
    MatrixOrder order(h_, w_);
    return order;
}

bool MatrixOrder::operator==(const MatrixOrder &order) const
{
    bool ret = ((w_ == order.w_) && (h_ == order.h_));
    return ret;
}

bool MatrixOrder::operator!=(const MatrixOrder &order) const
{
    bool ret = !(*this == order);
    return ret;
}

MatrixOrder &MatrixOrder::operator=(const MatrixOrder &order)
{
    w_ = order.w_;
    h_ = order.h_;
    return *this;
}