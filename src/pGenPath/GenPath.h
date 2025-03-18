/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef GenPath_HEADER
#define GenPath_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "XYPoint.h"
#include <string>
#include <vector>
#include <queue>

class GenPath : public AppCastingMOOSApp
{
 public:
   GenPath();
   ~GenPath();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();

   std::vector<XYPoint> generatePath(std::vector<XYPoint> visit_points);
   void sendPath(std::vector<XYPoint> visit_points);

  double pythagorean(XYPoint a, XYPoint b);

 private: // Configuration variables

  double m_visit_radius;

 private: // State variables
  std::queue<std::string> m_point_strings;
  std::vector<XYPoint> m_visit_points;
  std::vector<XYPoint> m_path;
  std::vector<XYPoint> m_missed_waypoints;

  double m_current_x;
  double m_current_y;

  int m_next_wpt;
  
  bool m_genpath_regenerate;
  bool m_ready_to_visit;
  bool m_received_x;
  bool m_received_y;
  bool m_successfully_visited;
  bool m_new_wpt;
  bool m_receiving_points;
  bool m_ready_to_generate_path;
};

#endif 
