/// Sample Ccommandline Argument
// ./ninja_gui --groundmeshfile=/Users/saghli/code/datasets/meshes/lab.ply --paramfile=/Users/saghli/code/spirit/parameter_files/gui_params.csv

//#include <glog/logging.h>
#include <chrono>
#include <math.h>
#include <spirit/spirit.h>
#include <HAL/Gamepad.pb.h>
#include <HAL/Gamepad/GamepadDevice.h>
#include <HAL/Utils/GetPot>
#include <HAL/Car.pb.h>
#include <HAL/Car/CarDevice.h>
#include <memory>

#define SIM_CALIB

double steering_signal = 0;
double throttle_signal = 0;
void GamepadCallback(hal::GamepadMsg& _msg) {
  steering_signal = _msg.axes().data(0)*SP_PI_QUART;
  throttle_signal = -_msg.axes().data(3)*80;
//  std::cout << "steeromg command is " << steering_signal << std::endl;
//  std::cout << "throttle command is " << throttle_signal << std::endl;
}

int main(int argc, char** argv) {

  // get command line arguments
  GetPot cl_args(argc, argv);

//  bool has_ninjacar_ = cl_args.search("-car");
//  std::string car_uri = cl_args.follow("", "-car");

  hal::Gamepad gamepad("gamepad:/");
  gamepad.RegisterGamepadDataCallback(&GamepadCallback);

  spSettings settings_obj;
  settings_obj.SetGuiType(spGuiType::GUI_PANGOSCENEGRAPH);
  settings_obj.SetPhysicsEngineType(spPhyEngineType::PHY_BULLET);

  spirit spworld(settings_obj);

#ifdef SIM_CALIB
  // create a simulated car
  spworld.car_param.wheel_friction = 0.3;
  spworld.car_param.wheel_rollingfriction = 0.1;
  spworld.car_param.engine_torque = 0.0001;
  spworld.car_param.chassis_mass = 5;
//  spworld.car_param.steering_servo_max_velocity = 1;
  spObjectHandle car_handle = spworld.objects_.CreateVehicle(spworld.car_param);
  spworld.gui_.AddObject(spworld.objects_.GetObject(car_handle));
  spAWSDCar& car = (spAWSDCar&) spworld.objects_.GetObject(car_handle);
  // create a flat ground
  spPose gnd_pose_ = spPose::Identity();
  gnd_pose_.translate(spTranslation(0,0,-0.5));
  spObjectHandle gnd_handle = spworld.objects_.CreateBox(gnd_pose_,spBoxSize(50,50,1),0,spColor(0,1,0));
  spworld.gui_.AddObject(spworld.objects_.GetObject(gnd_handle));
  // set ground friction
  ((spBox&)spworld.objects_.GetObject(gnd_handle)).SetFriction(1);
  ((spBox&)spworld.objects_.GetObject(gnd_handle)).SetRollingFriction(0.1);

#endif

  spworld.car_param.wheel_friction = 0.5;
  double wheel_base = 0.40;
  spworld.car_param.wheels_anchor[0][1] = wheel_base/2;
  spworld.car_param.wheels_anchor[1][1] = -wheel_base/2;
  spworld.car_param.wheels_anchor[2][1] = -wheel_base/2;
  spworld.car_param.wheels_anchor[3][1] = wheel_base/2;

  unsigned int window_size = 10;
  unsigned int queue_size = 10;
  double batch_min_entropy = 10;
  // create a candidate_window and a priority queue
  PriorityQueue priority_queue(queue_size,spworld.car_param);
  // create a new candidate window
  CandidateWindow candidate_window(window_size,1,spworld.car_param/*,&spworld.gui_*/);
  candidate_window.prqueue_func_ptr_ = std::bind(&PriorityQueue::PushBackCandidateWindow,&priority_queue,std::placeholders::_1);
  int iter_cnt = 0;
  spPose vicon_pose;
  spPose vicon_prev_pose;
  spTimestamp timestamp;
  spTimestamp prev_timestamp;
  spLinVel linvel;
  spRotVel rotvel;
bool flag0 = true;
  while(spworld.ShouldRun()) {
//    if(iter_cnt<20) {
//      steering_signal = 0;
//      throttle_signal = 100;
//    } else if(iter_cnt<window_size/2) {
//      steering_signal = 0.7;
//      throttle_signal = 100;
//    } else {
//      steering_signal = 0.7;
//      throttle_signal = 150;
//    }
//    if(iter_cnt%20<12) {
//      steering_signal = -0.7;
//      throttle_signal = 100;
//    } else {
//      steering_signal = 0;
//      throttle_signal = 50;
//    }
    iter_cnt++;
    // Get Car's Pose
#ifdef SIM_CALIB
    vicon_prev_pose = vicon_pose;
    prev_timestamp = timestamp;

    vicon_pose = car.GetPose();
    timestamp = spGeneralTools::Tick();

    spPose diff = vicon_prev_pose.inverse()*vicon_pose;
    linvel = (vicon_pose.translation()-vicon_prev_pose.translation())/0.1;
    Eigen::AngleAxisd angleaxis(diff.rotation());
    Eigen::Vector3d rotvec(angleaxis.angle()*angleaxis.axis());
    rotvel = rotvec/0.1/*(spGeneralTools::TickTock_us(prev_timestamp,timestamp)/1e6)*/;
    if(flag0) {
      linvel = spLinVel::Zero();
      rotvel = spRotVel::Zero();
      flag0 = false;
    }
#endif
    // Ger Car's wheel speeds
    spWheelSpeedVec wheel_speeds;
    for(int ii=0; ii<4; ii++) {
      wheel_speeds[ii] = car.GetWheel(ii)->GetWheelSpeed();
    }
    // create car state from pose and wheel odometry
    spState current_state(car.GetState());
    current_state.substate_vec.clear();
    current_state.pose = vicon_pose;
    current_state.wheel_speeds = wheel_speeds;
    current_state.front_steering = 0.5*(car.GetWheel(0)->GetSteeringServoCurrentAngle()+car.GetWheel(3)->GetSteeringServoCurrentAngle());
    current_state.linvel = car.GetState().linvel;
    current_state.rotvel = car.GetState().rotvel;
    current_state.time_stamp = spGeneralTools::Tick();
    current_state.current_controls.first = steering_signal;
    current_state.current_controls.second = throttle_signal;
    candidate_window.PushBackState(current_state);
//    std::cout << "angle is " << car.GetWheel(0)->GetSteeringServoCurrentAngle() << std::endl;

    spVehicleConstructionInfo params;
    if(priority_queue.GetParameters(params)) {
      candidate_window.SetParams(params);
    }
//    std::cout << "**********************" << std::endl;
//    std::cout << "cars is\t" << car.GetState().rotvel.transpose() << std::endl;
//    std::cout << "mine is\t" << rotvel.transpose() << std::endl;
/*
    if(candidate_window.PushBackState(current_state) == window_size) {
      if(candidate_window.OptimizeParametersInWindow(spworld.car_param,&spworld.gui_)) {
        //      std::cout << "entropy is " << candidate_window.GetEntropy() << std::endl;
        // if candidate window has lower entropy than max entropy of queue then replace with highest one.
        // if there are at least two candidate windows then optimize parameters over whole queue
        if(priority_queue.PushBackCandidateWindow(candidate_window) > 1) {
          priority_queue.OptimizeParametersInQueue(spworld.car_param);
          // update new parameters to test car here
        }
      }
    }
    */
    // apply the controls and wait for next state update
#ifdef SIM_CALIB
    car.SetFrontSteeringAngle(current_state.current_controls.first);
    car.SetEngineMaxVel(current_state.current_controls.second);
    // step forward the simulated car
    spTimestamp t0 = spGeneralTools::Tick();
    spworld.objects_.StepPhySimulation(0.1);
    double tt = spGeneralTools::Tock_ms(t0);

//    spGeneralTools::Delay_ms(100);
    spworld.gui_.Iterate(spworld.objects_);

#endif
  }
  std::cout << "Done ... !" << std::endl;
  return 0;
}
