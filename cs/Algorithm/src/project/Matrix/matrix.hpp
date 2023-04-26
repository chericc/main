#pragma once

#include <initializer_list>
#include <ostream>
#include <vector>

#include "matrix_order.hpp"

typedef int MatrixDataType;

class Matrix
{
public:
    Matrix();
    Matrix(const MatrixOrder &order);
    Matrix(const MatrixOrder &order, const std::initializer_list<MatrixDataType> &init_list);
    Matrix(const MatrixOrder &order, const std::vector<MatrixDataType> &data);

    Matrix(const Matrix &matrix);

    const MatrixOrder &Order() const;

    const MatrixDataType &Element(int x, int y) const;
    int SetElement(int x, int y, const MatrixDataType &value);

    Matrix MakeTranspose() const;

    Matrix &operator=(const Matrix &Matrix);

    bool operator==(const Matrix &matrix) const;
    bool operator!=(const Matrix &matrix) const;

    Matrix operator-() const;
    Matrix operator+(const Matrix &matrix) const;
    Matrix operator-(const Matrix &matrix) const;
    Matrix operator*(const Matrix &matrix) const;

    Matrix operator*(int k) const;

    friend std::ostream &operator<<(std::ostream &ostream, const Matrix &matrix);
private:
    MatrixDataType &ElementRef(int x, int y);
    const MatrixDataType &ElementCRef(int x, int y) const;
    int Index(int x, int y) const;
private:
    MatrixOrder order_;
    std::vector<MatrixDataType> data_;
};