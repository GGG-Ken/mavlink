// Simple example receiving and sending MAVLink v2 over UDP
// based on POSIX APIs (e.g. Linux, BSD, macOS).

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <mavlink/common/mavlink.h>

void receive_some(int socket_fd, struct sockaddr_in* src_addr, socklen_t* src_addr_len, bool* src_addr_set);
void handle_heartbeat(const mavlink_message_t* message);

void send_some(int socket_fd, const struct sockaddr_in* src_addr, socklen_t src_addr_len);
void send_heartbeat(int socket_fd, const struct sockaddr_in* src_addr, socklen_t src_addr_len);

#if 0
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifdef _WIN32
#include <Windows.h>
#define close_socket(s) closesocket(s)
#else
#include <unistd.h>
#define close_socket(s) close(s)
#endif

const int PORT = 12345;
const char *SERVER_IP = "10.66.30.49"; // 如果其他设备想连接服务器，服务器的 IP 地址应该设置为其他设备可以访问的IP地址,不能选择本地回环地址
// const char *SERVER_IP = "127.0.0.1"; // IP地址是 127.0.0.1（本地回环地址）时，只有本地设备可以连接到服务器,同一台电脑自发自收

int main()
{
    // 创建 UDP 套接字
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0)
    {
        fprintf(stderr, "Failed to create socket\n");
        return 1;
    }

    // 绑定 IP 地址和端口号
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP); // 你想要绑定的特定 IP 地址 可以正常运行
    // serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 使用任意可用的IP地址 可以正常运行

    // 同一个电脑实现自发自收服务端和客户端也要相同的端口号
    serverAddr.sin_port = htons(PORT); // 端口号 12345

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        fprintf(stderr, "Bind failed\n");
        return 1;
    }

    printf("UDPserver SERVER_IP=%s PORT=%d start, waiting for udpclient...\n", SERVER_IP, PORT);

    // 设置接收超时
    // struct timeval tv;
    // tv.tv_sec = 100; // 设置超时时间为10秒 客户端接收超时设置（仅适用于Linux）
    // tv.tv_usec = 0;
    // setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0)
    {
        printf("setsockopt error: %s\n", strerror(errno));
        return -3;
    }
    
    while (1)
    {
#if 0

        // 接收消息
        char buffer[1024];
        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);

        // 如果没有消息到达，recvfrom函数将会阻塞程序，直到有消息到达为止。
        // int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrSize);
        // if (bytesReceived < 0)
        // {
        //     fprintf(stderr, "Receive failed\n");
        //     return 1;
        // }

        // buffer[bytesReceived] = '\0';
        bool src_addr_set = false;
        receive_some(serverSocket, &clientAddr, &clientAddrSize, &src_addr_set);
        send_some(serverSocket, &clientAddr, clientAddrSize);
        printf("Received message from %s: %s\n", inet_ntoa(clientAddr.sin_addr), buffer);

        // 发送响应
        const char *response = "Message received! from:server ken";
        sendto(serverSocket, response, strlen(response), 0, (struct sockaddr *)&clientAddr, clientAddrSize);
        usleep(1000 * 1000); // 延时 1 秒

#else
        struct sockaddr_in src_addr = {};
        socklen_t src_addr_len = sizeof(src_addr);
        bool src_addr_set = false;

        // For illustration purposes we don't bother with threads or async here
        // and just interleave receiving and sending.
        // This only works  if receive_some returns every now and then.
        receive_some(serverSocket, &src_addr, &src_addr_len, &src_addr_set);

        if (src_addr_set)
        {
            // send_some(socket_fd, &src_addr, src_addr_len);
            const char *response = "Message received!";
            int message_len = strlen(response);
            int rest = sendto(serverSocket, response, strlen(response), 0, (struct sockaddr *)&src_addr, src_addr_len);
            if (rest != message_len)
            {
                printf("sendto ret=%d message_len=%d error: %s --\n", rest, message_len, strerror(errno));
            }
        }

#endif
    }

    // 关闭套接字（永远不会执行到这里）
    close_socket(serverSocket);

    return 0;
}

