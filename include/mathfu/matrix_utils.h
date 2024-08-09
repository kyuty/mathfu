//
// Created by wangdong on 2024/7/19.
//

#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include "constants.h"
#include "vector.h"
#include "quaternion.h"
#include "matrix.h"

namespace mathfu {

  template <class T>
  inline void offsetLeft(Matrix<T, 4, 4> &m, const Vector<T, 3>& v) {
    m.data_[3][0] += v[0];
    m.data_[3][1] -= v[1];
    m.data_[3][2] += v[2];
  }

  template <class T>
  inline void offsetLeft(Matrix<T, 4, 4> &m, const Vector<T, 3>& offsetPixel, const Vector<T, 2>& viewSize, float pixelToMeter) {
    m.data_[3][0] += offsetPixel[0] / viewSize[0];
    m.data_[3][1] -= offsetPixel[1] / viewSize[1];
    m.data_[3][2] += offsetPixel[2] * pixelToMeter;
  }

  template <class T>
  inline void scaleLeft(Matrix<T, 4, 4> &m, const Vector<T, 3>& scale, const Vector<T, 3>& anchor) {
    Matrix<T, 4, 4> anchorMat = Matrix<T, 4, 4>::FromTranslationVector(Vector<T, 3>(anchor.x, -anchor.y, anchor.z));
    Matrix<T, 4, 4> unAnchorMat = Matrix<T, 4, 4>::FromTranslationVector(Vector<T, 3>(-anchor.x, anchor.y, -anchor.z));
    Matrix<T, 4, 4> scaleMat = Matrix<T, 4, 4>::FromScaleVector(scale);
    m = anchorMat * scaleMat * m * unAnchorMat;
  }

  template <class T>
  inline void rotateLeft(Matrix<T, 4, 4> &m, T degree, const Vector<T, 3>& axis, const Vector<T, 3>& anchor) {
    Matrix<T, 4, 4> anchorMat = Matrix<T, 4, 4>::FromTranslationVector(Vector<T, 3>(anchor.x, -anchor.y, anchor.z));
    Matrix<T, 4, 4> unAnchorMat = Matrix<T, 4, 4>::FromTranslationVector(Vector<T, 3>(-anchor.x, anchor.y, -anchor.z));
    Matrix<T, 4, 4> rotMat = Quaternion<float>::FromAngleAxis(degree * kDegreesToRadians, axis).ToMatrix4();
    m = anchorMat * rotMat * m * unAnchorMat;
  }

  template <class T>
  inline void printMat(const std::string &s, Matrix<T, 4, 4> &m){
    std::stringstream ss;
    ss.flags(std::ios::fixed);
    ss.precision(4);
    ss << m;
    std::cout << s << ss.str() << std::endl;
  }

  template <class T>
  inline void printMat(const std::string &s, Matrix<T, 3> &m){
    std::stringstream ss;
    ss.flags(std::ios::fixed);
    ss.precision(4);
    ss << m;
    std::cout << s << ss.str() << std::endl;
  }

  template <class T>
  inline void printVector3(const std::string &s, const Vector<T, 3>& v){
    std::stringstream ss;
    ss.flags(std::ios::fixed);
    ss.precision(4);
    ss << v;
    std::cout << s << ss.str() << std::endl;
  }

  template <class T>
  inline void printQuat(const std::string &s, const Quaternion<T>& v){
    std::stringstream ss;
    ss.flags(std::ios::fixed);
    ss.precision(4);
    ss << v;
    std::cout << s << ss.str() << std::endl;
  }

  template <class T>
  inline void decompose(const Matrix<T, 4, 4> &m, Vector<T, 3> &outTrans, Quaternion<T> &outQuat, Vector<T, 3> &outScale) {
    outScale = m.ScaleVector3D();
    //printVector3("outScale = ", outScale);

    outTrans = m.TranslationVector3D();
    //printVector3("outTrans = ", outTrans);

    Matrix<T, 3> outRotMat = Matrix<T, 4, 4>::ToRotationMatrix(m * Matrix<T, 4, 4>::FromScaleVector(static_cast<T>(1) / outScale));
    //printMat("outRotMat = ", outRotMat);
    outQuat = Quaternion<T>::FromMatrix(outRotMat);
    //printQuat("outQuat = ", outQuat);

    // {
    //   T outAngle;
    //   Vector<T, 3> outAxis;
    //   outQuat.ToAngleAxis(&outAngle, &outAxis);
    //   //std::cout << "outAngle = " << outAngle << std::endl;
    //   //printVector3("outAxis = ", outAxis);
    //
    //   Vector<T, 3> outEuler = outQuat.ToEulerAngles();
    //   //printVector3("outEuler = ", outEuler);
    // }
  }

  /**
   *
   * @tparam T
   * @param layerPixelSize .xy = w h
   * @param layoutPixel .xyzw == left top right bottom
   * @param pixelToMeters
   * @return
   */
  template <class T>
  inline Matrix<T, 4, 4> get3DMatrixFromLayout(Vector<T, 2> layerPixelSize, Vector<T, 4> layoutPixel, T pixelToMeters, T zPixel) {
    Matrix<T, 4, 4> m = Matrix<T, 4, 4>::Identity();
    auto baseScaleX = layerPixelSize.x * pixelToMeters;
    auto baseScaleY = layerPixelSize.y * pixelToMeters;
    Vector<T, 3> scale;
    scale.x = 1.0f * baseScaleX * (layoutPixel.z - layoutPixel.x) / layerPixelSize.x;
    scale.y = 1.0f * baseScaleY * (layoutPixel.w - layoutPixel.y) / layerPixelSize.y;
    scale.z = 1.0f;
    Vector<T, 3> trans;
    trans.x = ((layoutPixel.x + layoutPixel.z) / 2.0f - layerPixelSize.x / 2.0f) * pixelToMeters;
    trans.y = (-(layoutPixel.y + layoutPixel.w) / 2.0f + layerPixelSize.y / 2.0f) * pixelToMeters;
    trans.z = zPixel * pixelToMeters;
    Matrix<T, 4> transMatrix = Matrix<T, 4>::FromTranslationVector(trans);
    Matrix<T, 4> scaleMatrix = Matrix<T, 4>::FromScaleVector(scale);
    m = transMatrix * scaleMatrix;
    return m;
  }

  template <class T>
  inline void anchorFromCenterToLeftTop(Matrix<T, 4, 4> &m) {
    Vector<T, 3> trans;
    trans.x = -1.0f / 2.0f;
    trans.y = 1.0f / 2.0f;
    trans.z = 0.0f;
    m = m * Matrix<T, 4>::FromTranslationVector(trans);
  }

  template <class T>
  inline void anchorFromLeftTopToCenter(Matrix<T, 4, 4> &m) {
    Vector<T, 3> trans;
    trans.x = 1.0f / 2.0f;
    trans.y = -1.0f / 2.0f;
    trans.z = 0.0f;
    m = m * Matrix<T, 4>::FromTranslationVector(trans);
  }


}

#endif //MATRIX_UTILS_H
