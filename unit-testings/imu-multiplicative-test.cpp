#include <iostream>
#include <fstream>

#include <state-observation/noise/gaussian-white-noise.hpp>
#include <state-observation/examples/imu-attitude-trajectory-reconstruction.hpp>
#include <state-observation/examples/imu-multiplicative-attitude-reconstruction.hpp>

using namespace stateObservation;

typedef kine::indexes<kine::rotationVector> indexes1;
typedef kine::indexes<kine::quaternion> indexes2;

int test()
{
    /// The number of samples
    const unsigned kmax=3000;

    ///sampling period
    const double dt=1e-3;



    ///The array containing all the states, the measurements and the inputs
    IndexedVectorArray x;
    IndexedVectorArray y;
    IndexedVectorArray u;

    const unsigned measurementSize=6;

    ///The covariance matrix of the process noise and the measurement noise
    Matrix q;
    Matrix r;

    {

        ///Sizes of the states for the state, the measurement, and the input vector
        const unsigned stateSize=18;

        //const unsigned inputSize=6;
        ///simulation of the signal
        /// the IMU dynamical system functor
        IMUDynamicalSystem imu;

        ///The process noise initialization
        Matrix q1=Matrix::Identity(stateSize,stateSize)*0.001;
        GaussianWhiteNoise processNoise(imu.getStateSize());
        processNoise.setStandardDeviation(q1);
        imu.setProcessNoise( & processNoise );
        q=q1*q1.transpose();

        ///The measurement noise initialization
        Matrix r1=Matrix::Identity(measurementSize,measurementSize)*0.01;
        GaussianWhiteNoise MeasurementNoise(imu.getMeasurementSize());
        MeasurementNoise.setStandardDeviation(r1);
        imu.setMeasurementNoise( & MeasurementNoise );
        r=r1*r1.transpose();

        ///the simulator initalization
        DynamicalSystemSimulator sim;
        sim.setDynamicsFunctor(&imu);

        ///initialization of the state vector
        Vector x0=Vector::Zero(stateSize,1);
        sim.setState(x0,0);

        ///construction of the input
        /// the input is constant over 10 time samples
        for (unsigned i=0;i<kmax/10;++i)
        {
            Vector uk=Vector::Zero(imu.getInputSize(),1);

            uk[0]=0.4 * sin(M_PI/10*i);
            uk[1]=0.6 * sin(M_PI/12*i);
            uk[2]=0.2 * sin(M_PI/5*i);

            uk[3]=10  * sin(M_PI/12*i);
            uk[4]=0.07  * sin(M_PI/15*i);
            uk[5]=0.05 * sin(M_PI/5*i);

            ///filling the 10 time samples of the constant input
            for (int j=0;j<10;++j)
            {
                u.setValue(uk,i*10+j);
            }

            ///give the input to the simulator
            ///we only need to give one value and the
            ///simulator takes automatically the appropriate value
            sim.setInput(uk,10*i);

        }

        ///set the sampling perdiod to the functor
        imu.setSamplingPeriod(dt);

        ///launched the simulation to the time kmax+1
        sim.simulateDynamicsTo(kmax+1);

        ///extract the array of measurements and states
        y = sim.getMeasurementArray(1,kmax);
        x = sim.getStateArray(1,kmax);
    }

    const unsigned stateSize=19;
    const unsigned stateTangentSize=18;

    Matrix q1=Matrix::Identity(stateTangentSize,stateTangentSize)*0.001;
    q=q1*q1.transpose();


    ///the initalization of a random estimation of the initial state
    Vector xh0=Vector::Random(stateSize,1)*3.14;
    xh0.segment<4>(indexes2::ori).normalize();
//    xh0.segment<4>(indexes2::ori)=Quaternion::Identity().coeffs();


    ///computation and initialization of the covariance matrix of the initial state
    Matrix p=Matrix::Identity(stateTangentSize,stateTangentSize);


    IndexedVectorArray xh = examples::imuMultiplicativeAttitudeReconstruction
                                                    (y, u, xh0, p, q, r, dt);


    ///file of output
    std::ofstream f;
    f.open("trajectory.dat");

    double dx;

    ///the reconstruction of the state
    for (TimeIndex i=y.getFirstIndex();i<y.getNextIndex();++i)
    {
        ///display part, useless
        Vector3 g;
        {
            Matrix3 R;
            Vector3 orientationV=Vector(x[i]).segment(indexes1::ori,3);
            double angle=orientationV.norm();
            if (angle > cst::epsilonAngle)
                R = AngleAxis(angle, orientationV/angle).toRotationMatrix();
            else
                R = Matrix3::Identity();
            g=R.transpose()*Vector3::UnitZ();
            g.normalize();
        }

        Vector3 gh;
        {
            Matrix3 Rh;
            Quaternion orientation(Vector(xh[i]).segment<4>(indexes2::ori));
            Rh = orientation.toRotationMatrix();
            gh=Rh.transpose()*Vector3::UnitZ();
            gh.normalize();
        }

        dx= acos(double(g.transpose()*gh));

        f << i<< " \t "<<  dx * 180 / M_PI << " \t\t\t "
        << g.transpose() << " \t\t\t " << gh.transpose() << std::endl;
    }

    std::cout << "Verticality estimation error (degrees):" << dx* 180 / M_PI;

    if (dx* 180 / M_PI < 1)
    {
        std::cout<<" Test succeeded" <<std::endl;
        return 0;
    }
    else
    {
        std::cout<<" Test failed" << std::endl;
        return 1;
    }
}

int main()
{
    return test();

}
