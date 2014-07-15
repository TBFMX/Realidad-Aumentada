#include "GeometryTypes.hpp"

#ifndef MarkerBasedAR_CameraCalibration_h
#define MarkerBasedAR_CameraCalibration_h

/**
 * A camera calibraiton class that stores intrinsic matrix
 * and distorsion vector.
 */
class CameraCalibration
{
public:
  CameraCalibration();
  CameraCalibration(float fx, float fy, float cx, float cy);
  CameraCalibration(float fx, float fy, float cx, float cy, float distorsionCoeff[4]);
  
  void set4Params(float fx, float fy, float cx, float cy);
  void getMatrix34(float cparam[3][4]) const;

  const Matrix33& getIntrinsic() const;
  const Vector4&  getDistorsion() const;
  
private:
  Matrix33 m_intrinsic;
  Vector4  m_distorsion;
};

#endif
