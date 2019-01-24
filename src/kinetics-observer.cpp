#include <state-observation/dynamics-estimators/kinetics-observer.hpp>





namespace stateObservation
{
  inline Matrix6 blockMat6(const Matrix3 & m1, const Matrix3 & m2, const Matrix3 & m3, const Matrix3 & m4)
  {
    Matrix6 m;
    m<< m1,m2,
        m3,m4;
    
    return m;
  }

  /// resets one block on the diagonal of the state Covariance Matrix
        /// i.e. sets value of a square block on the diagonal of the covMat
        /// and sets to zero all the values related to their lines and columns
  template <int blockSize>
  void setBlockStateCovariance(Matrix & covMat, const Matrix & covBlock, int blockIndex,int matrixSize)
  {
    covMat.block<blockSize,blockSize>(blockIndex,blockIndex)=covBlock;
    covMat.block(blockIndex,0,blockSize,blockIndex).setZero();
    covMat.block(0,blockIndex,blockIndex,blockSize).setZero();
    covMat.block(blockIndex+blockSize,blockIndex,matrixSize-blockIndex-blockSize,blockSize).setZero();
    covMat.block(blockIndex,blockIndex+blockSize,blockSize,matrixSize-blockIndex-blockSize).setZero();
  }

  inline void fillSymmetricMatrix(Matrix3 & m,const Vector3 &vdiag, double e1, double e2, double e3)
  {
    m.diagonal() = vdiag;
    m(1,0)=m(0,1)=e1;
    m(2,0)=m(0,2)=e2;
    m(2,1)=m(1,2)=e3; 
  }

  const double KineticsObserver::defaultMass = 50;

  const double KineticsObserver::statePoseInitVarianceDefault = 1e-4;
  const double KineticsObserver::stateOriInitVarianceDefault = 1e-4;
  const double KineticsObserver::stateLinVelInitVarianceDefault = 1e-6;
  const double KineticsObserver::stateAngVelInitVarianceDefault = 1e-6;
  const double KineticsObserver::gyroBiasInitVarianceDefault = 1e-10;
  const double KineticsObserver::unmodeledWrenchInitVarianceDefault = 1e100;
  const double KineticsObserver::contactForceInitVarianceDefault = 1e100;
  const double KineticsObserver::contactTorqueInitVarianceDefault = 1e100;

  const double KineticsObserver::statePoseProcessVarianceDefault = 1e-8;
  const double KineticsObserver::stateOriProcessVarianceDefault = 1e-8;
  const double KineticsObserver::stateLinVelProcessVarianceDefault = 1e-8;
  const double KineticsObserver::stateAngVelProcessVarianceDefault = 1e-8;
  const double KineticsObserver::gyroBiasProcessVarianceDefault = 1e-12;
  const double KineticsObserver::unmodeledWrenchProcessVarianceDefault = 1e-8;
  const double KineticsObserver::contactForceProcessVarianceDefault = 1e-8;
  const double KineticsObserver::contactTorqueProcessVarianceDefault = 1e-8;

  const double KineticsObserver::acceleroVarianceDefault = 1e-4;
  const double KineticsObserver::gyroVarianceDefault = 1e-8;
  const double KineticsObserver::forceSensorVarianceDefault = 1e-8;
  const double KineticsObserver::torqueSensorVarianceDefault = 1e-10;
  const double KineticsObserver::positionSensorVarianceDefault = 1e-4;
  const double KineticsObserver::orientationSensorVarianceDefault = 1e-3;

  const double linearStiffnessDefault = 40000;
  const double angularStiffnessDefault = 400;
  const double linearDampingDefault = 120;
  const double angularDampingDefault = 12;

  const double KineticsObserver::defaultdx = 1e-6;

