//
// Created by deslobodzian on 11/23/22.
//
#include "vision_runner.hpp"
#include "utils/logger.hpp"
#include "utils/timer.h"
#include "vision_pose_generated.h"
#include "vision_pose_array_generated.h"
#include "april_tag_generated.h"
#include "april_tag_array_generated.h"
#include "utils/zmq_flatbuffers_utils.hpp"
#include <random>

//namespace {
//float randomFloat(float min, float max) {
//    static std::mt19937 rng(std::random_device{}());
//    std::uniform_real_distribution<float> dist(min, max);
//    return dist(rng);
//}

// Simulate object with random position
//struct SimulatedObject {
//    int id;
//    struct Position {
//        float x, y, z;
//    } position;
//};
//}

VisionRunner::VisionRunner(
        std::shared_ptr<TaskManager> manager,
        double period,
        const std::string& name,
        const std::shared_ptr<ZmqManager> zmq_manager) :
        Task(manager, period, name),
        zmq_manager_(zmq_manager),
        use_detection_(false)
        {
#ifdef WITH_CUDA
    // Dennis's camera: 47502321
    // Outliers's camera: 41535987
    cfg_.serial_number = 41535987;
    cfg_.res = sl::RESOLUTION::SVGA;
    //cfg_.res = sl::RESOLUTION::VGA;
    cfg_.sdk_verbose = false;
    cfg_.enable_tracking = true;
    cfg_.depth_mode = sl::DEPTH_MODE::PERFORMANCE;
    cfg_.coordinate_system = sl::COORDINATE_SYSTEM::RIGHT_HANDED_Z_UP_X_FWD;
    cfg_.max_depth = 20;
    cfg_.prediction_timeout_s = 0.2f;
    cfg_.batch_latency = 0.2f;
    cfg_.id_retention_time = 0.f;
    cfg_.detection_confidence_threshold = 50;
    cfg_.default_memory = sl::MEM::GPU;
    camera_.configure(cfg_);
    camera_.open();

    const sl::Resolution display_resolution = camera_.get_resolution();
    const uint32_t img_height = display_resolution.height;
    const uint32_t img_width = display_resolution.width;
    constexpr uint32_t tile_size = 4; 
    constexpr cuAprilTagsFamily tag_family = NVAT_TAG36H11; 
    constexpr float tag_dim = 0.16f;
    tag_detector_.init_detector(img_width, img_height, tile_size, tag_family, tag_dim);
#endif
}

void VisionRunner::init() {
    LOG_INFO("Initializing [VisionRunner]");
#ifdef WITH_CUDA
    camera_.enable_tracking();
    camera_.enable_object_detection();
    detection_config det_cfg;
    det_cfg.nms_thres = 0.5;
    det_cfg.obj_thres = 0.5;
    detector_.configure(det_cfg);
#endif
}

