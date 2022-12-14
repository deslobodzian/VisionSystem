//
// Created by ubuntuvm on 11/24/22.
//

#ifndef VISION_SYSTEM_MCL_POSE_ESTIMATOR_HPP
#define VISION_SYSTEM_MCL_POSE_ESTIMATOR_HPP

#define DT 0.05
#define ALPHA_ROTATION 0.0002
#define ALPHA_TRANSLATION 2
#define NUM_PARTICLES 200
#define RESAMPLE_PARTICLES 100

#include <vector>
#include <iostream>
#include <random>
#include <cmath>
#include <Eigen/Dense>
#include "map.hpp"
#include "utils/utils.hpp"
#include "estimator.hpp"
#include "utils/probability_utils.hpp"

using namespace Eigen;

template <typename T>
struct Particle {
    Eigen::Vector3<T> x;
    T weight;
};

template <typename T>
class MCLPoseEstimator : public Estimator<T>{
public:
    MCLPoseEstimator() = default;
    explicit MCLPoseEstimator(const std::vector<Landmark> &map);

    void run() override;
    void setup() override;

    std::vector<Measurement<T>> measurement_model(const Eigen::Vector3<T> &x);

    T sample_measurement_model(
            const Measurement<T> &measurement,
            const Eigen::Vector3<T> &x,
            const Landmark &landmark);

    Eigen::Vector3<T> sample_motion_model(
            ControlInput<T>* u,
            const Eigen::Vector3<T> &x);

    T calculate_weight(const std::vector<Measurement<T>> &z,
                            const Eigen::Vector3<T> &x,
                            T weight,
                            const std::vector<Landmark> &map);

    std::vector<Particle<T>> monte_carlo_localization(
            ControlInput<T>* u,
            std::vector<Measurement<T>>* z);

    std::vector<Particle<T>> low_variance_sampler(const std::vector<Particle<T>> &X);

    std::vector<Particle<T>> get_particle_set();
    Eigen::Vector3<T> get_estimated_pose();


private:
    Eigen::Vector3<T> x_est_;

    std::vector<Particle<T>> X_; // particle set for filter
    std::vector<Landmark> map_;
};
#endif //VISION_SYSTEM_MCL_POSE_ESTIMATOR_HPP
