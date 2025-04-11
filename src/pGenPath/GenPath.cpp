/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
#include <limits>
#include "MBUtils.h"
#include "ACTable.h"
#include "GenPath.h"
#include "XYFormatUtilsPoint.h"
#include "XYSegList.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

GenPath::GenPath()
{
  m_current_x = 0;
  m_current_y = 0;

  m_visit_radius = 0;

  m_next_wpt = 0;
  m_ready_to_visit = false;
  m_ready_to_generate_path = false;
  m_received_x = false;
  m_received_y = false;
  m_new_wpt = false;
  m_successfully_visited = false;
  m_genpath_regenerate = false;
  m_handshaken = false;

  m_handshake_var = "LISTENING_FOR_POINTS";
}

//---------------------------------------------------------
// Destructor

GenPath::~GenPath()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool GenPath::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();
    string sval  = msg.GetString(); 
    double dval  = msg.GetDouble();

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

     if(key == "VISIT_POINT"){
       if(sval == "firstpoint"){
         m_handshaken = true;
         m_receiving_points = true;
       }
       else if(sval == "lastpoint"){
         m_receiving_points = false;
         m_ready_to_generate_path = true;
       }
       else if(m_receiving_points){
         m_point_strings.push(sval);
       }
     }
     else if(key == "NAV_X"){
       m_current_x = dval;
       m_received_x = true;
     }
     else if(key == "NAV_Y"){
       m_current_y = dval;
       m_received_y = true;
     }
     else if(key == "WPT_INDEX_VISIT"){
       // relies on a waypoint behavior tagged with the _VISIT suffix
        
       m_new_wpt = true;
       m_next_wpt = (int)dval;
       m_successfully_visited = false;

       cout << "missed waypoints: " << m_missed_waypoints.size() << " at index: " << m_next_wpt << endl;
     }
     else if(key == "GENPATH_REGENERATE"){
       m_genpath_regenerate = true;
     }

     else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool GenPath::OnConnectToServer()
{
   registerVariables();
   Notify("VISIT_POINTS_REQ", "true");
   Notify("LISTENING_FOR_POINTS", "true");
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool GenPath::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!

  // todo: make the handshake var a configurable var,
  // but you have to do it in iterate
  // if(!m_handshaken){
  //   Notify(m_handshake_var, "true");
  // }

  while(m_point_strings.size() > 0){
    XYPoint current_point = string2Point(m_point_strings.front());
    m_visit_points.push_back(current_point);
    m_point_strings.pop();
  }

  if (m_genpath_regenerate){
    cout << "m_visit_points at regenerate: " << m_visit_points.size() << endl;
    cout << "m_visit_points at regenerate: " << m_visit_points.size() << endl;
    m_visit_points.clear();
    m_visit_points = m_missed_waypoints;
    m_missed_waypoints.clear();
    m_ready_to_visit = false;
    m_ready_to_generate_path = true;
    m_genpath_regenerate = false;
  }

  if (m_ready_to_generate_path){
    m_path.clear();
    m_path = generatePath(m_visit_points);
    sendPath(m_path);
    cout << "done generating path and ready to visit!" << endl;
    m_ready_to_visit = true;
    m_ready_to_generate_path = false;
  }

  if (m_ready_to_visit && m_received_x && m_received_y && m_new_wpt){

    XYPoint current_location(m_current_x, m_current_y);
    double distance_to_next = pythagorean(m_path[m_next_wpt], current_location);
    if (distance_to_next <= m_visit_radius){
      // if our distance is less than configured visit radius
      m_successfully_visited = true;
      cout << "captured point: " << m_next_wpt << endl;
    }
    //else we never successfully visited

    if (m_new_wpt){
      if (!m_successfully_visited){
        m_missed_waypoints.push_back(m_path[m_next_wpt]);
      }
      m_new_wpt = false; 
    }

  }



  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool GenPath::OnStartUp()
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
    if(param == "visit_radius") {
      m_visit_radius = stod(value);
      handled = true;
    }
    else if(param == "handshake_var") {
      m_handshake_var = value;
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

void GenPath::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("VISIT_POINT", 0);
  Register("NAV_X", 0);
  Register("NAV_Y", 0);
  Register("WPT_INDEX_VISIT", 0);
  Register("GENPATH_REGENERATE", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool GenPath::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "File:                                       " << endl;
  m_msgs << "============================================" << endl;

  ACTable actab(4);
  actab << "Pts Recvd | Pts Missed | Wpt Idx | Delta";
  actab.addHeaderLines();
  actab << to_string(m_path.size()) << to_string(m_missed_waypoints.size()) << to_string(m_next_wpt) << "four";
  m_msgs << actab.getFormattedString();

  return(true);
}

void GenPath::sendPath(std::vector<XYPoint> visit_points){

  XYSegList return_path;

  for (int i=0; i<visit_points.size(); i++){
    XYPoint current_point = visit_points[i];

    // cout << "path point: "<< current_point.x() << ", "<< current_point.y() << endl;

    return_path.add_vertex(current_point.x(), current_point.y());
  }
  string pts_string = return_path.get_spec();
  cout << "getting spec" << endl;
  cout << "visit_points: " << visit_points.size() << endl;
  cout << "return_path: " << return_path.size() << endl;
  // biteStringX(pts_string, 'pts=');
  // biteStringX(pts_string, '{');
  string update_string = "points = " + pts_string;
  Notify("UPDATES_WAYPOINT", update_string);
  cout << "notified updates_waypoint" << endl;
}

vector<XYPoint> GenPath::generatePath(std::vector<XYPoint> visit_points){

// create empty path vector
// first point a is start location
// 

  vector<XYPoint> greedy_path;

  XYPoint current_point = XYPoint(m_current_x, m_current_y);

  greedy_path.push_back(current_point);

  while(m_visit_points.size() > 0){

    int best_index = 0;
    double best_distance = numeric_limits<double>::infinity(); 

    for (int i = 0; i < m_visit_points.size(); i++){

      XYPoint ith_point = m_visit_points[i];
    
      double distance = pythagorean(current_point, m_visit_points[i]);
      
      // cout << "current_point: "<< current_point.x() << ", " << current_point.y() << endl;
      // cout << "ith_point: "<< ith_point.x() << ", " << ith_point.y() << endl;
      // cout << "distance: " << distance << ", " << "best_distance: " << best_distance << endl;
      // cout << "visit_points: " << m_visit_points.size() << endl;

      if (distance < best_distance) {
        best_distance = distance;
        best_index = i;
      }
    }

    // mark this as the next step and remove this point from m_visit_points
    current_point = m_visit_points[best_index];
    greedy_path.push_back(current_point);
    m_visit_points.erase(m_visit_points.begin() + best_index);
    cout << "m_visit_points in while: " << m_visit_points.size() << endl;
  }
  return greedy_path;
}

double GenPath::pythagorean(XYPoint a, XYPoint b){

  cout << "a: " << to_string(a.x()) << ", " << to_string(a.y()) << endl;
  cout << "b: " << to_string(b.x()) << ", " << to_string(b.y()) << endl;

  double diff_x = b.x() - a.x();
  double diff_y = b.y() - a.y();

  double x_diff_squared = diff_x * diff_x;
  double y_diff_squared = diff_y * diff_y;
  double distance = sqrt(x_diff_squared + y_diff_squared);

  cout << "successfully did pythagorean" << endl;

  return distance;
}