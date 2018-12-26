#include <vector>
#include <math.h> /* floor, abs */
#include <assert.h>
#include <algorithm> /* min, min_element, max_element */

#include "ros/ros.h"
#include "ros/console.h"
#include <std_msgs/UInt16.h>
#include <std_msgs/UInt32.h>
#include <std_msgs/Float32.h>

#include "mpc.h"


#define PI_BY_180 3.14159265 / 180


UltimateRacer::UltimateRacer(
  ros::NodeHandle* nodehandle,
  int slow_esc,
  int medium_esc,
  int fast_esc,
  float drive_medium_thr,
  float drive_fast_thr,
  int drive_fast_angle_margin,
  float yaw_multiplier,
  int num_points_below_sod3_to_stop,
  float karols_modifier,
  float margin_drive_slow,
  float margin_drive_fast,
  int scan_range_deleted
) {

  nh = *nodehandle;
  this->slow_esc = slow_esc;
  this->medium_esc = medium_esc;
  this->fast_esc = fast_esc;
  this->drive_medium_thr = drive_medium_thr;
  this->drive_fast_thr = drive_fast_thr;
  this->drive_fast_angle_margin = drive_fast_angle_margin;
  this->yaw_multiplier = yaw_multiplier;
  this->num_points_below_sod3_to_stop = num_points_below_sod3_to_stop;
  this->karols_modifier = karols_modifier;
  this->margin_drive_slow = margin_drive_slow;
  this->margin_drive_fast = margin_drive_fast;
  this->scan_range_deleted = scan_range_deleted;

  float angle_step = 1. / STEPS_PER_DEGREE;
  for (int i=0; i<NUM_ANGLES_TO_STORE; i++)
    SIN_ALPHA[i] = std::sin((i+1)*angle_step * PI_BY_180);

  old_time_ = ros::Time::now();
  last_stop_msg_ts = ros::Time::now().toSec();

  pub_esc = nh.advertise<std_msgs::UInt16>(
    "/esc",
    1
  );

  pub_servo = nh.advertise<std_msgs::UInt16>(
    "/servo",
    1
  );

  sub_scan = nh.subscribe(
    "/scan",
    1,
    &UltimateRacer::scan_cb,
    this,
    ros::TransportHints().tcpNoDelay()
  );

  sub_estop = nh.subscribe(
    "/eStop",
    10,
    &UltimateRacer::estop_cb,
    this
  );
}


