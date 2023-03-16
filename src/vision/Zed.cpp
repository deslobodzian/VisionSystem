//
// Created by DSlobodzian on 4/21/2022.
//
#include "vision/Zed.hpp"

Zed::Zed(const zed_config &config) {
    // Initial Parameters
    init_params_.camera_resolution = config.res;
    init_params_.camera_fps = config.fps;
    init_params_.camera_image_flip = config.flip_camera;
    init_params_.depth_mode = config.depth_mode;
    init_params_.sdk_verbose = config.sdk_verbose;
    init_params_.coordinate_system = config.coordinate_system;
    init_params_.coordinate_units = config.units;
    init_params_.depth_maximum_distance = config.max_depth;

    runtime_params_.measure3D_reference_frame = config.reference_frame;

    obj_detection_params_.enable_tracking = config.enable_tracking;
    obj_detection_params_.enable_mask_output = config.enable_mask_output;
    obj_detection_params_.detection_model = config.model;
    obj_detection_params_.prediction_timeout_s = config.prediction_timeout_s;

    obj_rt_params_.detection_confidence_threshold = config.detection_confidence_threshold;
    batch_params_.enable = config.enable_batch;
    batch_params_.id_retention_time = config.id_retention_time;
    batch_params_.latency = config.batch_latency;

    obj_detection_params_.batch_parameters = batch_params_;

}
bool Zed::successful_grab() {
    return (zed_.grab() == ERROR_CODE::SUCCESS);
}

int Zed::open_camera() {
    auto return_state = zed_.open(init_params_);
    calibration_params_ = zed_.getCameraInformation().camera_configuration.calibration_parameters;
    return (return_state == ERROR_CODE::SUCCESS);
}

int Zed::enable_tracking() {
    PositionalTrackingParameters tracking_params;
    if (!zed_.isOpened()) {
        error("[Zed]: Opening vision failed");
        return -1;
    }
    tracking_params.enable_area_memory = true;
    sl::Transform initial_position;
    zed_.enablePositionalTracking(tracking_params);
    return 0;
}

int Zed::enable_tracking(const Eigen::Vector3f &init_pose) {
    PositionalTrackingParameters tracking_params;
    if (!zed_.isOpened()) {
        error("[Zed]: Tracking failed to enable.");
        return -1;
    }
    tracking_params.enable_area_memory = true;
    sl::Transform initial_position;
    initial_position.setTranslation(sl::Translation(init_pose(0), init_pose(1), 0));
    tracking_params.initial_world_transform = initial_position;
    zed_.enablePositionalTracking(tracking_params);
    return 0;
}

void Zed::enable_object_detection() {
    if (zed_.enableObjectDetection(obj_detection_params_) != sl::ERROR_CODE::SUCCESS) {
        error("[ZED] Failed to enable object detection!");
    }
}

void Zed::fetch_measurements() {
    if (successful_grab()) {
        zed_.retrieveImage(measurements_.left_image, VIEW::LEFT);
        zed_.getSensorsData(measurements_.sensor_data, TIME_REFERENCE::IMAGE);
        measurements_.imu_data = measurements_.sensor_data.imu;
        measurements_.timestamp = measurements_.left_image.timestamp;
//        zed_.retrieveMeasure(measurements_.depth_map, MEASURE::DEPTH);
        zed_.retrieveMeasure(measurements_.point_cloud, MEASURE::XYZ);
//        zed_.getPosition(measurements_.camera_pose);
    }
}


sl::Mat Zed::get_depth_map() const {
    return measurements_.depth_map;
}

sl::Mat Zed::get_point_cloud() const {
    return measurements_.point_cloud;
}


sl::float3 Zed::get_position_from_pixel(int x, int y) const {
    sl::float4 point3d;
    get_point_cloud().getValue(x, y, &point3d);
    return {point3d.x, point3d.y, point3d.z};
}

sl::float3 Zed::get_position_from_pixel(const cv::Point &p) {
    return get_position_from_pixel(p.x, p.y);
}

float Zed::get_distance_from_point(const sl::float3& p) {
    return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

sl::Transform Zed::get_calibration_stereo_transform() const {
    return calibration_params_.stereo_transform;
}

sl::Mat Zed::get_left_image() const {
    return measurements_.left_image;
}
void Zed::get_left_image(sl::Mat& img) const {
    img = measurements_.left_image;
}

sl::Pose Zed::get_camera_pose() const {
    return measurements_.camera_pose;
}

Timestamp Zed::get_measurement_timestamp() const {
    return measurements_.timestamp;
}

SensorsData::IMUData Zed::get_imu_data() const {
    return measurements_.imu_data;
}

void Zed::ingest_custom_objects(std::vector<sl::CustomBoxObjectData>& objs) {
    zed_.ingestCustomBoxObjects(objs);
}

void Zed::retrieve_objects(sl::Objects &objs) {
    zed_.retrieveObjects(objs, obj_rt_params_);
}


void Zed::close() {
    zed_.close();
}

Zed::~Zed() {
    close();
};



