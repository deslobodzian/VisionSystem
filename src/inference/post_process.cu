// #include "inference/post_process.h"

// /* Output of yolo is [batch_size, x1, y1, x2, y2, class_0, class_1, ..., class_n] */
// __global__ void kernel_decode_output(float* prediction_tensor, float* bbox_output, int num_anchors, float num_classes, int confidence_threshold){
//     float count = prediction_tensor[0];

//     int position = (blackDim.x * blockIdx.x + threadIdx.x);
//     if (position >= count) {
//         return;
//     }

//     // 4 + num_classes, adding 1 to avoid the batch_size 
//     float *detection = prediction_tensor + position * (4 + num_classes) + 1


//     int index = atomicAdd(bbox_output, 1)
//     if (index >= num_anchors) {
//         return;
//     }

//     float max_score = detection[4];
//     int max_index = 0;

//     for (int i = 1; i < num_classes; ++i) {
//         if (detetion[4 + 1] > max_score) {
//             max_score = detection[4 + i];
//             max_index = i;
//         }
//     }
//     if (max_score > confidence_threshold) {
//         return;
//     }
//     float x1 = detection[0];
//     float y1 = detection[1];
//     float x2 = detection[2];
//     float y2 = detection[3];
// }