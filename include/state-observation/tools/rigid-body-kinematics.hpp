/**
 * \file      rigid-body-kinematics.hpp
 * \author    Mehdi Benallegue
 * \date       2013
 * \brief      Implements integrators for the kinematics, in terms or rotations
 *             and translations.
 *
 * \details
 *
 *
 */


#ifndef StATEOBSERVATIONRIGIDBODYKINEMATICS_H
#define StATEOBSERVATIONRIGIDBODYKINEMATICS_H

#include <state-observation/tools/definitions.hpp>
#include <state-observation/tools/miscellaneous-algorithms.hpp>

namespace stateObservation
{
  namespace kine
  {
    inline void integrateKinematics(Vector3 & position, const Vector3 & velocity, double dt);

    inline void integrateKinematics(Vector3 & position, Vector3 & velocity,
    const Vector3 & acceleration, double dt);

    inline void integrateKinematics( Matrix3 & orientation, const Vector3 & rotationVelocity,
    double dt);

    inline void integrateKinematics( Matrix3 & orientation, Vector3 & rotationVelocity,
    const Vector3 & rotationAcceleration, double dt);

    inline void integrateKinematics( Quaternion & orientation, const Vector3 & rotationVelocity,
    double dt);

    inline void integrateKinematics( Quaternion & orientation, Vector3 & rotationVelocity,
    const Vector3 & rotationAcceleration, double dt);


    ///integrates the position/orientation and their time derivatives, given the
    ///accelerations, and initial velocities and positions. The rotations are
    ///expressed by rotation matrix
    inline void integrateKinematics
    (Vector3 & position, Vector3 & velocity, const Vector3 & acceleration,
    Matrix3 & orientation, Vector3 & rotationVelocity,
    const Vector3 & rotationAcceleration, double dt);

    ///integrates the position/orientation and their time derivatives, given the
    ///accelerations, and initial velocities and positions. The orientations are
    ///expressed by quaternions
    inline void integrateKinematics
    (Vector3 & position, Vector3 & velocity, const Vector3 & acceleration,
    Quaternion & orientation, Vector3 & rotationVelocity,
    const Vector3 & rotationAcceleration, double dt);

    ///integrates the postition/orientation given the velocities
    inline void integrateKinematics(Vector3 & position, const Vector3 & velocity,
    Matrix3 & orientation, const Vector3 & rotationVelocity, double dt);

    ///integrates the postition/orientation given the velocities
    inline void integrateKinematics(Vector3 & position, const Vector3 & velocity,
    Quaternion & orientation, const Vector3 & rotationVelocity, double dt);




    /// Puts the orientation vector norm between 0 and Pi if it
    /// gets close to 2pi
    inline Vector regulateOrientationVector(const Vector3 & v );

    /// Transform the rotation vector into angle axis
    inline AngleAxis rotationVectorToAngleAxis(const Vector3 & v);

    /// Tranbsform the rotation vector into rotation matrix
    inline Matrix3 rotationVectorToRotationMatrix(const Vector3 & v);

    /// Tranbsform the rotation vector into quaternion
    inline Quaternion rotationVectorToQuaternion(const Vector3 & v);

    /// Tranbsform the rotation matrix into rotation vector
    inline Vector3 rotationMatrixToRotationVector(const Matrix3 & R);

    /// Tranbsform a quaternion into rotation vector
    inline Vector3 quaternionToRotationVector(const Quaternion &q);

    /// Tranbsform a quaternion into rotation vector
    inline Vector3 quaternionToRotationVector(const Vector4 &v);

    /// Transform the rotation matrix into roll pitch yaw
    ///(decompose R into Ry*Rp*Rr)
    inline Vector3 rotationMatrixToRollPitchYaw(const Matrix3 & R, Vector3 & v );

    inline Vector3 rotationMatrixToRollPitchYaw(const Matrix3 & R);

    /// Transform the roll pitch yaw into rotation matrix
    ///( R = Ry*Rp*Rr)
    inline Matrix3 rollPitchYawToRotationMatrix(double roll, double pitch, double yaw);

    inline Matrix3 rollPitchYawToRotationMatrix(Vector3 rpy);

    ///transform a 3d vector into a skew symmetric 3x3 matrix
    inline Matrix3 skewSymmetric(const Vector3 & v, Matrix3 & R);

    ///transform a 3d vector into a skew symmetric 3x3 matrix
    inline Matrix3 skewSymmetric(const Vector3 & v);

    ///transform a 3d vector into a squared skew symmetric 3x3 matrix
    inline Matrix3 skewSymmetric2(const Vector3 & v, Matrix3 & R);

    ///transform a 3d vector into a squared skew symmetric 3x3 matrix
    inline Matrix3 skewSymmetric2(const Vector3 & v);

