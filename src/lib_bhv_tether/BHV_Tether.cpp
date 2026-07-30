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
#include <math.h>
#include "ZAIC_PEAK.h"
#include "AngleUtils.h"
#include "GeomUtils.h"
#include "OF_Coupler.h"

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
  addInfoVars("NAV_X, NAV_Y, NAV_DEPTH, NODE_REPORT");
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
  
  if((param == "max_speed") && isNumber(val)) {
    // Set local member variables here
    m_max_speed = double_val;
    return(true);
  }
  else if ((param == "tether_length") && isNumber(val)) {
    m_tether_length = double_val;
    return(true);
  }
  else if ((param == "inner_limit") && isNumber(val)) {
    m_inner_ring = double_val;
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
  bool ok_node_report, ok_os_depth, ok_os_x, ok_os_y;

  m_osx = getBufferDoubleVal("NAV_X", ok_os_x);
  m_osy = getBufferDoubleVal("NAV_Y", ok_os_y);
  
  m_contact_node_report = string2NodeRecord(getBufferStringVal("NODE_REPORT", ok_node_report));
  m_contact_depth = m_contact_node_report.getDepth();
  m_ownship_depth = getBufferDoubleVal("NAV_DEPTH", ok_os_depth);

  postRepeatableMessage("TETHER_LENGTH", m_tether_length);
  postRepeatableMessage("TETHER_DEPTH", m_contact_depth);
  postRepeatableMessage("TETHER_SNAPS", m_tether_snaps);
  postRepeatableMessage("TETHER_HOCKLES", m_tether_hockles);

  // Part 1: Build the IvP function
  IvPFunction *ipf = buildFunctionWithZAIC();

  m_outer_ring = calculateOuterRing();
  m_ideal_ring = calculateIdeal();
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
//   Purpose: Use the pythagorean theorem to calculate max horizontal distance

double BHV_Tether::calculateOuterRing(){

  double hypotenuse = m_tether_length;
  double depth_delta = m_contact_depth - m_ownship_depth;

  double outer = sqrt((hypotenuse * hypotenuse) - (depth_delta * depth_delta));

  return outer;
}

//---------------------------------------------------------------
// Procedure: calculateOuterRing()
//   Purpose: Use the pythagorean theorem to calculate max horizontal distance

double BHV_Tether::calculateIdeal(){

  double ideal = m_inner_ring + ((m_outer_ring - m_inner_ring) / 2);

  return ideal;
}


//---------------------------------------------------------------
// Procedure: drawGraphics()
//   Purpose: On each iteration, draw any visual indicators for pMarineViewer

void BHV_Tether::drawGraphics(){

  string inner_circle = "x=" + to_string(m_cnx) + ",y=" + to_string(m_cny) + ",radius=" + to_string(m_inner_ring) +  ",edge_size=1,duration=0.1,label=inner";
  string outer_circle = "x=" + to_string(m_cnx) + ",y=" + to_string(m_cny) + ",radius=" + to_string(m_outer_ring) +  ",edge_size=1,duration=0.1,label=rov@";
  string ideal_circle = "x=" + to_string(m_cnx) + ",y=" + to_string(m_cny) + ",radius=" + to_string(m_ideal_ring) +  ",edge_size=1,duration=0.1,label=ideal,color=teal";

  postRepeatableMessage("VIEW_CIRCLE", inner_circle);
  postRepeatableMessage("VIEW_CIRCLE", outer_circle);
  // postRepeatableMessage("VIEW_CIRCLE", ideal_circle);
}

//---------------------------------------------------------------
// Procedure: buildFunctionWithZAIC()
//   Purpose: Build the ivp function simply using the ideal radius
//      TODO: Take into account direction of travel of ROV

IvPFunction *BHV_Tether::buildFunctionWithZAIC() {

  double contact_x = m_contact_node_report.getX();
  double contact_y = m_contact_node_report.getY();
  double rel_ang = relAng(m_osx, m_osy, contact_x, contact_y);

  // calculate state
  double distance_to_leader = distPointToPoint(m_osx, m_osy, contact_x, contact_y);

  bool inside_outer = (distance_to_leader < m_outer_ring);
  bool inside_ideal = (distance_to_leader < m_ideal_ring);
  bool inside_inner = (distance_to_leader < m_inner_ring);

  double error = std::abs(distance_to_leader - m_ideal_ring);
  double max_error = m_outer_ring - m_ideal_ring;

  if (inside_outer) {
    m_speed_summit = (error * m_max_speed) / max_error;
    if (!inside_ideal){
      postRepeatableMessage("TETHER_STATUS", "within outer ring, spd: " + to_string(m_speed_summit)); 
    }
    else {
      rel_ang = rel_ang + 180;
      if (inside_inner){
        m_tether_hockles++;
      }
    }
  }
  else {
    m_speed_summit = m_max_speed;
    m_tether_snaps++;
  }

  postRepeatableMessage("TETHER_SPEED", m_speed_summit);


  // generate course ipf
  rel_ang = angle360(rel_ang);
  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(rel_ang);
  crs_zaic.setPeakWidth(0);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setSummitDelta(0);
  crs_zaic.setValueWrap(true);

  if(crs_zaic.stateOK() == false) {
    string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }
  IvPFunction *crs_ipf = crs_zaic.extractIvPFunction();


  // generate speed ipf
  ZAIC_PEAK spd_zaic(m_domain, "speed");

  spd_zaic.setSummit(m_speed_summit);
  spd_zaic.setPeakWidth(0.1);
  spd_zaic.setBaseWidth(2.0);
  spd_zaic.setSummitDelta(50.0);

  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();

  OF_Coupler coupler;

  return(coupler.couple(crs_ipf, spd_ipf));
}
