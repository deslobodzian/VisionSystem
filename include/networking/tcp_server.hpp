//
// Created by DSlobodzian on 2/17/2022.
//

#pragma once

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "utils.hpp"
#include <thread>

#define PORT 27002
#define SA struct sockaddr
#define BUFFER_SIZE 1024

struct output_frame {
    long millis;
    double est_x;
    double est_y;
    double est_heading;
    bool has_target;
    double goal_distance;
    double goal_angle;
    output_frame() {
        millis = 0;
        est_x = 0;
        est_y = 0;
        est_heading = 0;
        has_target = false;
        goal_distance = 0;
        goal_angle = 0;
    }
    output_frame(long m, double x, double y, double heading, bool target, double goal_dist, double goal_ang) {
        millis = m;
        est_x = x;
        est_y = y;
        est_heading = heading;
        has_target = target;
        goal_distance = goal_dist;
        goal_angle = goal_ang;
    }
    std::string to_udp_string() {
        std::string value = std::to_string(millis) + ";" +
                            std::to_string(est_x) + ";" +
                            std::to_string(est_y) + ";" +
                            std::to_string(est_heading) +  ";" +
                            std::to_string(has_target) +  ";" +
                            std::to_string(goal_distance) +  ";" +
                            std::to_string(goal_angle);
        return value;
    }
};

struct input_frame{
    int id;
    long millis;
    double u[3]; // odometry [dx, dy, dTheta]
    double init_pose[3]; // initial position [x, y, theta]
    input_frame() {
        id = -1;
        millis = 0;
        u[0] = 0;
        u[1] = 0;
        u[2] = 0;
        init_pose[0] = 0;
        init_pose[1] = 0;
        init_pose[2] = 0;
    };
    input_frame(std::vector<std::string> values) {
        if (atof(values.at(0).c_str()) == 0) {
            id = 0;
            init_pose[0] = atof(values.at(1).c_str());
            init_pose[1] = atof(values.at(2).c_str());
            init_pose[2] = atof(values.at(3).c_str());
        } else {
            id = 1;
            millis = atof(values.at(1).c_str());
            u[0] = atof(values.at(2).c_str());
            u[1] = atof(values.at(3).c_str());
            u[2] = atof(values.at(4).c_str());
        }
    }
};
class TCPServer {

private:
    int sockfd_;
    int connfd_;
    socklen_t len_;
    struct sockaddr_in server_address_;
    struct sockaddr_in client_address_;
    char buf[BUFFER_SIZE];
    char receive_buf[BUFFER_SIZE];

    bool has_client_ = false;
    bool has_init_pose_ = false;
    bool real_data_started_ = false;
    input_frame latest_frame_;
    input_frame prev_frame_;
    input_frame init_pose_;
    output_frame data_frame_;

    std::thread data_thread_;


public:
    TCPServer() {
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ == -1) {
            error("socket creation failed");
        } else {
            info("Socket created");
        }
        bzero(&server_address_, sizeof(server_address_));
        server_address_.sin_family = AF_INET;
        server_address_.sin_addr.s_addr = htonl(INADDR_ANY);
        server_address_.sin_port = htons(PORT);

        if ((bind(sockfd_, (SA*)&server_address_, sizeof(server_address_))) != 0) {
            error("Bind failed");
        } else {
            info("Bind success");
        }

        if (listen(sockfd_, 3) > 0) {
            error("Listening failed");
        } else {
            info("Listening for client");
        }

        len_ = sizeof(client_address_);
    }

    void listening_for_client() {
        while (!has_client_) {
            connfd_ = accept(sockfd_, (SA*) &client_address_, &len_);
            if (connfd_ < 0) {
                info("Waiting for connection\r");
            } else {
                info("Connection found!");
                has_client_ = true;
            }
        }
    }

    std::vector<std::string> split( const std::string& str, char delimiter = ';' ) {
        std::vector<std::string> result ;
        std::istringstream stm(str) ;
        std::string fragment ;
        while( std::getline( stm, fragment, delimiter ) ) result.push_back(fragment) ;
        return result ;
    }

    int receive() {
//        error("receiving");
        bzero(receive_buf, BUFFER_SIZE);
        return recv(connfd_, receive_buf, sizeof(receive_buf), 0);
    }

    input_frame get_new_frame() {
        std::string s(receive_buf, sizeof(receive_buf));
        std::vector<std::string> values = split(s);
        if (atof(values.at(0).c_str()) == 0) {
            init_pose_ = input_frame(values);
            has_init_pose_ = true;
            return input_frame();
        } else {
            real_data_started_ = true;
            return input_frame(values);
        }
    }
    int send_msg(std::string msg) {
//        error("sending");
        bzero(buf, BUFFER_SIZE);
        msg.copy(buf, BUFFER_SIZE);
        return send(connfd_, buf, sizeof(buf), 0);
    }

    int send_frame(output_frame &frame) {
        return send_msg(frame.to_udp_string());
    }
    input_frame get_latest_frame() {
        return latest_frame_;
    }
    input_frame get_init_pose_frame() {
        return init_pose_;
    }

    bool received_init_pose() {
        return has_init_pose_;
    }

    bool real_data_started() {
        return real_data_started_;
    }

    void set_data_frame(output_frame &frame) {
        data_frame_ = frame;
    }

    void data_thread() {
        listening_for_client();
        while (true) {
            if (receive() < 0 && send_frame(data_frame_) < 0) {
                error("Lost Client, Looking for new connection");
                has_client_ = false;
                listening_for_client();
            }
            receive();
            send_frame(data_frame_);
        }
    }
    void start_thread() {
        data_thread_ = std::thread(&TCPServer::data_thread, this);
    }


};

