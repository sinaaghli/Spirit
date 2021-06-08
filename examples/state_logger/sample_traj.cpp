
#include <ceres/ceres.h>
#include <ceres/problem.h>
#include "LogParser.h"
#include "AnalyticalCalibCostFunc.h"

int main(){

  // read dataset and store in variables
  LogParser log("/Users/saghli/code/remotelog/log2.csv");

  if(!log.ParseCsvLog()){
    std::cerr << "Error while parsing the log file. Exiting ..." << std::endl;
    return -1;
  }

  log.ComputeStateInputVec();
//  log.ReplaceWithSimData();

  RK4<CalibDerODE> rk4solver(0.01);
  Eigen::VectorXd parameters(14);

  parameters[0] = 4.3;
  parameters[1] = 4.3;
  parameters[2] = 2.27;
  parameters[3] = 0.00008;

  parameters[4] = 0.7;
  parameters[5] = 0;
  parameters[6] = 1;
  parameters[7] = 0;

  parameters[8] = 25;
  parameters[9] = 0;
  parameters[10] = 1;
  parameters[11] = 0;

  parameters[12] = 0.01;
  parameters[13] = 0.04;

  rk4solver.SetParameterVec(parameters);
  Eigen::MatrixXd iter_jac(11,14);
  Eigen::VectorXd curr_u(2);
  Eigen::VectorXd curr_state(11);
  curr_state = Eigen::VectorXd::Zero(11);
  if(log.state_input_vec_[1].data_type == 1){
    curr_state[3] = log.state_input_vec_[1].state[0];
    curr_state[4] = log.state_input_vec_[1].state[1];
    curr_state[5] = log.state_input_vec_[1].state[2];
    curr_state[6] = log.state_input_vec_[1].state[3];
    curr_state[10] = log.state_input_vec_[1].state[4];
  }
  if(log.state_input_vec_[0].data_type == 2){
    curr_state[7] = log.state_input_vec_[0].state[0];
    curr_state[8] = log.state_input_vec_[0].state[1];
    curr_state[9] = log.state_input_vec_[0].state[2];
    curr_state[0] = log.state_input_vec_[0].state[3];
    curr_state[1] = log.state_input_vec_[0].state[4];
    curr_state[2] = log.state_input_vec_[0].state[5];
    std::cout << "vx " << curr_state[0] << std::endl;
    std::cout << "vy " << curr_state[1] << std::endl;
  }

  if(curr_state[0] == 0){
    curr_state[0] = 0.01;
  }

  double time_diff;
  std::ofstream myfile;

  myfile.open("path.csv",std::ofstream::trunc);

//  for(int ii=0; ii<10000; ii++){
  for(int ii=0; ii<log.state_input_vec_.size()-1; ii++){
    curr_u[0] = log.state_input_vec_[ii].input[0];
    curr_u[1] = log.state_input_vec_[ii].input[1];
//    curr_u[0] = 0.7;
//    curr_u[1] = 1;
    time_diff = log.state_input_vec_[ii+1].timestamp - log.state_input_vec_[ii].timestamp;//0.01;
    rk4solver.SolveOnce(curr_state,curr_u,time_diff,iter_jac);

    double linvel_x = std::cos(curr_state[9])*curr_state[0] - std::sin(curr_state[9])*curr_state[1];
    double linvel_y = std::sin(curr_state[9])*curr_state[0] + std::cos(curr_state[9])*curr_state[1];
    for(int ii=0; ii<11; ii++){
      if(ii==0){
        myfile << linvel_x << ",";
      } else if(ii==1){
        myfile << linvel_y << ",";
//      } else if(ii==7){
//        myfile << pose.translation()[0] << ",";
//      } else if(ii==8){
//        myfile << pose.translation()[1] << ",";
      } else {
        myfile << curr_state[ii] << ",";
      }
    }
    myfile << log.state_input_vec_[ii+1].timestamp << ",";
    myfile << "\n";


  }
  myfile.close();


  return 0;
}