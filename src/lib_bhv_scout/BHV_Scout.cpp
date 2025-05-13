/*****************************************************************/
/*    NAME: M.Benjamin                                           */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: BHV_Scout.cpp                                        */
/*    DATE: April 30th 2022                                      */
/*****************************************************************/

#include <cstdlib>
#include <math.h>
#include "BHV_Scout.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "BuildUtils.h"
#include "GeomUtils.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"
#include "XYFormatUtilsPoly.h"
#include "XYFormatUtilsPoint.h"
#include <algorithm>

using namespace std;

//-----------------------------------------------------------
// Constructor()

BHV_Scout::BHV_Scout(IvPDomain gdomain) : 
  IvPBehavior(gdomain)
{
  IvPBehavior::setParam("name", "scout");
 
  // Default values for behavior state variables
  m_osx  = 0;
  m_osy  = 0;

  // All distances are in meters, all speed in meters per second
  // Default values for configuration parameters 
  m_desired_speed  = 1; 
  m_capture_radius = 10;

  m_rand_points_checked = 2;
  m_gen_hex = true;

  m_pt_set = false;
  m_being_chased = false;

  m_max_chase_iterations = 20; // divide this number by 4 for the number of seconds that a vessel must be near us in order to count as chasing
  m_chase_radius = 300;  // how close a vessel must be to be considered chasing us

  m_rand_points_to_check = 2; // number of rand points in check hex to go to before regenerating hex
  m_check_radius = 9.0;  // size of the check hex to generate
  m_check_amount = 3; // max number of KNOWN swimmers in hex, if any more, we will put a hex somewhere else
  m_distance_to_rescuer = 20; // min distance from center point of hex to the rescue boat

  addInfoVars("NAV_X, NAV_Y");
  addInfoVars("RESCUE_REGION");
  addInfoVars("SCOUTED_SWIMMER");
  addInfoVars("SWIMMER_ALERT");
  addInfoVars("NODE_REPORT");
  addInfoVars("BEING_CHASED");
}

//---------------------------------------------------------------
// Procedure: setParam() - handle behavior configuration parameters

bool BHV_Scout::setParam(string param, string val) 
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);
  
  bool handled = true;
  if(param == "capture_radius")
    handled = setPosDoubleOnString(m_capture_radius, val);
  else if(param == "desired_speed")
    handled = setPosDoubleOnString(m_desired_speed, val);
  else if(param == "tmate")
    handled = setNonWhiteVarOnString(m_tmate, val);
  else if(param == "check_radius")
    handled = setPosDoubleOnString(m_check_radius, val);
  else if(param == "check_amount")
    handled = setPosUIntOnString(m_check_amount, val);
  else
    handled = false;

  srand(time(NULL));
  
  return(handled);
}

//-----------------------------------------------------------
// Procedure: onEveryState()

void BHV_Scout::onEveryState(string str) 
{
  if(!getBufferVarUpdated("SCOUTED_SWIMMER"))
    return;

  string report = getBufferStringVal("SCOUTED_SWIMMER");
  if(report == "")
    return;

  if(m_tmate == "") {
    postWMessage("Mandatory Teammate name is null");
    return;
  }
  postOffboardMessage(m_tmate, "SWIMMER_ALERT", report);
}

//-----------------------------------------------------------
// Procedure: onIdleState()

void BHV_Scout::onIdleState() 
{
  m_curr_time = getBufferCurrTime();
}

//-----------------------------------------------------------
// Procedure: onRunState()

