#ifndef VISION_SYSTEM_POST_PROCESS_H
#define VISION_SYSTEM_POST_PROCESS_H

#include <sl/Camera.hpp>
#include <cassert>
#include "cuda_utils.h"
#include "inference_utils.hpp"
#include <cuda_runtime.h>

void init_post_process_resources(int num_anchors);

void cuda_post_process(
    float* d_output, 
    std::vector<BBoxInfo>& binfo,
    float thres, 
    float xOffset, 
    float yOffset,
    float scalingFactor_x, 
    float scalingFactor_y,
    int orig_image_w, 
    int orig_image_h,
    int num_anchors, 
    int out_box_struct_number, 
    int out_class_number, 
    cudaStream_t& stream
);
    
void free_post_process_resources();

#endif /* VISION_SYSTEM_POST_PROCESS_H */