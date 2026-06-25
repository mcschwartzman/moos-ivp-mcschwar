/************************************************************/
/*    NAME: Mathew C. Schwartzman                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Tether.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef Tether_HEADER
#define Tether_HEADER

#include <string>
#include "IvPContactBehavior.h"
#include "NodeRecordUtils.h"

class BHV_Tether : public IvPContactBehavior {
public:
  BHV_Tether(IvPDomain);
  ~BHV_Tether() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  void         drawGraphics();
  float        calculateOuterRing();

  NodeRecord m_contact_node_report;
  float m_contact_depth;
  IvPFunction* onRunState();

protected: // Local Utility functions

protected: // Configuration parameters
  std::string m_leader;
  
  float m_tether_length = 10; // in meters

protected: // State variables
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_Tether(domain);}
}
#endif
