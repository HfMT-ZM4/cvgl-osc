#include "cvglUDPServer.hpp"

using namespace std;

void cvglUDPServer::openSendSocket()
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sockfd = -1;
    char buf[INET_ADDRSTRLEN];
    
    
    if (m_fd >= 0)
    {
        cout << "udpsend: already connected" << endl;
        return;
    }
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // could be ipv6 also
    hints.ai_socktype = SOCK_DGRAM;
    
    char portStr[100];
    sprintf(portStr, "%d", m_send_port);
    
    int s = getaddrinfo(m_send_ip_addr.c_str(), portStr, &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return;
    }
    
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        void *addr;
        char *ipver;
        
        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (rp->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = (char *)"IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = (char *)"IPv6";
        }
        
        // convert the IP to a string and print it:
        inet_ntop(rp->ai_family, addr, buf, sizeof buf);
        printf(" Send %s: %s\n  Send Port: %s \n", ipver, buf, portStr);
        
        sockfd = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol );
        if ( sockfd == -1 )
            continue;
        
        if ( connect( sockfd, rp->ai_addr, rp->ai_addrlen ) != -1 )
            break;
        
        ::close(sockfd);
    }
    
    if ( rp == NULL )
    {
        cout << "Could not connect" << endl;
        return;
    }
    
    freeaddrinfo(result);
    m_fd = sockfd;
    
    cout << "connected! on socket " << m_fd << endl;

    
    MapOSC overflowMessage;
    
    overflowMessage["/error"].appendValue("send buffer exceeded max size");
    
    m_overflowMessage.resize( overflowMessage.getSerializedSizeInBytes() );
    
    overflowMessage.serializeIntoBuffer( m_overflowMessage.data(), m_overflowMessage.size() );
    
}



void cvglUDPServer::openReceiveSocket()
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sockfd = -1;
    char buf[INET_ADDRSTRLEN];
    
    
    if (m_recv_fd >= 0)
    {
        cout << "udpsend: already connected" << endl;
        return;
    }
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // could be ipv6 also
    hints.ai_socktype = SOCK_DGRAM;
    
    char portStr[100];
    sprintf(portStr, "%d", m_recv_port);
    
    int s = getaddrinfo("0.0.0.0", portStr, &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return;
    }
    
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        void *addr;
        char *ipver;
        
        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (rp->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = (char *)"IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = (char *)"IPv6";
        }
        
        // convert the IP to a string and print it:
        inet_ntop(rp->ai_family, addr, buf, sizeof buf);
        printf("  Receive %s: %s\n  Receive Port: %s \n", ipver, buf, portStr);
        
        sockfd = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol );
        if ( sockfd == -1 )
            continue;
        
        if ( ::bind(sockfd, rp->ai_addr, rp->ai_addrlen) != -1 )
            break;
        
        ::close(sockfd);
    }
    
    if ( rp == NULL )
    {
        cout << "Could not connect" << endl;
        return;
    }
    
    freeaddrinfo(result);
    m_recv_fd = sockfd;
    
    cout << "receiving on socket " << m_recv_fd << endl;
    
}

void cvglUDPServer::resizeSendBuffer()
{
    
    int res = 0 ;
    socklen_t optlen = sizeof(int);
    int val;
    
    getsockopt(m_fd, SOL_SOCKET, SO_SNDBUF , &val, &optlen);
    if(res == -1)
        printf("Error getsockopt one");
    else
        printf("send buffer size = %d\n", val);
    
    int new_size = 512000 ;
    setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF , &new_size, sizeof(new_size));
    getsockopt(m_fd, SOL_SOCKET, SO_SNDBUF , &val, &optlen);
    if(res == -1)
        printf("Error getsockopt one");
    else
        printf("send buffer size = %d\n", val);
    
    m_max_buf_size = val;
}


cvglUDPServer::cvglUDPServer(){}

void cvglUDPServer::init(int receivePort, int sendPort, std::string sendIP)
{
    m_recv_port = receivePort;
    m_send_port = sendPort;
    m_send_ip_addr = sendIP;
    
    openSendSocket();
    resizeSendBuffer();
    openReceiveSocket();

}

