/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
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
       m_current_x.set(dval);
     }
     else if(key == "NAV_Y"){
       m_current_y.set(dval);
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
    sendPath(m_visit_points);
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

    return_path.add_vertex(current_point.x(), current_point.y());
  }
  string update_string = "points = " + return_path.get_spec();
  Notify("UPDATES_WAYPOINT", update_string);
}

void GenPath::generatePath(std::vector<XYPoint> visit_points){

  // create empty sorted list
  // append current point to sorted list
  // calculate distance from current point to all points
  // take least distance point and make that the current point
  // append current point to sorted list (and so we repeat)

  vector<XYPoint> greedy_path;

  for (int i=0; i<visit_points.size(); i++){
    XYPoint current_point(m_current_x, m_current_y);
    
  }

  


}