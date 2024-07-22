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

}

#endif //MATRIX_UTILS_H