void UltimateRacer::scan_cb(const sensor_msgs::LaserScan & data) {
  time_ = ros::Time::now();
  // Stop if you've lost connection with the master
  if (time_.toSec() - last_stop_msg_ts > 0.5 and (ego or estart))
    exec_estop();

  // Find the smallest range near the center
  const auto mid_point = data.ranges.size() / 2;
  int count = 0;
  for (int i=mid_point-MARGIN_FOR_SOD3; i<mid_point+MARGIN_FOR_SOD3; i++)
    if (data.ranges[i] < SAFE_OBSTACLE_DIST3)
        count += 1;
  // Stop if the obstacle is too close
  if (count >= num_points_below_sod3_to_stop) {
    throttle = ESC_BRAKE;
    tmp_uint16.data = throttle;
    pub_esc.publish(tmp_uint16);
    return;
  }

  // Clip the values in `data.ranges`
  const size_t data_size = data.ranges.size();
  std::vector<float> scan;  // TODO: use `std::array` for performance
  float one_scan;
  for (int i=0; i<data_size; i++) {
    one_scan = data.ranges[i];
    // We clip the values (to deal with noise) to three magic numbers
    // 30 is the actual range limit for our LIDAR (UST-10LX), but it might
    // also be due to noise, BUT we choose to
    // 10 is the nominal limit for the LIDAR, and we cap the scan to this value
    // everywhere, except in front of the car
    one_scan = one_scan < 0.06 ? 0.06 : one_scan > 30 ? 30 : one_scan;
    if (i < (MID_SCAN_IDX-drive_fast_angle_margin) or i > (MID_SCAN_IDX+drive_fast_angle_margin))
        one_scan = one_scan > 10 ? 10 : one_scan;

    scan.push_back(one_scan);
  }

  int idx = steerMAX(scan, margin_drive_fast);

  bool idx_is_minus_one = (idx == -1);
  bool scan_below_fast_thr = (scan[idx] < drive_fast_thr);
  bool idx_far_from_center = (
    idx < (MID_SCAN_IDX-drive_fast_angle_margin)) or (idx > (MID_SCAN_IDX+drive_fast_angle_margin)
  );

  if (idx_is_minus_one or scan_below_fast_thr or idx_far_from_center) {
    // Unable to go fast

    // If `margin_drive_fast` and `margin_drive_slow` are different,
    // call `steerMAX` again
    if (fabs(margin_drive_fast-margin_drive_slow) > 0.0001)
      idx = steerMAX(scan, margin_drive_slow);

    if (idx >= 0 and scan[idx] < SAFE_OBSTACLE_DIST4)
      idx = -1;

    if (idx == -1) {
      throttle = ESC_BRAKE;
    } else {
      if (scan[idx] > drive_medium_thr)
        throttle = medium_esc;
      else
        throttle = slow_esc;
    }
  } else {
    // The road ahead is clear -- go fast
    throttle = fast_esc;
  }

  float idx2yaw;
  if (idx >= 0) {
    idx2yaw = 1.0 * idx / scan.size() - 0.5;
    idx2yaw = yaw_multiplier*idx2yaw;
    idx2yaw = idx2yaw < -0.5 ? -0.5 : idx2yaw > 0.5 ? 0.5 : idx2yaw;
    yaw = floor(idx2yaw * YAW_RANGE + YAW_MID);
    if (throttle < fast_esc) {
        float min_left = 10000;  // Large number to initialize
        float min_right = 10000;
        for (auto i=0; i<scan.size()/2; i++)
            if (scan[i] < min_left)
                min_left = scan[i];
        for (auto i=scan.size()/2; i<scan.size(); i++)
            if (scan[i] < min_right)
                min_right = scan[i];

        if (min_left < KAROLS_DIST or min_right < KAROLS_DIST) {
            if (min_left > min_right)
                yaw -= karols_modifier;
            else
                yaw += karols_modifier;
        }
    }
  }

  if (estart and !estop and idx >= 0) {
    tmp_uint16.data = throttle;
    pub_esc.publish(tmp_uint16);
    tmp_uint16.data = yaw;
    pub_servo.publish(tmp_uint16);
  } else {
    throttle = ESC_BRAKE;
    tmp_uint16.data = throttle;
    pub_esc.publish(tmp_uint16);
  }

  // Log everything
  double delta_between_callbacks = time_.toSec() - old_time_.toSec();
  double delta_within_callback = ros::Time::now().toSec() - time_.toSec();
  ROS_WARN(
    "y: %d t: %d dt_bet_cb: %.4f dt_in_cb: %.4f",
    yaw, throttle, delta_between_callbacks, delta_within_callback
  );

  old_time_ = time_;
}


void UltimateRacer::estop_cb(const std_msgs::UInt16 & data) {
  last_stop_msg_ts = ros::Time::now().toSec();
  if (data.data == 0) {
    ROS_WARN("Emergency stop!");
    exec_estop();
  } else if (data.data == 2309) {
    ROS_WARN("GO!");
    estart = true;
  }
}


void UltimateRacer::exec_estop() {
  estop = true;
  yaw = YAW_MID;
  throttle = ESC_BRAKE;
  tmp_uint16.data = throttle;
  pub_esc.publish(tmp_uint16);
  tmp_uint16.data = yaw;
  pub_servo.publish(tmp_uint16);
}


