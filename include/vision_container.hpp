//
// Created by deslobodzian on 11/22/22.
//

#ifndef VISION_SYSTEM_VISION_CONTAINER_HPP
#define VISION_SYSTEM_VISION_CONTAINER_HPP

#define PRIORITY 49

#include "estimator/estimator.hpp"
#include <sys/mman.h>
#include "utils/task.hpp"
#include "utils/utils.hpp"
#include "networking/odometry_sub.hpp"
#include "networking/vision_pub.hpp"
#include "networking/nt_manager.hpp"
#include "vision_runner.hpp"
#include "vision/Zed.hpp"
#include "vision/apriltag_manager.hpp"

class VisionContainer {
public:
    VisionContainer();

    void init_scheduler();
    void init();
    void run();

    virtual ~VisionContainer();

    std::vector<Measurement<double>>* measurements_;
    ControlInput<double>* control_input_;

    void get_odometry_data();

protected:
    TaskManager task_manager_;
    AprilTagManager tag_manager_;
    NTManager nt_manager_;

    odometry_subscribable odometry_sub_;

    Zed* zed_camera_ = nullptr;
    MonocularCamera<float>* monocular_camera_ = nullptr;

    VisionRunner* vision_runner_ = nullptr;

};

#endif //VISION_SYSTEM_VISION_CONTAINER_HPP