  KineticsObserver::KineticsObserver(int maxContacts):
    maxContacts_(maxContacts),
    stateSize_(sizeStateBase + maxContacts*sizeStatePerContact),
    stateTangentSize_(sizeStateTangentBase + sizeStateTangentPerContact* maxContacts),
    measurementSize_(0),
    stateVector_(stateSize_),
    stateVectorDx_(stateTangentSize_),
    oldStateVector_(stateSize_),
    additionalForce_(Vector3::Zero()),    
    additionalTorque_(Vector3::Zero()),    
    ekf_(stateSize_, stateTangentSize_, sizeIMUSignal, sizeIMUSignal,  0,false,false),
    finiteDifferencesJacobians_(true),
    withGyroBias_(true), withUnmodeledWrench_(false), withAccelerationEstimation_(false),
    k_est(0),k_data(0), mass_(defaultMass), dt_(defaultdx),
    linearStiffnessMatDefault_(Matrix3::Identity()*linearStiffnessDefault),
    angularStiffnessMatDefault_(Matrix3::Identity()*angularStiffnessDefault),
    linearDampingMatDefault_(Matrix3::Identity()*linearDampingDefault),
    angularDampingMatDefault_(Matrix3::Identity()*angularDampingDefault),
    acceleroCovMatDefault_(Matrix3::Identity()*acceleroVarianceDefault),
    gyroCovMatDefault_( Matrix3::Identity()*gyroVarianceDefault),
    contactWrenchSensorCovMatDefault_(blockMat6( Matrix3::Identity()*forceSensorVarianceDefault, Matrix3::Zero(),
                                Matrix3::Zero(), Matrix3::Identity()*torqueSensorVarianceDefault )),
    absPoseSensorCovMatDefault_(blockMat6( Matrix3::Identity()*positionSensorVarianceDefault, Matrix3::Zero(),
                                Matrix3::Zero(), Matrix3::Identity()*orientationSensorVarianceDefault )),
    statePosInitCovMat_(Matrix3::Identity()*statePoseInitVarianceDefault),
    stateOriInitCovMat_(Matrix3::Identity()*stateOriInitVarianceDefault),
    stateLinVelInitCovMat_(Matrix3::Identity()*stateLinVelInitVarianceDefault),
    stateAngVelInitCovMat_(Matrix3::Identity()*stateAngVelInitVarianceDefault),
    gyroBiasInitCovMat_(Matrix3::Identity()*gyroBiasInitVarianceDefault),
    unmodeledWrenchInitCovMat_(Matrix6::Identity()*unmodeledWrenchInitVarianceDefault),
    statePosProcessCovMat_(Matrix3::Identity()*statePoseProcessVarianceDefault),
    stateOriProcessCovMat_(Matrix3::Identity()*stateOriProcessVarianceDefault),
    stateLinVelProcessCovMat_(Matrix3::Identity()*stateLinVelProcessVarianceDefault),
    stateAngVelProcessCovMat_(Matrix3::Identity()*stateAngVelProcessVarianceDefault),
    gyroBiasProcessCovMat_(Matrix3::Identity()*gyroBiasProcessVarianceDefault),
    unmodeledWrenchProcessCovMat_(Matrix6::Identity()*unmodeledWrenchProcessVarianceDefault)
  {
    ekf_.setFunctor(this);

    stateVector_.setZero();

    ekf_.setState(stateVector_,k_est);

    const Matrix3 & Zero3 = Matrix3::Zero();

    stateKineMatricsInitCovMat_ << 
        statePosInitCovMat_, Zero3	            , Zero3	                , Zero3,
        Zero3              , stateOriInitCovMat_, Zero3	                , Zero3,
        Zero3              , Zero3              , stateLinVelInitCovMat_, Zero3,
        Zero3              , Zero3              , Zero3	                , stateAngVelInitCovMat_;


    stateKineMatricsProcessCovMat_ << 
        Zero3, Zero3, Zero3	                         , Zero3,
        Zero3, Zero3, Zero3	                         , Zero3,
        Zero3, Zero3, contactForceInitVarianceDefault, Zero3,
        Zero3, Zero3, Zero3	                         , contactTorqueInitVarianceDefault;

    
    contactProcessCovMat_ << 
        Zero3, Zero3, Zero3	                         , Zero3,
        Zero3, Zero3, Zero3	                         , Zero3,
        Zero3, Zero3, contactForceProcessVarianceDefault, Zero3,
        Zero3, Zero3, Zero3	                         , contactTorqueProcessVarianceDefault;

    
    Id_.set(Matrix3::Zero(),k_data);
    comd_.set(Vector3::Zero(),k_data);
    comdd_.set(Vector3::Zero(),k_data);
    sigmad_.set(Vector3::Zero(),k_data);
    
    

    resetStateCovarianceMat();
    resetProcessCovarianceMat();

    updateKine_();

    Contact::numberOfRealSensors = 0;
  }

  KineticsObserver::~KineticsObserver()
  {
  }

  
  unsigned KineticsObserver::getStateSize() const
  {
    return stateSize_;
  }

  unsigned KineticsObserver::getMeasurementSize() const
  {
    int size = 0;
  /// Synchronizing the sensors
    MapIMUConstIterator i = imuSensors_.begin();
    while (i != imuSensors_.end()) 
    {
      if (i->second.time==k_data) 
      {
        size += sizeIMUSignal;
      } 
    }
    
    for (MapContactConstIterator i= contacts_.begin(), ie = contacts_.end(); i!=ie ; ++i) 
    {
      if (i->second.time == k_data && i->second.withRealSensor)
      {
        size += sizeWrench;
      }
    }

    if (absPoseSensor_.time == k_data)
    {
      size += sizePose;
    }
    return size;
  }

  double KineticsObserver::getSamplingTime() const
  {
    return dt_;
  }

  void KineticsObserver::setSamplingTime( double dt) 
  {
    dt_ = dt;
  }

  void KineticsObserver::setMass(double m)
  {
    mass_=m;
  }
  
