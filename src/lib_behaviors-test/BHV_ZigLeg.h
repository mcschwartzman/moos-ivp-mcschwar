/************************************************************/
/*    NAME: Mathew Schwartzman                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ZigLeg.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef ZigLeg_HEADER
#define ZigLeg_HEADER

#include <string>
#include "IvPBehavior.h"

class BHV_ZigLeg : public IvPBehavior {
public:
  BHV_ZigLeg(IvPDomain);
  ~BHV_ZigLeg() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  void         sendPulse();
  IvPFunction* buildFunctionWithZAIC(double current_heading);
  IvPFunction* onRunState();

protected: // Local Utility functions

protected: // Configuration parameters
  double m_pulse_radius;
  double m_pulse_duration;
  double m_pulse_delay;
  double m_zig_duration;
  double m_zig_angle;

protected: // State variables
  double m_position_x;
  double m_position_y;
  double m_heading;
  double m_current_time;
  double m_arrival_time;

  double m_zig_start;
  double m_leg_heading;

  bool m_pulse_needed;
  bool m_zig_needed;

  int m_waypoint_index;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_ZigLeg(domain);}
}
#endif
