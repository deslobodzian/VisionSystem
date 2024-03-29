#ifdef WITH_CUDA
#ifndef VISION_SYSTEM_DETECTIONS_PLAYBACK_HPP
#define VISION_SYSTEM_DETECTIONS_PLAYBACK_HPP

#include <string>
#include <sl/Camera.hpp>
#include "zed.hpp"
#include "object_detector.hpp"

class DetectionsPlayback {
public:
    DetectionsPlayback(const std::string& svo_file);
    ~DetectionsPlayback();
    
    void detect();
    void export_video();
    
private:
    zed_config cfg;
    detection_config det_cfg;
    ZedCamera zed_;
    ObjectDetector<ZedCamera> detector_;
    //sl::Camera zed_;
    sl::Mat left_sl;
    sl::Resolution display_resolution;
    cv::Mat left_cv;
    cv::VideoWriter video_writer;
};

#endif /* VISION_SYSTEM_DETECTIONS_PLAYBACK_HPP */
#endif /* WITH_CUDA */
