/************************************************************/
/*    NAME: Mathew Schwartzman                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: Odometry.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
#include <cmath>
#include "MBUtils.h"
#include "ACTable.h"
#include "Odometry.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

Odometry::Odometry()
{
  m_last_x = 0;
  m_last_y = 0;
  m_current_x = 0;
  m_current_y = 0;
  m_odometry_dist = 0;
  m_staleness_threshold = 10;

  m_stale_nav = false;
  m_odometry_ready = true;

  m_units_map = {
    {"m", 1},
    {"mi", 0.000621371},
    {"km", 0.001},
    {"nmi", 0.000539957}
  };
}

//---------------------------------------------------------
// Destructor

Odometry::~Odometry()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool Odometry::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();
    double value    = msg.GetDouble();

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

     if(key == "NAV_X"){
       m_time_of_last_location = MOOSTime();
       m_last_x = m_current_x;
       m_current_x = value;
       m_stale_nav = false;
     }
     else if(key == "NAV_Y"){
       m_time_of_last_location = MOOSTime();
       m_last_y = m_current_y;
       m_current_y = value;
       m_stale_nav = false;
     }

     else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool Odometry::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Odometry::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  AppCastingMOOSApp::PostReport();

  double current_time = MOOSTime();

  string m_stale_message = "No NAV data for over " + to_string(m_staleness_threshold) + " seconds!";

  if (current_time - m_time_of_last_location > m_staleness_threshold) {
    m_stale_nav = true;
  }

  if (m_stale_nav) {
    reportRunWarning(m_stale_message);
  }
  else {
    retractRunWarning(m_stale_message);
  }
  
  if(!m_odometry_ready){
    return(true);
  }

  if ((m_last_x != 0) && (m_last_y != 0)){

    double diff_x = m_current_x - m_last_x;
    double diff_y = m_current_y - m_last_y;

    double x_diff_squared = diff_x * diff_x;
    double y_diff_squared = diff_y * diff_y;

    m_odometry_dist += sqrt(x_diff_squared + y_diff_squared);

  }
  Notify("ODOMETRY_DIST", m_odometry_dist);

  if (m_secondary_multiplier > 0){
    Notify("ODOMETRY_DIST_" + toupper(m_secondary_units), m_odometry_dist * m_secondary_multiplier);
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Odometry::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = tolower(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "staleness_threshold") {

      if (stoi(value) <= 0) {
        m_odometry_ready = false;
        reportConfigWarning("Staleness threshold (" + value + " seconds) invalid!");
      }
      else {
        m_staleness_threshold = stoi(value);
      }
      handled = true;
    }
    else if(param == "odometry_units") {
      if (m_units_map.find(value) != m_units_map.end()){
        m_secondary_multiplier = m_units_map[value];
        m_secondary_units = value;
      }
      else {
        reportConfigWarning("Invalid secondary units (" + value + ")!");
      }
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables()

void Odometry::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("NAV_X", 0);
  Register("NAV_Y", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool Odometry::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "File: Odometry.cpp                          " << endl;
  m_msgs << "============================================" << endl;

  ACTable actab(3);
  actab << "Last NAV_X | Last NAV_Y | ODOMETRY_DIST";
  actab.addHeaderLines();
  actab << m_last_x << m_last_y << m_odometry_dist;
  m_msgs << actab.getFormattedString();

  return(true);
}