  Vector KineticsObserver::update()
  {
    if (k_est!=k_data)
    {
      /// Synchronizing the sensors
      MapIMUIterator i = imuSensors_.begin();
      while (i != imuSensors_.end()) 
      {
        if (i->second.time!=k_data) 
        {
          imuSensors_.erase(i++); /// remove the i-th element and move to the next
        } 
        else 
        {
          ++i;
        }
      }
      
      for (MapContactIterator i= contacts_.begin(), ie = contacts_.end(); i!=ie ; ++i) 
      {
        BOOST_ASSERT((i->second.time == k_data) && "The contacts have not all been updated. \
              Either remove lost contacts using removeContact \
              or Run setContactFTSensor or setContactWithNoSensor on every existing contact");

        /// the following code is only an attempt to maintain a coherent state of the state observer
        if (i->second.time != k_data && i->second.withRealSensor)
        {
          i->second.withRealSensor=false;
          i->second.localKine.integrate(dt_);
          Contact::numberOfRealSensors--;
        }
      }

      ///////////// initialize the measurement Vector and matrix //////////////

      measurementSize_ = sizeIMUSignal*int(imuSensors_.size()) + sizeWrench*Contact::numberOfRealSensors;
      int measurementTangentSize = measurementSize_;
      if (absPoseSensor_.time == k_data)
      {
        measurementSize_ += sizePose;
        measurementTangentSize += sizePoseTangent;
      }
      
      measurementVector_.resize(measurementSize_);
      measurementCovMatrix_.resize(measurementTangentSize,measurementTangentSize);
      measurementCovMatrix_.setZero();
    
      int localIndex = 0;
      
      for (MapIMUIterator i= imuSensors_.begin(), ie = imuSensors_.end(); i!=ie ; ++i)
      {
        i->second.index = localIndex;
        measurementVector_.segment<sizeIMUSignal>(localIndex) = i->second.acceleroGyro;
        measurementCovMatrix_.block<sizeAcceleroSignal,sizeAcceleroSignal>(localIndex,localIndex)=i->second.covMatrixAccelero;
        localIndex+=sizeAcceleroSignal;
        measurementCovMatrix_.block<sizeGyroSignal,sizeGyroSignal>(localIndex,localIndex)=i->second.covMatrixGyro;
        localIndex+=sizeGyroSignal;
      }

      for (MapContactIterator i=contacts_.begin(), ie = contacts_.end();i!=ie;++i) 
      {
        if (i->second.withRealSensor)
        {
          i->second.index = localIndex;
          measurementVector_.segment<sizeWrench>(localIndex) = i->second.wrench;
          measurementCovMatrix_.block<sizeWrench,sizeWrench>(localIndex,localIndex)=i->second.sensorCovMatrix();
          localIndex+=sizeWrench;
        }
      }

      if (absPoseSensor_.time == k_data)
      {
        absPoseSensor_.index= localIndex;
        BOOST_ASSERT(absPoseSensor_.pose.position.isSet() && absPoseSensor_.pose.orientation.isSet() \
                    && "The absolute pose needs to contain the position and the orientation");
        measurementVector_.segment<sizePose>(localIndex) = absPoseSensor_.pose.toVector(flagsKineSensor);
        measurementCovMatrix_.block<sizePoseTangent,sizePoseTangent>(localIndex,localIndex)=absPoseSensor_.covMatrix();
      }

      ekf_.setMeasureSize(measurementSize_,measurementTangentSize);
      ekf_.setMeasurement(measurementVector_,k_data);
      ekf_.setR(measurementCovMatrix_);
      if (finiteDifferencesJacobians_)
      {
        ekf_.setA(ekf_.getAMatrixFD(stateVectorDx_));
        ekf_.setC(ekf_.getCMatrixFD(stateVectorDx_));
      }

      stateVector_ = ekf_.getEstimatedState(k_data);

      if (stateVector_.hasNaN())
      {
  #ifndef NDEBUG
        std::cout << "Kinetics observer: NaN value detected" << std::endl;
  #endif 
        stateVector_ = stateNaNCorrection_();
      }
      else
      {
        oldStateVector_ = stateVector_;
      }

      ++k_est; //the timestamp of the state we estimated

      stateKinematics_.reset();

      updateKine_();

      if (withAccelerationEstimation_)
      {
        estimateAccelerations();
      }

    }
    
    return stateVector_;
  }

  Vector KineticsObserver::getStateVector() const
  {
    return stateVector_;
  }

  kine::Kinematics KineticsObserver::getKinematics() const
  {
    return stateKinematics_; 
  }  

  kine::Kinematics KineticsObserver::getKinematics( const Kinematics & local) const
  {
    return getKinematics()*local;
  }

  Vector6 KineticsObserver::getContactWrench(int contactNbr) const
  {
    return stateVector_.segment<sizeWrench>(contactWrenchIndex(contactNbr));
  }

  kine::Kinematics KineticsObserver::getContactPosition(int contactNbr) const
  {
    return Kinematics(stateVector_.segment<sizeStateKine>(contactKineIndex(contactNbr)),
                                                          flagsContactKine);
  }

  Vector6 KineticsObserver::getUnmodeledWrench() const
  {
    return stateVector_.segment<sizeWrench>(unmodeledWrenchIndex());
  }

  kine::Kinematics KineticsObserver::estimateAccelerations()
  {
    stateKinematics_.fromVector(computeAccelerations_(),
                                Kinematics::Flags::linAcc|
                                Kinematics::Flags::angAcc);


    return stateKinematics_;
  }

  void KineticsObserver::setStateKinematics(const Kinematics & kine, bool resetForces,
                                            bool resetCovariance)
  {
    stateKinematics_ = kine;
    stateVector_.segment<sizeStateKine>(kineIndex()) = stateKinematics_.toVector(flagsStateKine);

    if (resetForces)
    {
      for (MapContactIterator i = contacts_.begin(); i != contacts_.end(); ++i)
      {
        stateVector_.segment<sizeWrench>(contactWrenchIndex(i->first)).setZero();
      }
    }

    ekf_.setState(stateVector_,k_est);

    if (resetCovariance)
    {
      Matrix stateCovariance = ekf_.getStateCovariance();
      setBlockStateCovariance<sizeStateKine>(stateCovariance,stateKineMatricsInitCovMat_,kineIndex(),stateSize_);

      if (resetForces)
      {
        for (MapContactIterator i = contacts_.begin(); i != contacts_.end(); ++i)
        {
          setBlockStateCovariance<sizeContact>(stateCovariance,contactInitCovMat_,contactIndex(i->first),stateSize_);
        }
      }
      ekf_.setStateCovariance(stateCovariance);
    }

  }

  void KineticsObserver::setGyroBias(const Vector3 &,  bool resetCovariance)
  {
    stateVector_.segment<sizeGyroBias>(gyroBiasIndex());
    ekf_.setState(stateVector_,k_est);

    if (resetCovariance)
    {
      Matrix stateCovariance = ekf_.getStateCovariance();
      setBlockStateCovariance<sizeGyroBias>(stateCovariance,gyroBiasInitCovMat_,gyroBiasIndex(),stateSize_);
      
      ekf_.setStateCovariance(stateCovariance);
    }
  }

  void KineticsObserver::setStateUnmodeledWrench(const Vector6 &, bool resetCovariance)
  {
    stateVector_.segment<sizeWrench>(unmodeledWrenchIndex());
    ekf_.setState(stateVector_,k_est);

    if (resetCovariance)
    {
      Matrix stateCovariance = ekf_.getStateCovariance();
      setBlockStateCovariance<sizeWrench>(stateCovariance,unmodeledWrenchInitCovMat_,unmodeledWrenchIndex(),stateSize_);
      
      ekf_.setStateCovariance(stateCovariance);
    }
  }


