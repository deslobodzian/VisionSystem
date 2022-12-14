//
// Created by deslobodzian on 11/22/22.
//

#include "vision_container.hpp"

VisionContainer::VisionContainer() {}

void VisionContainer::init() {
    info("[VisionContainer]: Subscribing to odometry");
    nt_manager_.add_subscriber(&odometry_sub_);
    info("[VisionContainer]: Starting odometry subscription thread");
    odometry_thread_ = std::thread(&VisionContainer::odometry_handle, this);

    info("[VisionContainer]: Setting up zed camera");
    zed_config zed_config{};
    zed_config.res = sl::RESOLUTION::VGA;
    zed_config.fps = 100;
    zed_config.depth_mode = sl::DEPTH_MODE::ULTRA;
    zed_config.sdk_verbose = true;
    zed_config.coordinate_system = sl::COORDINATE_SYSTEM::RIGHT_HANDED_Z_UP_X_FWD;
    zed_config.units = sl::UNIT::METER;
    zed_config.max_depth = 20.0;

    zed_camera_ = new Zed(zed_config);
    zed_camera_->open_camera();

    info("[VisionContainer]: Setting up monocular cameras");
    IntrinsicParameters<float> parameters{1116.821, 1113.573, 678.58, 367.73};
    CameraConfig<float> monocular_config (
            "/dev/video0", // need to find a better way for device id.
            68.5,
            resolution(320, 240),
            30,
            parameters
    );
    monocular_camera_ = new MonocularCamera<float>(monocular_config);
    monocular_camera_->open_camera();

    info("[VisionContainer]: Setting up AprilTag manager");
    detector_config apriltag_config {};
    apriltag_config.tf = tag16h5;
    apriltag_config.quad_decimate = 1;
    apriltag_config.quad_sigma = 0.5;
    apriltag_config.nthreads = 2;
    apriltag_config.debug = false;
    apriltag_config.refine_edges = true;

    tag_manager_ = new AprilTagManager<float>(apriltag_config);


}

void VisionContainer::detect_zed_targets() {
    tag_manager_->detect_tags_zed(zed_camera_);
}

void VisionContainer::detect_monocular_targets() {
    tag_manager_->detect_tags_monocular(monocular_camera_);
}

void VisionContainer::run() {
    init();
    info("[VisionContainer]: Starting system");

    vision_runner_ = new VisionRunner(&task_manager_, 0.05, "vision-runner");

    vision_runner_->control_input_ = control_input_;
    vision_runner_->measurements_ = measurements_;

    // init threads;
    info("[VisionContainer]: Starting detection zed task");
    PeriodicMemberFunction<VisionContainer> zed_task(
            &task_manager_,
            0.02,
            "detection",
            &VisionContainer::detect_zed_targets,
            this
    );
    // monocular task runs slower as we don't need this to update as fast as the zed camera.
    info("[VisionContainer]: Starting detection monocular task");
    PeriodicMemberFunction<VisionContainer> monocular_task(
            &task_manager_,
            0.05,
            "detection",
            &VisionContainer::detect_zed_targets,
            this
    );

    zed_task.start();
    monocular_task.start();

    vision_runner_->start();

    for (;;) {
        usleep(1000000);
    }
}

void VisionContainer::odometry_handle() {
    nt_manager_.get_subscription(&odometry_sub_);
    control_input_->set_odometry_input(&odometry_sub_);
}

VisionContainer::~VisionContainer() {
    delete vision_runner_;
    delete tag_manager_;
    delete monocular_camera_;
    delete zed_camera_;
}
