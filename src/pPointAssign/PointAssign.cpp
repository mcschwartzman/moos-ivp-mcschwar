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
      }
      else if (m_receiving_points){
        m_point_queue.push(sval);
        cout << "queue size: " << m_point_queue.size() << endl;
      }
       
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

  // while processing queue points
  while(m_point_queue.size() > 0){

    XYPoint current_point = string2Point(m_point_queue.front());
    m_points.push_back(current_point);
    postViewPoint(current_point.x(), current_point.y(), to_string(m_point_queue.size()), "yellow");
    m_point_queue.pop();
    
  }

  m_most_west_point = XYPoint(-25, -175); // mostWestPoint(m_points); todo, implement mostWestPoint method
  m_most_east_point = XYPoint(200, -25);

  double x_range = m_most_east_point.x() - m_most_west_point.x();
  double vehicle_region_width = x_range / m_vehicles.size();

  for (int i = 0; i<m_points.size(); i++) {

    XYPoint current_point = m_points[i];

    for (int j = 0; j<m_vehicles.size(); j++){

      string color = "white";

      if (j == 1){
        color = "red";
      }

      double region_west = m_most_west_point.x() + (vehicle_region_width * j);
      double region_east = region_west + vehicle_region_width;

      if ((current_point.x() > region_west) && (current_point.x() < region_east)){
        postViewPoint(current_point.x(), current_point.y(), to_string(i), color);
      }

    }
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

void PointAssign::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("VISIT_POINT", 0);
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
  actab << "registered" << "registered";
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



