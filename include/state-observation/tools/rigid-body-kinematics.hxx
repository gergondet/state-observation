namespace stateObservation
{
  namespace kine

  {


    inline void integrateKinematics(Vector3 & position, const Vector3 & velocity, double dt)
    {
      position.noalias() +=  dt * velocity;
    }

    inline void integrateKinematics(Vector3 & position, Vector3 & velocity,
                                    const Vector3 & acceleration, double dt)
    {
      position.noalias() +=  dt * velocity + 0.5 * dt * dt * acceleration;
      velocity.noalias() +=  dt * acceleration;
    }

    inline void integrateKinematics( Matrix3 & orientation, const Vector3 & rotationVelocityVector,
                                     double dt)
    {
      orientation =  kine::rotationVectorToRotationMatrix
                                (rotationVelocityVector* dt) * orientation;
    }


    inline void integrateKinematics( Matrix3 & orientation, Vector3 & rotationVelocityVector,
                                     const Vector3 & rotationVelocityVectorRate, double dt)
    {
      orientation = kine::rotationVectorToRotationMatrix (rotationVelocityVector*dt + 0.5 * dt * dt*rotationVelocityVectorRate)
                    * orientation;

      rotationVelocityVector += dt * rotationVelocityVectorRate;

    }

    inline void integrateKinematics( Quaternion & orientation, const Vector3 & rotationVelocityVector,
                                     double dt)
    {
      orientation =  kine::rotationVectorToQuaternion
                                (rotationVelocityVector* dt )
                    * orientation;
    }

    inline void integrateKinematics( Quaternion & orientation, Vector3 & rotationVelocityVector,
                                     const Vector3 & rotationVelocityVectorRate, double dt)
    {
      orientation = kine::rotationVectorToQuaternion
                                (rotationVelocityVector* dt + 0.5  * dt * dt*rotationVelocityVectorRate )
                    * orientation;

      rotationVelocityVector.noalias()+= dt * rotationVelocityVectorRate;
    }

    inline void integrateKinematics
    (Vector3 & position, Vector3 & velocity, const Vector3 & acceleration,
     Quaternion & orientation, Vector3 & rotationVelocityVector,
     const Vector3 & rotationVelocityVectorRate, double dt)
    {
      integrateKinematics(position,velocity,acceleration,dt);
      integrateKinematics(orientation,rotationVelocityVector,rotationVelocityVectorRate,dt);
    }

    inline void integrateKinematics
    (Vector3 & position, Vector3 & velocity, const Vector3 & acceleration,
     Matrix3 & orientation, Vector3 & rotationVelocityVector,
     const Vector3 & rotationVelocityVectorRate, double dt)
    {
      integrateKinematics(position,velocity,acceleration,dt);
      integrateKinematics(orientation,rotationVelocityVector,rotationVelocityVectorRate,dt);
    }

    inline void integrateKinematics(Vector3 & position,
        const Vector3 & velocity, Matrix3 & orientation,
        const Vector3 & rotationVelocity, double dt)
    {
      integrateKinematics(position,velocity,dt);
      integrateKinematics(orientation,rotationVelocity,dt);
    }

    inline void integrateKinematics(Vector3 & position,
        const Vector3 & velocity, Quaternion & orientation,
        const Vector3 & rotationVelocity, double dt)
    {
      integrateKinematics(position,velocity,dt);
      integrateKinematics(orientation,rotationVelocity,dt);
    }

        /// Puts the orientation vector norm between 0 and Pi if it
    /// gets close to 2pi
    inline Vector regulateOrientationVector(const Vector3 & v )
    {
      double n2=v.squaredNorm();
      if (n2 > (3./2.) * M_PI * (3./2.) * M_PI )
      {
        double n=sqrt(n2);
        unsigned k =  unsigned(ceil((n - M_PI) / (2*M_PI))) ;
        return (v / n) * ( n - k*2*M_PI );
      }
      else
        return v;
    }

    /// Transform the rotation vector into angle axis
    inline AngleAxis rotationVectorToAngleAxis(const Vector3 & v)
    {
      double angle(v.squaredNorm());
      if (angle > cst::epsilonAngle * cst::epsilonAngle)
      {
        angle=sqrt(angle);
        return AngleAxis(angle, v/angle);
      }
      else
      {
        return AngleAxis(0.0, Vector3::UnitZ());
      }
    }

    /// Tranbsform the rotation vector into rotation matrix
    inline Matrix3 rotationVectorToRotationMatrix(const Vector3 & v)
    {
      return (rotationVectorToAngleAxis(Vector3(v))).toRotationMatrix();
    }

    /// Tranbsform the rotation vector into rotation matrix
    inline Quaternion rotationVectorToQuaternion(const Vector3 & v)
    {
      return Quaternion(rotationVectorToAngleAxis(Vector3(v)));
    }

    /// Tranbsform the rotation matrix into rotation vector
    inline Vector3 rotationMatrixToRotationVector(const Matrix3 & R)
    {
      AngleAxis a(R);
      Vector3 v(a.axis());
      v.noalias()=a.angle()*v;
      return a.angle()*a.axis();
    }

    /// Tranbsform a quaternion into rotation vector
    inline Vector3 quaternionToRotationVector(const Quaternion &q)
    {
      AngleAxis aa(q);

      return aa.angle()*aa.axis();
    }

    /// Tranbsform a quaternion into rotation vector
    inline Vector3 quaternionToRotationVector(const Vector4 &v)
    {
      Quaternion q(v);
      AngleAxis aa(q);

      return aa.angle()*aa.axis();
    }

    /// Transform the rotation matrix into roll pitch yaw
    ///(decompose R into Ry*Rp*Rr)
    inline Vector3 rotationMatrixToRollPitchYaw(const Matrix3 & R, Vector3 & v )
    {
      /// source http://planning.cs.uiuc.edu/node102.html
      /// and http://planning.cs.uiuc.edu/node103.html

//      v<<atan2(R(2,1),R(2,2)),
//      atan2(-R(2,0),sqrt(tools::square(R(2,1))+tools::square(R(2,2)))),
//      atan2(R(1,0),R(0,0));
//      return v;

        return v=rotationMatrixToRollPitchYaw(R);
    }

    inline Vector3 rotationMatrixToRollPitchYaw(const Matrix3 & R)
    {
      //      v<<atan2(R(2,1),R(2,2)),
//      atan2(-R(2,0),sqrt(tools::square(R(2,1))+tools::square(R(2,2)))),
//      atan2(R(1,0),R(0,0));
//      return v;
        return R.eulerAngles(2,1,0).reverse();
    }

    /// Transform the roll pitch yaw into rotation matrix
    ///( R = Ry*Rp*Rr)
    inline Matrix3 rollPitchYawToRotationMatrix(double roll, double pitch, double yaw)
    {
      AngleAxis rollAngle(roll, Eigen::Vector3d::UnitX());
      AngleAxis pitchAngle(pitch, Eigen::Vector3d::UnitY());
      AngleAxis yawAngle(yaw, Eigen::Vector3d::UnitZ());

      Quaternion q;

      q=yawAngle;
      q=q*pitchAngle;
      q=q*rollAngle;

      return q.toRotationMatrix() ;
    }

    inline Matrix3 rollPitchYawToRotationMatrix(Vector3 rpy)
    {
      return rollPitchYawToRotationMatrix(rpy[0],rpy[1],rpy[2]);
    }

    /// Transform the roll pitch yaw into rotation matrix
    ///( R = Ry*Rp*Rr)
    inline Quaternion rollPitchYawToQuaternion(double roll, double pitch, double yaw)
    {
      AngleAxis rollAngle(roll, Eigen::Vector3d::UnitX());
      AngleAxis pitchAngle(pitch, Eigen::Vector3d::UnitY());
      AngleAxis yawAngle(yaw, Eigen::Vector3d::UnitZ());

      Quaternion q;

      q=yawAngle;
      q=q*pitchAngle;
      q=q*rollAngle;

      return q;
    }

    inline Quaternion rollPitchYawToQuaternion(Vector3 rpy)
    {
      return rollPitchYawToQuaternion(rpy[0],rpy[1],rpy[2]);
    }





    ///transform a 3d vector into a skew symmetric 3x3 matrix
    inline Matrix3 skewSymmetric(const Vector3 & v, Matrix3 & R)
    {
      //R <<     0, -v[2],  v[1],
      //      v[2],     0, -v[0],
      //     -v[1],  v[0],     0;

      R(0,0)=  R(1,1) = R(2,2) = 0.;
      R(0,1)= -( R(1,0)= v[2] );
      R(2,0)= -( R(0,2)= v[1] );
      R(1,2)= -( R(2,1)= v[0] );

      return R;
    }


    ///transform a 3d vector into a skew symmetric 3x3 matrix
    inline Matrix3 skewSymmetric(const Vector3 & v)
    {
      Matrix3 R;

      return skewSymmetric(v,R);
    }



    ///transform a 3d vector into a squared skew symmetric 3x3 matrix
    inline Matrix3 skewSymmetric2(const Vector3 & v, Matrix3 & R)
    {
      R.noalias()= v * v.transpose();

      double n = R.trace();

      R(0,0) -= n;
      R(1,1) -= n;
      R(2,2) -= n;

      return R;
    }

    ///transform a 3d vector into a squared skew symmetric 3x3 matrix
    inline Matrix3 skewSymmetric2(const Vector3 & v)
    {
      Matrix3 R;

      return skewSymmetric2(v,R);
    }

    ///transforms a homogeneous matrix into 6d vector (position theta mu)
    inline Vector6 homogeneousMatrixToVector6(const Matrix4 & M)
    {
      Vector6 v;
      AngleAxis a (AngleAxis(Matrix3(M.block(0,0,3,3))));

      v.head(3) = M.block(0,3,3,1);
      v.tail(3) = a.angle() * a.axis();

      return v;
    }

    ///transforms a 6d vector (position theta mu) into a homogeneous matrix
    inline Matrix4 vector6ToHomogeneousMatrix(const Vector6 & v)
    {
      Matrix4 M(Matrix4::Identity());
      M.block(0,0,3,3) = rotationVectorToAngleAxis(Vector3(v.tail(3)))
                         .toRotationMatrix();
      M.block(0,3,3,1) = v.head(3);
      return M;
    }

    ///transforms a rotation into translation given a constraint of a fixed point
    inline void fixedPointRotationToTranslation
    (const Matrix3 & R, const Vector3 & rotationVelocity,
     const Vector3 & rotationAcceleration, const Vector3 & fixedPoint,
     Vector3 & outputTranslation, Vector3 & outputLinearVelocity,
     Vector3 & outputLinearAcceleration)
    {
      Matrix3 omega_x = skewSymmetric(rotationVelocity);
      outputTranslation = fixedPoint - R * fixedPoint;
      outputLinearVelocity = -omega_x * R * fixedPoint;
      outputLinearAcceleration =-(skewSymmetric(rotationAcceleration) + tools::square(omega_x))
                                * R * fixedPoint;
    }

    ///Computes the multiplicative Jacobians for
    ///Kalman filtering for examole
    inline void derivateRotationMultiplicative
        (const Vector3 & deltaR, Matrix3 & dRdR, Matrix3& dRddeltaR)
    {
      dRdR = rotationVectorToRotationMatrix(deltaR);
      double d = deltaR.norm();

      if (d<cst::epsilonAngle)
      {
        dRddeltaR = Matrix3::Identity();
      }
      else
      {
        double cosd = cos(d);
        double sind = sin(d);

        double d2=d*d;
        double d3=d2*d;

        dRddeltaR.noalias()= deltaR*deltaR.transpose() * (d-sind)/d3
                              + Matrix3::Identity() * sind / d
                              + skewSymmetric(deltaR) * (1-cosd)/d2;
      }
    }

    ///Computes the multiplicative Jacobians for a vector expressed in
    ///local frame
    inline Matrix3 derivateRtvMultiplicative(const Matrix3 & R, const Vector3 &v)
    {
      return -R.transpose()*skewSymmetric(v);
    }




    ///derivates a quaternion using finite difference to get a angular velocity vector
    inline Vector3 derivateRotationFD
    (const Quaternion & q1, const Quaternion & q2, double dt)
    {
      AngleAxis aa (q2 * q1.conjugate());

      return (aa.angle()/dt)*aa.axis();
    }

    ///derivates a quaternion using finite difference to get a angular velocity vector
    inline Vector3 derivateRotationFD
    (const Vector3 & o1, const Vector3 & o2, double dt)
    {
      Quaternion q1(rotationVectorToAngleAxis(o1));
      Quaternion q2(rotationVectorToAngleAxis(o2));

      return derivateRotationFD(q1, q2, dt);
    }

    inline Vector6 derivateHomogeneousMatrixFD
    (const Matrix4 & m1, const Matrix4 & m2, double dt )
    {
      Vector6 out;

      Matrix3 r1 = m1.block(0,0,3,3);
      Matrix3 r2 = m2.block(0,0,3,3);

      AngleAxis aa (r2 * r1.transpose());

      out.tail(3) = (aa.angle()/dt)*aa.axis();

      out.head(3) = (m2.block(0,3,3,1) - m1.block(0,3,3,1))/dt;

      return out;
    }

    inline Vector6 derivatePoseThetaUFD
    (const Vector6 & v1, const Vector6 & v2, double dt )
    {
      Vector6 out;

      out.tail(3) = derivateRotationFD(v1.tail(3), v2.tail(3), dt);

      out.head(3) = (v2.head(3) - v1.head(3))/dt;

      return out;
    }



    ///uses the derivation to reconstruct the velocities and accelerations given
    ///trajectories in positions and orientations only
    inline IndexedVectorArray reconstructStateTrajectory
        (const IndexedVectorArray & positionOrientation,
         double dt)
    {
      typedef kine::indexes<kine::rotationVector> indexes;

      Vector r(Vector::Zero(18,1));

      const IndexedVectorArray & po= positionOrientation;

      TimeIndex i0=positionOrientation.getFirstIndex();
      TimeIndex i1=positionOrientation.getNextIndex();

      IndexedVectorArray a;
      a.setValue(r,i0);
      a.resize(po.size(),r);

      for (TimeIndex i=i0; i<i1; ++i)
      {
        Vector poi = po[i];

        r.segment(indexes::pos,3) = poi.head(3);
        r.segment(indexes::ori,3) = poi.tail(3);
        a.setValue(r,i);
      }

      for (TimeIndex i=i0; i<i1-1; ++i)
      {
        r = a[i];

        Vector poi = po[i];
        Vector poi1 = po[i+1];

        r.segment(indexes::linVel,3)  = tools::derivate(Vector3(poi.head(3)),
                                               Vector3(poi1.head(3)), dt);
        r.segment(indexes::angVel,3) = derivateRotationFD(
                                Quaternion(rotationVectorToAngleAxis (poi.tail(3))),
                                Quaternion(rotationVectorToAngleAxis (poi1.tail(3))),
                                dt);

        a.setValue(r,i);

      }

      for (TimeIndex i=i0; i<i1-2; ++i)
      {
        r = a[i];
        Vector r2 = a[i+1];

        r.segment(indexes::linAcc,3) = tools::derivate(Vector3(r.segment(indexes::linVel,3)),
                                              Vector3(r2.segment(indexes::linVel,3)), dt);
        r.segment(indexes::angAcc,3) = tools::derivate(Vector3(r.segment(indexes::angVel,3)),
                                              Vector3(r2.segment(indexes::angVel,3)), dt);

        a.setValue(r,i);

      }

      return a;
    }

    inline Vector invertState( const Vector & state)
    {
      typedef kine::indexes<kine::rotationVector> indexes;
      Matrix3 r2 = (rotationVectorToAngleAxis( - state.segment(indexes::ori,3))).
                   toRotationMatrix();//inverse
      Vector3 omega1 = state.segment(indexes::angVel,3);
      Vector3 omega1dot = state.segment(indexes::angAcc,3);
      Matrix3 omega1x = skewSymmetric(omega1);
      Matrix3 omega1dotx = skewSymmetric(omega1dot);

      Vector3 t1 = state.segment(indexes::pos,3);
      Vector3 t1dot = state.segment(indexes::linVel,3);
      Vector3 t1dotdot = state.segment(indexes::linAcc,3);

      Vector state2(Vector::Zero(18,1));
      state2.segment(indexes::pos,3)= - r2 * t1 ;   //t2
      state2.segment(indexes::linVel,3)= r2 * ( omega1x * t1 - t1dot); //t2dot
      state2.segment(indexes::linAcc,3)=  r2 * ( omega1x * (2 * t1dot - omega1x * t1)
                                              - t1dotdot + omega1dotx * t1); //t2dotdot
      state2.segment(indexes::ori,3)= -state.segment(indexes::ori,3);   //thetaU2
      state2.segment(indexes::angVel,3)= -r2 * omega1; //omega2
      state2.segment(indexes::angAcc,3)=  r2 * (omega1x * omega1 - omega1dot); //omega2dot
      return state2;

    }

    inline Matrix4 invertHomoMatrix (Matrix4 m)
    {
      Matrix4 m2(Matrix4::Identity());
      Matrix3 rt = m.block(0,0,3,3).transpose();
      m2.block(0,0,3,3) = rt;
      m2.block(0,3,3,1) = - rt * m.block(0,3,3,1);
      return m2;
    }

    inline Orientation::Orientation()
    {}

    inline Orientation::Orientation(const Vector3& v):
    q_(rotationVectorToQuaternion(v))
    {
    }

    inline Orientation::Orientation(const Quaternion& q):
      q_(q)
    {
    }

    inline Orientation::Orientation(const Matrix3& m):
      m_(m)
    {
    }

    inline Orientation::Orientation(const AngleAxis& aa):
      q_(Quaternion(aa))
    {
    }

    inline Orientation::Orientation(const double& roll, const double & pitch, const double & yaw):
      q_(kine::rollPitchYawToQuaternion(roll,pitch,yaw))
    {
    }

    inline Orientation & Orientation::operator=(const Vector3& v)
    {
      m_.reset();
      q_=rotationVectorToQuaternion(v);
      return *this;
    }

    inline Orientation & Orientation::operator=(const Quaternion& q)
    {
      m_.reset();
      q_=q;
      return *this;
    }

    inline Orientation & Orientation::operator=(const Matrix3& m)
    {
      q_.reset();
      m_=m;
      return *this;
    }

    inline Orientation & Orientation::operator=(const AngleAxis& aa)
    {
      m_.reset();
      q_=Quaternion(aa);
      return *this;
    }

    inline Orientation & Orientation::setValue(const Quaternion& q,const Matrix3&m)
    {
      q_=q;
      m_=m;
      return *this;
    }

    inline Orientation::operator Matrix3()
    {
      return getMatrixRef();
    }

    inline Orientation::operator Quaternion()
    {
      return getQuaternionRef();
    }

    inline Orientation::operator Matrix3() const
    {
      check_();
      if (!isMatrixSet())
      {
        return q_().toRotationMatrix();
      }
      else
      {
        return m_();
      }

    }

    inline Orientation::operator Quaternion() const
    {
      check_();
      if (isQuaternionSet() )
      {
        return q_();
      }
      else
      {
        return Quaternion(m_());
      }
    }

    inline Vector3 Orientation::toRotationVector() const
    {
      check_();
      if (isQuaternionSet() )
      {
        return kine::quaternionToRotationVector(q_());
      }
      else
      {
        return kine::rotationMatrixToRotationVector(m_());
      }
    }

    inline Vector3 Orientation::toRollPitchYaw() const
    {
      check_();
      if (isMatrixSet())
      {
        return kine::rotationMatrixToRollPitchYaw(m_());
      }
      else
      {
        return kine::rotationMatrixToRollPitchYaw(q_().toRotationMatrix());
      }
    }

    inline Vector3 Orientation::toRollPitchYaw()
    {
      check_();
      if (!isMatrixSet())
      {
        quaternionToMatrix_();
      }
      else
        return kine::rotationMatrixToRollPitchYaw(m_());

    }

    inline AngleAxis Orientation::toAngleAxis() const
    {
      check_();
      if (isMatrixSet())
      {
        return AngleAxis(m_());
      }
      else
      {
        return AngleAxis(q_());
      }
    }

    inline const Matrix3& Orientation::getMatrixRef()
    {
      check_();
      if (!isMatrixSet())
      {
        quaternionToMatrix_();
      }
      return m_;
    }

    inline const Quaternion& Orientation::getQuaternionRef()
    {
      check_();
      if (!isQuaternionSet() )
      {
        matrixToQuaternion_();
      }
      return q_;
    }

    inline const Matrix3& Orientation::getMatrixRef() const
    {
      BOOST_ASSERT (isMatrixSet() && "The matrix is not set");
      return m_;
    }

    inline const Quaternion& Orientation::getQuaternionRef() const
    {
      BOOST_ASSERT (isQuaternionSet() && "The quaternion is not set");
      return q_;
    }

    inline Orientation Orientation::operator*( Orientation& R2)
    {
      check_();
      R2.check_();
      if (isQuaternionSet()  && R2.isQuaternionSet() )
      {
        if (isMatrixSet() && R2.isMatrixSet())
        {
          return Orientation(Matrix3(m_()*R2.m_()));
        }
        else
        {
          return Orientation(q_()*R2.q_(),m_()*R2.m_());
        }
      }
      else if (isQuaternionSet() )
      {
        return Orientation(Matrix3(quaternionToMatrix_()*R2.m_()));
      }
      else
      {
        return (Matrix3(m_()*R2.quaternionToMatrix_()));
      }
    }

    inline Orientation Orientation::operator*(const Orientation& R2)
    {
      check_();
      R2.check_();
      if (isQuaternionSet()  && R2.isQuaternionSet() )
      {
        if (isMatrixSet() && R2.isMatrixSet())
        {
          return Orientation(Matrix3(m_()*R2.m_()));
        }
        else
        {
          return Orientation(q_()*R2.q_(),m_()*R2.m_());
        }
      }
      else if (isQuaternionSet() )
      {
        return Orientation(Matrix3(quaternionToMatrix_()*R2.m_()));
      }
      else
      {
        return (Matrix3(m_()*R2.q_().toRotationMatrix()));
      }
    }

    inline Orientation Orientation::operator*( Orientation& R2) const
    {
      check_();
      R2.check_();
      if (isQuaternionSet()  && R2.isQuaternionSet() )
      {
        if (isMatrixSet() && R2.isMatrixSet())
        {
          return Orientation(Matrix3(m_()*R2.m_()));
        }
        else
        {
          return Orientation(q_()*R2.q_(),m_()*R2.m_());
        }
      }
      else if (isQuaternionSet() )
      {
        return Orientation(Matrix3(q_().toRotationMatrix()*R2.m_()));
      }
      else
      {
        return (Matrix3(m_()*R2.quaternionToMatrix_()));
      }
    }


    inline Orientation Orientation::operator*(const Orientation& R2) const
    {
      check_();
      R2.check_();
      if (isQuaternionSet()  && R2.isQuaternionSet() )
      {
        if (isMatrixSet() && R2.isMatrixSet())
        {
          return Orientation(Matrix3(m_()*R2.m_()));
        }
        else
        {
          return Orientation(q_()*R2.q_(),m_()*R2.m_());
        }
      }
      else if (q_.isSet())
      {
        return Orientation(Matrix3(q_().toRotationMatrix()*R2.m_()));
      }
      else
      {
        return (Matrix3(m_()*R2.q_().toRotationMatrix()));
      }
    }

    inline Orientation Orientation::inverse() const
    {
      check_();
      if (q_.isSet())
      {
        if (isMatrixSet())
        {
          return Orientation(q_().conjugate(), m_().transpose());
        }
        else
        {
          return Orientation(q_().conjugate());
        }
      }
      else
      {
        return Orientation(Matrix3(m_().transpose()));
      }
    }

    inline const Orientation & Orientation::integrate(Vector3 dt_x_omega)
    {
      check_();
      if (q_.isSet())
      {
        if (isMatrixSet())
        {
          Quaternion q=kine::rotationVectorToQuaternion(dt_x_omega );
          q_ = q* q_();
          m_ = q.toRotationMatrix()*m_();
        }
        else
        {
          q_ = kine::rotationVectorToQuaternion(dt_x_omega )* q_();
        }
      }
      else
      {
        m_ = kine::rotationVectorToRotationMatrix(dt_x_omega )*m_();
      }
      return *this;
    }

    inline Vector3 Orientation::differentiate(Orientation R_k1) const
    {
      check_();
      return (inverse()*R_k1).toRotationVector();
    }

    inline bool Orientation::isSet() const
    {
      return (isMatrixSet() || q_.isSet());
    }

    inline void Orientation::synchronize()
    {
      check_();
      if (isMatrixSet())
      {
        if (!isQuaternionSet())
        {
          matrixToQuaternion_();
        }
      }
      else
      {
        if (isQuaternionSet())
        {
          quaternionToMatrix_();
        }
      }
    }

    inline void Orientation::reset()
    {
      q_.reset();
      m_.reset();
    }


    inline bool Orientation::isMatrixSet() const
    {
      return (m_.isSet());
    }

    inline bool Orientation::isQuaternionSet() const
    {
      return ( q_.isSet());
    }


    inline Vector3 Orientation::operator*( const Vector3& v)
    {
      check_();
      if (!isMatrixSet())
      {
        quaternionToMatrix_();
        return m_()*v;

      }
      else
      {
        return m_()*v;
      }

    }

    inline Vector3 Orientation::operator*( const Vector3& v) const
    {
      check_();
      if (m_.isSet())
      {
        return m_()*v;
      }
      else
      {
        return q_()*v;
      }
    }

    inline CheckedMatrix3 & Orientation::getMatrixRefUnsafe()
    {
      return m_;
    }

    inline CheckedQuaternion & Orientation::getQuaternionRefUnsafe()
    {
      return q_;
    }

    inline void Orientation::check_() const
    {
      BOOST_ASSERT((isQuaternionSet()  || isMatrixSet()) && "The orientation is not initialized");
    }

    inline Matrix3 & Orientation::quaternionToMatrix_()
    {
      return m_=q_().toRotationMatrix();
    }

    inline Quaternion & Orientation::matrixToQuaternion_()
    {
      return q_=Quaternion(m_());
    }




    /// -------------------Kinematics structure implementation--------------
    inline Kinematics::Kinematics(const Vector & v, Kinematics::Flags::Byte flags)
    {
      fromVector (v,flags);
    }

    inline Kinematics Kinematics::fromVector(const Vector & v, Kinematics::Flags::Byte flags)
    {
      int index=0;

      bool flagPos = flags & Flags::position;
      bool flagLinVel = flags & Flags::linVel;
      bool flagLinAcc = flags & Flags::linAcc;
      bool flagOri = flags & Flags::orientation;
      bool flagAngVel = flags & Flags::angVel;
      bool flagAngAcc = flags & Flags::angAcc;

      if (flagPos )
      {
        BOOST_ASSERT (v.size()>index+3 && "The kinematics vector size is incorrect (loading position)");
        if (v.size()>index+3)
        {
          position = v.segment<3>(index);
          index+=3;
        }
        
      }

      if (flagOri )
      {
        BOOST_ASSERT (v.size()>index+4 && "The kinematics vector size is incorrect (loading orientaTion)");
        if (v.size()>index+4)
        {
          orientation = Quaternion(v.segment<4>(index));
          index+=4;
        }       
      }

      if (flagLinVel )
      {
        BOOST_ASSERT (v.size()>index+3 && "The kinematics vector size is incorrect (loading linear velocity)");
        if (v.size()>index+3)
        {
          linVel = v.segment<3>(index);
          index+=3;
        }        
      }

      if (v.size()>index+3 && flagAngVel )
      {
        BOOST_ASSERT (v.size()>index+3 && "The kinematics vector size is incorrect (loading angular velocity)");
        if (v.size()>index+3)
        {
          angVel = v.segment<3>(index);
          index+=3;
        }

      }

      if (v.size()>index+3 && flagLinAcc )
      {
        BOOST_ASSERT (v.size()>index+3 && "The kinematics vector size is incorrect (loading linear acceleration)");
        if (v.size()>index+3)
        {
          linAcc = v.segment<3>(index);
          index+=3;
        }       
      }

      if (v.size()>index+3 && flagAngAcc )
      {
        BOOST_ASSERT (v.size()>index+3 && "The kinematics vector size is incorrect (loading angular acceleration)");
        if (v.size()>index+3)
        {
          angAcc = v.segment<3>(index);
          index+=3;
        }
      }

    }

    inline const Kinematics & Kinematics::integrate(double dt)
    {
      if (angVel.isSet())
      {
        if (angAcc.isSet())
        {
            if (orientation.isSet())
            {
              orientation.integrate(angVel()*dt+angAcc()*dt*dt/2);
            }
            angVel()+=angAcc()*dt;
        }
        else
        {
          if (orientation.isSet())
          {
            orientation.integrate(angVel()*dt);
          }
        }
      }

      if (linVel.isSet())
      {
        if (linAcc.isSet())
        {
          if (position.isSet())
          {
            position()+=linVel()*dt+linAcc()*dt*dt/2;
          }
          linVel()+=linAcc()*dt;
        }
        else
        {
          if (position.isSet())
          {
            position()+=linVel()*dt;
          }

        }
      }

      return *this;


    }

    inline const Kinematics & Kinematics::update(const Kinematics & newValue, double dt, Flags::byte flags)
    {


      bool flagPos = flags|Flags::position;
      bool flagLinVel = flags|Flags::linVel;
      bool flagLinAcc = flags|Flags::linAcc;
      bool flagOri = flags|Flags::orientation;
      bool flagAngVel = flags|Flags::angVel;
      bool flagAngAcc = flags|Flags::angAcc;

      {
        CheckedVector3 curPos,curLinVel;

        if (flagPos)
        {
          if (flagLinVel  && !newValue.linVel.isSet() &&  position.isSet() && newValue.position.isSet())
          {
            curPos=position;
          }
          if (newValue.position.isSet())
          {
            position=newValue.position;
          }
          else
          {
            if (position.isSet() )
            {
              if(linVel.isSet())
              {
                position()+=linVel()*dt;
                if (linAcc.isSet())
                {
                  position()+=linAcc()*dt*dt/2;
                }
              }
            }
            else
            {
              position.set();
              position().setZero();
            }
          }

        }

        if (flagLinVel)
        {
          if (flagLinAcc && !newValue.linAcc.isSet() && newValue.linVel.isSet())
          {
            curLinVel=linVel;
          }

          if (newValue.linVel.isSet())
          {
            linVel = newValue.linVel;
          }
          else
          {
            if (
              newValue.position.isSet() &&
              (curPos.isSet() || ( position.isSet() && !flagPos))
            )
            {
              if (curPos.isSet())
              {
                linVel=(newValue.position() - curPos())/dt;
              }
              else
              {
                linVel=(newValue.position() - position())/dt;
              }
            }
            else
            {
              if (linVel.isSet())
              {
                if (linAcc.isSet())
                {
                  linVel()+=linAcc()*dt;
                }
              }
              else
              {
                linVel.set();
                linVel().setZero();
              }
            }

          }
        }

        if (flagLinAcc)
        {
          if (newValue.linAcc.isSet())
          {
            linAcc =  newValue.linAcc;
          }
          else
          {
            if (
              newValue.linVel.isSet() &&
              (curLinVel.isSet() || (linVel.isSet() && !flagLinVel))
            )
            {
              if (curLinVel.isSet())
              {
                linAcc=(newValue.linVel()-curLinVel())/dt;
              }
              else
              {
                linAcc=(newValue.linVel()-linVel())/dt;
              }
            }
            else
            {
              if (!linAcc.isSet())
              {
                linAcc.set();
                linAcc().setZero();
              }
            }
          }
        }
      }

      {
        Orientation curOri;
        CheckedVector3 curAngVel;


        if (flagOri)
        {
          if (flagAngVel && !newValue.angVel.isSet() && orientation.isSet()  && newValue.orientation.isSet())
          {
            curOri=orientation;
          }
          if (newValue.orientation.isSet())
          {
            orientation=newValue.orientation;
          }
          else
          {
            if (orientation.isSet() )
            {
              if(angVel.isSet())
              {
                Vector3 increment = Vector3::Zero();
                increment+=angVel()*dt;
                if (angAcc.isSet())
                {
                  increment+=angAcc()*dt*dt/2;
                }
                orientation.integrate(increment);
              }
            }
            else
            {
              orientation=Quaternion::Identity();
            }
          }
        }

        if (flagAngVel)
        {
          if (flagAngAcc && !newValue.angAcc.isSet() && newValue.angVel.isSet())
          {
            curAngVel=angVel;
          }

          if (newValue.angVel.isSet())
          {
            angVel = newValue.angVel;
          }
          else
          {
            if (
              newValue.orientation.isSet() &&
              (curOri.isSet() || ( orientation.isSet() && !flagOri))
            )
            {
              if (curOri.isSet())
              {
                angVel=curOri.differentiate(newValue.orientation)/dt;
              }
              else
              {
                angVel=orientation.differentiate(newValue.orientation)/dt;
              }
            }
            else
            {
              if (angVel.isSet())
              {
                if (angAcc.isSet())
                {
                  angVel()+=angAcc()*dt;
                }
              }
              else
              {
                angVel.set();
                angVel().setZero();
              }
            }

          }
        }

        if (flagAngAcc)
        {
          if (newValue.angAcc.isSet())
          {
            angAcc =  newValue.angAcc;
          }
          else
          {
            if (
              newValue.angVel.isSet() &&
              (curAngVel.isSet() || (angVel.isSet() && !flagAngVel))
            )
            {
              if (curAngVel.isSet())
              {
                angAcc=(newValue.angVel()-curAngVel())/dt;
              }
              else
              {
                angAcc=(newValue.angVel()-angVel())/dt;
              }
            }
            else
            {
              if (!angAcc.isSet())
              {
                angAcc.set();
                angAcc().setZero();
              }
            }
          }
        }
      }

      return *this;
    }

    inline Kinematics Kinematics::inverse() const
    {
      Kinematics inverted;

      if (orientation.isSet())
      {
        inverted.orientation = orientation.inverse();
        Orientation & r2 = inverted.orientation;

        if (angVel.isSet())
        {
          inverted.angVel =  -(r2 * angVel()); //omega2

          if (angAcc.isSet())
          {
            inverted.angAcc =   r2 * (angVel().cross(angVel()) - angAcc()); //omega2dot
          }
        }

        if (position.isSet())
        {
          inverted.position= - (r2 * position()) ;

          if (linVel.isSet())
          {
            Vector3 omegaxp= angVel().cross(position());
            inverted.linVel= r2 * ( omegaxp - linVel()); //t2dot
            if (linAcc.isSet())
            {
                inverted.linAcc=  r2 * ( angVel().cross(2 * linVel - omegaxp)
                                        - linAcc() + angAcc().cross(position())); //t2dotdot
            }
          }
        }
      }

      return inverted;

    }

    ///composition of transformation
    inline Kinematics Kinematics::operator* (const Kinematics & multiplier) const
    {
      return multiply_(this,multiplier);
    }

    inline Kinematics Kinematics::operator* (const Kinematics & multiplier)
    {
      return multiply_(this,multiplier);
    }

    inline Kinematics Kinematics::operator* ( Kinematics & multiplier) const
    {
      return multiply_(this,multiplier);
    }

    inline Kinematics Kinematics::operator* (Kinematics & multiplier)
    {
      return multiply_(this,multiplier);
    }

    inline Vector Kinematics::toVector(Flags::byte flags) const
    {
      int size =0;

      if (flags & Flags::position)
      {
        size +=3;
      }
      if (flags & Flags::orientation)
      {
        size +=4;
      }
      if (flags & Flags::linVel)
      {
        size +=3;
      }
      if (flags & Flags::angVel)
      {
        size +=3;
      }
      if (flags & Flags::linAcc)
      {
        size +=3;
      }
      if (flags & Flags::angAcc)
      {
        size +=3;
      }

      Vector output(size);
      
      int curIndex = 0;
      if ((flags & Flags::position))
      {
        output.segment<3>(curIndex)=position();
        curIndex+=3;
      }
      if ((flags & Flags::orientation))
      {
        output.segment<4>(curIndex)=Quaternion(orientation).coeffs();
        curIndex+=4;
      }
      if ((flags & Flags::linVel))
      {
        output.segment<3>(curIndex)=linVel();
        curIndex+=3;
      }
      if ((flags & Flags::angVel))
      {
        output.segment<3>(curIndex)=angVel();
        curIndex+=3;
      }      
      if ((flags & Flags::linAcc))
      {
        output.segment<3>(curIndex)=linAcc();
        curIndex+=3;
      }      
      if ((flags & Flags::angAcc))
      {
        output.segment<3>(curIndex)=angAcc();
      }      

      return output;
    }

    inline Vector Kinematics::toVector() const
    {
      int size = 0;
      if (position.isSet())
      {
       size+=3;
      }
      if (orientation.isSet())
      {
        size+=4;
      }
      if (linVel.isSet())
      {
        size+=3;
      }
      if (angVel.isSet())
      {
        size+=3;
      }
      if (linAcc.isSet())
      {
        size+=3;
      }
      if (angAcc.isSet())
      {
        size+=3;
      }      

      Vector output(size);
     
      int curIndex = 0;
      if (position.isSet())
      {
        output.segment<3>(curIndex)=position();
        curIndex+=3;
      }
      if (orientation.isSet())
      {
        output.segment<4>(curIndex)=Quaternion(orientation).coeffs();
        curIndex+=4;
      }
      if (linVel.isSet())
      {
        output.segment<3>(curIndex)=linVel();
        curIndex+=3;
      }
      if (angVel.isSet())
      {
        output.segment<3>(curIndex)=angVel();
        curIndex+=3;
      }      
      if (linAcc.isSet())
      {
        output.segment<3>(curIndex)=linAcc();
        curIndex+=3;
      }      
      if (angAcc.isSet())
      {
        output.segment<3>(curIndex)=angAcc();
      }      

      return output;
    }

    inline void Kinematics::reset()
    {
      position.reset();
      orientation.reset();
      linVel.reset();
      angVel.reset();
      linAcc.reset();
      angAcc.reset();
    }

    template<typename thistype,typename kine>
    inline Kinematics Kinematics::multiply_(thistype* self, kine& multiplier)
    {
      Kinematics result;

      BOOST_ASSERT(self->orientation.isSet()
       && "The multiplied orientation is not initialized, the multiplication is not possible");

      BOOST_ASSERT((multiplier.position.isSet() || multiplier.orientation.isSet())
        &&"The multiplier kinematics is not initialized, the multiplication is not possible");


      if (multiplier.position.isSet() && self->position.isSet())
      {
        Vector3 R1p2 = self->orientation*multiplier.position();

        result.position = R1p2 + self->position();


        if (multiplier.linVel.isSet() && self->linVel.isSet() && self->angVel.isSet())
        {
          Vector3 R1p2d = self->orientation*multiplier.linVel;
          Vector3 w1xR1p2 = self->angVel().cross(R1p2);

          result.linVel = R1p2d + w1xR1p2 + self->linVel();

          if (multiplier.linAcc.isSet() && self->linAcc.isSet() && self->angAcc.isSet())
          {
            result.linAcc = self->orientation*multiplier.linAcc()
                          + self->angAcc().cross(R1p2)
                          + self->angVel().cross(w1xR1p2+2*R1p2d)
                          + self->linAcc();

          }

        }

      }

      if (multiplier.orientation.isSet())
      {
        result.orientation = self->orientation * multiplier.orientation;

        if (multiplier.angVel.isSet() && self->angVel.isSet())
        {
          Vector3 R1w2 = self->orientation * multiplier.angVel;
          result.angVel = R1w2 + self->angVel();

          if (multiplier.angAcc.isSet() && self->angAcc.isSet())
          {
            result.angAcc = self->orientation * multiplier.angAcc()
                                + self->angVel().cross(R1w2)
                                      + self->angAcc();
          }
        }

      }

      return result;

    }

  }
}
