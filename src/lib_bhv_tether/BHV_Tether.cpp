/************************************************************/
/*    NAME: Mathew C. Schwartzman                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Tether.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Tether.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_Tether::BHV_Tether(IvPDomain domain) :
  IvPContactBehavior(domain)
{
  // Provide a default behavior name
  IvPContactBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Tether::setParam(string param, string val)
{
  if(IvPContactBehavior::setParam(param, val))
    return(true);
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "foo") && isNumber(val)) {
    // Set local member variables here
    return(true);
  }
  else if (param == "leader") {
    m_leader = val;
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

void BHV_Tether::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Tether::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Tether::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Tether::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Tether::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Tether::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Tether::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_Tether::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;

  drawGraphics();

  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

//---------------------------------------------------------------
// Procedure: calculateOuterRing()
//   Purpose: Use the pythagorean theorem to calculate horizontal distance

float BHV_Tether::calculateOuterRing(){

  float hypotenuse = m_tether_length;

  // horizontal^2 = hypotenuse^2 - depth_delta^2 

  return hypotenuse;
}


//---------------------------------------------------------------
// Procedure: drawGraphics()
//   Purpose: On each iteration, draw any visual indicators for pMarineViewer

void BHV_Tether::drawGraphics(){

  // outer circle message
  string outer_circle = "x=" + to_string(m_cnx) + ",y=" + to_string(m_cny) + ",radius=10,edge_size=1,duration=1";

  postRepeatableMessage("VIEW_CIRCLE", outer_circle);
}