void VisionRunner::run() {
    using namespace std::chrono;

    if (const auto received = zmq_manager_->get_subscriber("UseDetection").receive()) {
        const auto& [topic, msg] = *received;
        if (topic == "UseDetection") { 
            use_detection_ = process_use_detection(msg);
        }
    }

#ifdef WITH_CUDA 
    const auto start_time = high_resolution_clock::now();
    auto& builder = zmq_manager_->get_publisher("main").get_builder(); 
    const char* topic_name = use_detection_ ? "Objects" : "BackAprilTags";

    if (use_detection_) {
        camera_.fetch_measurements(MeasurementType::IMAGE);
        detector_.detect_objects(camera_);
        camera_.fetch_measurements(MeasurementType::OBJECTS);
        const sl::Objects& objects = camera_.retrieve_objects();

        const auto object_detection_time = high_resolution_clock::now();
        const auto object_detection_ms = duration_cast<milliseconds>(object_detection_time - start_time).count();

        std::vector<flatbuffers::Offset<Messages::VisionPose>> vision_pose_offsets;
        vision_pose_offsets.reserve(objects.object_list.size());

        for (const auto& obj : objects.object_list) {
            auto vision_pose = Messages::CreateVisionPose(
                    builder, 
                    obj.id + 100,
                    obj.position.x, 
                    obj.position.y, 
                    obj.position.z, 
                    object_detection_ms
            );
            vision_pose_offsets.push_back(vision_pose);
        }
        auto poses_vector = builder.CreateVector(vision_pose_offsets);
        auto vision_pose_array = Messages::CreateVisionPoseArray(builder, poses_vector);

        builder.Finish(vision_pose_array);
        zmq_manager_->get_publisher("main").publish_prebuilt(
                topic_name,
                builder.GetBufferPointer(),
                builder.GetSize()  
        );
    } else {
        camera_.fetch_measurements(MeasurementType::IMAGE_AND_POINT_CLOUD);
        auto tags = tag_detector_.detect_april_tags_in_sl_image(camera_.get_left_image());
        auto zed_tags = tag_detector_.calculate_zed_apriltag(camera_.get_point_cloud(), camera_.get_normals(), tags);

        const auto apriltag_detection_time = high_resolution_clock::now();
        const auto apriltag_detection_ms = duration_cast<milliseconds>(apriltag_detection_time - start_time).count();
        LOG_DEBUG("April tag detection with positioning took: ", apriltag_detection_ms, " ms");


        std::vector<flatbuffers::Offset<Messages::AprilTag>> april_tag_offsets;
        april_tag_offsets.reserve(zed_tags.size());

        for (const auto& tag : zed_tags) {
            auto april_tag = Messages::CreateAprilTag(
                    builder,
                    tag.tag_id,
                    tag.center.x,
                    tag.center.y,
                    tag.center.z,
                    tag.orientation.data[0],
                    tag.orientation.data[1],
                    tag.orientation.data[2],
                    tag.orientation.data[3],
                    apriltag_detection_ms
            );
            april_tag_offsets.push_back(april_tag);
        }

        auto tags_vector = builder.CreateVector(april_tag_offsets);
        auto april_tags_array = Messages::CreateAprilTagArray(builder, tags_vector);

        builder.Finish(april_tags_array);
        zmq_manager_->get_publisher("main").publish_prebuilt(
                topic_name,
                builder.GetBufferPointer(),
                builder.GetSize()  
        );
    }
#endif
/*
 *
    std::vector<flatbuffers::Offset<Messages::VisionPose>> visionPoseOffsets;
    auto now = std::chrono::high_resolution_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // Access the FlatBufferBuilder from your ZMQ manager's publisher
    auto& builder = zmq_manager_->get_publisher("main").get_builder();

    std::vector<SimulatedObject> objects;
    const int numberOfObjects = 10;
    for (int i = 0; i < numberOfObjects; ++i) {
        SimulatedObject obj;
        obj.id = i;
        obj.position.x = randomFloat(-10.0f, 10.0f);
        obj.position.y = randomFloat(-10.0f, 10.0f);
        obj.position.z = randomFloat(-10.0f, 10.0f);
        objects.push_back(obj);
    }

    for (const auto& obj : objects) {
        auto visionPose = Messages::CreateVisionPose(
                builder,
                obj.id,
                obj.position.x,
                obj.position.y,
                obj.position.z,
                now_ms
                );
        visionPoseOffsets.push_back(visionPose);
    }

    auto posesVector = builder.CreateVector(visionPoseOffsets);
    auto visionPoseArray = Messages::CreateVisionPoseArray(builder, posesVector);

<<<<<<< HEAD
    zmq_manager_->get_publisher("main").publish(
        "VisionPoseArray", 
        Messages::CreateVisionPoseArray,
        posesVector 
    );
    LOG_DEBUG("Zed end to end took: ", t.get_ms(), " ms");
#endif
=======
    // Finish the FlatBuffer
    builder.Finish(visionPoseArray);

    // Use the publish_prebuilt method to send the constructed message
    zmq_manager_->get_publisher("main").publish_prebuilt(
            "Objects", // Topic name
            builder.GetBufferPointer(), // The buffer containing the serialized data
            builder.GetSize() // The size of the serialized data
            );
>>>>>>> 124603d (new zmq changes)
    zmq_manager_->get_publisher("main").publish(
            "VisionPose",
            Messages::CreateVisionPose,
            123,     1.0f, 2.0f, 3.0f, 0.0
    );

*/
}

VisionRunner::~VisionRunner() {

}
