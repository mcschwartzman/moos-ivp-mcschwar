/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "PointAssign.h"
#include "XYFormatUtilsPoint.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

PointAssign::PointAssign()
{
  m_receiving_points = false;

  m_vehicle_checkins = 0;

  m_vehicle_colors = {"red", "yellow", "orange", "yellow", "green"};
}

//---------------------------------------------------------
// Destructor

PointAssign::~PointAssign()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool PointAssign::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();
    string sval  = msg.GetString(); 

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

    if(key == "VISIT_POINT"){
      if (sval == "firstpoint"){
        m_receiving_points = true;
      }
      if (sval == "lastpoint"){
        m_receiving_points = false;
        m_received_lastpoint = true;
      }
      else if (m_receiving_points){
        m_point_queue.push(sval);
        cout << "queue size: " << m_point_queue.size() << endl;
      }
     }
     else if(key == "LISTENING_FOR_POINTS"){

       

      if (sval == "true"){
        m_vehicle_checkins++;
      }
      if (m_vehicle_checkins == m_vehicles.size()){
        m_vehicles_registered = true;
      }
      cout << "vehicle_checkins: " << m_vehicle_checkins << endl;

     }

     else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool PointAssign::OnConnectToServer()
{
   registerVariables();
   Notify("UTS_PAUSE", "false");
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool PointAssign::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!

  if (m_vehicles_registered && m_received_lastpoint){
    m_ready_to_assign = true;
    m_received_lastpoint = false;
  }

  // while processing queue points
  while(m_point_queue.size() > 0){

    XYPoint current_point = string2Point(m_point_queue.front());
    m_points.push_back(current_point);
    postViewPoint(current_point.x(), current_point.y(), to_string(m_point_queue.size()), "white");
    m_point_queue.pop();
    
  }

  if (m_ready_to_assign){
    if (m_assign_by_region){
      regionalAssign(m_points, m_vehicles);
    }
    else {
      // assign by alternating
      alternatingAssign(m_points, m_vehicles);
    }
    m_ready_to_assign = false;
  }
  


  


  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool PointAssign::OnStartUp()
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
    if(param == "vname") {
      m_vehicles.push_back(value);
      m_vehicle_assigned_points.push_back({"firstpoint"});
      handled = true;
    }
    else if(param == "assign_by_region") {
      // todo, use a true/false parser utility from somewhere else in moos-ivp
      if (value == "true"){
        m_assign_by_region = true;
        handled = true;
      }
      else if (value == "false"){
        m_assign_by_region = false;
        handled = true;
      }
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables()

void PointAssign::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("VISIT_POINT", 0);
  Register("LISTENING_FOR_POINTS", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool PointAssign::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "File:                                       " << endl;
  m_msgs << "============================================" << endl;

  ACTable actab(m_vehicles.size());
  actab.addHeaderLines();
  actab << "Points Received: " + to_string(m_points.size());
  actab << "Most West: " + to_string(m_most_west_point.x()) + ", " + to_string(m_most_west_point.y());
  actab.addHeaderLines();
  for (int i; i < m_vehicles.size(); i++){
    actab << m_vehicles[i] + " |";
  }
  actab.addHeaderLines();
  actab << "vehicle checkins: " << m_vehicle_checkins;
  m_msgs << actab.getFormattedString();

  return(true);
}

void PointAssign::postViewPoint(double x, double y, string label, string color){
  XYPoint point(x, y);
  point.set_label(label);
  point.set_color("vertex", color);
  point.set_param("vertex_size", "4");
  string spec = point.get_spec();
  Notify("VIEW_POINT", spec);
}

XYPoint PointAssign::mostWestPoint(vector<XYPoint> points){

  XYPoint current_most_west = points[0];

  for (int i; i<points.size(); i++){
    if (points[i].x() < current_most_west.x()){
      current_most_west = points[i];
    }
  }

  return current_most_west;
}

void PointAssign::regionalAssign(vector<XYPoint> points, vector<string> vehicles) {

  m_most_west_point = XYPoint(-25, -175); // mostWestPoint(m_points); todo, implement mostWestPoint method, to find programmatically
  m_most_east_point = XYPoint(200, -25);

  double x_range = m_most_east_point.x() - m_most_west_point.x();
  double vehicle_region_width = x_range / vehicles.size();

  for (int i = 0; i<points.size(); i++) {
    XYPoint current_point = points[i];
    for (int j = 0; j<vehicles.size(); j++){

      double region_west = m_most_west_point.x() + (vehicle_region_width * j);
      double region_east = region_west + vehicle_region_width;

      if ((current_point.x() > region_west) && (current_point.x() < region_east)){
        postViewPoint(current_point.x(), current_point.y(), to_string(i), m_vehicle_colors[j]);
        // post message to VISIT_POINT_{vehicles[j]}
        m_vehicle_assigned_points[j].push_back(current_point.get_spec());
      }
    }
  }
  sendAssignMessages(m_vehicle_assigned_points);
}

void PointAssign::alternatingAssign(vector<XYPoint> points, vector<string> vehicles){

  for (int i=0; i<points.size(); i++) {
    XYPoint current_point = points[i];

    int vehicle_index = i % vehicles.size();

    postViewPoint(current_point.x(), current_point.y(), to_string(i), m_vehicle_colors[vehicle_index]);

    m_vehicle_assigned_points[vehicle_index].push_back(current_point.get_spec());

  }

  sendAssignMessages(m_vehicle_assigned_points);

}

void PointAssign::sendAssignMessages(vector<vector<string>> vehicle_assigned_points){
  for (int i=0; i<vehicle_assigned_points.size(); i++){
    for (int j=0; j<vehicle_assigned_points[i].size(); j++){
      string current_point = vehicle_assigned_points[i][j];
      Notify("VISIT_POINT_" + toupper(m_vehicles[i]), current_point);
    }
    Notify("VISIT_POINT_" + toupper(m_vehicles[i]), "lastpoint");
  }
}