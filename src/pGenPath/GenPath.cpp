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
     }
     else if(key == "NAV_Y"){
       m_current_y = dval;
     }
     else if(key == "WPT_INDEX"){
       m_current_wpt = dval;
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

  while(m_point_strings.size() > 0){
    XYPoint current_point = string2Point(m_point_strings.front());
    m_visit_points.push_back(current_point);
    m_point_strings.pop();
  }

  if (m_ready_to_generate_path){
    vector<XYPoint> greedy_path = generatePath(m_visit_points);
    cout << "greedy_path: " << greedy_path.size() << endl;
    sendPath(greedy_path);
    m_ready_to_generate_path = false;
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
    if(param == "foo") {
      handled = true;
    }
    else if(param == "bar") {
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
  Register("WPT_INDEX", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool GenPath::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "File:                                       " << endl;
  m_msgs << "============================================" << endl;

  ACTable actab(4);
  actab << "Pts Recvd | Bravo | Charlie | Delta";
  actab.addHeaderLines();
  actab << to_string(m_visit_points.size()) << "two" << "three" << "four";
  m_msgs << actab.getFormattedString();

  return(true);
}

void GenPath::sendPath(std::vector<XYPoint> visit_points){

  XYSegList return_path;

  for (int i=0; i<visit_points.size(); i++){
    XYPoint current_point = visit_points[i];

    cout << "path point: "<< current_point.x() << ", "<< current_point.y() << endl;

    return_path.add_vertex(current_point.x(), current_point.y());
  }
  string pts_string = return_path.get_spec();
  // biteStringX(pts_string, 'pts=');
  // biteStringX(pts_string, '{');
  string update_string = "points = " + pts_string;
  Notify("UPDATES_WAYPOINT", update_string);
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
      
      cout << "current_point: "<< current_point.x() << ", " << current_point.y() << endl;
      cout << "ith_point: "<< ith_point.x() << ", " << ith_point.y() << endl;
      cout << "distance: " << distance << ", " << "best_distance: " << best_distance << endl;
      cout << "visit_points: " << m_visit_points.size() << endl;

      if (distance < best_distance) {
        best_distance = distance;
        best_index = i;
      }
    }

    // mark this as the next step and remove this point from m_visit_points
    current_point = m_visit_points[best_index];
    greedy_path.push_back(current_point);
    m_visit_points.erase(m_visit_points.begin() + best_index);
  }
  return greedy_path;
}

double GenPath::pythagorean(XYPoint a, XYPoint b){

  double diff_x = b.x() - a.x();
  double diff_y = b.y() - a.y();

  double x_diff_squared = diff_x * diff_x;
  double y_diff_squared = diff_y * diff_y;
  double distance = sqrt(x_diff_squared + y_diff_squared);

  return distance;
}