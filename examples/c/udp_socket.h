
//封装一个udpsocket类
//通过实例化对象来调用成员函数完成客户端服务器搭建
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>

using namespace std;

class UdpSocket {
    private:
        int _sockfd;
    public:
        UdpSocket():_sockfd(-1) {}
        ~UdpSocket()
        {
            if(_sockfd != -1)
            {
                close(_sockfd);
            }
        }
        bool Socket()
        {
            _sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
            if(_sockfd < 0)
            {
                perror("socket error");
                return false;
            }
            return true;
        }
        bool Bind(const string &ip,uint16_t port)
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
            socklen_t len = sizeof(struct sockaddr_in);
            int ret;
            ret = bind(_sockfd,(struct sockaddr*)&addr,len);
            if(ret < 0)
            {
                perror("bind error");
                return false;
            }
            return true;
        }
        bool Send(uint8_t *data, uint32_t datalen, const string &ip, uint16_t port)
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
            socklen_t len = sizeof(struct sockaddr_in);
            int ret;
            ret = sendto(_sockfd, data, datalen, 0, (struct sockaddr*) &addr, len);
            if(ret < 0)
            {
                perror("sendto error");
                return false;
            }
            return true;
        }
        uint32_t Recv(uint8_t *data, uint32_t datalen, string *ip = NULL, uint16_t *port = NULL)
        {
            struct sockaddr_in addr;
            socklen_t len = sizeof(struct sockaddr_in);
            int ret;
            ret = recvfrom(_sockfd, data, datalen, 0, (struct sockaddr *)&addr, &len);
            if(ret < 0)
            {
                perror("recvfrom error");
                return 0;
            }
            //printf("recvfrom ok\n");
            if(ip != NULL)
            {
                *ip = inet_ntoa(addr.sin_addr);
            }
            if(port != NULL)
            {
                *port = ntohs(addr.sin_port);
            }
            return ret;
        }
        bool Close()
        {
            if(_sockfd != -1)
            {
                close(_sockfd);
            }
            return true;
        }
        int GetFd()
        {
            return _sockfd;
        }
};


