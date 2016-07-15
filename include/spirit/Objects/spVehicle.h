#ifndef SP_VEHICLE_H__
#define SP_VEHICLE_H__

#include <spirit/Objects/spCommonObject.h>
#include <vector>
#include <spirit/Objects/spWheel.h>

// 1 - Geometric center of the chassis box will be the center of vehicle object and
// any local transformation will be with respect to coordinate system in this
// center.
// 2 - every transformation is in global coordinates unless it has name "Local"
// in its fucntion name.
class spVehicle : public spCommonObject {
 public:
  spVehicle(const spVehicleConstructionInfo& vehicle_info);
  ~spVehicle();
  void SetPose(const spPose& pose);
  const spPose& GetPose();
  void SetColor(const spColor& color);
  int GetNumberOfWheels();
  spWheel* GetWheel(int index);
  double GetChassisMass();
  void SetChassisMass(double mass);
  const spPose& GetWheelOrigin(int index);
  const spBoxSize& GetChassisSize();
  const spPose& GetLocalCOG();
  const spPose& GetGlobalCOG();
  void MoveWheelsToAnchors(void);

 private:
  std::vector<std::shared_ptr<spWheel>> wheel_;
  spPose pose_;        // this pose will represent geometric center of the car
  spPose cog_local_;  // center of gravity
  spColor color_;
  spBoxSize chassis_size_;
  void SetWheelOrigin(int index, const spPose& pose);
  void SetWheelAnchor(int index, const spTranslation& tr);
};

#endif  //  SP_VEHICLE_H__