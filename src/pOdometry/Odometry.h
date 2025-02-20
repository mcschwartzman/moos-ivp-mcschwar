/************************************************************/
/*    NAME: Mathew Schwartzman                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: Odometry.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef Odometry_HEADER
#define Odometry_HEADER

#include <string>
#include <unordered_map>
#include <vector>
#include <queue>

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class Odometry : public AppCastingMOOSApp
{
public:
  Odometry();
  ~Odometry();

protected: // Standard MOOSApp functions to overload  
  bool OnNewMail(MOOSMSG_LIST &NewMail);
  bool Iterate();
  bool OnConnectToServer();
  bool OnStartUp();

protected: // Standard AppCastingMOOSApp function to overload 
  bool buildReport();

protected:
  void registerVariables();
  bool setParam(std::string line);

private: // Configuration variables
  int m_staleness_threshold;

private: // State variables
  double m_last_x;
  double m_last_y;
  double m_odometry_dist;
  double m_time_of_last_location;

  bool m_stale_nav;
  bool m_odometry_ready;

  std::unordered_map<std::string, double> m_units_map;

  std::queue<double> m_x_queue;
  std::queue<double> m_y_queue;

  std::vector<std::string> m_additional_units_list;
};

#endif 
