/// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

/**
 * @brief provide x,y,z-coordinate components of a parameter (in a metric 3D space)
 */
struct XYZComponents {
  float x{};  //!< x-component
  float y{};  //!< y-component
  float z{};  //!< z-component
};

/**
 * @brief Position (position + rotation) of an object.
 */
struct ObjectPosition {
  double xPos{};   //!< [m]
  double yPos{};   //!< [m]
  double zPos{};   //!< [m]
  double yaw{};    //!< [rad]
  double pitch{};  //!< [rad]
  double roll{};   //!< [rad]
};

/**
 * @brief Data of the ego vehicle
 */
struct EgoVehicleData {
  ObjectPosition position{}; /*!< position + rotation of the ego vehicle **/

  float steeringAngle{}; /*!< [rad] **/
  float speed{};         /*!< [m/s] **/
  double timeStamp{};    /*!< [s] **/

  XYZComponents velocity{};     /*!< component-wise velocity [m/s] **/
  XYZComponents acceleration{}; /*!< component-wise acceleration [m/s^2] **/

  float rollChange{};  /*!< change in roll over time [rad/s] **/
  float pitchChange{}; /*!< change in pitch over time [rad/s] **/
  float yawChange{};   /*!< change in yaw over time [rad/s] **/

  float steeringAngleChange{}; /*!< change of steeringAngle over time [rad/s] **/

  bool valid{false}; /*!< valid flag for this struct **/
};
