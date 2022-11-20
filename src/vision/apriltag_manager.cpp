#include "vision/apriltag_manager.hpp"

void AprilTagManager::add_detector_thread(Zed &camera) {
	threads_.emplace_back(&AprilTagManager::detector_zed, this, std::ref(camera));
}

void AprilTagManager::add_detector_thread(MonocularCamera &camera) {
	threads_.emplace_back(&AprilTagManager::detector_monocular, this, std::ref(camera));
}

void AprilTagManager::detector_zed(Zed &camera) {
    DetectorConfig cfg = {
            tag36h11,
            0.5,
            0.5,
            1,
            false,
            true
    };
    zed_detector_ = TagDetector(cfg);

	while (true) {
        auto start = std::chrono::high_resolution_clock::now();
        camera.fetch_measurements();
        zed_detector_.fetch_detections(slMat_to_cvMat(camera.get_left_image()));
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        zed_dt_ = duration.count();
        std::vector<TrackedTargetInfo> targets;

        apriltag_detection_t *det;
        for (int i = 0; i < zed_detector_.get_current_number_of_targets(); i++) {
            zarray_get(zed_detector_.get_current_detections(), i, &det);
            Corners c = zed_detector_.get_detection_corners(det);
            sl::float3 tr = camera.get_position_from_pixel(c.tr);
            sl::float3 tl = camera.get_position_from_pixel(c.tl);
            sl::float3 br = camera.get_position_from_pixel(c.br);
            if (is_vec_nan(tr) || is_vec_nan(tl) || is_vec_nan(br)){
                error("Vec is nan");
            } else {
                sl::Pose pose = zed_detector_.get_estimated_target_pose(tr, tl, br);
                targets.emplace_back(TrackedTargetInfo(pose, det->id));
            }
        }
        const std::lock_guard<std::mutex> lock(zed_mtx_);
        zed_targets_ = targets;
        apriltag_detection_destroy(det);
//        info("Zed thread took " + std::to_string(duration.count()) + " milliseconds");

    }
}

std::vector<TrackedTargetInfo> AprilTagManager::get_zed_targets() {
    const std::lock_guard<std::mutex> lock(zed_mtx_);
    return zed_targets_;
}

void AprilTagManager::print_dt() {
    info(std::to_string(zed_dt_));
}

void AprilTagManager::detector_monocular(MonocularCamera &camera) {
}