cvglUDPServer::cvglUDPServer(int receivePort, int sendPort, std::string sendIP) :
    m_recv_port(receivePort), m_send_port(sendPort), m_send_ip_addr(sendIP)
{

    openSendSocket();
    resizeSendBuffer();
    openReceiveSocket();

    start();
}

cvglUDPServer::~cvglUDPServer()
{
    cout << "closing socket " << m_fd << endl;
    
    cvglUDPServer::close();
    
    if( m_fd > -1 )
        ::close(m_fd);
    
    if( m_recv_fd > -1 )
        ::close(m_recv_fd);
    
    m_fd = -1;
    m_recv_fd = -1;
    
}

void cvglUDPServer::sendBundle( MapOSC & b )
{
    if( m_fd < 0 )
        return;
    
    size_t len = b.getSerializedSizeInBytes();
    if( len < 9 )
        return;
    
    //char buf[len];
    std::vector<char> buf(len);
    /*
    char *buf = NULL;
    buf = (char *)malloc(len);
    */
    b.serializeIntoBuffer(buf.data(), len);
    
    ssize_t sentbytes = send(m_fd, buf.data(), len, 0);
    if( sentbytes < 0 )
    {
        send(m_fd, m_overflowMessage.data(), m_overflowMessage.size(), 0);
//        cout << "failed to send " << len << " returned: " << sentbytes << " errno " << strerror(errno) << endl;
    }
       
   // free(buf);
    //buf = NULL;
    
    /*
    t_osc_bundle_s *serialized = b.getBundle();
        
    ssize_t sentbytes = send(m_fd, osc_bundle_s_getPtr(serialized), (size_t)osc_bundle_s_getLen(serialized), 0);
    if( sentbytes < 0 )
    {
      // cout << "failed to send " << serialized.getLen() << " returned: " << sentbytes << " errno " << strerror(errno) << endl;
    }
    
    osc_bundle_s_deepFree(serialized);
    */
}

void cvglUDPServer::start()
{
/*
    std::ifstream readFile;
    readFile.open ( "cvglThreadlog.txt" );
    
    std::string str;
    while (std::getline(readFile, str)) {
        // output the line
        std::cout << "reading : " << str << std::endl;
        pthread_cancel(str);
        // now we loop back and get the next line in 'str'
    }
    readFile.close();

    */
    m_listenLoop = std::thread(&cvglUDPServer::loop, this);
    cout << "started loop thread id " << m_listenLoop.native_handle() << endl;
    
/*
    std::ofstream writeFile;
    writeFile.open ( "cvglThreadlog.txt" );

    writeFile << m_listenLoop.native_handle() << "\n";
    writeFile.close();
  */
}


void cvglUDPServer::close()
{
    cout << "stoping " << endl;

    m_closing = true;
    
    if( m_listenLoop.joinable() )
        m_listenLoop.join();
    
    cout << "stopped " << endl;

    
}

void cvglUDPServer::loop()
{
    
    if( m_recv_fd < 0 )
    {
        printf("no open port!\n");
        return;
    }
    
   
    
   // struct sockaddr_in from;
  //  socklen_t fromlen = sizeof(from);
    
    char buf[m_max_buf_size];
    
    printf("starting loop \n");

    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    int selectN = m_recv_fd + 1;
    
    
    while( !m_closing && m_recv_fd > -1 )
    {
        
        FD_ZERO(&readfds);
        FD_SET(m_recv_fd, &readfds);
        
        if( select(selectN, &readfds, NULL, NULL, &timeout) )
        {
            /*
            if( FD_ISSET(m_recv_fd, &readfds) )
                cout << "set selected" << endl;
            */
            
            size_t read = recvfrom(m_recv_fd, buf, m_max_buf_size, 0, NULL, NULL);
           // cout << " read " << read << endl;
            
            if( read )
            {
/*
                for( size_t i = 0; i < read; ++i )
                {

                    printf("%c[%ld] ", (buf[i] == '\0' ? '_' : buf[i]), i);
                }

                cout << endl;
*/
                MapOSC o;
                o.inputOSC(read, buf);       
                receivedBundle(o);

             
            }
        }
        
    }
    
    printf("exited loop\n");

    
}
