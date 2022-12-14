//
// Created by deslobodzian on 11/23/22.
//

#ifndef VISION_SYSTEM_VISION_RUNNER_HPP
#define VISION_SYSTEM_VISION_RUNNER_HPP

#include "utils/task.hpp"
#include "estimator/estimator.hpp"
#include "networking/state_estimate_pub.hpp"
#include "estimator/mcl_pose_estimator.hpp"

class VisionRunner : public Task {
public:
    VisionRunner(TaskManager*, double, const std::string&);
    using Task::Task;
    void init() override;
    void run() override;

    void initialize_state_estimator();
    virtual ~VisionRunner();

    ControlInput<float>* control_input_;
    std::vector<Measurement<float>>* measurements_;
private:
    StateEstimate<float> state_estimate_;
    EstimatorContainer<float>* state_estimator_;

    state_estimate_publishable state_est_pub_;

};

#endif //VISION_SYSTEM_VISION_RUNNER_HPP
