//
// Created by DSlobodzian on 4/21/2022.
//

#ifndef VISION_SYSTEM_TARGET_INFO_HPP
#define VISION_SYSTEM_TARGET_INFO_HPP

#include <sl/Camera.hpp>
#include <Eigen/Dense>
#include "map.hpp"

class TrackedObjectInfo {
private:
    game_elements element_;
    double x_;
    double y_;
    double z_;

    double vx_;
    double vy_;
    double vz_;

public:
    TrackedObjectInfo();
    explicit TrackedObjectInfo(sl::ObjectData object);
    ~TrackedObjectInfo();

    const double get_x();
    const double get_y();
    const double get_z();

    const double get_vx();
    const double get_vy();
    const double get_vz();

    double get_distance(double x_offset, double y_offset, double z_offset);
    double get_distance(sl::Transform offset);
    double get_yaw_angle();
    double get_pitch_angle();
    std::string to_packet();
};

#endif //VISION_SYSTEM_TARGET_INFO_HPP