IvPFunction *BHV_Scout::onRunState() 
{

  // this part looks through all vehicles and checks if they're chasing the rescue boat
  for (int i = 0; i < m_vehicles.size(); i++){
    string ith_vehicle_name = m_vehicles[i];
    if (ith_vehicle_name != m_tmate){
      NodeRecord ith_node = m_node_map[ith_vehicle_name];
      XYPoint node_pos(ith_node.getX(), ith_node.getY());

      NodeRecord rescue_ship = m_node_map[m_tmate];
      XYPoint rescue_pos(rescue_ship.getX(), rescue_ship.getY());

      if (pythagorean(node_pos, rescue_pos) < m_chase_radius){

        if (m_chasing_map.find(ith_vehicle_name) != m_chasing_map.end()){
          m_chasing_map[ith_vehicle_name] = m_chasing_map[ith_vehicle_name] + 1;
        }
        else {
          m_chasing_map[ith_vehicle_name] = 0;
        }
      }
      else {
        m_chasing_map[ith_vehicle_name] = 0;
      }
    }
  }

  m_being_chased = false;
  for (int i = 0; i < m_vehicles.size(); i++){
    string ith_vehicle_name = m_vehicles[i];
    string msg_key = "CHASE_ITER_" + ith_vehicle_name;
    int chase_iter = m_chasing_map[ith_vehicle_name];
    postMessage(msg_key, chase_iter);
    if (chase_iter > m_max_chase_iterations){
      m_being_chased = true;
    }
  }



  bool ok;

  vector<string> swimmer_alerts = getBufferStringVector("SWIMMER_ALERT", ok);

  for (int i = 0; i < swimmer_alerts.size(); i++){
    string swimmer_alert = swimmer_alerts[i];
    XYPoint received_point = string2Point(swimmer_alert);
    postMessage("LAST_SWIMMER", swimmer_alert);
    m_last_point_id = received_point.get_id();
    // check to see if this point has been seen before
    bool seen_point = false;
    for (int i = 0; i < m_known_swimmers.size(); i++){
      XYPoint ith_point = m_known_swimmers[i];
      if (m_last_point_id == ith_point.get_id()){
        seen_point = true;
      }
    }
    if (!seen_point){
      m_known_swimmers.push_back(received_point);
    }
  }

  postMessage("AVAILABLE_SWIMMERS", m_known_swimmers.size());

  vector<string> node_reports = getBufferStringVector("NODE_REPORT", ok);
  for (int i = 0; i < node_reports.size(); i++){
    string node_report = node_reports[i];
    NodeRecord node_record = string2NodeRecord(node_report);
    string node_name = node_record.getName();
    
    bool seen_name = false;
    for (int j = 0; j < m_vehicles.size(); j++){
      if (node_name == m_vehicles[j]){
        seen_name = true;
      }
    }
    if (!seen_name){
      m_vehicles.push_back(node_name);
    }
    m_node_map[node_name] = node_record;
  }
  postMessage("REGISTERED_NODES", m_vehicles.size());

  vector<string> vehicle_list = {"abe", "ben", "pip", "cal", "deb", "eve", "hal", "max", "oak"};

  for (int i = 0; i < vehicle_list.size(); i++){
    string ith_vehicle = vehicle_list[i];
    auto it = std::find(m_vehicles.begin(), m_vehicles.end(), ith_vehicle);
    if (it == m_vehicles.end()){
      m_spoofs.push_back(ith_vehicle);
    }
  }

  string being_chased = getBufferStringVal("BEING_CHASED");

  if (being_chased == "true"){
    m_being_chased = true;
  }

  if (m_being_chased == true){
    for (int i = 0; i < m_vehicles.size(); i++){
      string ith_vehicle = m_vehicles[i];
      if (m_tmate != ith_vehicle){

        NodeRecord node = m_node_map[ith_vehicle];

        double node_x = node.getX();
        double node_y = node.getY();
        double node_heading = node.getHeading();

        //string spoof_name = m_spoofs[0]; // ensures we use different spoof for each adversary

        //todo, different name other than us name or tmate, maybe an xy offset

        double offset = 3;
        double speed = 1;
        spoofNode(m_spoofs[0], "heron", node_x + offset, node_y + offset, node_heading + 180, speed, ith_vehicle);
        spoofNode(m_spoofs[1], "KAYAK", node_x - offset, node_y - offset, node_heading - 180, speed, ith_vehicle);
        spoofNode(m_spoofs[2], "heron", node_x - offset, node_y + offset, node_heading + 180, speed, ith_vehicle);
        spoofNode(m_spoofs[3], "KAYAK", node_x + offset, node_y - offset, node_heading + 180, speed, ith_vehicle);
      }
    }
  }

  // Part 1: Get vehicle position from InfoBuffer and post a 
  // warning if problem is encountered
  bool ok1, ok2;
  m_osx = getBufferDoubleVal("NAV_X", ok1);
  m_osy = getBufferDoubleVal("NAV_Y", ok2);
  if(!ok1 || !ok2) {
    postWMessage("No ownship X/Y info in info_buffer.");
    return(0);
  }

  double this_x = m_waypoint_engine.getPointX();
  double this_y = m_waypoint_engine.getPointY();
  int    this_i = m_waypoint_engine.getCurrIndex();

  
  // Part 2: Determine if the vehicle has reached the destination 
  // point and if so, declare completion.
  updateScoutPoint();
  double dist = hypot((m_ptx-m_osx), (m_pty-m_osy));
  //postEventMessage("Dist=" + doubleToStringX(dist,1));
  if(dist <= m_capture_radius) {
    m_pt_set = false;
    postViewPoint(false);
    return(0);
  }

  // Part 3: Post the waypoint as a string for consumption by 
  // a viewer application.
  postViewPoint(true);

  // Part 4: Build the IvP function 
  IvPFunction *ipf = buildFunction();
  if(ipf == 0) 
    postWMessage("Problem Creating the IvP Function");
  
  return(ipf);
}

