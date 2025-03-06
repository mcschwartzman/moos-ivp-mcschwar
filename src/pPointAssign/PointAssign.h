/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef PointAssign_HEADER
#define PointAssign_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "XYPoint.h"
#include <string>
#include <vector>
#include <queue>

class PointAssign : public AppCastingMOOSApp
{
 public:
   PointAssign();
   ~PointAssign();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();
   void postViewPoint(double x, double y, std::string label, std::string color);
   void postAllViewPoints(std::vector<std::string> points_to_post);
   
   XYPoint mostWestPoint(std::vector<XYPoint> points);

 private: // Configuration variables

  bool m_receiving_points;
  bool m_sending_points;

  XYPoint m_most_west_point;
  XYPoint m_most_east_point;

  std::vector<std::string> m_vehicles;
  std::vector<XYPoint> m_points;
  std::queue<std::string> m_point_queue;


 private: // State variables
};

#endif 
