/************************************************************/
/*    NAME: Mathew Schwartzman                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: WatchTether.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "WatchTether.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

WatchTether::WatchTether()
{
}

//---------------------------------------------------------
// Destructor

WatchTether::~WatchTether()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool WatchTether::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

     if(key == "TETHER_LENGTH"){
      double dval  = msg.GetDouble();
      m_tether_length = dval;
     } 
     
     else if(key == "TETHER_DEPTH"){
      int dval  = msg.GetDouble();
      m_tether_depth = dval;
     } 

     else if(key == "TETHER_SNAPS"){
      int ival  = int(msg.GetDouble());
      m_tether_snaps = ival;
     } 

     else if(key == "TETHER_HOCKLES"){
      double ival  = int(msg.GetDouble());
      m_tether_hockles = ival;
     } 



     else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool WatchTether::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool WatchTether::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool WatchTether::OnStartUp()
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

void WatchTether::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("TETHER_LENGTH", 0);
  Register("TETHER_DEPTH", 0);
  Register("TETHER_SNAPS", 0);
  Register("TETHER_HOCKLES", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool WatchTether::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "File:                                       " << endl;
  m_msgs << "============================================" << endl;

  ACTable actab(4);
  actab << "Tether Length | Tether Depth | Snaps | Hockles";
  actab.addHeaderLines();
  actab << to_string(m_tether_length) << to_string(m_tether_depth) << to_string(m_tether_snaps) << to_string(m_tether_hockles);
  m_msgs << actab.getFormattedString();

  return(true);
}