  void KineticsObserver::setStateVector(const Vector & v, bool resetCovariance)
  {
    stateVector_ = v;
    ekf_.setState(v,k_est);
    updateKine_();

    if (resetCovariance)
    {
      resetStateCovarianceMat();
    }

  }

  void KineticsObserver::setAdditionalWrench(const Vector3& force,const Vector3& moment)
  {
    
    additionalForce_=force;
    additionalTorque_=moment;
  }
  
  void KineticsObserver::setWithUnmodeledWrench(bool b)
  {
    withUnmodeledWrench_ = b;
  }

  void KineticsObserver::setWithAccelerationEstimation(bool b)
  {
    withAccelerationEstimation_=b;    
  }

  void KineticsObserver::setWithGyroBias(bool b)
  {
    withAccelerationEstimation_=b;
  }

  int KineticsObserver::setIMU(const Vector3 & accelero, const  Vector3 & gyrometer, const Kinematics &localKine, int num)
  {
    ///ensure the measuements are labeled with the good time stamp
    startNewIteration_();    
   
    if (num==-1)
    {
      num=0;

      for(MapIMUIterator i = imuSensors_.begin(); i != imuSensors_.end(); ++i)
      {
        if (i->first >= num)
          num = i->first+1;
      }      
    }
    
    std::pair<MapIMUIterator,bool> inserted = imuSensors_.insert(PairIMU(num,IMU()));

    bool & isNew = inserted.second;

    IMU & imu = inserted.first->second;


    imu.acceleroGyro.head<3>()=accelero;
    imu.acceleroGyro.tail<3>()=gyrometer;
    if (isNew)
    {
      imu.covMatrixAccelero = acceleroCovMatDefault_;
      imu.covMatrixGyro = gyroCovMatDefault_;
    }    
    
    imu.kinematics.update(localKine, dt_,flagsIMUKine);
    imu.time = k_data;
    
    return num;
  }

  int KineticsObserver::setIMU(const Vector3 & accelero, const  Vector3 & gyrometer, const Matrix3& acceleroCov, 
                                                        const Matrix3 gyroCov, const Kinematics &localKine, int num)
  {
    ///ensure the measuements are labeled with the good time stamp
    startNewIteration_();

    if (num==-1)
    {
      num=0;
      
      for(MapIMUIterator i = imuSensors_.begin(); i != imuSensors_.end(); ++i)
      {
        if (i->first >= num)
          num = i->first+1;
      }
    }

    std::pair<MapIMUIterator,bool> inserted = imuSensors_.insert(PairIMU(num,IMU()));

    IMU & imu = inserted.first->second;

    imu.acceleroGyro.head<3>()=accelero;
    imu.acceleroGyro.tail<3>()=gyrometer;
    imu.covMatrixAccelero  = acceleroCov;
    imu.covMatrixGyro = gyroCov;
    imu.kinematics.update(localKine, dt_,flagsIMUKine);
    imu.time = k_data;
    
    return num;
  }

  void KineticsObserver::setIMUDefaultCovarianceMatrix(const Matrix3& acceleroCov, const Matrix3 &gyroCov)
  {
    acceleroCovMatDefault_=acceleroCov;
    gyroCovMatDefault_=gyroCov;
  }

  void KineticsObserver::setContactWrenchSensor(const Vector6 & wrench, const Kinematics &localKine, int contactNumber)
  {
    ///ensure the measuements are labeled with the good time stamp
    startNewIteration_();


    MapContactIterator i = contacts_.find(contactNumber);
    BOOST_ASSERT((i != contacts_.end()) && "Tried to set the wrench of non-existing contact. \
                                            The contact must be added BEFORE setting a contact wrench Sensor");

    i->second.localKine = localKine;
    i->second.wrench=wrench;
    i->second.time = k_data;

    if (!i->second.sensorCovMatrix.isSet())
    {
      i->second.sensorCovMatrix = contactWrenchSensorCovMatDefault_;
    }    

    if (!(i->second.withRealSensor))
    {
      i->second.withRealSensor=true;
      Contact::numberOfRealSensors++;
    }
  }

   void KineticsObserver::setContactWrenchSensor(const Vector6 & wrench, const Matrix6 & wrenchCovMatrix, 
                                                                                    const Kinematics &localKine, int contactNumber)
  {
    ///ensure the measuements are labeled with the good time stamp
    startNewIteration_();

    MapContactIterator i = contacts_.find(contactNumber);
    BOOST_ASSERT((i != contacts_.end()) && "Tried to set the wrench of non-existing contact. \
                                            The contact must be added BEFORE setting a contact wrench Sensor");

    i->second.localKine = localKine;
    i->second.wrench=wrench;
    i->second.time = k_data;
    i->second.sensorCovMatrix = wrenchCovMatrix;
    if (!(i->second.withRealSensor))
    {
      i->second.withRealSensor=true;
      Contact::numberOfRealSensors++;
    }
              
  }

  void KineticsObserver::setContactWrenchSensorDefaultCovarianceMatrix(const Matrix6 & wrenchSensorCovMat)
  {
    contactWrenchSensorCovMatDefault_=wrenchSensorCovMat;
  }

  void KineticsObserver::setContactWithNoSensor(const Kinematics &localKine, int contactNumber)
  {
     ///ensure the measuements are labeled with the good time stamp
    startNewIteration_();

    MapContactIterator i = contacts_.find(contactNumber);
    BOOST_ASSERT((i != contacts_.end()) && "Tried to set the kinematics of non existibng contact. \
                                            The contact must be added BEFORE setting a contact kinmatics");

    i->second.localKine = localKine;
    i->second.time = k_data;
    
    if (i->second.withRealSensor)
    {
      i->second.withRealSensor=false;
      Contact::numberOfRealSensors--;
    }

  }

