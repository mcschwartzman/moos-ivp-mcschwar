/************************************************************/
/*    NAME: Mathew Schwartzman                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Pulse.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef Pulse_HEADER
#define Pulse_HEADER

#include <string>
#include "IvPBehavior.h"

class BHV_Pulse : public IvPBehavior {
public:
  BHV_Pulse(IvPDomain);
  ~BHV_Pulse() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  void         sendPulse();
  IvPFunction* onRunState();

protected: // Local Utility functions

protected: // Configuration parameters
  double m_pulse_radius;
  double m_pulse_duration;
  double m_pulse_delay;

protected: // State variables
  double m_position_x;
  double m_position_y;
  double m_current_time;
  double m_arrival_time;

  bool m_pulse_needed;

  int m_waypoint_index;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_Pulse(domain);}
}
#endif