//-----------------------------------------------------------
// Procedure: updateScoutPoint()

void BHV_Scout::updateScoutPoint()
{
  if(m_pt_set)
    return;

  string region_str = getBufferStringVal("RESCUE_REGION");
  if(region_str == "")
    postWMessage("Unknown RESCUE_REGION");
  else
    postRetractWMessage("Unknown RESCUE_REGION");

  XYPolygon region = string2Poly(region_str);
  if(!region.is_convex()) {
    postWMessage("Badly formed RESCUE_REGION");
    return;
  }
  m_rescue_region = region;
  
  cout << "updateScoutPoint(): " << endl;
  
  double ptx = 0;
  double pty = 0;
  

  if (m_rand_points_checked < m_rand_points_to_check){
    m_rand_points_checked++;
    bool ok = randPointInPoly(m_hex_to_check, ptx, pty);
  }
  else {
    m_rand_points_checked = 0;
    bool ok = randPointInPoly(m_rescue_region, ptx, pty);
    if(!ok) {
      postWMessage("Unable to generate scout point");
      return;
    }

    m_hex_to_check.initialize(ptx, pty, m_check_radius);
    m_hex_to_check.set_label("check_hex");
    vector<XYPoint> points_in_hex = knownInHex(m_hex_to_check);
    string hex_msg = m_hex_to_check.get_spec();

    postMessage("VIEW_POLYGON", hex_msg);
    postMessage("POINTS_IN_POLY", points_in_hex.size());

    NodeRecord rescue_ship = m_node_map[m_tmate];
    XYPoint rescue_pos(rescue_ship.getX(), rescue_ship.getY());

    XYPoint hex_pos(ptx, pty);

    if (m_rescue_region.dist_to_poly(ptx, pty, 90) < m_check_radius){
      postWMessage("Check Poly overlapped with bound area");
      return;
    }

    if (points_in_hex.size() > m_check_amount){
      postWMessage("Check Poly contained too many swimmers");
      return;
    }

    if (pythagorean(rescue_pos, hex_pos)){
      postWMessage("Check Poly to close to rescue boat");
      return;
    }
  }

  m_ptx = ptx;
  m_pty = pty;

  m_pt_set = true;
  string msg = "New pt: " + doubleToStringX(ptx) + "," + doubleToStringX(pty);
  postEventMessage(msg);
}