  void KineticsObserver::setAbsolutePoseSensor(const Kinematics & pose)
  {
    ///ensure the measuements are labeled with the good time stamp
    startNewIteration_();

    absPoseSensor_.time = k_data;
    absPoseSensor_.pose = pose;
        
    if (!(absPoseSensor_.covMatrix.isSet()))
    {
      absPoseSensor_.covMatrix = absPoseSensorCovMatDefault_;
    }      
  }

  void KineticsObserver::setAbsolutePoseSensor(const Kinematics & pose, const Matrix6 & CovarianceMatrix)
  {
    ///ensure the measuements are labeled with the good time stamp
    startNewIteration_();

    absPoseSensor_.time = k_data;
    absPoseSensor_.pose = pose;
        
    absPoseSensor_.covMatrix = CovarianceMatrix;      
  }

  void KineticsObserver::setInertiaMatrix(const Matrix3& I, const Matrix3& I_dot)
  {
    startNewIteration_();
    I_.set(I,k_data);
    Id_.set(I_dot,k_data);

  }

  void KineticsObserver::setInertiaMatrix(const Matrix3& I)
  {
    startNewIteration_();

    if (I_.getTime()<k_data)
    {
      Id_.set(tools::derivate(I_(),I,dt_*double(k_data-I_.getTime())),k_data);
    }
    I_.set(I,k_data);
  }

  void KineticsObserver::setInertiaMatrix(const Vector6& Iv, const Vector6& Iv_dot)
  {
    startNewIteration_();
    
    I_.set();
    I_.setIndex(k_data);
    fillSymmetricMatrix(I_(),Iv.head<3>(),Iv(3),Iv(4),Iv(5));

    Id_.set();
    Id_.setIndex(k_data);
    fillSymmetricMatrix(Id_(),Iv_dot.head<3>(),Iv_dot(3),Iv_dot(4),Iv_dot(5));
  }

  void KineticsObserver::setInertiaMatrix(const Vector6& Iv)
  {
    startNewIteration_();
    namespace t = tools;

    if (I_.getTime()<k_data)
    {
      Id_.set();
      Id_.setIndex(k_data);
      double dt = dt_*double(k_data-I_.getTime());
      fillSymmetricMatrix(Id_(),t::derivate<Vector3>(I_().diagonal(),Iv.head<3>(),dt),
                                t::derivate(I_()(1,0), Iv(3), dt),
                                t::derivate(I_()(2,0), Iv(4), dt),
                                t::derivate(I_()(2,1), Iv(5), dt));
    }
    
    I_.set();
    I_.setIndex(k_data);
    fillSymmetricMatrix(I_(),Iv.head<3>(),Iv(3),Iv(4),Iv(5));
  }

  void KineticsObserver::setCenterOfMass(const Vector3& com, const Vector3& com_dot, const Vector3& com_dot_dot)
  {
    startNewIteration_();
    com_.set(com,k_data);
    comd_.set(com_dot,k_data);
    comdd_.set(com_dot_dot,k_data);
  }

  void KineticsObserver::setCenterOfMass(const Vector3& com, const Vector3& com_dot)
  {
    startNewIteration_();
    com_.set(com,k_data);
    

    if (comd_.getTime()<k_data)
    {
      comdd_.set( tools::derivate(comd_(),com_dot,dt_ * double(k_data- comd_.getTime())),k_data);
    }
    comd_.set(com_dot,k_data);

    
  }

  void KineticsObserver::setCenterOfMass(const Vector3& com)
  {
    startNewIteration_();

    if (com_.getTime()<k_data )
    {
      double dt = dt_ * double(k_data- com_.getTime());
      Vector3 com_dot = tools::derivate(com_(),com,dt);
      
      comdd_.set( tools::derivate(comd_(),com_dot,dt),k_data);

      comd_.set(com_dot,k_data);      
    }

    com_.set(com,k_data);
  }

  void KineticsObserver::setAngularMomentum (const Vector3& sigma, const Vector3& sigma_dot)
  {
    startNewIteration_();
    sigma_.set(sigma, k_data);
    sigmad_.set(sigma_dot, k_data);    
  }

  void KineticsObserver::setAngularMomentum (const Vector3& sigma)
  {
    startNewIteration_();
    if (sigma_.getTime()<k_data)
    {
      sigmad_.set(tools::derivate(sigma_(),sigma,dt_*double(k_data-sigma_.getTime())),
                  k_data);
    }    
    sigma_.set(sigma,k_data);
  }


  int KineticsObserver::addContact(const Kinematics & pose, 
                            const Matrix12 & initialCovarianceMatrix, const Matrix12 & processCovarianceMatrix, 
                            const Matrix3 & linearStiffness,  const Matrix3 & linearDamping, 
                            const Matrix3 & angularStiffness, const Matrix3 & angularDamping, 
                            int contactNumber)
  {
    if (contactNumber == -1)
    {
      contactNumber=0;

      while (contactNumber< maxContacts_ && (contacts_.find(contactNumber)!=contacts_.end()))
      {
        ++contactNumber;
      }
    }

    BOOST_ASSERT (contactNumber<maxContacts_ && "Contact number bigger than maxContacts_, please increase the maximum number of contacts");
    std::pair<MapContactIterator,bool> inserted = contacts_.insert(PairContact(contactNumber,Contact()));
    Contact & contact = inserted.first->second;
    bool & isNewContact = inserted.second;
    BOOST_ASSERT ( isNewContact && "The contact already exists, please remove it before adding it again");

    contact.stateIndex = contactsIndex()+contactNumber*sizeContact;

    contact.absPose = pose;

    contact.linearStiffness = linearStiffness;
    contact.linearDamping = linearDamping;
    contact.angularStiffness = angularStiffness;
    contact.angularDamping = angularDamping;

    stateVector_.segment<sizeContact> (contact.stateIndex) << pose.toVector(flagsContactKine) , Vector6::Zero();

    /// sets the initial covariance matrix
    Matrix stateCovMat= ekf_.getStateCovariance();
    setBlockStateCovariance<sizeContact>(stateCovMat,initialCovarianceMatrix,contact.stateIndex,stateSize_);
    ekf_.setStateCovariance(stateCovMat);

    ///Sets the process cov mat
    Matrix processCovMat= ekf_.getQ();
    setBlockStateCovariance<sizeContact>(processCovMat,processCovarianceMatrix,contact.stateIndex,stateSize_);
    ekf_.setQ(processCovarianceMatrix);

    return contactNumber;
   
  }

