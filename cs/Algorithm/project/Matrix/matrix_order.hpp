#pragma once

/* 矩阵的阶 */
class MatrixOrder {
   public:
    MatrixOrder();
    MatrixOrder(int w, int h);
    MatrixOrder(const MatrixOrder& order);

    int Width() const;
    int Height() const;
    int Set(int w, int h);
    int Size() const;

    bool Joinable(const MatrixOrder& order) const;
    MatrixOrder MakeJoined(const MatrixOrder& order) const;
    MatrixOrder MakeTranspose() const;

    bool operator==(const MatrixOrder& order) const;
    bool operator!=(const MatrixOrder& order) const;
    MatrixOrder& operator=(const MatrixOrder& order);

   private:
    int w_{0};
    int h_{0};
};