#else
int main(int argc, char* argv[])
{
    // Open UDP socket
    const static int socket_fd = socket(PF_INET, SOCK_DGRAM, 0);

    if (socket_fd < 0) {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    // Bind to port
    struct sockaddr_in addr = {};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    int inet_pton_ret = inet_pton(AF_INET, "10.66.30.49", &(addr.sin_addr)); // listen on all network interfaces
    addr.sin_port = htons(12346); // default port on the ground

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    printf("Sending to IP: %s, Port: %d\n", ip, ntohs(addr.sin_port));

    printf("src_addr.sin_family = %d\n", addr.sin_family);
    printf("src_addr.sin_port = %d\n", ntohs(addr.sin_port));
    printf("src_addr.sin_addr.s_addr = %s\n", inet_ntoa(addr.sin_addr));

    // printf("inet_pton_ret: %d, Port: %d\n", inet_pton_ret, ntohs(addr.sin_port));

    if (bind(socket_fd, (struct sockaddr*)(&addr), sizeof(addr)) != 0) {
        printf("bind error: %s\n", strerror(errno));
        return -2;
    }

    // We set a timeout at 100ms to prevent being stuck in recvfrom for too
    // long and missing our chance to send some stuff.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        printf("setsockopt error: %s\n", strerror(errno));
        return -3;
    }


    while (true) {
        struct sockaddr_in src_addr = {};
        socklen_t src_addr_len = sizeof(src_addr);
        bool src_addr_set = true;

        // For illustration purposes we don't bother with threads or async here
        // and just interleave receiving and sending.
        // This only works  if receive_some returns every now and then.
        printf("src_addr_len1: %d\n", src_addr_len);
        // receive_some(socket_fd, &src_addr, &src_addr_len, &src_addr_set);

        if (src_addr_set) {
            // send_some(socket_fd, &src_addr, src_addr_len);
            const char *response = "Message received!";
            int message_len = strlen(response);
            printf("src_addr_len2: %d\n", src_addr_len);
            int rest = sendto(socket_fd, response, strlen(response), 0, (struct sockaddr *)&src_addr, src_addr_len);
            if (rest != message_len && errno != 0)
            {
                printf("sendto ret=%d message_len=%d error: %s -errno=%d-\n", rest, message_len, strerror(errno), errno);
           }
           printf("sendto ret=%d message_len=%d error: %s -errno=%d-\n", rest, message_len, strerror(errno), errno);
        }

        if (socket_fd < 0 )
        {
            printf("===ggg FILE= %-40s FUNC= %-15s LINE= %-4d ===\n",
                   __FILE__, __FUNCTION__, __LINE__);
            break;
        }
        sleep(1);
    }

    return 0;
}
#endif 
void receive_some(int socket_fd, struct sockaddr_in* src_addr, socklen_t* src_addr_len, bool* src_addr_set)
{
    // We just receive one UDP datagram and then return again.
    char buffer[2048]; // enough for MTU 1500 bytes

    const int ret = recvfrom(
            socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)(src_addr), src_addr_len);

    if (ret < 0) {
        // printf("recvfrom error: %s\n", strerror(errno));
    } else if (ret == 0) {
        // peer has done an orderly shutdown
        // return;
    }

    *src_addr_set = true;

    mavlink_message_t message;
    mavlink_status_t status;
    for (int i = 0; i < ret; ++i) {
        if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &message, &status) == 1) {

            // printf(
            //     "Received message %d from %d/%d\n",
            //     message.msgid, message.sysid, message.compid);

            switch (message.msgid) {
            case MAVLINK_MSG_ID_HEARTBEAT:
                handle_heartbeat(&message);
                break;
                default:
                printf("===ggg FILE= %-40s FUNC= %-15s LINE= %-4d ===\n", 
      __FILE__, __FUNCTION__, __LINE__ );
                break;
            }
        }
        printf("===ggg FILE= %-40s FUNC= %-15s LINE= %-4d ===ret=%d\n",
               __FILE__, __FUNCTION__, __LINE__, ret);
    }
    // printf("===ggg FILE= %-40s FUNC= %-15s LINE= %-4d ===ret=%d\n",
    //        __FILE__, __FUNCTION__, __LINE__, ret);
    return;
}

void handle_heartbeat(const mavlink_message_t* message)
{
    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(message, &heartbeat);

    printf("Got heartbeat from ");
    switch (heartbeat.autopilot) {
        case MAV_AUTOPILOT_GENERIC:
            printf("generic");
            break;
        case MAV_AUTOPILOT_ARDUPILOTMEGA:
            printf("ArduPilot");
            break;
        case MAV_AUTOPILOT_PX4:
            printf("PX4");
            break;
        default:
            printf("other");
            break;
    }
    printf(" autopilot\n");
}

void send_some(int socket_fd, const struct sockaddr_in* src_addr, socklen_t src_addr_len)
{
    // Whenever a second has passed, we send a heartbeat.
    static time_t last_time = 0;
    time_t current_time = time(NULL);
    if (current_time - last_time >= 1) {
        send_heartbeat(socket_fd, src_addr, src_addr_len);
        last_time = current_time;
    }
}

void send_heartbeat(int socket_fd, const struct sockaddr_in* src_addr, socklen_t src_addr_len)
{
    mavlink_message_t message;

    const uint8_t system_id = 42;
    const uint8_t base_mode = 0;
    const uint8_t custom_mode = 0;
    mavlink_msg_heartbeat_pack_chan(
        system_id,
        MAV_COMP_ID_PERIPHERAL,
        MAVLINK_COMM_0,
        &message,
        MAV_TYPE_GENERIC,
        MAV_AUTOPILOT_GENERIC,
        base_mode,
        custom_mode,
        MAV_STATE_STANDBY);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    const int len = mavlink_msg_to_send_buffer(buffer, &message);
    const char *messageken = "send:Hello, server! from:client ken ken ";
    int message_len = strlen(messageken);
    int ret = 0;
    ret = sendto(socket_fd, messageken, message_len, 0, (const struct sockaddr *)src_addr, src_addr_len);

    // int ret = sendto(socket_fd, buffer, len, 0, (const struct sockaddr*)src_addr, src_addr_len);
    if (ret != message_len)
    // if (ret != len)
    {
        // printf("sendto ret=%d message_len=%d error: %s --len=%d\n", ret, message_len, strerror(errno), len);
    }
    else
    {
        // printf("Sent heartbeat\n");
    }
}