  /// version with default stiffness and damping
  /// use when the contact parameters are known
  int KineticsObserver::addContact(const Kinematics & pose, 
                            const Matrix12 & initialCovarianceMatrix, const Matrix12 & processCovarianceMatrix, 
                            int contactNumber)
  {
    return addContact(pose,initialCovarianceMatrix,processCovarianceMatrix,
                      linearStiffnessMatDefault_,linearDampingMatDefault_,
                      angularDampingMatDefault_,angularDampingMatDefault_,contactNumber);

  }

  /// version when the contact position is perfectly known
  int KineticsObserver::addContact(const Kinematics & pose, 
                            const Matrix3 & linearStiffness,  const Matrix3 & linearDamping, 
                            const Matrix3 & angularStiffness, const Matrix3 & angularDamping, 
                            int contactNumber)
  {
    return addContact(pose,contactInitCovMat_,contactProcessCovMat_,linearStiffness,linearDamping,
                      angularStiffness,angularDamping,contactNumber);


  }

  /// version when the position is perfectly known but not the stiffness and damping
  int KineticsObserver::addContact(const Kinematics & pose, int contactNumber)
  {
    return addContact(pose,contactInitCovMat_,contactProcessCovMat_,
                      linearStiffnessMatDefault_,linearDampingMatDefault_,
                      angularDampingMatDefault_,angularDampingMatDefault_,contactNumber);
  }

  void KineticsObserver::removeContact(int contactNbr)
  {
    MapContactIterator i = contacts_.find(contactNbr);
    BOOST_ASSERT(i!=contacts_.end() && "Tried to remove a non-existing contact.");
    if (i != contacts_.end()) 
    {
      contacts_.erase(i);
    }
  }

  void KineticsObserver::clearContacts()
  {
    contacts_.clear();
  }

  size_t KineticsObserver::getNumberOfContacts() const
  {
    return contacts_.size();
  }

  std::vector<int> KineticsObserver::getListOfContacts() const
  {
    std::vector<int> v;

    for (MapContactConstIterator i = contacts_.begin(); i!=contacts_.end();++i)
    {
      v.push_back(i->first);
    }
    return v;
  }

  void KineticsObserver::setStateCovariance(const Matrix & P)
  {
    ekf_.setStateCovariance(P);
  }

  void KineticsObserver::setKinematicsStateCovariance(const Matrix & P_kine)
  {
    Matrix P = ekf_.getStateCovariance();
    setBlockStateCovariance<sizeStateKineTangent>(P,P_kine,kineIndex(),stateSize_);
    ekf_.setStateCovariance(P);
  }

  void KineticsObserver::setKinematicsInitCovarianceDefault(const Matrix & P_kine)
  {
    stateKineMatricsInitCovMat_=P_kine;
  }

  void KineticsObserver::setGyroBiasStateCovariance(const Matrix3 & covMat)
  {
    Matrix P = ekf_.getStateCovariance();
    setBlockStateCovariance<sizeGyroBias>(P,covMat,gyroBiasIndex(),stateSize_);
    ekf_.setStateCovariance(P);
  }

  void KineticsObserver::setGyroBiasInitCovarianceDefault(const Matrix3 & covMat)
  {
    gyroBiasInitCovMat_ = covMat;
  }
  
  void KineticsObserver::setGyroBiasProcessCovariance(const Matrix3 & covMat)
  {
    Matrix P = ekf_.getProcessCovariance();
    setBlockStateCovariance<sizeGyroBias>(P,covMat,gyroBiasIndex(),stateSize_);
    ekf_.setProcessCovariance(P);
  }

  void KineticsObserver::setUnmodeledWrenchStateCovMat(const Matrix6 & currentCovMat)
  {
    Matrix P = ekf_.getStateCovariance();
    setBlockStateCovariance<sizeWrench>(P,currentCovMat,unmodeledWrenchIndex(),stateSize_);
    ekf_.setStateCovariance(P);
  }

  void KineticsObserver::setUnmodeledWrenchIniCovMatDefault(const Matrix6 & initCovMat)
  {
    unmodeledWrenchInitCovMat_=initCovMat;
  }

  void KineticsObserver::setUnmodeledWrenchProcessCovMat(const Matrix6 & processCovMat)
  {
    Matrix P = ekf_.getProcessCovariance();
    setBlockStateCovariance<sizeWrench>(P,processCovMat,unmodeledWrenchIndex(),stateSize_);
    ekf_.setProcessCovariance(P);
  }

  void KineticsObserver::setContactStateCovMat(int contactNbr, const Matrix12 & contactCovMat)
  {
    Matrix P = ekf_.getStateCovariance();
    setBlockStateCovariance<sizeContactTangent>(P,contactCovMat,contactIndex(contactNbr),stateSize_);
    ekf_.setStateCovariance(P);
  }

  void KineticsObserver::setContactInitCovMatDefault(const Matrix12 & contactCovMat)
  {
    contactInitCovMat_ = contactCovMat;
  }

  void KineticsObserver::setContactProcessCovMat(int contactNbr, const Matrix12 & contactCovMat)
  {
    Matrix P = ekf_.getProcessCovariance();
    setBlockStateCovariance<sizeContactTangent>(P,contactCovMat,contactIndex(contactNbr),stateSize_);
    ekf_.setProcessCovariance(P);
  }

