#include "matrix.hpp"

#include <assert.h>

#include "log.hpp"

Matrix::Matrix() {}

Matrix::Matrix(const MatrixOrder& order) {
    if (order.Width() > 0 && order.Height() > 0) {
        std::vector<MatrixDataType> data;
        data.resize(order.Size());
        *this = Matrix(order, data);
    } else {
        loge("invalid order(%d,%d)", order.Width(), order.Height());
        *this = Matrix();
    }
}

Matrix::Matrix(const MatrixOrder& order,
               const std::initializer_list<MatrixDataType>& init_list) {
    if (order.Width() > 0 && order.Height() > 0) {
        std::vector<MatrixDataType> data(init_list);
        data.resize(order.Size(), 0);
        *this = Matrix(order, data);
    } else {
        loge("invalid order(%d,%d)", order.Width(), order.Height());
        *this = Matrix();
    }
}

Matrix::Matrix(const MatrixOrder& order,
               const std::vector<MatrixDataType>& init_data) {
    if (order.Width() > 0 && order.Height() > 0) {
        data_ = init_data;
        data_.resize(order.Size(), 0);
        order_ = order;
    } else {
        loge("invalid order(%d,%d)", order.Width(), order.Height());
        *this = Matrix();
    }
}

Matrix::Matrix(const Matrix& matrix) {
    order_ = matrix.order_;
    data_ = matrix.data_;
}

const MatrixOrder& Matrix::Order() const { return order_; }

const MatrixDataType& Matrix::Element(int x, int y) const {
    int index = Index(x, y);

    if (x < 0 || y < 0 || index < 0 ||
        static_cast<std::size_t>(index) >= data_.size()) {
        loge("invalid param(%d,%d)", x, y);
    }

    return data_.at(index);
}

int Matrix::SetElement(int x, int y, const MatrixDataType& value) {
    int index = Index(x, y);

    if (x < 0 || y < 0 || index < 0 ||
        static_cast<std::size_t>(index) >= data_.size()) {
        loge("invalid param(%d,%d)", x, y);
        return -1;
    }

    data_[index] = value;
    return 0;
}

Matrix Matrix::MakeTranspose() const {
    Matrix matrix_tmp(order_.MakeTranspose());
    int w = matrix_tmp.Order().Width();
    int h = matrix_tmp.Order().Height();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            matrix_tmp.ElementRef(x, y) = ElementCRef(y, x);
        }
    }
    return matrix_tmp;
}

Matrix& Matrix::operator=(const Matrix& matrix) {
    data_ = matrix.data_;
    order_ = matrix.order_;
    return *this;
}

bool Matrix::operator==(const Matrix& matrix) const {
    return (order_ == matrix.order_) && (data_ == matrix.data_);
}

bool Matrix::operator!=(const Matrix& matrix) const {
    return !(*this == matrix);
}

Matrix Matrix::operator-() const {
    Matrix matrix_tmp(order_);
    for (int y = 0; y < order_.Height(); ++y) {
        for (int x = 0; x < order_.Width(); ++x) {
            matrix_tmp.ElementRef(x, y) = 0 - ElementCRef(x, y);
        }
    }
    return matrix_tmp;
}

Matrix Matrix::operator+(const Matrix& matrix) const {
    if (order_ != matrix.order_) {
        loge("order not equal");
        return Matrix{};
    }

    Matrix matrix_tmp(order_);
    for (int y = 0; y < order_.Height(); ++y) {
        for (int x = 0; x < order_.Width(); ++x) {
            matrix_tmp.ElementRef(x, y) =
                ElementCRef(x, y) + matrix.ElementCRef(x, y);
        }
    }
    return matrix_tmp;
}

Matrix Matrix::operator-(const Matrix& matrix) const {
    if (order_ != matrix.order_) {
        loge("order not equal");
        return Matrix{};
    }

    Matrix matrix_tmp(order_);
    for (int y = 0; y < order_.Height(); ++y) {
        for (int x = 0; x < order_.Width(); ++x) {
            matrix_tmp.ElementRef(x, y) =
                ElementCRef(x, y) - matrix.ElementCRef(x, y);
        }
    }
    return matrix_tmp;
}

Matrix Matrix::operator*(const Matrix& matrix) const {
    if (!order_.Joinable(matrix.order_)) {
        loge("order not joinable");
        return Matrix{};
    }

    const MatrixOrder order_tmp = order_.MakeJoined(matrix.order_);

    const int w = order_tmp.Width();
    const int h = order_tmp.Height();
    const int w_original = order_.Width();

    Matrix matrix_tmp(order_tmp);
    MatrixDataType sum = 0;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            sum = 0;
            for (int k = 0; k < w_original; ++k) {
                sum += ElementCRef(k, y) * matrix.ElementCRef(x, k);
            }
            matrix_tmp.ElementRef(x, y) = sum;
        }
    }
    return matrix_tmp;
}

Matrix Matrix::operator*(int k) const {
    Matrix matrix_tmp(*this);
    int w = matrix_tmp.Order().Width();
    int h = matrix_tmp.Order().Height();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            matrix_tmp.ElementRef(x, y) = matrix_tmp.ElementCRef(x, y) * k;
        }
    }

    return matrix_tmp;
}

std::ostream& operator<<(std::ostream& ostream, const Matrix& matrix) {
    const int w = matrix.order_.Width();
    const int h = matrix.order_.Height();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            ostream << matrix.ElementCRef(x, y);
            if (x < w - 1) {
                ostream << " ";
            }
        }
        if (y < h - 1) {
            ostream << std::endl;
        }
    }

    return ostream;
}

MatrixDataType& Matrix::ElementRef(int x, int y) { return data_[Index(x, y)]; }

const MatrixDataType& Matrix::ElementCRef(int x, int y) const {
    return data_[Index(x, y)];
}

int Matrix::Index(int x, int y) const { return order_.Width() * y + x; }