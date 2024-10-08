/*
* Copyright 2014 Google Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "mathfu/matrix.h"

#include "mathfu/io.h"
#include "mathfu/quaternion.h"
#include "mathfu/utilities.h"
#include "mathfu/vector.h"

#include <cmath>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "precision.h"
#include "mathfu/matrix_utils.h"

static const float kUnProjectFloatPrecision = 0.0012f;
static const double kLookAtDoublePrecision = 1e-8;
class MatrixTests : public ::testing::Test {
 protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

template <class T, int rows, int columns>
struct MatrixExpectation {
  const char* description;
  mathfu::Matrix<T, rows, columns> calculated; // 通过计算得出的矩阵
  mathfu::Matrix<T, rows, columns> expected; // 给定的答案预期，目的是为了对比计算的矩阵对不对
};

// This will automatically generate tests for each template parameter.
#define TEST_ALL_F(MY_TEST, FLOAT_PRECISION_VALUE, DOUBLE_PRECISION_VALUE) \
  TEST_F(MatrixTests, MY_TEST##_float_2) {                                 \
    MY_TEST##_Test<float, 2>(FLOAT_PRECISION_VALUE);                       \
  }                                                                        \
  TEST_F(MatrixTests, MY_TEST##_double_2) {                                \
    MY_TEST##_Test<double, 2>(DOUBLE_PRECISION_VALUE);                     \
  }                                                                        \
  TEST_F(MatrixTests, MY_TEST##_float_3) {                                 \
    MY_TEST##_Test<float, 3>(FLOAT_PRECISION_VALUE);                       \
  }                                                                        \
  TEST_F(MatrixTests, MY_TEST##_double_3) {                                \
    MY_TEST##_Test<double, 3>(DOUBLE_PRECISION_VALUE);                     \
  }                                                                        \
  TEST_F(MatrixTests, MY_TEST##_float_4) {                                 \
    MY_TEST##_Test<float, 4>(FLOAT_PRECISION_VALUE);                       \
  }                                                                        \
  TEST_F(MatrixTests, MY_TEST##_double_4) {                                \
    MY_TEST##_Test<double, 4>(DOUBLE_PRECISION_VALUE);                     \
  }

// This will automatically generate tests for each scalar template parameter.
#define TEST_SCALAR_F(MY_TEST, FLOAT_PRECISION_VALUE, DOUBLE_PRECISION_VALUE) \
  TEST_F(MatrixTests, MY_TEST##_float) {                                      \
    MY_TEST##_Test<float>(FLOAT_PRECISION_VALUE);                             \
  }                                                                           \
  TEST_F(MatrixTests, MY_TEST##_double) {                                     \
    MY_TEST##_Test<double>(DOUBLE_PRECISION_VALUE);                           \
  }

// This will test initialization by passing in values. The template paramter d
// corresponds to the number of rows and columns.
template <class T, int d>
void Initialize_Test(const T& precision) {
  // This will test initialization of the matrix using a random single value.
  // The expected result is that all entries equal the given value.
  // 这将使用随机单值测试矩阵的初始化
  // 预期结果是所有条目等于给定值
  // api
  // 1. matrix_splat(3.1) 全部值都是 3.1
  // 2. matrix_splat[i]
  // 3. matrix_splat(i, j)
  mathfu::Matrix<T, d> matrix_splat(static_cast<T>(3.1));
  for (int i = 0; i < d * d; ++i) {
    EXPECT_NEAR(3.1, matrix_splat[i], precision);
  }
  // This will verify that the value is correct when using the (i, j) form
  // of indexing.
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(3.1, matrix_splat(i, j), precision);
    }
  }
  // This will test initialization of the matrix using a c style array of
  // values.
  T x[d * d];
  for (int i = 0; i < d * d; ++i) {
    x[i] = rand() / static_cast<T>(RAND_MAX) * 100.f;
  }
  mathfu::Matrix<T, d> matrix_arr(x);
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(x[i + d * j], matrix_arr(i, j), precision);
    }
  }

  // This will test the copy constructor making sure that the new matrix
  // equals the old one.
  // api
  // m(m1)
  // 测试拷贝函数
  mathfu::Matrix<T, d> matrix_copy(matrix_arr);
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(x[i + d * j], matrix_copy(i, j), precision);
    }
  }

  // This will verify that the copy was deep and changing the values of the
  // copied matrix does not effect the original.
  matrix_copy = matrix_copy - mathfu::Matrix<T, d>(1);
  EXPECT_NE(matrix_copy(0, 0), matrix_arr(0, 0));

  // This will test creation of the identity matrix.
  // api
  // mathfu::Matrix<T, d>::Identity()
  mathfu::Matrix<T, d> identity(mathfu::Matrix<T, d>::Identity());
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(i == j ? 1 : 0, identity(i, j), precision);
    }
  }
}
TEST_ALL_F(Initialize, FLOAT_PRECISION, DOUBLE_PRECISION)

// This will test initialization by specifying all values explicitly.
template <class T>
void InitializePerDimension_Test(const T& precision) {
  // api
  // m(1,2,3,4)
  mathfu::Matrix<T, 2> matrix_f2x2(static_cast<T>(4.5), static_cast<T>(3.4),
                                   static_cast<T>(2.6), static_cast<T>(9.8));
  EXPECT_NEAR(4.5, matrix_f2x2(0, 0), precision);
  EXPECT_NEAR(3.4, matrix_f2x2(1, 0), precision);
  EXPECT_NEAR(2.6, matrix_f2x2(0, 1), precision);
  EXPECT_NEAR(9.8, matrix_f2x2(1, 1), precision);
  mathfu::Matrix<T, 3> matrix_f3x3(
      static_cast<T>(3.7), static_cast<T>(2.4), static_cast<T>(6.4),
      static_cast<T>(1.1), static_cast<T>(5.2), static_cast<T>(6.4),
      static_cast<T>(2.7), static_cast<T>(7.4), static_cast<T>(0.1));
  EXPECT_NEAR(3.7, matrix_f3x3(0, 0), precision);
  EXPECT_NEAR(2.4, matrix_f3x3(1, 0), precision);
  EXPECT_NEAR(6.4, matrix_f3x3(2, 0), precision);
  EXPECT_NEAR(1.1, matrix_f3x3(0, 1), precision);
  EXPECT_NEAR(5.2, matrix_f3x3(1, 1), precision);
  EXPECT_NEAR(6.4, matrix_f3x3(2, 1), precision);
  EXPECT_NEAR(2.7, matrix_f3x3(0, 2), precision);
  EXPECT_NEAR(7.4, matrix_f3x3(1, 2), precision);
  EXPECT_NEAR(0.1, matrix_f3x3(2, 2), precision);
  mathfu::Matrix<T, 4> matrix_f4x4(
      static_cast<T>(4.1), static_cast<T>(8.4), static_cast<T>(7.2),
      static_cast<T>(4.8), static_cast<T>(0.9), static_cast<T>(7.8),
      static_cast<T>(5.6), static_cast<T>(8.7), static_cast<T>(2.3),
      static_cast<T>(4.2), static_cast<T>(6.1), static_cast<T>(2.7),
      static_cast<T>(0.1), static_cast<T>(1.4), static_cast<T>(9.4),
      static_cast<T>(3.6));
  EXPECT_NEAR(4.1, matrix_f4x4(0, 0), precision);
  EXPECT_NEAR(8.4, matrix_f4x4(1, 0), precision);
  EXPECT_NEAR(7.2, matrix_f4x4(2, 0), precision);
  EXPECT_NEAR(4.8, matrix_f4x4(3, 0), precision);
  EXPECT_NEAR(0.9, matrix_f4x4(0, 1), precision);
  EXPECT_NEAR(7.8, matrix_f4x4(1, 1), precision);
  EXPECT_NEAR(5.6, matrix_f4x4(2, 1), precision);
  EXPECT_NEAR(8.7, matrix_f4x4(3, 1), precision);
  EXPECT_NEAR(2.3, matrix_f4x4(0, 2), precision);
  EXPECT_NEAR(4.2, matrix_f4x4(1, 2), precision);
  EXPECT_NEAR(6.1, matrix_f4x4(2, 2), precision);
  EXPECT_NEAR(2.7, matrix_f4x4(3, 2), precision);
  EXPECT_NEAR(0.1, matrix_f4x4(0, 3), precision);
  EXPECT_NEAR(1.4, matrix_f4x4(1, 3), precision);
  EXPECT_NEAR(9.4, matrix_f4x4(2, 3), precision);
  EXPECT_NEAR(3.6, matrix_f4x4(3, 3), precision);
}
TEST_SCALAR_F(InitializePerDimension, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test initialization of a matrix from an array of packed vectors.
template <class T, int d>
void InitializePacked_Test(const T& precision) {
  (void)precision;
  mathfu::VectorPacked<T, d> packed[d];
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      packed[i].data_[j] = static_cast<T>((i * d) + j);
    }
  }
  mathfu::Matrix<T, d> matrix(packed);
  for (int i = 0; i < d * d; ++i) {
    EXPECT_NEAR(packed[i / d].data_[i % d], matrix[i], static_cast<T>(0))
        << "Element " << i;
  }
}
TEST_ALL_F(InitializePacked, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test serialization to a packed array of vectors.
template <class T, int d>
void PackedSerialization_Test(const T& precision) {
  (void)precision;
  mathfu::Matrix<T, d> matrix;
  for (int i = 0; i < d * d; ++i) {
    matrix[i] = static_cast<T>(i);
  }
  mathfu::VectorPacked<T, d> packed[d];
  matrix.Pack(packed);
  for (int i = 0; i < d * d; ++i) {
    EXPECT_NEAR(matrix[i], packed[i / d].data_[i % d], static_cast<T>(0))
        << "Element " << i;
  }
}
TEST_ALL_F(PackedSerialization, FLOAT_PRECISION, DOUBLE_PRECISION)

// This will test the Addition and Subtraction of matrices. The template
// parameter d corresponds to the number of rows and columns.
template <class T, int d>
void AddSub_Test(const T& precision) {
  T x1[d * d], x2[d * d];
  for (int i = 0; i < d * d; ++i) {
    x1[i] = rand() / static_cast<T>(RAND_MAX) * 100.f;
  }
  for (int i = 0; i < d * d; ++i) {
    x2[i] = rand() / static_cast<T>(RAND_MAX) * 100.f;
  }
  // api
  // matrix(float[4])
  mathfu::Matrix<T, d> matrix1(x1), matrix2(x2);
  // This will test the negation of a matrix and verify that each element
  // is negated.
  // api
  // matrix(-matrix1)
  mathfu::Matrix<T, d> neg_mat1(- matrix1);
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(-x1[i + d * j], neg_mat1(i, j), precision);
    }
  }
  // This will test the addition of two matrices and verify that each element
  // equal to the sum of the input values.
  // api
  // matrix(matrix1 + matrix2)
  mathfu::Matrix<T, d> matrix_add(matrix1 + matrix2);
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(x1[i + d * j] + x2[i + d * j], matrix_add(i, j), precision);
    }
  }
  // This will test the subtraction of two matrices and verify that each
  // element equal to the difference of the input values.
  // api
  // matrix(matrix1 - matrix2)
  mathfu::Matrix<T, d> matrix_sub(matrix1 - matrix2);
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(x1[i + d * j] - x2[i + d * j], matrix_sub(i, j), precision);
    }
  }
}
TEST_ALL_F(AddSub, FLOAT_PRECISION, DOUBLE_PRECISION)

// This will test the multiplication of matrices by matrices, vectors,
// and scalars. The template parameter d corresponds to the number of rows and
// columns.
template <class T, int d>
void Mult_Test(const T& precision) {
  T x1[d * d], x2[d * d];
  for (int i = 0; i < d * d; ++i) x1[i] = rand() / static_cast<T>(RAND_MAX);
  for (int i = 0; i < d * d; ++i) x2[i] = rand() / static_cast<T>(RAND_MAX);
  mathfu::Matrix<T, d> matrix1(x1), matrix2(x2);
  // This will test scalar matrix multiplication and verify that each element
  // is equal to multiplication by the scalar.
  mathfu::Matrix<T, d> matrix_mults(matrix1 * static_cast<T>(1.1));
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(x1[i + d * j] * 1.1, matrix_mults(i, j), precision);
    }
  }
  T v[d];
  for (int i = 0; i < d; ++i) v[i] = rand() / static_cast<T>(RAND_MAX);
  mathfu::Vector<T, d> vector(v);
  // This will test matrix vector multiplication and verify that the resulting
  // vector is mathematically correct.
  // api
  // matrix * vector = vector  [4x4] * [4x1] = [4x1]
  // [2x2] [2x1] = [2x1]
  // a, b * x = (a b) dot (x y)
  // c, d   y   (c d) dot (x y)
  mathfu::Vector<T, d> vector_multv(matrix1 * vector);
  for (int i = 0; i < d; ++i) {
    T v1[d];
    for (int k = 0; k < d; ++k) v1[k] = matrix1(i, k);
    mathfu::Vector<T, d> vec1(v1);
    T dot = mathfu::Vector<T, d>::DotProduct(vec1, vector);
    EXPECT_NEAR(dot, vector_multv[i], precision);
  }
  // This will test matrix multiplication and verify that the resulting
  // matrix is mathematically correct.
  // api
  // matrix * matrix = matrix  [4x4] * [4x4] = [4x4]
  // [2x2] [2x2] = [2x2]
  // a, b * x, m = (a b) dot x, (a b) dot m
  // c, d   y, n             y            n
  //               (c d) dot x, (c d) dot m
  //                         y            n
  // [2x2](遍历行) [2x2](遍历列) = [2x2]
  mathfu::Matrix<T, d> matrix_multm(matrix1 * matrix2);
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      T v1[d], v2[d];
      for (int k = 0; k < d; ++k) v1[k] = matrix1(i, k); // 取行
      for (int k = 0; k < d; ++k) v2[k] = matrix2(k, j); // 取列
      mathfu::Vector<T, d> vec1(v1), vec2(v2);
      T dot = mathfu::Vector<T, d>::DotProduct(vec1, vec2);
      EXPECT_NEAR(dot, matrix_multm(i, j), precision);
    }
  }
}
TEST_ALL_F(Mult, FLOAT_PRECISION, DOUBLE_PRECISION)
TEST_F(MatrixTests, Mult_float_5) {
  Mult_Test<float, 5>(FLOAT_PRECISION);
}
TEST_F(MatrixTests, Mult_double_5) {
  Mult_Test<double, 5>(DOUBLE_PRECISION);
}

// This will test the outer product of two vectors. The template parameter d
// corresponds to the number of rows and columns.
template <class T, int d>
void OuterProduct_Test(const T& precision) {
  T x1[d], x2[d];
  for (int i = 0; i < d; ++i) x1[i] = rand() / static_cast<T>(RAND_MAX);
  for (int i = 0; i < d; ++i) x2[i] = rand() / static_cast<T>(RAND_MAX);
  // api
  // vector_行数_列数
  //     vector_1_2 : 1行x2列 （行向量）
  //     vector_2_1 : 2行x1列 （列向量）
  // OuterProduct(vector_1_2, vector_2_1) = matrix_2_2
  //     2x1 * 1x2 = 2x2
  //     2行x1列 * 1行x2列 = 2行x2列
  //     2行x1列 = v2 = columns
  //     1行x2列 = v1 = rows
  //     OuterProduct(1x2, 2x1) // 我觉得它这里的 api 设计得不好，顺序不好，如果是 OuterProduct(2x1, 1x2) = 2x2 matrix 更好
  //     OuterProduct(rowsVector, columnsVector)
  //     OuterProduct(行向量, 列向量)
  // 扩展到 n 行 m 列
  //     OuterProduct(vector_1_n, vector_m_1) = matrix_m_n
  mathfu::Vector<T, d> vector1(x1), vector2(x2);
  mathfu::Matrix<T, d> matrix(
      mathfu::Matrix<T, d>::OuterProduct(vector1, vector2));
  // This will verify that each element is mathematically correct.
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(vector1[i] * vector2[j], matrix(i, j), precision);
    }
  }
}
TEST_ALL_F(OuterProduct, FLOAT_PRECISION, DOUBLE_PRECISION)

// Print the specified matrix to output_string in the form.
template <class T, int rows, int columns>
std::string MatrixToString(const mathfu::Matrix<T, rows, columns>& matrix) {
  std::stringstream ss;
  ss.flags(std::ios::fixed);
  ss.precision(4);
  ss << matrix;
  return ss.str();
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)  // conditional expression is constant
#endif                           // _MSC_VER

// Test the inverse of a set of noninvertible matrices.
// 针对一系列 不可逆矩阵，测试 取逆函数
// 行列式 != 0 是 可逆矩阵 的条件，== 0 则 不可逆
template <class T, int d>
void InverseNonInvertible_Test(const T& precision) {
  (void)precision;
  T m[d * d];
  const size_t matrix_size = sizeof(m) / sizeof(m[0]);
  static const T kDeterminantThreshold =
      mathfu::Constants<T>::GetDeterminantThreshold();
  static const T kDeterminantThresholdSmall =
      kDeterminantThreshold / 100;
  static const T kDeterminantThresholdLarge =
      kDeterminantThreshold * 100;
  static const T kDeterminantThresholdInverse = 1 / kDeterminantThreshold;
  static const T kDeterminantThresholdInverseSmall =
      kDeterminantThresholdInverse / 100;
  static const T kDeterminantThresholdInverseLarge =
      kDeterminantThresholdInverse * 100;
  // Create a matrix with all zeros.
  for (size_t i = 0; i < matrix_size; ++i) m[i] = 0;
  // Verify that it's not possible to invert the matrix.
  {
    mathfu::Matrix<T, d> matrix(m);
    mathfu::Matrix<T, d> inverse_matrix;
    EXPECT_FALSE(matrix.InverseWithDeterminantCheck(&inverse_matrix));
    EXPECT_FALSE(matrix.InverseWithDeterminantCheck(
        &inverse_matrix, kDeterminantThresholdSmall));
    EXPECT_FALSE(matrix.InverseWithDeterminantCheck(
        &inverse_matrix, kDeterminantThresholdLarge));
  }
  // Check a matrix with all elements at the determinant threshold.
  // 给所有元素设置为 行列式值的阈值（float 的阈值 or double 的阈值）
  for (size_t i = 0; i < matrix_size; ++i) m[i] = kDeterminantThreshold;
  {
    mathfu::Matrix<T, d> matrix(m);
    mathfu::Matrix<T, d> inverse_matrix;
    EXPECT_FALSE(matrix.InverseWithDeterminantCheck(&inverse_matrix));
    EXPECT_FALSE(matrix.InverseWithDeterminantCheck(
        &inverse_matrix, kDeterminantThresholdSmall));
    EXPECT_FALSE(matrix.InverseWithDeterminantCheck(
        &inverse_matrix, kDeterminantThresholdLarge));
  }
  // Check a matrix with all very large elements.
  // 设置非常大的值给 matrix
  for (size_t i = 0; i < matrix_size - 1; ++i) {
    m[i] = kDeterminantThresholdInverse;
  }
  m[matrix_size - 1] = kDeterminantThresholdInverseLarge;
  // NOTE: Due to precision, this case will pass the determinant check with a
  // 2x2 matrix since the determinant of the matrix will be calculated as 1.
  if (d != 2) {
    // Create a matrix with all elements at the determinant threshold and one
    // large value in the matrix.
    for (size_t i = 0; i < matrix_size - 1; ++i) {
      m[i] = kDeterminantThresholdInverseSmall;
    }
    m[matrix_size - 1] = kDeterminantThresholdInverseLarge;
    {
      mathfu::Matrix<T, d> matrix(m);
      mathfu::Matrix<T, d> inverse_matrix;
      EXPECT_FALSE(matrix.InverseWithDeterminantCheck(&inverse_matrix));
      EXPECT_FALSE(matrix.InverseWithDeterminantCheck(
          &inverse_matrix, kDeterminantThresholdSmall));
      EXPECT_FALSE(matrix.InverseWithDeterminantCheck(
          &inverse_matrix, kDeterminantThresholdLarge));
    }
  }
}
TEST_ALL_F(InverseNonInvertible, FLOAT_PRECISION, DOUBLE_PRECISION)

#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER

// Test that matrices with small scale pass or fail the determinant check based
// on the threshold.
// 测试是否可以 inverse，调整精度值的阈值 threshold
// 阈值小，则可以 inverse
// 阈值大，则不可以 inverse
template <class T, int d>
void InverseSmallScale_Test(const T& precision) {
  (void)precision;
  static const T kDeterminantThreshold =
      mathfu::Constants<T>::GetDeterminantThreshold();
  static const T kDeterminantThresholdSmall =
      kDeterminantThreshold / 100;
  static const T kDeterminantThresholdLarge =
      kDeterminantThreshold * 100;

  // The scale of the determinant grows with the square for 2x2 matrices, and
  // with the cube for both 3x3 and 4x4 matrices.
  static const T kDeterminantPower = static_cast<T>(1) / (d == 2 ? 2 : 3);
  static const T kScaleMin = pow(kDeterminantThreshold, kDeterminantPower);

  mathfu::Matrix<T, d> matrix = mathfu::Matrix<T, d>::Identity();
  mathfu::Matrix<T, d> inverse_matrix;

  // Scale too small - non-invertible.
  // 矩阵的 scale 足够小 - 不可以求逆
  {
    for (int i = 0; i != d; ++i) matrix(i, i) = kScaleMin / 2;
    EXPECT_FALSE(matrix.InverseWithDeterminantCheck(&inverse_matrix));
    EXPECT_TRUE(matrix.InverseWithDeterminantCheck(
        &inverse_matrix, kDeterminantThresholdSmall));
    EXPECT_FALSE(matrix.InverseWithDeterminantCheck(
        &inverse_matrix, kDeterminantThresholdLarge));
  }

  // Scale large enough - invertible.
  // 矩阵的 scale 足够大 - 可以求逆
  {
    for (int i = 0; i != d; ++i) matrix(i, i) = kScaleMin * 2;
    EXPECT_TRUE(matrix.InverseWithDeterminantCheck(&inverse_matrix));
    EXPECT_TRUE(matrix.InverseWithDeterminantCheck(
        &inverse_matrix, kDeterminantThresholdSmall));
    EXPECT_FALSE(matrix.InverseWithDeterminantCheck(
        &inverse_matrix, kDeterminantThresholdLarge));
  }
}
TEST_ALL_F(InverseSmallScale, FLOAT_PRECISION, DOUBLE_PRECISION)

// This will test calculating the inverse of a matrix. The template parameter d
// corresponds to the number of rows and columns.
// 测试 求逆 的正确性
// A * A 的逆 = 单位矩阵
template <class T, int d>
void Inverse_Test(const T& precision) {
  T x[d * d];
  for (int iterations = 0; iterations < 10; ++iterations) {
    // NOTE: This assumes that matrices generated here are invertible since
    // there is a tiny probability that a randomly generated matrix will be
    // noninvertible.  This does mean that this test can be flakey by
    // occasionally generating noninvertible matrices.
    // api
    // kElements
    for (int i = 0; i < mathfu::Matrix<T, d>::kElements; ++i) {
      x[i] = mathfu::RandomRange<T>(1);
    }
    mathfu::Matrix<T, d> matrix(x);
    std::string error_string = MatrixToString(matrix);
    error_string += "\n";
    mathfu::Matrix<T, d> inverse_matrix(matrix.Inverse());
    mathfu::Matrix<T, d> identity_matrix(matrix * inverse_matrix);

    error_string += MatrixToString(inverse_matrix);
    error_string += "\n";
    error_string += MatrixToString(identity_matrix);

    // This will verify that M * Minv is equal to the identity.
    for (int i = 0; i < d; ++i) {
      for (int j = 0; j < d; ++j) {
        EXPECT_NEAR(i == j ? 1 : 0, identity_matrix(i, j), 100 * precision)
            << error_string << " row=" << i << " column=" << j;
      }
    }
  }
}
// Due to the number of operations involved and the random numbers used to
// generate the test matrices, the precision the inverse matrix is calculated
// to is relatively low.
TEST_ALL_F(Inverse, 1e-4f, 1e-8)

// This will test converting from a translation into a matrix and back again.
template <class T>
void TranslationVector3D_Test(const T& precision) {
  (void)precision;
  const mathfu::Vector<T, 3> trans(static_cast<T>(-100.0), static_cast<T>(0.0),
                                   static_cast<T>(0.00003));
  // api
  // FromTranslationVector
  const mathfu::Matrix<T, 4> trans_matrix =
      mathfu::Matrix<T, 4>::FromTranslationVector(trans);
  // api
  // TranslationVector3D
  const mathfu::Vector<T, 3> trans_back = trans_matrix.TranslationVector3D();

  // This will verify that the translation vector has not changed.
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(trans[i], trans_back[i]);
  }
}
TEST_SCALAR_F(TranslationVector3D, FLOAT_PRECISION, DOUBLE_PRECISION)

// This will test converting from a translation into a matrix and back again.
template <class T>
void TranslationVector2D_Test(const T& precision) {
  (void)precision;
  const mathfu::Vector<T, 2> trans(static_cast<T>(-100.0),
                                   static_cast<T>(0.00003));
  const mathfu::Matrix<T, 3> trans_matrix =
      mathfu::Matrix<T, 3>::FromTranslationVector(trans);
  // api
  // TranslationVector2D
  const mathfu::Vector<T, 2> trans_back = trans_matrix.TranslationVector2D();

  // This will verify that the translation vector has not changed.
  for (int i = 0; i < 2; ++i) {
    EXPECT_EQ(trans[i], trans_back[i]);
  }
}
TEST_SCALAR_F(TranslationVector2D, FLOAT_PRECISION, DOUBLE_PRECISION)

// This will test converting from a scale into a matrix, then multiply by
// a vector of 1's, which should produce the original scale again.
template <class T, int d>
void FromScaleVector_Test(const T& precision) {
  mathfu::Vector<T, d> ones(static_cast<T>(1));
  mathfu::Vector<T, d - 1> v;

  // Tests that the scale vector is placed in the correct order in the matrix.
  for (int i = 0; i < d - 1; ++i) {
    v[i] = static_cast<T>(i + 10);
  }
  // api
  // FromScaleVector
  mathfu::Matrix<T, d> m = mathfu::Matrix<T, d>::FromScaleVector(v);

  // Ensure that the v is on the diagonal.
  // 斜对角就是 scale 值
  for (int i = 0; i < d - 1; ++i) {
    EXPECT_NEAR(v[i], m(i, i), precision);
  }

  // Ensure that the last diagonal element is one.
  EXPECT_NEAR(m(d - 1, d - 1), 1, precision);

  // Ensure that all non-diagonal elements are zero.
  for (int i = 0; i < d - 1; ++i) {
    for (int j = 0; j < d - 1; ++j) {
      if (i == j) continue;
      EXPECT_NEAR(m(i, j), 0, precision);
    }
  }
}
// Precision is zero. Results must be perfect for this test.
TEST_ALL_F(FromScaleVector, 0.0f, 0.0)

// Compare a set of Matrix<T, rows, columns> with expected values.
template <class T, int rows, int columns>
void VerifyMatrixExpectations(
    const MatrixExpectation<T, rows, columns>* test_cases,
    const size_t number_of_test_cases, const T& precision) {
  for (size_t i = 0; i < number_of_test_cases; ++i) {
    const MatrixExpectation<T, rows, columns>& test = test_cases[i];
    for (int j = 0; j < mathfu::Matrix<T, rows, columns>::kElements; ++j) {
      const mathfu::Matrix<T, rows, columns>& calculated = test.calculated;
      const mathfu::Matrix<T, rows, columns>& expected = test.expected;
      EXPECT_NEAR(calculated[j], expected[j], precision)
          << "element " << j << " (" << (j / columns) << ", " << (j % columns)
          << "), case " << test.description << "\n"
          << MatrixToString(calculated) << "vs expected\n"
          << MatrixToString(expected);
    }
  }
}

// Test perspective matrix calculation.
template <class T>
void Perspective_Test(const T& precision) {
  // clang-format off
  static const MatrixExpectation<T, 4, 4> kTestCases[] = {
    // fov 90 度，一半 45 度
    // 右手坐标系
    // znear 是 0，平常很少会设置为 0
    {
      "normalized handedness=1",
      // api
      // Perspective
      mathfu::Matrix<T, 4>::Perspective(
          atan(static_cast<T>(1)) * 2, 1, 0, 1, 1),
      mathfu::Matrix<T, 4>(1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, -1, -1,
                           0, 0, 0, 0),
    },
    // fov 90 度，一半 45 度
    // 左手坐标系
    {
      "normalized handedness=-1",
      mathfu::Matrix<T, 4>::Perspective(
          atan(static_cast<T>(1)) * 2, 1, 0, 1, -1),
      mathfu::Matrix<T, 4>(1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 1,
                           0, 0, 0, 0),
    },
    // fov 180 度，一半 90 度
    {
      "widefov",
      mathfu::Matrix<T, 4>::Perspective(
          atan(static_cast<T>(2)) * 2, 1, 0, 1, 1),
      mathfu::Matrix<T, 4>(0.5, 0, 0, 0,
                           0, 0.5, 0, 0,
                           0, 0, -1, -1,
                           0, 0, 0, 0),
    },
    // fov 很小. narrow fov 狭窄 fov
    {
      "narrowfov",
      mathfu::Matrix<T, 4>::Perspective(
          atan(static_cast<T>(0.1)) * 2, 1, 0, 1, 1),
      mathfu::Matrix<T, 4>(10, 0, 0, 0,
                           0, 10, 0, 0,
                           0, 0, -1, -1,
                           0, 0, 0, 0),
    },
    // fov 90 度，一半 45 度
    // 宽高比 2:1
    {
      "2:1 aspect ratio",
      mathfu::Matrix<T, 4>::Perspective(
          atan(static_cast<T>(1)) * 2, 0.5, 0, 1, 1),
      mathfu::Matrix<T, 4>(2, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, -1, -1,
                           0, 0, 0, 0),
    },
    // fov 90 度，一半 45 度
    // 更深的视锥体: near 是 -2
    // 对应的现象会是什么？
    {
      "deeper view frustrum",
      mathfu::Matrix<T, 4>::Perspective(
          atan(static_cast<T>(1)) * 2, 1, -2, 2, 1),
      mathfu::Matrix<T, 4>(1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, -0.5, -1,
                           0, 0, 2, 0),
    },
  };
  // clang-format on
  VerifyMatrixExpectations(
      kTestCases, sizeof(kTestCases) / sizeof(kTestCases[0]), precision);
}
TEST_SCALAR_F(Perspective, FLOAT_PRECISION, DOUBLE_PRECISION * 10)

// Test orthographic matrix calculation.
template <class T>
void Ortho_Test(const T& precision) {
  // clang-format off
  static const MatrixExpectation<T, 4, 4> kTestCases[] = {
    // 这里没有明白为什么 znear 是 2，zfar 是 0，
    // zfar 比 znear 要近？ 跟摄像机的位置/朝向是否有关
    {
      "normalized",
      mathfu::Matrix<T, 4, 4>::Ortho(0, 2, 0, 2, 2, 0),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              -1, -1, 1, 1),
    },
    {
      "normalized RH",
      mathfu::Matrix<T, 4, 4>::Ortho(0, 2, 0, 2, 2, 0, 1),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              -1, -1, 1, 1),
    },
    // 对应的现象是什么？
    // right - left 还是 2，为什么会是 narrow 狭窄的？
    {
      "narrow RH",
      mathfu::Matrix<T, 4, 4>::Ortho(1, 3, 0, 2, 2, 0, 1),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              -2, -1, 1, 1),

    },
    // 对应的现象是什么？
    // 为什么会是 suqat 矮胖的？
    {
      "squat RH",
      mathfu::Matrix<T, 4, 4>::Ortho(0, 2, 1, 3, 2, 0, 1),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              -1, -2, 1, 1),

    },
    // 对应的现象是什么？
    // 为什么会是 geep 更深的？
    {
      "deep RH",
      mathfu::Matrix<T, 4, 4>::Ortho(0, 2, 0, 2, 3, 1, 1),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              -1, -1, 2, 1),

    },
    // 归一化的矩阵
    // 宽高深都是 2
    {
      "normalized LH",
      mathfu::Matrix<T, 4, 4>::Ortho(0, 2, 0, 2, 2, 0, -1),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, -1, 0,
                              -1, -1, 1, 1),
    },
    // 规范的矩阵
    // 宽高深都是 2
    {
      "Canonical LH",
      mathfu::Matrix<T, 4, 4>::Ortho(1, 3, 1, 3, 1, 3, -1),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                             -2,-2,-2, 1),

    },
  };
  // clang-format on
  VerifyMatrixExpectations(
      kTestCases, sizeof(kTestCases) / sizeof(kTestCases[0]), precision);
}
TEST_SCALAR_F(Ortho, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test look-at matrix calculation.
template <class T>
void LookAt_Test(const T& precision) {
  // clang-format off
  static const MatrixExpectation<T, 4, 4> kTestCases[] = {
    // 左手坐标系 camera 矩阵，位置在 z 轴上，看向 z 轴负方向，up 向量是 y 轴，结果是一个单位矩阵
    {
      "origin along z",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(0, 0, 1), mathfu::Vector<T, 3>(0, 0, 0),
          mathfu::Vector<T, 3>(0, 1, 0)),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1),
    },
    {
      "origin along z",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(0, 0, 2), mathfu::Vector<T, 3>(0, 0, 0),
          mathfu::Vector<T, 3>(0, 1, 0), -1),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1),
    },
    // 右手坐标系 camera 矩阵，位置在 z 轴上，看向 z 轴负方向，up 向量是 y 轴，结果是一个单位矩阵 * scale(-1, 1, -1)
    {
      "right-handed origin along z",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(0, 0, 1), mathfu::Vector<T, 3>(0, 0, 0),
          mathfu::Vector<T, 3>(0, 1, 0), 1),
      mathfu::Matrix<T, 4, 4>(-1, 0, 0, 0,
                               0, 1, 0, 0,
                               0, 0, -1,0,
                               0, 0, 0, 1),
    },
    {
      "origin along x",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(1, 0, 0), mathfu::Vector<T, 3>(0, 0, 0),
          mathfu::Vector<T, 3>(0, 1, 0), -1),
      mathfu::Matrix<T, 4, 4>(0, 0, 1, 0,
                              0, 1, 0, 0,
                              -1, 0, 0, 0,
                              0, 0, 0, 1),
    },
    {
      "origin along y",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(0, 1, 0), mathfu::Vector<T, 3>(0, 0, 0),
          mathfu::Vector<T, 3>(1, 0, 0), -1),
      mathfu::Matrix<T, 4, 4>(0, 1, 0, 0,
                              0, 0, 1, 0,
                              1, 0, 0, 0,
                              0, 0, 0, 1),
    },
    {
      "right-handed origin along x",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(1, 0, 0), mathfu::Vector<T, 3>(0, 0, 0),
          mathfu::Vector<T, 3>(0, 1, 0), 1),
      mathfu::Matrix<T, 4, 4>(0, 0, -1, 0,
                              0, 1, 0, 0,
                              1, 0, 0, 0,
                              0, 0, 0, 1),
    },
    {
      "right-handed origin along y",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(0, 1, 0), mathfu::Vector<T, 3>(0, 0, 0),
          mathfu::Vector<T, 3>(1, 0, 0), 1),
      mathfu::Matrix<T, 4, 4>(0, 1, 0, 0,
                              0, 0, -1, 0,
                              -1, 0, 0, 0,
                              0,  0, 0, 1),
    },
    // 看向 diagonal 对角
    {
      "origin along diagonal",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(0, 0, 0), mathfu::Vector<T, 3>(1, 1, 1),
          mathfu::Vector<T, 3>(0, 1, 0), -1),
      mathfu::Matrix<T, 4, 4>(
          static_cast<T>(-0.707106781), static_cast<T>(-0.408248290),
              static_cast<T>(-0.577350269), 0,
          0, static_cast<T>(0.816496580), static_cast<T>(-0.577350269), 0,
          static_cast<T>(0.707106781), static_cast<T>(-0.408248290),
              static_cast<T>(-0.577350269), 0,
          0, 0, static_cast<T>(1.732050808), 1),
    },
    {
      "right-handed diagonal along diagonal",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(0, 0, 0), mathfu::Vector<T, 3>(1, 1, 1),
          mathfu::Vector<T, 3>(0, 1, 0), 1),
      mathfu::Matrix<T, 4, 4>(
          static_cast<T>(0.707106781), static_cast<T>(-0.408248290),
              static_cast<T>(0.577350269), 0,
          0, static_cast<T>(0.816496581), static_cast<T>(0.577350269), 0,
          static_cast<T>(-0.707106781), static_cast<T>(-0.408248290),
              static_cast<T>(0.577350269), 0,
          0, 0, static_cast<T>(-1.732050808), 1),
    },
    {
      "translated eye, looking along z",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(1, 1, 2), mathfu::Vector<T, 3>(1, 1, 1),
          mathfu::Vector<T, 3>(0, 1, 0), -1),
      mathfu::Matrix<T, 4, 4>(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              -1, -1, -1, 1),
    },
    {
      "right-handed translated eye along x",
      mathfu::Matrix<T, 4, 4>::LookAt(
          mathfu::Vector<T, 3>(2, 1, 1), mathfu::Vector<T, 3>(1, 1, 1),
          mathfu::Vector<T, 3>(0, 1, 0), 1),
      mathfu::Matrix<T, 4, 4>(0, 0, -1, 0,
                              0, 1, 0, 0,
                              1, 0, 0, 0,
                             -1,-1, 1, 1),
    },
  };
  // clang-format on
  VerifyMatrixExpectations(
      kTestCases, sizeof(kTestCases) / sizeof(kTestCases[0]), precision);
}
TEST_SCALAR_F(LookAt, FLOAT_PRECISION, kLookAtDoublePrecision)

// Test UnProject calculation.
template <class T>
void UnProject_Test(const T& precision) {
  // clang-format off
  mathfu::Matrix<T, 4, 4> modelView =
      mathfu::Matrix<T, 4, 4>(-1, 0,                   0, 0,
                               0, 1,                   0, 0,
                               0, 0,                  -1, 0,
                               0, 0, static_cast<T>(-10), 1);
  mathfu::Matrix<T, 4, 4> projection =
      mathfu::Matrix<T, 4, 4>(
          static_cast<T>(1.81066),  0,                            0,   0,
                        0, static_cast<T>(2.41421342),            0,   0,
                        0,          0,   static_cast<T>(-1.00001991), -1,
                        0,          0,  static_cast<T>(-0.200001985),  0);
  // clang-format on
  // api
  // UnProject
  mathfu::Vector<T, 3> result = mathfu::Matrix<T, 4, 4>::UnProject(
      mathfu::Vector<T, 3>(754, 1049, 1), modelView, projection, 1600, 1200);
  EXPECT_NEAR(result.x, 319.0024240119667, 300.0 * precision);
  EXPECT_NEAR(result.y, 3113.7409399903145, 3000.0 * precision);
  EXPECT_NEAR(result.z, 10035.303114113103, 10000.0 * precision);
}
TEST_SCALAR_F(UnProject, kUnProjectFloatPrecision, DOUBLE_PRECISION)

// Test matrix transposition.
template <class T, int d>
void Transpose_Test(const T& precision) {
  (void)precision;
  mathfu::Matrix<T, d> matrix;
  for (int i = 0; i < d * d; ++i) {
    matrix[i] = static_cast<T>(i);
  }
  // api
  // Transpose
  mathfu::Matrix<T, d> transpose = matrix.Transpose();
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(matrix(i, j), transpose(j, i), static_cast<T>(0));
    }
  }
}

TEST_ALL_F(Transpose, FLOAT_PRECISION, DOUBLE_PRECISION)

// Return one of the numbers:
//   offset, offset + width/d, offset + 2*width/d, ... , offset + (d-1)*width/d
// For each i=0..d-1, the number returned is different.
//
// The order of the numbers is determined by 'prime' which should be a prime
// number > d.
template <class T, int d>
static T WellSpacedNumber(int i, int prime, T width, T offset) {
  // Reorder the i's from 0..(d-1) by using a prime number.
  // The numbers get reordered (i.e. no duplicates) because 'd' and 'prime' are
  // relatively prime.
  const int remapped = ((i + 1) * prime) % d;

  // Map to [0,1) range. That is, inclusive of 0, exclusive of 1.
  const T zero_to_one = static_cast<T>(remapped) / static_cast<T>(d);

  // Map to [offset, offset + width) range.
  const T offset_and_scaled = zero_to_one * width + offset;
  return offset_and_scaled;
}

// 生成一个可逆的矩阵
// Generate a (probably) invertable matrix. I haven't gone through the math
// to verify that it's actually invertable for all d, but for small d it is.
//
// We adjusting each element of an identity matrix by a small amount.
// The amount must be small so that the matrix stays reasonably close to
// the identity. Matrices become progressively less numerically stable the
// further the get from identity.
template <class T, int d>
static mathfu::Matrix<T, d> InvertableMatrix() {
  mathfu::Matrix<T, d> invertable = mathfu::Matrix<T, d>::Identity();

  for (int i = 0; i < d; ++i) {
    // The width and offset constants are arbitrary. We do want to keep the
    // pseudo-random values centered near 0 though.
    const T rand_i = WellSpacedNumber<T, d>(i, 7, 0.8f, -0.33f);

    for (int j = 0; j < d; ++j) {
      const T rand_j = WellSpacedNumber<T, d>(j, 13, 0.6f, -0.4f);
      invertable(i, j) += rand_i * rand_j;
    }
  }
  return invertable;
}

template <class T, int d>
static void ExpectEqualMatrices(const mathfu::Matrix<T, d>& a,
                                const mathfu::Matrix<T, d>& b, T precision) {
  for (int i = 0; i < d; ++i) {
    for (int j = 0; j < d; ++j) {
      EXPECT_NEAR(a(i, j), b(i, j), precision);
    }
  }
}

// Test matrix operator*() by multiplying an invertable matrix by its
// inverse. Should end up with identity.
template <class T, int d>
void MultiplyOperatorInverse_Test(const T& precision) {
  typedef typename mathfu::Matrix<T, d> Mat;
  const Mat identity = Mat::Identity();
  const Mat invertable = InvertableMatrix<T, d>();

  // Use operator*() to go way from the identity and then get back to it.
  Mat product = identity;
  product *= invertable;
  product *= invertable.Inverse();

  // Test that operator*() gets is back to where we need to go.
  ExpectEqualMatrices(product, identity, precision);
}

TEST_ALL_F(MultiplyOperatorInverse, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test static operator*(a, b) by multiplying an invertable matrix by its
// inverse. Should end up with identity.
template <class T, int d>
void ExternalMultiplyOperatorInverse_Test(const T& precision) {
  typedef typename mathfu::Matrix<T, d> Mat;
  const Mat identity = Mat::Identity();
  const Mat invertable = InvertableMatrix<T, d>();

  // Use operator*() to go way from the identity and then get back to it.
  Mat product = identity;
  product = product * invertable;
  product = product * invertable.Inverse();

  // Test that operator*() gets is back to where we need to go.
  ExpectEqualMatrices(product, identity, precision);
}

TEST_ALL_F(ExternalMultiplyOperatorInverse, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test matrix operator*() by multiplying a non-zero matrix by identity.
// Should be no change.
template <class T, int d>
void MultiplyOperatorIdentity_Test(const T& precision) {
  typedef typename mathfu::Matrix<T, d> Mat;
  const Mat identity = Mat::Identity();
  const Mat invertable = InvertableMatrix<T, d>();

  // Use operator*() to multipy by the identity.
  Mat product = invertable;
  product *= identity; // 乘上单位矩阵，等于没乘

  // Test that operator*() didn't change anything when multiplying by identity.
  ExpectEqualMatrices(product, invertable, precision);
}

TEST_ALL_F(MultiplyOperatorIdentity, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test static operator*(a, b) by multiplying a non-zero matrix by identity.
// Should be no change.
template <class T, int d>
void ExternalMultiplyOperatorIdentity_Test(const T& precision) {
  typedef typename mathfu::Matrix<T, d> Mat;
  const Mat identity = Mat::Identity();
  const Mat invertable = InvertableMatrix<T, d>();

  // Use operator*() to multipy by the identity.
  const Mat product = invertable * identity; // 乘上单位矩阵，等于没乘

  // Test that operator*() didn't change anything when multiplying by identity.
  ExpectEqualMatrices(product, invertable, precision);
}

TEST_ALL_F(ExternalMultiplyOperatorIdentity, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test matrix operator*() by multiplying a non-zero matrix by zero.
// Should be no change.
template <class T, int d>
void MultiplyOperatorZero_Test(const T& precision) {
  typedef typename mathfu::Matrix<T, d> Mat;
  const Mat zero(static_cast<T>(0));
  const Mat invertable = InvertableMatrix<T, d>();

  // Use operator*() to multipy by the zero.
  Mat product = invertable;
  product *= zero;

  // Test that operator*() didn't change anything when multiplying by zero.
  ExpectEqualMatrices(product, zero, precision);
}

TEST_ALL_F(MultiplyOperatorZero, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test static operator*(a, b) by multiplying a non-zero matrix by zero.
// Should be no change.
template <class T, int d>
void ExternalMultiplyOperatorZero_Test(const T& precision) {
  typedef typename mathfu::Matrix<T, d> Mat;
  const Mat zero(static_cast<T>(0));
  const Mat invertable = InvertableMatrix<T, d>();

  // Use operator*() to multipy by the zero.
  const Mat product = invertable * zero;

  // Test that operator*() didn't change anything when multiplying by zero.
  ExpectEqualMatrices(product, zero, precision);
}

TEST_ALL_F(ExternalMultiplyOperatorZero, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test Matrix<>::ToAffineTransform().
template <class T>
void Mat4ToAffine_Test(const T&) {
  typedef typename mathfu::Matrix<T, 4> Mat4;
  typedef typename mathfu::Matrix<T, 4, 3> Affine;
  const Mat4 indices4(0, 1, 2, 0, 4, 5, 6, 0, 8, 9, 10, 0, 12, 13, 14, 1);
  const Affine indices_affine(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14);

  // api
  // ToAffineTransform
  const Affine to_affine = Mat4::ToAffineTransform(indices4);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_EQ(to_affine(i, j), indices_affine(i, j));
    }
  }
}

TEST_SCALAR_F(Mat4ToAffine, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test Matrix<>::FromAffineTransform().
template <class T>
void Mat4FromAffine_Test(const T&) {
  typedef typename mathfu::Matrix<T, 4> Mat4;
  typedef typename mathfu::Matrix<T, 4, 3> Affine;
  const Mat4 indices4(0, 1, 2, 0, 4, 5, 6, 0, 8, 9, 10, 0, 12, 13, 14, 1);
  const Affine indices_affine(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14);

  // api
  // FromAffineTransform
  const Mat4 to_mat4 = Mat4::FromAffineTransform(indices_affine);
  ExpectEqualMatrices(to_mat4, indices4, static_cast<T>(0));
}

TEST_SCALAR_F(Mat4FromAffine, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test converting back and forth via Matrix<>::To/FromAffineTransform().
template <class T>
void Mat4ToAndFromAffine_Test(const T&) {
  typedef typename mathfu::Matrix<T, 4> Mat4;
  typedef typename mathfu::Matrix<T, 4, 3> Affine;
  const Mat4 indices4(0, 1, 2, 0, 4, 5, 6, 0, 8, 9, 10, 0, 12, 13, 14, 1);
  const Affine indices_affine(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14);

  // Convert to/from AffineTransform and check to ensure the result is the same
  // as the original.
  const Mat4 converted =
      Mat4::FromAffineTransform(Mat4::ToAffineTransform(indices4));

  ExpectEqualMatrices(indices4, converted, static_cast<T>(0));

  // Perform a normal mat4 * mat4 multiplication and compare its result with
  // multiplications involving conversions.
  const Mat4 mat4_multiplication = indices4 * indices4;
  const Mat4 affine_multiplication = Mat4::FromAffineTransform(indices_affine) *
                                     Mat4::FromAffineTransform(indices_affine);
  const Mat4 affine_and_mat4_multiplication =
      indices4 * Mat4::FromAffineTransform(indices_affine);

  ExpectEqualMatrices(mat4_multiplication, affine_multiplication,
                      static_cast<T>(0));
  ExpectEqualMatrices(mat4_multiplication, affine_and_mat4_multiplication,
                      static_cast<T>(0));

  // Ensure that the result from the multiplication produces the expected
  // AffineTransform result.
  const Affine expected_result(20, 68, 116, 176, 23, 83, 143, 216, 26, 98, 170,
                               256);
  const Affine affine_result = Mat4::ToAffineTransform(affine_multiplication);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_EQ(expected_result(i, j), affine_result(i, j));
    }
  }
}

TEST_SCALAR_F(Mat4ToAndFromAffine, FLOAT_PRECISION, DOUBLE_PRECISION)

// Test extracting the 3x3 rotation Matrix portion from a 4x4 Matrix.
template <class T>
void Mat4ToRotationMatrix_Test(const T&) {
  typedef typename mathfu::Matrix<T, 4> Mat4;
  typedef typename mathfu::Matrix<T, 3> Mat3;
  const Mat4 input(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  const Mat3 expect(1, 2, 3, 5, 6, 7, 9, 10, 11);
  // api
  // ToRotationMatrix
  Mat3 result = input.ToRotationMatrix(input);
  for (int i = 0; i < 9; i++) {
    EXPECT_EQ(expect[i], result[i]);
  }
}

TEST_SCALAR_F(Mat4ToRotationMatrix, FLOAT_PRECISION, DOUBLE_PRECISION)

// This will test converting from a translation into a matrix and back again.
// Test the compilation of basic matrix operations given in the sample file.
// This will test transforming a vector with a matrix.
TEST_F(MatrixTests, MatrixSample) {
  using namespace mathfu;
  /// @doxysnippetstart Chapter04_Matrices.md Matrix_Sample
  Vector<float, 3> trans(3.f, 2.f, 8.f);
  Vector<float, 3> rotation(0.4f, 1.4f, 0.33f);
  Vector<float, 3> vector(4.f, 8.f, 1.f);

  // api
  // FromEulerAngles(vec3) = quat
  Quaternion<float> rotQuat = Quaternion<float>::FromEulerAngles(rotation);
  // api
  // quat.ToMatrix() = mat3x3
  Matrix<float, 3> rotMatrix = rotQuat.ToMatrix();
  Matrix<float, 4> transMatrix = Matrix<float, 4>::FromTranslationVector(trans);
  // api
  // FromRotationMatrix(mat3x3) = mat4x4
  Matrix<float, 4> rotHMatrix = Matrix<float, 4>::FromRotationMatrix(rotMatrix);

  Matrix<float, 4> matrix = transMatrix * rotHMatrix;
  Vector<float, 3> rotatedVector = matrix * vector;
  /// @doxysnippetend
  const float precision = 1e-2f;
  EXPECT_NEAR(5.14f, rotatedVector[0], precision);
  EXPECT_NEAR(10.11f, rotatedVector[1], precision);
  EXPECT_NEAR(4.74f, rotatedVector[2], precision);
}

// This will test the == matrices operator. The template paramter d corresponds
// to the number of rows and columns.
template <class T, int d>
void Equal_Test(const T& precision) {
  mathfu::Matrix<T, d> expected;
  for (int i = 0; i < d * d; ++i) {
    expected[i] = static_cast<T>(i * precision);
  }
  mathfu::Matrix<T, d> copy(expected);
  // api
  // ==
  EXPECT_TRUE(expected == copy);

  mathfu::Matrix<T, d> close(expected - static_cast<T>(1));
  EXPECT_FALSE(expected == close);
}
TEST_ALL_F(Equal, FLOAT_PRECISION, DOUBLE_PRECISION)

// This will test the != matrices operator. The template paramter d corresponds
// to the number of rows and columns.
template <class T, int d>
void NotEqual_Test(const T& precision) {
  mathfu::Matrix<T, d> expected;
  for (int i = 0; i < d * d; ++i) {
    expected[i] = static_cast<T>(i * precision);
  }
  mathfu::Matrix<T, d> copy(expected);
  // api
  // !=
  EXPECT_FALSE(expected != copy);

  mathfu::Matrix<T, d> close(expected - static_cast<T>(1));
  EXPECT_TRUE(expected != close);
}
TEST_ALL_F(NotEqual, FLOAT_PRECISION, DOUBLE_PRECISION)

// Simple class that represents a possible compatible type for a vector.
// That is, it's just an array of T of length d, so can be loaded and
// stored from mathfu::Vector<T,d> using ToType() and FromType().
template <class T, int d>
struct SimpleMatrix {
  T values[d * d];
};

// This will test the FromType() conversion functions.
template <class T, int d>
void FromType_Test(const T& precision) {
  typedef SimpleMatrix<T, d> CompatibleT; // Compatible adj. 兼容的；可共存的；可和睦相处的；与……一致的
  typedef mathfu::Matrix<T, d> MatrixT;

  CompatibleT compatible;
  for (int i = 0; i < d * d; ++i) {
    compatible.values[i] = static_cast<T>(i * precision);
  }

  // api
  // padding 内存对齐相关的
#ifdef MATHFU_COMPILE_WITH_PADDING
  // With padding, vec3s take up 4-floats worth of memory, so byte-wise
  // conversion won't work.
  if (sizeof(CompatibleT) != sizeof(MatrixT)) {
    EXPECT_EQ(d, 3);
    return;
  }
#endif  // MATHFU_COMPILE_WITH_PADDING

  // api
  // FromType(struct)
  const MatrixT matrix = MatrixT::FromType(compatible);

  for (int i = 0; i < d * d; ++i) {
    EXPECT_EQ(compatible.values[i], matrix[i]);
  }
}
TEST_ALL_F(FromType, 1.0f, 0.0)

// This will test the ToType() conversion functions.
template <class T, int d>
void ToType_Test(const T& precision) {
  typedef SimpleMatrix<T, d> CompatibleT;
  typedef mathfu::Matrix<T, d> MatrixT;

  MatrixT matrix;
  for (int i = 0; i < d * d; ++i) {
    matrix[i] = static_cast<T>(i * precision);
  }

#ifdef MATHFU_COMPILE_WITH_PADDING
  // With padding, vec3s take up 4-floats worth of memory, so byte-wise
  // conversion won't work.
  if (sizeof(CompatibleT) != sizeof(MatrixT)) {
    EXPECT_EQ(d, 3);
    return;
  }
#endif  // MATHFU_COMPILE_WITH_PADDING

  // api
  // ToType<struct>(matrix) = struct
  const CompatibleT compatible = MatrixT::template ToType<CompatibleT>(matrix);

  for (int i = 0; i < d * d; ++i) {
    EXPECT_EQ(compatible.values[i], matrix[i]);
  }
}
TEST_ALL_F(ToType, 0.0f, 0.0)

template <class T, int d>
void OutputStream_Test(const T&) {
  mathfu::Matrix<T, d> matrix;
  for (int i = 0; i < d * d; ++i) {
    matrix[i] = static_cast<T>(i);
  }

  std::stringstream ss;
  // api
  // inline std::ostream& operator<<(std::ostream& os, const Quaternion<T>& q)
  ss << matrix;

  switch (d) {
    case 1:
      EXPECT_EQ("(0)", ss.str());
      break;
    case 2:
      EXPECT_EQ("(0, 1, 2, 3)", ss.str());
      break;
    case 3:
      EXPECT_EQ("(0, 1, 2, 3, 4, 5, 6, 7, 8)", ss.str());
      break;
    case 4:
      EXPECT_EQ("(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)",
                ss.str());
      break;
  }
}
TEST_ALL_F(OutputStream, 0.0f, 0.0)
TEST_F(MatrixTests, OutputStream_Test_float_1) {
  OutputStream_Test<float, 1>(0.0f);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  printf("%s (%s)\n", argv[0], MATHFU_BUILD_OPTIONS_STRING);

  {
    std::cout << "atan " << atan(static_cast<float>(1)) * 2 << std::endl;
    std::cout << "atan " << atan(static_cast<float>(1)) << std::endl;
  }

  using namespace mathfu;
  using Matrix4_4 = Matrix<float, 4, 4>;
  using Vector3 = Vector<float, 3>;
  using Vector2 = Vector<float, 2>;
  using Vector4 = Vector<float, 4>;


  {
    Matrix4_4 lookAt = mathfu::Matrix<float, 4, 4>::LookAt(
        mathfu::Vector<float, 3>(1, 1, 2), mathfu::Vector<float, 3>(1, 1, 1),
        mathfu::Vector<float, 3>(0, 1, 0), -1);
    printMat("lookAt 1 = ", lookAt);
    lookAt = mathfu::Matrix<float, 4, 4>::LookAt(
        mathfu::Vector<float, 3>(1, 1, 2), mathfu::Vector<float, 3>(1, 1, 0),
        mathfu::Vector<float, 3>(0, 1, 0), -1);
    printMat("lookAt 2 = ", lookAt);
    lookAt = mathfu::Matrix<float, 4, 4>::LookAt(
        mathfu::Vector<float, 3>(1, 1, 3), mathfu::Vector<float, 3>(1, 1, 0),
        mathfu::Vector<float, 3>(0, 1, 0), -1);
    printMat("lookAt 3 = ", lookAt);
    lookAt = mathfu::Matrix<float, 4, 4>::LookAt(
        mathfu::Vector<float, 3>(1, 1, 3), mathfu::Vector<float, 3>(1, 1, 1),
        mathfu::Vector<float, 3>(0, 1, 0), -1);
    printMat("lookAt 4 = ", lookAt);
    lookAt = mathfu::Matrix<float, 4, 4>::LookAt(
        mathfu::Vector<float, 3>(1, 1, 3), mathfu::Vector<float, 3>(1, 1, 2),
        mathfu::Vector<float, 3>(0, 1, 0), -1);
    printMat("lookAt 5 = ", lookAt);
  }

  float cubeWidth = 1.0f;
  float cubeHeight = 1.0f;
  float cubeThickness = 1.0f;
  Vector3 scaleVec3(0.5f, 0.5f, 1.0f);
  Vector3 scaleAnchor(-cubeWidth, cubeHeight, 0);
  Vector3 axisY(0, -1.0f, 0);
  Vector3 rotateAnchor(-cubeWidth, cubeHeight, 0);
  float degree = 90.0f;
  Matrix4_4 anchorMat = Matrix4_4::FromTranslationVector(Vector3(-0.5f, 0.5f, -0.5f));
  Matrix4_4 unAnchorMat = Matrix4_4::FromTranslationVector(Vector3(0.5f, -0.5f, 0.5f));

  Matrix4_4 modelA = Matrix4_4::Identity();
  modelA = anchorMat * unAnchorMat;
  printMat("modelA 1 = ", modelA);
  {
    offsetLeft(modelA, Vector3(cubeWidth / 2.0f, 0, cubeThickness));
    printMat("modelA 2 = ", modelA);

    scaleLeft(modelA, scaleVec3, scaleAnchor);
    printMat("modelA 3 = ", modelA);

    rotateLeft(modelA, degree, axisY, rotateAnchor);
    printMat("modelA 4 = ", modelA);

    offsetLeft(modelA, Vector3(-cubeWidth / 2.0f, -cubeHeight / 2.0f, cubeThickness));
    printMat("modelA 5 = ", modelA);
  }
  modelA = modelA * unAnchorMat;
  printMat("modelA 6 = ", modelA);

  {
    Vector3 outTrans, outScale;
    Quaternion<float> outQuat;
    decompose(modelA, outTrans, outQuat, outScale);
    printVector3("outTrans = ", outTrans);
    printQuat("outQuat = ", outQuat);
    printVector3("outScale = ", outScale);
    {
      float outAngle;
      Vector3 outAxis;
      outQuat.ToAngleAxis(&outAngle, &outAxis);
      std::cout << "outAngle = " << outAngle << std::endl;
      printVector3("outAxis = ", outAxis);

      Vector3 outEuler = outQuat.ToEulerAngles();
      printVector3("outEuler = ", outEuler);
    }

  }

  {
    Vector4 layoutPixel(0, 0, 488, 216); // view layout
    float width = layoutPixel.z - layoutPixel.x;
    float height = layoutPixel.w - layoutPixel.y;
    float pixelToMeter = 0.001995f;

    Matrix4_4 change = Matrix4_4::Identity();
    offsetLeft(change, Vector3(width, height, 0.0f), Vector2(width, height), pixelToMeter);
    scaleLeft(change, Vector3(0.5f, 0.5f, 1.0f), Vector3(0.0f, 0.0f, 0.0f));
    printMat("change = ", change);

    Matrix4_4 mat4 = get3DMatrixFromLayout(Vector2{1920.0f, 1080.0f}, layoutPixel, pixelToMeter, 10.0f);
    anchorFromCenterToLeftTop(mat4);
    mat4 = mat4 * change;
    anchorFromLeftTopToCenter(mat4);
    printMat("anchorFromLeftTopToCenter = ", mat4);
  }

  return RUN_ALL_TESTS();
}
/*
atan 1.5708
atan 0.785398
lookAt 1 = (1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, -1.0000, -1.0000, -1.0000, 1.0000)
lookAt 2 = (1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, -1.0000, -1.0000, -0.0000, 1.0000)
lookAt 3 = (1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, -1.0000, -1.0000, -0.0000, 1.0000)
lookAt 4 = (1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, -1.0000, -1.0000, -1.0000, 1.0000)
lookAt 5 = (1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, -1.0000, -1.0000, -2.0000, 1.0000)
modelA 1 = (1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000)
modelA 2 = (1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.5000, 0.0000, 1.0000, 1.0000)
modelA 3 = (0.5000, 0.0000, 0.0000, 0.0000, 0.0000, 0.5000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, -0.2500, -0.5000, 1.0000, 1.0000)
modelA 4 = (0.0000, 0.0000, 0.5000, 0.0000, 0.0000, 0.5000, 0.0000, 0.0000, -1.0000, 0.0000, 0.0000, 0.0000, -2.0000, -1.0000, 0.2500, 1.0000)
modelA 5 = (0.0000, 0.0000, 0.5000, 0.0000, 0.0000, 0.5000, 0.0000, 0.0000, -1.0000, 0.0000, 0.0000, 0.0000, -2.5000, -0.5000, 1.2500, 1.0000)
modelA 6 = (0.0000, 0.0000, 0.5000, 0.0000, 0.0000, 0.5000, 0.0000, 0.0000, -1.0000, 0.0000, 0.0000, 0.0000, -3.0000, -0.7500, 1.5000, 1.0000)
outTrans = (-3.0000, -0.7500, 1.5000)
outQuat = (0.7071, 0.0000, -0.7071, 0.0000)
outScale = (0.5000, 0.5000, 1.0000)
outAngle = 1.5708
outAxis = (0.0000, -1.0000, 0.0000)
outEuler = (0.0000, -1.5708, 0.0000)
*/