    inline Matrix3 computeInertiaTensor(const Vector6 inputInertia, Matrix3& inertiaTensor);

    ///transforms a homogeneous matrix into 6d vector (position theta mu)
    inline Vector6 homogeneousMatrixToVector6(const Matrix4 & M);

    ///transforms a 6d vector (position theta mu) into a homogeneous matrix
    inline Matrix4 vector6ToHomogeneousMatrix(const Vector6 & v);

    ///Merge the roll and pitch from the tilt (R^T e_z) with the yaw from a rotation 
    ///matrix (minimizes the deviation of the x axis)
    inline Matrix3 mergeTiltWithYaw(const Vector3 & Rtez, const Matrix3 & R2);

    ///Merge the roll pitch from R1 with yaw from R2
    inline Matrix3 mergeRoll1Pitch1WithYaw2(const Matrix3 & R1, const Matrix3 & R2);

    ///transforms a rotation into translation given a constraint of a fixed point
    inline void fixedPointRotationToTranslation
    (const Matrix3 & R, const Vector3 & rotationVelocity,
     const Vector3 & rotationAcceleration, const Vector3 & fixedPoint,
     Vector3 & outputTranslation, Vector3 & outputLinearVelocity,
     Vector3 & outputLinearAcceleration);

    ///derivates a quaternion using finite difference to get a angular velocity vector
    inline Vector3 derivateRotationFD
    (const Quaternion & q1, const Quaternion & q2, double dt);

    ///derivates a quaternion using finite difference to get a angular velocity vector
    inline Vector3 derivateRotationFD
    (const Vector3 & o1, const Vector3 & o2, double dt);

    inline Vector6 derivateHomogeneousMatrixFD
    (const Matrix4 & m1, const Matrix4 & m2, double dt );

    inline Vector6 derivatePoseThetaUFD
    (const Vector6 & v1, const Vector6 & v2, double dt );

    ///uses the derivation to reconstruct the velocities and accelerations given
    ///trajectories in positions and orientations only
    inline IndexedVectorArray reconstructStateTrajectory
    (const IndexedVectorArray & positionOrientation,
     double dt);

    inline Vector invertState( const Vector & state);

    inline Matrix4 invertHomoMatrix (Matrix4 m);


    enum rotationType
    {
      matrix =0,
      rotationVector =1,
      quaternion = 2,
      angleaxis = 3
    };



    template <rotationType = rotationVector>
    struct indexes
    {
    };

    template<>
    struct indexes<rotationVector>
    {
      ///indexes of the different components of a vector of the kinematic state
      /// when the orientation is represented using a 3D rotation vector
      static const unsigned pos = 0;
      static const unsigned ori = 3;
      static const unsigned linVel = 6;
      static const unsigned angVel = 9;
      static const unsigned linAcc = 12;
      static const unsigned angAcc = 15;
      static const unsigned size = 18;
    };

    template<>
    struct indexes<quaternion>
    {
      ///indexes of the different components of a vector of the kinematic state
      /// when the orientation is represented using a quaternion
      static const unsigned pos = 0;
      static const unsigned ori = 3;
      static const unsigned linVel = 7;
      static const unsigned angVel = 10;
      static const unsigned linAcc = 13;
      static const unsigned angAcc = 16;
      static const unsigned size = 19;
    };


    ///relative tolereance to the square of quaternion norm.
    const double quatNormTol = 1e-6;








    struct kinematics

    {
      struct Flags
      {
        typedef unsigned char byte;
        static const byte position= BOOST_BINARY(0000001);
        static const byte quaternion= BOOST_BINARY(0000010);
        static const byte rotationMatrix= BOOST_BINARY(0000100);
        static const byte linVel= BOOST_BINARY(0001000);
        static const byte angVel= BOOST_BINARY(0010000);
        static const byte linAcc= BOOST_BINARY(0100000);
        static const byte angAcc= BOOST_BINARY(1000000);

        static const byte all= position | quaternion | rotationMatrix |
        linVel | angVel | linAcc | angAcc;
      };


      CheckedVector3 position;
      CheckedQuaternion orienation;
      CheckedMatrix3 rotationMat;

      CheckedVector3 linVel;
      CheckedVector3 angVel;

      CheckedVector3 linAcc;
      CheckedVector3 angAcc;

      kinematics integrate(double dt, Flags::byte=Flags::all);

      kinematics update(const kinematics & newValue, double dt, Flags::byte=Flags::all);

      kinematics synchronizeRotations();

      Vector3 rotateVector( const Vector3 & input);


      EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    };


  }
}

#include <state-observation/tools/rigid-body-kinematics.hxx>

#endif // StATEOBSERVATIONRIGIDBODYKINEMATICS_H
