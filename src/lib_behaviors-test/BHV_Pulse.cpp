/************************************************************/
/*    NAME: Mathew Schwartzman                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Pulse.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Pulse.h"
#include "XYRangePulse.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_Pulse::BHV_Pulse(IvPDomain domain) :
  IvPBehavior(domain)
{
  m_position_x = 0;
  m_position_y = 0;
  m_arrival_time = 0;
  m_current_time = 0;
  m_waypoint_index = -1;
  
  m_pulse_duration = 4;
  m_pulse_radius = 20;
  m_pulse_needed = false;
  m_pulse_delay  = 5;

  // Provide a default behavior name
  IvPBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y");
  addInfoVars("WPT_INDEX");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Pulse::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "pulse_radius") && isNumber(val)) {
    // Set local member variables here
    m_pulse_radius = double_val;
    return(true);
  }
  else if((param == "pulse_duration") && isNumber(val)) {
    // Set local member variables here
    m_pulse_duration = double_val;
    return(true);
  }
  else if ((param == "pulse_delay") && isNumber(val)) {
    m_pulse_delay = double_val;
    return(true);
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_Pulse::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Pulse::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Pulse::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Pulse::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Pulse::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Pulse::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Pulse::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_Pulse::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;

  bool okx, oky, oki, transitioned_points;

  m_position_x = getBufferDoubleVal("NAV_X", okx);
  m_position_y = getBufferDoubleVal("NAV_Y", oky);
  m_current_time = getBufferCurrTime();
  int last_index = m_waypoint_index;
  postMessage("CURRI", m_waypoint_index);
  postMessage("LASTI", last_index);
  postMessage("ARRIVAL_TIME", m_arrival_time);
  postMessage("CURRENT_TIME", m_current_time);
  m_waypoint_index = getBufferDoubleVal("WPT_INDEX", oki);

  if(!okx || !oky) {
    postWMessage("No ownship X/Y info in info_buffer.");
    return(0);
  }
  if(!oki) {
    postWMessage("No wpt index info found in buffer!");
    return(0);
  }

  if (m_waypoint_index != last_index){
    transitioned_points = true;
    
  }

  if (transitioned_points){
    m_arrival_time = getBufferCurrTime();
    transitioned_points = false;
    m_pulse_needed = true;
  }

  if ((m_current_time - m_arrival_time) > m_pulse_delay){
    if (m_pulse_needed){
      sendPulse();
      m_pulse_needed = false;
    }
  }

  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

//---------------------------------------------------------------
// Procedure: sendPulse()
//   Purpose: Generate the xyrangepulse object and publish it to the MOOSDB.

void BHV_Pulse::sendPulse(){

  XYRangePulse pulse;
  pulse.set_x(m_position_x);
  pulse.set_y(m_position_y);
  pulse.set_label("bhv_pulse");
  pulse.set_rad(m_pulse_radius);
  pulse.set_time(m_current_time);       
  pulse.set_color("edge", "yellow");
  pulse.set_color("fill", "yellow");
  pulse.set_duration(m_pulse_duration);

  string spec = pulse.get_spec();
  postMessage("VIEW_RANGE_PULSE", spec);

}

