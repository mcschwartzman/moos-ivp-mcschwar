/************************************************************/
/*    NAME: Mathew Schwartzman                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: Odometry.cpp                                    */
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
  m_staleness_threshold = 10;

  m_last_x = 0;
  m_last_y = 0;
  m_odometry_dist = 0;
  m_time_of_last_location = 0;

  m_stale_nav = false;
  m_odometry_ready = true;

  // If you'd like to support new units, 
  // just add their string representation,
  // and their multiplier from meters (m),
  // to this hashmap! 
  // Unsupported units will throw a config warning!
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
    string string_value = msg.GetString();
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
      m_stale_nav = false;

      cout << "nav_x: " << value << endl;

      // pushes latest x value to x queue, to be processed later in iterate()
      m_x_queue.push(value);
    }
    else if(key == "NAV_Y"){
      m_time_of_last_location = MOOSTime();
      m_stale_nav = false;

      cout << "nav_y: " << value << endl;

    // pushes latest y value to y queue, to be processed later in iterate()
      m_y_queue.push(value);
    }
    else if(key == "RESET_REQUEST"){
      if (string_value == "true"){
        cout << "reset request received!" << endl;
        m_odometry_dist = 0;
      }
    }
     // if we receive an update to the configs
     else if(key == "ODOMETRY_UPDATES"){
       bool handled = setParam(string_value);
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

  // capture current time to calculate staleness
  double current_time = MOOSTime();

  // create staleness warning, the retractor and reporter input must match
  string stale_threshold = to_string(m_staleness_threshold);
  string stale_message = "No NAV data for over " + stale_threshold + " seconds";

  // determine staleness of coordinates
  if (current_time - m_time_of_last_location > m_staleness_threshold) {
    m_stale_nav = true;
  }

  // report a run warning if the coordinates are stale
  if (m_stale_nav) {
    reportRunWarning(stale_message);
  }
  else {
    retractRunWarning(stale_message);
  }
  
  // if for some reason we cannot publish odometry (i.e. some bad configs)
  if(!m_odometry_ready){
    return(true);
  }

  while ((m_x_queue.size() > 1) && (m_y_queue.size() > 1)){

    // buffer the last point...
    m_last_x = m_x_queue.front(); //m_current_x;
    m_last_y = m_y_queue.front(); // m_current_y;

    // ... and clear the queue
    m_x_queue.pop();
    m_y_queue.pop();

    // use pythagorean theorem to get 
    // straight-line distance between 
    // current and last point 
    double diff_x = m_x_queue.front() - m_last_x;
    double diff_y = m_y_queue.front() - m_last_y;

    double x_diff_squared = diff_x * diff_x;
    double y_diff_squared = diff_y * diff_y;
    m_odometry_dist = m_odometry_dist + sqrt(x_diff_squared + y_diff_squared);

    cout << "odometry_dist: " << m_odometry_dist << endl;

  }
  // publish default odometry distance as meters
  Notify("ODOMETRY_DIST", m_odometry_dist);

  // for every item in the additional unit list, 
  // publish the odometry converted from meters
  // i.e. km would be published to ODOMETRY_DIST_KM
  for (int i=0; i<m_additional_units_list.size(); i++){
    string additional_units = m_additional_units_list[i];
    double odometry_value = m_odometry_dist * m_units_map[additional_units];
    Notify("ODOMETRY_DIST_" + toupper(additional_units), odometry_value);
  }

  AppCastingMOOSApp::PostReport();

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
    
    // line should be of format key=value for use by setParam method
    bool handled = setParam(line);

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
  // register for local coordinates
  Register("NAV_X", 0);
  Register("NAV_Y", 0);
  Register("RESET_REQUEST", 0);
  
  // variable to receive runtime updates to the config
  Register("ODOMETRY_UPDATES", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool Odometry::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "File: Odometry.cpp                          " << endl;
  m_msgs << "============================================" << endl;

  ACTable actab(6);
  actab << "Last NAV_X | Last NAV_Y | ODOMETRY_DIST | X Queue Size | Y Queue Size | Odom Ready";
  actab.addHeaderLines();
  actab << m_last_x << m_last_y << m_odometry_dist << to_string(m_x_queue.size()) << to_string(m_y_queue.size()) << m_odometry_ready;
  m_msgs << actab.getFormattedString();

  return(true);
}

//---------------------------------------------------------
// Procedure: setParam(string line)
// Abstracts out parameter setting for reuse during runtime
// Input line should be of the format key=value, like in the .moos config
bool Odometry::setParam(string line){

    string param = tolower(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "staleness_threshold") {

      if (stoi(value) <= 0) {
        m_odometry_ready = false;
        reportConfigWarning("Staleness threshold " + value + " seconds invalid");
      }
      else {
        m_staleness_threshold = stoi(value);
      }
      handled = true;
    }
    else if(param == "other_odometry_units") {

      // if the new units to use are NOT supported in the hashmap
      if (m_units_map.find(value) == m_units_map.end()){
        reportConfigWarning("Invalid additional units (" + value + ")!");
      }
      // if the new runtime units are already in list of units to publish
      else if (find(m_additional_units_list.begin(), m_additional_units_list.end(), value) != m_additional_units_list.end()) {
        cout << "skipping duplicate units to publish!" << endl;
      }
      // we're good, add these new units to the list of units to publish
      else {
        m_additional_units_list.push_back(value);
      }
      handled = true;
    }

  return true;
}