//-----------------------------------------------------------
// Procedure: postViewPoint()

void BHV_Scout::postViewPoint(bool viewable) 
{

  XYPoint pt(m_ptx, m_pty);
  pt.set_vertex_size(5);
  pt.set_vertex_color("orange");
  pt.set_label(m_us_name + "'s next waypoint");
  
  string point_spec;
  if(viewable)
    point_spec = pt.get_spec("active=true");
  else
    point_spec = pt.get_spec("active=false");
  postMessage("VIEW_POINT", point_spec);
}


//-----------------------------------------------------------
// Procedure: buildFunction()

IvPFunction *BHV_Scout::buildFunction() 
{
  if(!m_pt_set)
    return(0);
  
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setSummit(m_desired_speed);
  spd_zaic.setPeakWidth(0.5);
  spd_zaic.setBaseWidth(1.0);
  spd_zaic.setSummitDelta(0.8);  
  if(spd_zaic.stateOK() == false) {
    string warnings = "Speed ZAIC problems " + spd_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }
  
  double rel_ang_to_wpt = relAng(m_osx, m_osy, m_ptx, m_pty);
  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(rel_ang_to_wpt);
  crs_zaic.setPeakWidth(0);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setSummitDelta(0);  
  crs_zaic.setValueWrap(true);
  if(crs_zaic.stateOK() == false) {
    string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }

  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();
  IvPFunction *crs_ipf = crs_zaic.extractIvPFunction();

  OF_Coupler coupler;
  IvPFunction *ivp_function = coupler.couple(crs_ipf, spd_ipf, 50, 50);

  return(ivp_function);
}

vector<XYPoint> BHV_Scout::knownInRect(XYPolygon rectangle) {

  vector<XYPoint> results;

  for (int i; i < m_known_swimmers.size(); i++){
    XYPoint ith_swimmer = m_known_swimmers[i];
    if (rectangle.contains(ith_swimmer.x(), ith_swimmer.y())){
      results.push_back(ith_swimmer);
    }
  }

  return results;
}

vector<XYPoint> BHV_Scout::knownInHex(XYHexagon hexagon) {

  vector<XYPoint> results;

  for (int i = 0; i < m_known_swimmers.size(); i++){
    XYPoint ith_swimmer = m_known_swimmers[i];
    if (hexagon.contains(ith_swimmer.x(), ith_swimmer.y())){
      results.push_back(ith_swimmer);
    }
  }

  return results;
}

vector<XYPolygon> BHV_Scout::childrenRect(XYPolygon rectangle) {
  vector<XYPolygon> results;

  return results;
}

void BHV_Scout::spoofNode(string node_name, string node_type, double x, double y, double heading, double speed, string destination){
  NodeRecord  node_record;
  node_record.setX(x);
  node_record.setY(y);
  node_record.setHeading(heading);
  node_record.setSpeed(speed);
  node_record.setName(node_name);
  node_record.setLength(3);
  node_record.setColor("yellow");
  node_record.setMode("MODE@ACTIVE:RETURNING");
  node_record.setAllStop("clear");
  node_record.setType(node_type);
  node_record.setYaw(-1);
  double moos_tstamp = getBufferCurrTime();
  node_record.setTimeStamp(moos_tstamp);

  NodeMessage node_message;
  node_message.setSourceNode(m_us_name);
  node_message.setVarName("NODE_REPORT");
  node_message.setStringVal(node_record.getSpec());
  node_message.setDestNode(destination);

  string msg_val = node_message.getSpec();

  postMessage("NODE_MESSAGE_LOCAL", msg_val);
}

double BHV_Scout::pythagorean(XYPoint a, XYPoint b){

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