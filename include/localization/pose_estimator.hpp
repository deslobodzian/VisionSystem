//
// Created by DSlobodzian on 1/27/2022.
//

#pragma once

#include <iostream>
#include <thread>
#include "inference/yolov5.hpp"
#include "particle_filter.hpp"
#include "map.hpp"
#include "vision/Zed.hpp"
#include "networking/udp_server.hpp"
#include "vision/monocular_camera.hpp"

class PoseEstimator {
private:
    int num_monocular_cameras_;
    int num_zed_cameras_;
    std::vector<Measurement> z_;
    Eigen::Vector3d init_pose_;
    UDPServer server_;
    Zed zed_;
    ParticleFilter filter_;
    std::string engine_name_ = "custom.engine";
    std::vector<std::thread> inference_threads_;
    std::thread pose_estimation_thread_;
    std::vector<MonocularCamera> monocular_cameras_;
    std::vector<Yolov5> inference_engines_;
    std::vector<bool> threads_started_;
public:
    PoseEstimator() = default;
    PoseEstimator(int num_monocular_cameras);
    PoseEstimator(int num_monocular_cameras, int num_zed_cameras, std::vector<Landmark> landmarks);
    ~PoseEstimator();

    void run_zed(Eigen::Vector3d init_pose);
    void run_inference(MonocularCamera& camera);
    void run_inference_zed(Zed& camera);
    void update_measurements();
    void estimate_pose();
    void init();
    void kill();

    void print_measurements(int camera_id);
    void print_zed_measurements(int object_id);
    void send_message();
    void display_frame(int camera_id);

    bool threads_started();
    bool start_estimator();
    Zed& get_zed();
};
