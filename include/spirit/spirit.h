#ifndef SPIRIT_H__
#define SPIRIT_H__

#include <spirit/Gui.h>
#include <spirit/Controllers/spPID.h>
#include <spirit/spSettings.h>
#include <spirit/Types/spTypes.h>
#include <spirit/Objects.h>
#include <spirit/Planners/spBezierPlanner.h>
#include <functional>

class spirit{
public:
  spirit(spSettings& user_settings);
  ~spirit();
  void Create();
  bool ShouldRun();
  void IterateWorld();
  void CheckKeyboardAction();
  void ScenarioWorldBoxFall();
  void ScenarioWorldCarFall();
  void ScenarioPlannerTest();
  void CalcJacobianTest(spVehicleConstructionInfo& car_param,spPlannerJacobian& jacobian, spStateVec& end_state, const spCtrlPts2ord_2dof& cntrl_vars, unsigned int num_sim_steps, double sim_step_size, const spPose& init_pose, double fd_delta);
  void ScenarioGNTest();
  void ScenarioPIDController();
  void CalcLocalPlannerJacobian();
private:
  void InitCarPool(int num_cars);
  std::vector<Objects> pool_objects_vec_;
  std::vector<std::thread> pool_threads_vec_;
  Gui gui_;
  spSettings user_settings_;
  Objects objects_;
  spObjectHandle obj_gnd_index;
  spObjectHandle obj_box_index;
  spObjectHandle obj_car_index;
  spObjectHandle obj_cars_index[9];
  spObjectHandle obj_waypoints_index[9];
  spObjectHandle obj_waypoint_index0;
  spObjectHandle obj_waypoint_index1;
  spObjectHandle obj_waypoint_index2;
  spObjectHandle obj_linestrip_index;
};

#endif  //SPIRIT_H__
