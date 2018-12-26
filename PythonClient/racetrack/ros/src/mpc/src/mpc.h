#pragma once

#include <vector>
#include <math.h> /* floor */

#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <std_msgs/UInt16.h>
#include <std_msgs/Float32.h>


class UltimateRacer {
private:

  ///* The PWM signals passed to the ESC that causes the car to brake
  static constexpr int ESC_BRAKE = 1000;

  ///* The PWM signal that keeps the car heading straight
  static constexpr int YAW_MID = 1585;

  ///* The PWM range for our servo -- you might want to change it
  static constexpr int YAW_RANGE = 1200;

  ///* The index of the lidar scan that's exactly in the middle (and should in principle be exactly aiming ahead of the vehicle)
  static constexpr int MID_SCAN_IDX = 540;

  ///* The `SAFE_OBSTACLE_DISTX` variables (in meters) control the distance
  // at which (depending on the scenario) the car uses the `ESC_BRAKE` signal to stop
  static constexpr float SAFE_OBSTACLE_DIST1 = 0.6;
  static constexpr float SAFE_OBSTACLE_DIST2 = 0.25;
  static constexpr float SAFE_OBSTACLE_DIST3 = 0.5;
  static constexpr float SAFE_OBSTACLE_DIST4 = 0.5;

  // The margin around `MID_SCAN_IDX` considered when deciding whether there's an obstacle ahead
  static constexpr int MARGIN_FOR_SOD3 = 20;

  ///* If there's a difference between consecutive scan values that exceeds `NON_CONT_DIST`,
  // the indeces of the scans are used as boundaries of so-called "segments" (see the `steerMAX` method)
  static constexpr float NON_CONT_DIST = 0.2;

  ///* If the car comes closer to a boundary than the following parameter, it will get a "bump" to the
  // PWM passed to the ESC that will cause it to steer away from the boundary
  static constexpr float KAROLS_DIST = 1.0;

  ///* For performance reasons, we pre-compute sine values for 1080 angles corresponding to
  // 1080 indeces of the lidar scan
  static constexpr int FULL_ANGLE = 360;
  static constexpr int STEPS_PER_DEGREE = 4;
  static constexpr int NUM_ANGLES_TO_STORE = FULL_ANGLE * STEPS_PER_DEGREE;
  float SIN_ALPHA[NUM_ANGLES_TO_STORE];

  ///* The PWM value effectively passed to the ESC (controls RPMs of the engine)
  // An important NOTE: the RPMs *do not* scale linearly with the PWM!
  int throttle = 1500;

  ///* The PWM value effectively passed to the servo (controls the angle of the wheels, aka "yaw")
  int yaw = YAW_MID;

  ///* Bookkeeping for whether an emergency STOP signal was passed to the car (from the master script)
  bool estop = false;
  bool estart = false;
  bool ego = false;

  ///* Parameters of the naive strategy
  int slow_esc;  // PWM signal corresponding to "slow speed"
  int medium_esc; // PWM signal corresponding to "medium speed"
  int fast_esc; // PWM signal corresponding to "fast speed"
  float drive_medium_thr;  // distance (in meters) up to which the car uses `slow_esc`
  float drive_fast_thr; // distance (in meters) up to which the car uses `medium_esc`
  int drive_fast_angle_margin; // margin around `MID_SCAN_IDX` beyond which lidar scans are capped to 10m
  float yaw_multiplier;  // coefficient by which the resultant `yaw` is being multiplied (to make the car turn more aggresively)
  int num_points_below_sod3_to_stop; // number of scans that are below sod3 needed to stop the car
  float karols_modifier; // the amount by which the yaw PWM is being modified if there's a boundary too close to the car
  float margin_drive_slow; // margin passed to `steerMAX` when the car CAN'T go fast
  float margin_drive_fast; // margin passed to `steerMAX` when the car CAN go fast
  int scan_range_deleted; // the number of indeces of the scan being ignored (on the outskirts of the scan)

  ///* A placeholder used to pass `throttle` and `yaw` to their respective publishers
  std_msgs::UInt16 tmp_uint16;

  ///* Time variables used to "benchmark" the callbacks and to keep track of
  // how long has it been since the last contact with the master control
  ros::Time time_, old_time_;
  double last_stop_msg_ts;

  ///* ROS-related
  ros::NodeHandle nh;

  ///* Publisher for the ESC, and the second one: for the servo
  ros::Publisher pub_esc;
  ros::Publisher pub_servo;

  ///* Subscribers for the readings of the lidar, and the second one: for an emergency stop signal
  ros::Subscriber sub_scan;
  ros::Subscriber sub_estop;

  /**
   * This is where the magic happens: the car chooses which way it can go,
   * i.e. which scan reading looks the most promissing (in terms of: "I can go there")
   * @param scan The lidar scan
   * @param margin Parameter controling how wide an opening needs to be to be considered passable
   */
  float steerMAX(std::vector<float> & scan, float margin) const;

  /**
   * Method called by `steerMAX` to decide if a scan reading (`r1`) is reachable, considering
   *  considering that `r2` is an end of a continuous segment.
   * @param r1 The scan which we would like to know if it's reachable
   * @param r2 The scan that's an end of a segment
   * @param alpha Angle index (passed to an array of sine values)
   * @param margin
   */
  bool check_if_reachable(float r1, float r2, int alpha, float margin) const;

  /**
   * Helper method for executing an emergency STOP
   */
  void exec_estop();

public:
  UltimateRacer(
    ros::NodeHandle* nodehandle,
    int slow_esc, int medium_esc, int fast_esc,
    float drive_medium_thr, float drive_fast_thr,
    int drive_fast_angle_margin,
    float yaw_multiplier,
    int num_points_below_sod3_to_stop,
    float karols_modifier,
    float margin_drive_slow,
    float margin_drive_fast,
    int scan_range_deleted
  );

  ///* Callbacks
  /**
   * This callback calls `steerMAX` to choose the direction it should go,
   * and depending on a sequence of conditions, it also chooses the speed at which
   * the car will go
   * @param data Lidar scan coming directly from the lidar
   */
  void scan_cb(const sensor_msgs::LaserScan & data);

  /**
   * This callback receives the emergency STOP signal, and stops the car.
   * @param data The signal that can cause the car to stop (if it's the right value)
   */
  void estop_cb(const std_msgs::UInt16 & data);
};
