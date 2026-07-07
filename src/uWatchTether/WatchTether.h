/************************************************************/
/*    NAME: Mathew Schwartzman                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: WatchTether.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef WatchTether_HEADER
#define WatchTether_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class WatchTether : public AppCastingMOOSApp
{
 public:
   WatchTether();
   ~WatchTether();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();

 private: // Configuration variables

 private: // State variables
  double m_tether_length = 0;
  double m_tether_depth = 0;
  
  int m_tether_snaps = 0;
  int m_tether_hockles = 0;
};

#endif 