  Matrix KineticsObserver::getStateCovariance() const 
  {
    return ekf_.getStateCovariance();
  }

  void KineticsObserver::setProcessNoiseCovariance(const Matrix & Q)
  {
    ekf_.setProcessCovariance(Q);
  }

  size_t KineticsObserver::getMeasurementSize()
  {
    size_t size = 0;
    if (k_est!=k_data)
    {
      for (MapIMUIterator i = imuSensors_.begin(); i != imuSensors_.end(); ++i) 
      {
        if (i->second.time==k_data) 
        {
          size+=sizeIMUSignal;
        } 
      }

      for (MapContactIterator i= contacts_.begin(), ie = contacts_.end(); i!=ie ; ++i) 
      {
        if (i->second.time == k_data && i->second.withRealSensor)
        {
          size+=sizeWrench;
        }
      }

      if (absPoseSensor_.time == k_data)
      {
        size+=sizePose;
      }  

    }
    
    return size;
  }


  Vector KineticsObserver::getMeasurementVector()
  {
    Vector measurement(getMeasurementSize());
    size_t currIndex = 0;
    if (k_est!=k_data)
    {
      for (MapIMUIterator i = imuSensors_.begin(); i != imuSensors_.end(); ++i) 
      {
        if (i->second.time==k_data) 
        {
          measurement.segment<sizeIMUSignal>(currIndex) = i->second.acceleroGyro;
          currIndex+=sizeIMUSignal;
        } 
      }

      for (MapContactIterator i= contacts_.begin(), ie = contacts_.end(); i!=ie ; ++i) 
      {
        if (i->second.time == k_data && i->second.withRealSensor)
        {
          measurement.segment<sizeWrench>(currIndex) = i->second.wrench;
          currIndex+=sizeWrench;
        }
      }

      if (absPoseSensor_.time == k_data)
      {
        measurement.segment<sizePose>(currIndex) = absPoseSensor_.pose.toVector(flagsKineSensor);
        currIndex+=sizePose;
      }
    }    
    return measurement;
  }

  const ExtendedKalmanFilter & KineticsObserver::getEKF() const
  {
    return ekf_;
  }

  ExtendedKalmanFilter & KineticsObserver::getEKF()
  {
    return ekf_;
  }

  void KineticsObserver::resetStateCovarianceMat()
  {
    resetStateKinematicsCovMat();
    resetStateGyroBiasCovMat();
    resetStateUnmodeledWrenchCovMat();  
    resetStateContactsCovMat();
  }

  void KineticsObserver::resetStateKinematicsCovMat()
  {
    Matrix P = ekf_.getStateCovariance();
    setBlockStateCovariance<sizeContactTangent>(P,stateKineMatricsInitCovMat_,kineIndex(),stateSize_);
    ekf_.setStateCovariance(P);
  }

  void KineticsObserver::resetStateGyroBiasCovMat()
  {
    Matrix P = ekf_.getStateCovariance();
    setBlockStateCovariance<sizeGyroBias>(P,gyroBiasInitCovMat_,gyroBiasIndex(),stateSize_);
    ekf_.setStateCovariance(P);    
  }


  void KineticsObserver::resetStateUnmodeledWrenchCovMat()
  {
    Matrix P = ekf_.getStateCovariance();
    setBlockStateCovariance<sizeWrench>(P,unmodeledWrenchInitCovMat_,unmodeledForceIndex(),stateSize_);
    ekf_.setStateCovariance(P);    
  }

  void KineticsObserver::resetStateContactsCovMat()
  {
    for (MapContactIterator i = contacts_.begin(); i!=contacts_.end();++i)
    {
      resetStateContactCovMat_(i);
    }
  }

  void KineticsObserver::resetStateContactCovMat_(MapContactIterator i)
  {
    Matrix P = ekf_.getStateCovariance();
    setBlockStateCovariance<sizeContactTangent>(P,contactInitCovMat_,i->second.stateIndex,stateSize_);
    ekf_.setStateCovariance(P);
  }

  void KineticsObserver::resetStateContactCovMat(int contactNbr)
  {
    MapContactIterator i = contacts_.find(contactNbr);

    BOOST_ASSERT(i!=contacts_.end() && "Tried to set the covariance of a non existant contact");
    
    if (i!=contacts_.end())
    {
      resetStateContactCovMat_(i);
    }
  }

  void KineticsObserver::resetProcessCovarianceMat()
  {
    resetProcessKinematicsCovMat();
    resetProcessGyroBiasCovMat();
    resetProcessUnmodeledWrenchCovMat();  
    resetProcessContactsCovMat();
  }

  void KineticsObserver::resetProcessKinematicsCovMat()
  {
    Matrix P = ekf_.getProcessCovariance();
    setBlockStateCovariance<sizeContactTangent>(P,stateKineMatricsProcessCovMat_,kineIndex(),stateSize_);
    ekf_.setProcessCovariance(P);
  }

  void KineticsObserver::resetProcessGyroBiasCovMat()
  {
    Matrix P = ekf_.getProcessCovariance();
    setBlockStateCovariance<sizeGyroBias>(P,gyroBiasProcessCovMat_,gyroBiasIndex(),stateSize_);
    ekf_.setProcessCovariance(P);   
  }

  void KineticsObserver::resetProcessUnmodeledWrenchCovMat()
  {
    Matrix P = ekf_.getProcessCovariance();
    setBlockStateCovariance<sizeWrench>(P,unmodeledWrenchProcessCovMat_,unmodeledForceIndex(),stateSize_);
    ekf_.setProcessCovariance(P);   
  }

  void KineticsObserver::resetProcessContactsCovMat()
  {
    for (MapContactIterator i = contacts_.begin(); i!=contacts_.end();++i)
    {
      resetProcessContactCovMat_(i);
    }
  }