float UltimateRacer::steerMAX(std::vector<float> & scan, float width_margin) const {
  float min_scan = *std::min_element(scan.begin(), scan.end());
  if (min_scan <= SAFE_OBSTACLE_DIST2)
    return -1;

  // The following actually copies the vector `scan`
  std::vector<float> scan2(scan);
  int idx = -1;
  bool is_reachable = false;
  int scan_size = scan.size();

  // First, prepare `segs`
  std::vector<int> segs = {180, scan_size-180};
  for (auto i=1; i<scan_size; i++)
    if (std::abs(scan[i]-scan[i-1]) > NON_CONT_DIST) {
      segs.push_back(i);
      segs.push_back(i-1);
    }

  // We're disregarding outskirts of the scan so that the car won't go backwards
  // (this happened when during a U-turn the car chose to go back because the
  // scan readings were higher than those down the track)
  for (auto i=0; i<scan_range_deleted; i++)
    scan2[i] = -1;
  for (auto i=scan2.size()-scan_range_deleted; i<scan2.size(); i++)
    scan2[i] = -1;

  while (!is_reachable) {
    float max_value = *std::max_element(scan2.begin(), scan2.end());
    if (max_value <= 0)
      break;

    // Search for argmax (https://en.cppreference.com/w/cpp/algorithm/max_element)
    idx = std::distance(
      scan2.begin(),
      std::max_element(scan2.begin(), scan2.end())
    );

    for (auto s : segs) {
      if (s != idx) {
        bool could_be_reached = check_if_reachable(
          scan[idx],
          scan[s],
          std::abs(s-idx),
          width_margin
        );

        if (!could_be_reached) {
          int left_limit = std::max(0, idx-5);
          int right_limit = std::min(idx+5, int(scan2.size()-1));
          for (int i=left_limit; i<right_limit; i++)
            scan2[i] = -1;
          break;
        }
      }
    }
    // Instead of comparing to -1 (remember, we're dealing with floats),
    //  we check whether it's non-negative
    if (scan2[idx] >= 0)
      is_reachable = true;
  }

  if (is_reachable == false)
    idx = -1;

  return idx;
}


bool UltimateRacer::check_if_reachable(float r1, float r2, int alpha, float width_margin) const {
  if (SAFE_OBSTACLE_DIST1 < r1 and r1 < r2 + SAFE_OBSTACLE_DIST1)
    return true;
  else
    return (r2*SIN_ALPHA[alpha] > width_margin);
}


int main(int argc, char **argv) {

  ros::init(argc, argv, "ultimate_racer_in_cpp4");

  ros::NodeHandle nh;
  int slow_esc;
  int medium_esc;
  int fast_esc;
  float drive_medium_thr;
  float drive_fast_thr;
  int drive_fast_angle_margin;
  float yaw_multiplier;
  int num_points_below_sod3_to_stop;
  float karols_modifier;
  float margin_drive_slow;
  float margin_drive_fast;
  int scan_range_deleted;

  if (argc == 13) {
    slow_esc = atoi(argv[1]);
    medium_esc = atoi(argv[2]);
    fast_esc = atoi(argv[3]);
    drive_medium_thr = atof(argv[4]);
    drive_fast_thr = atof(argv[5]);
    drive_fast_angle_margin = atoi(argv[6]);
    yaw_multiplier = atof(argv[7]);
    num_points_below_sod3_to_stop = atoi(argv[8]);
    karols_modifier = atof(argv[9]);
    margin_drive_slow = atof(argv[10]);
    margin_drive_fast = atof(argv[11]);
    scan_range_deleted = atoi(argv[12]);
  } else {
    slow_esc = 1555;
    medium_esc = 1560;
    fast_esc = 1565;
    drive_medium_thr = 3.5;
    drive_fast_thr = 5.0;
    drive_fast_angle_margin = 60;
    yaw_multiplier = 1.2;
    num_points_below_sod3_to_stop = 20;
    karols_modifier = 50;
    margin_drive_slow = 0.3;
    margin_drive_fast = 0.5;
    scan_range_deleted = 170;
  }

  std::cout << "slow_esc: " << slow_esc
            << " medium_esc: " << medium_esc
            << " fast_esc: " << fast_esc
            << " drive_medium_thr: " << drive_medium_thr
            << " drive_fast_thr: " << drive_fast_thr
            << " drive_fast_angle_margin: " << drive_fast_angle_margin
            << " yaw_multiplier: " << yaw_multiplier
            << " num_points_below_sod3_to_stop: " << num_points_below_sod3_to_stop
            << " karols_modifier: " << karols_modifier
            << " margin_drive_slow: " << margin_drive_slow
            << " margin_drive_fast: " << margin_drive_fast
            << " scan_range_deleted: " << scan_range_deleted
            << std::endl;

  UltimateRacer racer(
    &nh,
    slow_esc, medium_esc, fast_esc,
    drive_medium_thr, drive_fast_thr,
    drive_fast_angle_margin,
    yaw_multiplier,
    num_points_below_sod3_to_stop,
    karols_modifier,
    margin_drive_slow, margin_drive_fast,
    scan_range_deleted
  );

  ros::Rate loop_rate(40);

  ros::spin();

  return 0;
}
