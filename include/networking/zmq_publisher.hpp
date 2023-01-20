//
// Created by robot on 1/16/23.
//

#ifndef VISION_SYSTEM_ZMQ_PUBLISHER_HPP
#define VISION_SYSTEM_ZMQ_PUBLISHER_HPP

#include <string>
#include <iostream>
#include <zmq.hpp>

struct publishable {
    virtual uint8_t* get_byte_array() {return nullptr;}
    virtual std::string get_topic() const = 0;
    virtual void encode(uint8_t* buffer) = 0;
    virtual size_t get_size() const = 0;
//    std::mutex publishable_mtx_;
};

class ZmqPublisher {
public:
    ZmqPublisher(const std::string& endpoint, publishable* p) {
        context_ = zmq::context_t(1);
        publisher_ = new zmq::socket_t(context_, zmq::socket_type::pub);
        publisher_->bind(endpoint);
        publishable_ = p;
    }

    ~ZmqPublisher() {
        publisher_->close();
        context_.close();
        delete publisher_;
    }

    void send() {
        zmq::message_t message(publishable_->get_topic());
        publisher_->send(message, ZMQ_SNDMORE);
        publisher_->send(publishable_->get_byte_array(), publishable_->get_size());
    }

private:
    zmq::context_t context_;
    zmq::socket_t* publisher_;
    publishable* publishable_;
};

#endif //VISION_SYSTEM_ZMQ_PUBLISHER_HPP
