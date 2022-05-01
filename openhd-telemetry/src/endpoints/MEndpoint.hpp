//
// Created by consti10 on 15.04.22.
//

#ifndef XMAVLINKSERVICE_MENDPOINT_H
#define XMAVLINKSERVICE_MENDPOINT_H

#include "../mav_include.h"
#include <iostream>
#include <sstream>
#include <chrono>

// Mavlink Endpoint
// A Mavlink endpoint hides away the underlying connection - e.g. UART, TCP, UDP.
// It has a (implementation-specific) method to send a message (sendMessage) and (implementation-specific)
// continuously forwards new incoming messages via a callback.
// It MUST also hide away any problems that could exist with this endpoint - e.g. a disconnecting UART.
// If (for example) in case of UART the connection is lost, it should just try to reconnect
// and as soon as the connection has been re-established, continue working as if nothing happened.
// This "send/receive data when possible, otherwise do nothing" behaviour fits well with the mavlink paradigm:
// https://mavlink.io/en/services/heartbeat.html
// "A component is considered to be connected to the network if its HEARTBEAT message is regularly received, and disconnected if a number of expected messages are not received."
class MEndpoint{
public:
    /**
     * The implementation-specific constructor SHOULD try and establish a connection as soon as possible
     * And re-establish the connection when disconnected.
     * @param tag a tag for debugging.
     */
    explicit MEndpoint(std::string tag):TAG(tag){};
    /**
     * send a message via this endpoint.
     * If the endpoint is silently disconnected, this MUST NOT FAIL/CRASH
     * @param message the message to send
     */
    virtual void sendMessage(const MavlinkMessage& message)=0;
    /**
     * register a callback that is called every time
     * this endpoint has received a new message
     * @param cb the callback function to register that is then called with a message every time a new full mavlink message has been parsed
     */
    void registerCallback(MAV_MSG_CALLBACK cb){
        if(callback!= nullptr){
            // this might be a common programming mistake - you can only register one callback here
            std::cerr<<"Overwriting already existing callback\n";
        }
        callback=std::move(cb);
    }
    /**
     * If (for some reason) you need to reason if this endpoint is alive, just check if it has received any mavlink messages
     * in the last X seconds
     */
     bool isAlive(){
        return (std::chrono::steady_clock::now()-lastMessage)<std::chrono::seconds(5);
     }
     /**
      * For debugging, print if this endpoint is alive (an endpoint is alive if it has received mavlink messages in the last X seconds).
      */
     void debugIfAlive(){
         std::stringstream ss;
         ss<<TAG<<" alive:"<<(isAlive() ? "Y":"N")<<"\n";
         std::cout<<ss.str();
     }
     // can be public since immutable
     const std::string TAG;
protected:
    MAV_MSG_CALLBACK callback=nullptr;
    // parse new data as it comes in, extract mavlink messages and forward them on the registered callback (if it has been registered)
    void parseNewData(const uint8_t* data, int data_len){
        //std::cout<<TAG<<"parseNewData\n";
        mavlink_message_t msg;
        for(int i=0;i<data_len;i++){
            uint8_t res = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)data[i], &msg, &receiveMavlinkStatus);
            if (res) {
                lastMessage=std::chrono::steady_clock::now();
                MavlinkMessage message{msg};
                //debugMavlinkMessage(message.m,TAG.c_str());
                if(callback!= nullptr){
                    callback(message);
                }else{
                    std::cerr<<"No callback set,did you forget to add it ?\n";
                }
            }
        }
    }
private:
    mavlink_status_t receiveMavlinkStatus{};
    std::chrono::steady_clock::time_point lastMessage{};
};

#endif //XMAVLINKSERVICE_MENDPOINT_H
