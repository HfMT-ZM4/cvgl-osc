
#pragma once

#include <iostream>
#include <fstream>

#include <string>
#include <mutex>
#include <thread>
//#include "OdotBundle.hpp"
#include "MapOSC.hpp"


#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



class cvglUDPServer
{
public:
    cvglUDPServer();
    cvglUDPServer(int receivePort, int sendPort, std::string sendIP);
    
    ~cvglUDPServer();
    
    void init(int receivePort, int sendPort, std::string sendIP);
    void loop();
    void start();
    void close();
    
    void sendBundle( MapOSC & b );

    virtual void receivedBundle( MapOSC & b ) {}

private:
    
    void openSendSocket();
    void resizeSendBuffer();
    void openReceiveSocket();
    
    
    struct addrinfo m_send_addr, m_recv_addr;

    int m_fd = -1;
    int m_send_port = 7777;
    std::string m_send_ip_addr = "127.0.0.1";
    size_t m_max_buf_size = 0;
    
    int m_recv_fd = -1;
    int m_recv_port = 8888;
    
    bool m_closing = false;
    
    std::thread m_listenLoop;
    
    std::mutex m_mutex;
   // OdotBundle m_state_bundle;
    
    std::vector<char> m_overflowMessage;
    
    
};
