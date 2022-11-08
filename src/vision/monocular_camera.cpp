//
// Created by DSlobodzian on 1/27/2022.
//
#include "vision/monocular_camera.hpp"



MonocularCamera::MonocularCamera(CameraConfig config) {
    config_ = config;
}
MonocularCamera::~MonocularCamera() {
    cap_.release();
}

bool MonocularCamera::open_camera() {
    //std::string c = 
//	    "v4l2src device=/dev/video" + std::to_string(device_id_) +
//	    " ! video/x-raw, width=" + std::to_string(config_.camera_resolution.width) +
//	    ", height=" + std::to_string(config_.camera_resolution.height) +
//	    ", framerate="+ std::to_string(config_.frames_per_second) + 
//	    "/1 ! videoconvert ! appsink";
    cap_.open(config_.get_device_id(), CAP_V4L2);
    cap_.set(CAP_PROP_FRAME_WIDTH, config_.get_camera_resolution().width);
    cap_.set(CAP_PROP_FRAME_HEIGHT, config_.get_camera_resolution().height);
    cap_.set(CAP_PROP_FPS, config_.get_fps());
    return cap_.isOpened();
}

bool MonocularCamera::read_frame() {
    cap_.read(frame_);
    return frame_.empty();
}

Mat MonocularCamera::get_frame() {
    return frame_;
}

void MonocularCamera::get_frame(cv::Mat& image) {
    cap_.read(image);
}

double MonocularCamera::yaw_angle_to_object(tracked_object &obj) {
    Point object_center = (obj.object.br() + obj.object.tl()) / 2;
    Point center(config_.get_camera_resolution().width / 2, config_.get_camera_resolution().width / 2);
    double focal_length = config_.get_camera_resolution().width / (2 * tan(config_.get_fov().horizontal / 2));
    return atan((center - object_center).x / focal_length);
}

double MonocularCamera::pitch_angle_to_object(tracked_object &obj) {
    Point object_center = (obj.object.br() + obj.object.tl()) / 2;
    Point center((int) config_.get_camera_resolution().height / 2, config_.get_camera_resolution().height / 2);
    double focal_length = config_.get_camera_resolution().height / (2 * tan(config_.get_fov().vertical / 2));
    return atan((center - object_center).x / focal_length);
}

void MonocularCamera::add_measurements(std::vector<Measurement> &z) {
    for (auto object : objects_) {
        z.push_back(Measurement(-1, yaw_angle_to_object(object), (game_elements) object.class_id));
    }
}

void MonocularCamera::draw_rect(Rect rect) {
    rectangle(frame_, rect, Scalar(0, 255, 255), 1);
}

void MonocularCamera::draw_crosshair(Rect rect) {
    Point object_center = (rect.br() + rect.tl()) / 2.0;
    Point top = object_center + Point(0, 10);
    Point bot = object_center - Point(0, 10);
    Point left = object_center - Point(10, 0);
    Point right = object_center + Point(10, 0);

    line(frame_, top, bot, Scalar(0, 255, 0), 1);
    line(frame_, left, right, Scalar(0, 255, 0), 1);
}

void MonocularCamera::add_tracked_objects(std::vector<tracked_object> objs) {
    objects_ = objs;
//    info("Objects found is: " + std::to_string(objects_.size()));
}

void MonocularCamera::draw_crosshair(tracked_object obj) {
    draw_crosshair(obj.object);
}

void MonocularCamera::draw_tracked_objects() {
	for (auto& i : objects_) {
		draw_rect(i.object);
		draw_crosshair(i.object);
	}
}

std::vector<tracked_object> MonocularCamera::get_objects(int class_id) {
    std::vector<tracked_object> temp;
    if (latest_objects_.size() > 0) {
        for (auto object : latest_objects_) {
            if (object.class_id == class_id) {
                temp.push_back(object);
            }
        }
    }
    return temp;
}

void MonocularCamera::update_objects() {
    latest_objects_ = objects_;
}

tracked_object MonocularCamera::get_object_at_index(int class_id, int index) {
    std::vector<tracked_object> temp = get_objects(class_id);
    if (temp.empty()) {
        Rect r(Point(0,0), Point(1,1));
        tracked_object empty(r, 99);
        return empty;
    }
    return temp.at(index);
}

tracked_object MonocularCamera::closest_object_to_camera(int class_id) {
    std::vector <tracked_object> temp = get_objects(class_id);
//    info("temp size is: " + std::to_string(temp.size()));
    int index = 0;
    if (temp.size() > 0) {
        for (int i = 0; i < temp.size(); ++i) {
            if (temp.at(i).object.area() > temp.at(index).object.area()) {
                index = i;
            }
        }
    } else {
        Rect r(Point(0,0), Point(1,1));
        tracked_object empty(r, 99);
        return empty;
    }
    return temp.at(index);
}

tracked_object MonocularCamera::closest_object_to_camera(game_elements game_element) {
    return closest_object_to_camera((int) game_element);
}

bool MonocularCamera::is_object_in_box(tracked_object &obj, Rect &box) {
    int box_tl_x = box.tl().x;
    int box_tl_y = box.tl().y;
    int box_br_x = box.br().x;
    int box_br_y = box.br().y;
    int obj_tl_x = obj.object.tl().x;
    int obj_tl_y = obj.object.tl().y;
    int obj_br_x = obj.object.br().x;
    int obj_br_y = obj.object.br().y;
    return box_tl_x >= obj_tl_x && box_tl_y >= obj_tl_y && box_br_y >= obj_br_y && box_br_x >= obj_br_x;
}

int MonocularCamera::get_id() {
	return device_id_;
}

