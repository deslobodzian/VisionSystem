//
// Created by deslobodzian on 5/2/22.
//

#include "inference/inference_manager.hpp"

InferenceManager::InferenceManager(const std::string& custom_engine) {
    engine_name_ = custom_engine;
}

void InferenceManager::init() {
    detector_.initialize_engine(engine_name_);
}

void InferenceManager::inference_on_device(Zed *camera) {
    camera->fetch_measurements();
    camera->get_left_image(zed_struct_.sl_mat);
    detector_.prepare_inference(zed_struct_.sl_mat, zed_struct_.cv_mat);
    detector_.run_inference(zed_struct_.cv_mat, &zed_struct_.custom_obj_data_);
    camera->ingest_custom_objects(zed_struct_.custom_obj_data_);
}

InferenceManager::~InferenceManager() {
    detector_.kill();
}

