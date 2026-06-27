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
  double       calculateOuterRing();
  double       calculateIdeal();

  NodeRecord m_contact_node_report;
  IvPFunction* onRunState();

protected: // Local Utility functions

protected: // Configuration parameters
  std::string m_leader;
  
  double m_tether_length = 30; // in meters
  double m_inner_ring    = 10;
  double m_ideal_ring    = 20;

protected: // State variables
    double m_contact_depth = 10; // meters
    double m_ownship_depth = 0;
    double m_outer_ring = m_tether_length;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_Tether(domain);}
}
#endif
