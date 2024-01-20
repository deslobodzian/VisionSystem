#ifdef WITH_CUDA
#ifndef VISION_SYSTEM_TENSOR_RT_ENGINE
#define VISION_SYSTEM_TENSOR_RT_ENGINE

#include <NvInfer.h>
#include <NvOnnxParser.h>
#include <string>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <vector>
#include "i_inference_engine.hpp"
#include "int8_entropy_calibrator.hpp"
#include "tensor.hpp"
#include "trt_logger.h"
#include "cuda_utils.h"

inline std::vector<std::string> split_str(std::string s, std::string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

inline bool readFile(std::string filename, std::vector<uint8_t> &file_content) {
    // open the file:
    std::ifstream instream(filename, std::ios::in | std::ios::binary);
    if (!instream.is_open()) return true;
    file_content = std::vector<uint8_t>((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    return false;
}

inline std::string remove_file_extension(const std::string& file) {
    size_t last_period = file.find_last_of('.');
    if (last_period != std::string::npos) {
        return file.substr(0, last_period);
    }
    return file;
}

enum class ModelPrecision {
    INT_8,
    FP_16,
    FP_32
};

struct EngineConfig {
    std::string onnx_path;
    std::string engine_path;
    ModelPrecision presicion;
    std::string int8_data_path;

    int max_threads;
    int optimization_level;
};

struct OptimDim {
    nvinfer1::Dims4 size;
    std::string tensor_name;

    bool setFromString(std::string &arg) {
        std::vector<std::string> v_ = split_str(arg, ":");
        if (v_.size() != 2) return true;

        std::string dims_str = v_.back();
        std::vector<std::string> v = split_str(dims_str, "x");

        size.nbDims = 4;
        // assuming batch is 1 and channel is 3
        size.d[0] = 1;
        size.d[1] = 3;

        if (v.size() == 2) {
            size.d[2] = stoi(v[0]);
            size.d[3] = stoi(v[1]);
        } else if (v.size() == 3) {
            size.d[2] = stoi(v[1]);
            size.d[3] = stoi(v[2]);
        } else if (v.size() == 4) {
            size.d[2] = stoi(v[2]);
            size.d[3] = stoi(v[3]);
        } else return true;

        if (size.d[2] != size.d[3]) std::cerr << "Warning only squared input are currently supported" << std::endl;

        tensor_name = v_.front();
        return false;
    }
};


class TensorRTEngine: public IInferenceEngine {
public:
    TensorRTEngine();
    ~TensorRTEngine();

    void set_execution_data(void* execution_data) override;
    static int build_engine(const EngineConfig& cfg, OptimDim dyn_dim_profile);
    void load_model(const std::string& model_path) override;
    void run_inference() override;

    // cuda graph testing
    void init_cuda_graph();
    void execute_cuda_graph();

    const Shape get_input_shape() const override;
    const Shape get_output_shape() const override;
    Tensor<float>& get_input_tensor() override; 
    Tensor<float>& get_output_tensor() override; 

private:
    nvinfer1::IRuntime* runtime_;
    nvinfer1::ICudaEngine* engine_;
    nvinfer1::IExecutionContext* context_;
    //std::unique_ptr<Int8EntropyCalibrator2> calibrator_ = nullptr;
 
    std::string input_name_;
    std::string output_name_;
    Shape input_shape_;
    Shape output_shape_;

    cudaStream_t stream_;
    cudaGraph_t graph_;
    cudaGraphExec_t instance_;
    bool graph_initialized_= false;

    Tensor<float> input_;
    Tensor<float> output_;
};

#endif /* VISION_SYSTEM_ONNXRUNTIME_INFERENCE_ENGINE_HPP */
#endif