  void KineticsObserver::resetProcessContactCovMat(int contactNbr)
  {
    MapContactIterator i = contacts_.find(contactNbr);

    BOOST_ASSERT(i!=contacts_.end() && "Tried to set the covariance of a non existant contact");
    
    if (i!=contacts_.end())
    {
      resetProcessContactCovMat_(i);
    }
  }

  void KineticsObserver::resetProcessContactCovMat_(MapContactIterator i)
  {
    Matrix P = ekf_.getProcessCovariance();
    setBlockStateCovariance<sizeContactTangent>(P,contactProcessCovMat_,i->second.stateIndex,stateSize_);
    ekf_.setProcessCovariance(P);
  }

  void KineticsObserver::resetSensorsDefaultCovMat()
  {
    acceleroCovMatDefault_=Matrix3::Identity()*acceleroVarianceDefault;
    gyroCovMatDefault_ = Matrix3::Identity()*gyroVarianceDefault;
    contactWrenchSensorCovMatDefault_ = blockMat6( Matrix3::Identity()*forceSensorVarianceDefault, Matrix3::Zero(),
                                Matrix3::Zero(), Matrix3::Identity()*torqueSensorVarianceDefault );
    absPoseSensorCovMatDefault_= blockMat6( Matrix3::Identity()*positionSensorVarianceDefault, Matrix3::Zero(),
                                Matrix3::Zero(), Matrix3::Identity()*orientationSensorVarianceDefault );
  }

  void KineticsObserver::resetInputs()
  {
    for (MapIMUIterator i = imuSensors_.begin(); i!= imuSensors_.end();++i)
    {
      i->second.time = k_est;
    }

    for (MapContactIterator i = contacts_.begin(); i != contacts_.end(); ++i)
    {
      i->second.time = k_est;
    }

    absPoseSensor_.time = k_est;
  }

  Vector KineticsObserver::stateNaNCorrection_()
  {
    ///TODO implement this function
    assert(false && "NaN Correction not yet implemented. Please Contact mehdi.benallegue@gmail.com");
    return oldStateVector_;
  }

  void KineticsObserver::startNewIteration_()
  {
    if (k_est==k_data)
    {
      ++k_data;
    }    
  } 

  void KineticsObserver::updateKine_()
  {
    stateKinematics_.fromVector(stateVector_.segment<sizeStateKine>(kineIndex()), 
                                flagsStateKine);
  }

  void KineticsObserver::addContactAndUnmodeledWrench(const Vector &stateVector, Vector3 & force, Vector3 & torque)
  {
    force += stateVector.segment<sizeForce>(unmodeledWrenchIndex());
    torque += stateVector.segment<sizeForce>(unmodeledTorqueIndex());

    for (MapContactIterator i = contacts_.begin(); i!= contacts_.end(); ++i)
    {
      Kinematics & localKinei= i->second.localKine;
      Vector3 localForcei = localKinei.orientation * stateVector.segment<sizeForce>(contactForceIndex(i));
      force += localForcei;
      torque += localKinei.orientation * stateVector.segment<sizeForce>(contactTorqueIndex(i)) + 
                localKinei.position().cross(localForcei);
    }
  }

  void  KineticsObserver::computeAccelerations(Kinematics & stateKine, const Vector3& totalForceLocal,
                                const Vector3& totalMomentLocal, Vector3 & linAcc, Vector3& angAcc)
  {
    Matrix3 Rt =  stateKine.orientation.getMatrixRef().inverse();
    Vector3 Rtw = Rt * stateKine.angVel();
    Vector3 corioCentri = 2* Rtw.cross(comd_()+Rtw.cross(com_()));


       
    angAcc = stateKine.orientation *( ( I_() + mass_ * kine::skewSymmetric2(com_())).inverse()
           * (totalMomentLocal - Id_()* Rtw -sigmad_() - Rtw.cross(I_()*Rtw+sigma_()) 
           - com_().cross(totalForceLocal - mass_*(comdd_() + corioCentri ))));

    linAcc =  stateKine.orientation * ((totalForceLocal/mass_) - comdd_() 
              - corioCentri + com_().cross(Rt * angAcc) ) - cst::gravity;
   
  }

  void KineticsObserver::contactForces( MapContactIterator i, Kinematics &stateKine, 
                                            Kinematics &contactPose , Vector3 & force, Vector3 torque) 
  {
    Contact & contact = i->second;

    Kinematics & localKine = contact.localKine;

    Kinematics globalKine = stateKine*localKine;

    Matrix3 globKineOriInverse = globalKine.orientation.inverse();

    force = globKineOriInverse * 
            (contact.linearStiffness* (contactPose.position()-globalKine.position())
            -  contact.linearDamping * globalKine.linVel());
    torque = globKineOriInverse * 
            (-0.5 * contact.angularStiffness *
            ( Quaternion(globalKine.orientation) * Quaternion(contactPose.orientation).inverse() ).vec()
           -contact.angularDamping * globalKine.angVel()) ;

  }

  Vector KineticsObserver::stateDynamics(const Vector &xInput, const Vector &u, TimeIndex k)
  {
    Vector x = xInput;
    Vector3 forceLocal = additionalForce_;
    Vector3 torqueLocal = additionalTorque_;

    addContactAndUnmodeledWrench(x,forceLocal,torqueLocal);

    Kinematics stateKine(x.segment<sizeStateKine>(kineIndex()), flagsStateKine);

    Vector3 linacc, angacc;

    computeAccelerations(stateKine,forceLocal,torqueLocal,linacc,angacc);

    stateKine.linAcc = linacc;
    stateKine.angAcc = angacc;

    stateKine.integrate(dt_);

    x.segment<sizeStateKine>(kineIndex()) = stateKine.toVector(flagsStateKine);

    
    

    
  }



}
