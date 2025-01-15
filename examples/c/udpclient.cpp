/* 
在UDP通信中，客户端和服务端之间的交互模式可以根据应用需求而定。UDP是一种无连接的协议，这意味着每个数据包的发送都是独立的，发送方不会等待接收方的确认。
这种特性使得UDP非常适合那些对实时性要求高、可以容忍丢包的应用场景，如流媒体传输、在线游戏等。
*/

/* 
客户端行为：
    发送后等待响应：在某些应用中，客户端发送消息后可能会等待服务端的响应。这种模式类似于请求-响应模型，客户端在发送请求后暂停执行，直到收到服务端的响应或超时。
    发送不等待：在其他应用中，客户端可能会连续发送多个消息，而不等待服务端的响应。这种模式适用于那些不需要即时反馈的场景。 
*/

#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <chrono>

// examples/c/mavlinkdemo/mavlink/checksum.h
#include "mavlinkdemo/mavlink/common/mavlink.h"

#include "udp_socket.h"
// /home/quan/share/mygithub/mavlink/examples/c/mavlinkdemo/mavlink_main.h
#include "mavlinkdemo/mavlink_main.h"
#ifdef _WIN32
#include <Windows.h>
#define close_socket(s) closesocket(s)
#else
#include <unistd.h>
#define close_socket(s) close(s)
#endif

    void
    receive_some(int socket_fd, struct sockaddr_in *src_addr, socklen_t *src_addr_len, bool *src_addr_set);
void handle_heartbeat(const mavlink_message_t *message);

void send_some(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len);
void send_heartbeat(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len);
void send_image_start_capture(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len);
const int PORT = 12345;
const char *SERVER_IP = "10.66.30.49";
// const char *SERVER_IP = "127.0.0.1";

int main() {
    // 创建 UDP 套接字
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Failed to create socket\n";
        printf("");
        return 1;
    }

    // 服务器地址和端口号
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP); // 服务器 IP 地址
    serverAddr.sin_port = htons(PORT); // 端口号 12345

    // 设置接收超时
    struct timeval tv;
    tv.tv_sec = 10; // 设置超时时间为10秒 客户端接收超时设置（仅适用于Linux）recv和recvfrom函数会阻塞，阻塞10s后返回错误码
    tv.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    printf("Udpclinet Connected to  SERVER_IP %s PORT%d tv.tv_sec= %d.\n", SERVER_IP, PORT, tv.tv_sec);

    while (true) {
        // 发送消息
        std::string message = "111111";
        // std::string message = "send:Hello, server! from:client";
        sendto(clientSocket, message.c_str(), message.length(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
        printf("%s\n", message.c_str());

        // 接收服务器的响应 可不接收
        char buffer[1024];
        sockaddr_in fromAddr;
        socklen_t fromAddrLen = sizeof(fromAddr);
        /* 这里会阻塞，同样要等待服务端的消息 */
        int bytesReceived = 0;
        // bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (sockaddr *)&fromAddr, &fromAddrLen);
        if (bytesReceived < 0)
        {
            // 超5s时会在这打印，recvfrom返回错误
            /* std::cerr默认不带缓冲，这意味着输出到 std::cerr 的数据会立即写入标准错误，无需等待缓冲区满。这对于报告错误很重要，因为它确保了即使在程序崩溃或异常终止时，错误信息也能被立即输出。 */
            std::cerr << "Receive failed ggg\n";
            //errno 是一个C标准库中定义的全局变量，通常用于表示发生错误的原因。在C和C++标准库中的很多函数在执行出错时会修改 errno 的值，以指示出错的原因。errno 的类型是 int，它可以取代表不同错误的整数值。标准规定了一些预定义的错误码，例如 EAGAIN、EBADF、ENOMEM 等。
            std::cerr << "Error in recvfrom: " << strerror(errno) << std::endl;
            printf("\n");
            //perror会在标准错误流中输出形如 "Error in recvfrom: 错误信息" 的错误消息
            perror("Error in recvfrom");
            close_socket(clientSocket);
            return -1;
        }

        // 打印接收到的响应
        buffer[bytesReceived] = '\0';
        std::cout << "Received response from ip: " << inet_ntoa(fromAddr.sin_addr)<< ":"  << buffer << std::endl;
        // 在发送消息之后添加一段延时，以免发送消息过快

    /**
     */
        printf("\n \n=======================");
        // struct sockaddr_in src_addr = {};
        // socklen_t src_addr_len = sizeof(src_addr);
        bool src_addr_set = true;

        printf("src_addr_len1: %d\n", sizeof(serverAddr));
        // receive_some(socket_fd, &src_addr, &src_addr_len, &src_addr_set);
        if (src_addr_set)
        {
            send_some(clientSocket, &serverAddr, sizeof(serverAddr));
            const char *response = "22222";
            int message_len = strlen(response);
            printf("src_addr_len2: %d\n", sizeof(serverAddr));
            int rest = message_len;

            // int rest = sendto(clientSocket, response, strlen(response), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
            if (rest != message_len && errno != 0)
            {
                printf("sendto rest=%d message_len=%d error: %s errno=%d--\n", rest, message_len, strerror(errno), errno);
            }else{
                printf("sendto rest=%d message_len=%d success: %s --errno=%d\n", rest, message_len, strerror(errno), errno);
            }

        }

        if (clientSocket < 0)
        {
            printf("===ggg FILE= %-40s FUNC= %-15s LINE= %-4d ===\n",
                   __FILE__, __FUNCTION__, __LINE__);
            break;
        }
        printf("=======================\n \n");
        mavlink_init();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 延时 1 秒
    }

    close_socket(clientSocket);

    return 0;
}
//
void receive_some(int socket_fd, struct sockaddr_in *src_addr, socklen_t *src_addr_len, bool *src_addr_set)
{
    // We just receive one UDP datagram and then return again.
    char buffer[2048]; // enough for MTU 1500 bytes

    const int ret = recvfrom(
        socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)(src_addr), src_addr_len);

    if (ret < 0)
    {
        // printf("recvfrom error: %s\n", strerror(errno));
    }
    else if (ret == 0)
    {
        // peer has done an orderly shutdown
        // return;
    }

    *src_addr_set = true;

    mavlink_message_t message;
    mavlink_status_t status;
    for (int i = 0; i < ret; ++i)
    {
        if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &message, &status) == 1)
        {

            printf(
                "Received message message.msgid=%d from message.sysid=%d/message.compid=%d  ret=$d\n",
                message.msgid, message.sysid, message.compid, ret);
            printf("===ggg FILE= %-40s FUNC= %-15s LINE= %-4d ===ret=%d\n",
                   __FILE__, __FUNCTION__, __LINE__, ret);
            switch (message.msgid)
            {
            case MAVLINK_MSG_ID_HEARTBEAT:
                handle_heartbeat(&message);
                break;
            default:
                printf("===ggg FILE= %-40s FUNC= %-15s LINE= %-4d ===\n",
                       __FILE__, __FUNCTION__, __LINE__);
                break;
            }
        }
    }
    // printf("===ggg FILE= %-40s FUNC= %-15s LINE= %-4d ===ret=%d\n",
    //        __FILE__, __FUNCTION__, __LINE__, ret);
    return;
}

void handle_heartbeat(const mavlink_message_t *message)
{
    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(message, &heartbeat);

    printf("Got heartbeat from ");
    switch (heartbeat.autopilot)
    {
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

void send_some(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len)
{
    // Whenever a second has passed, we send a heartbeat.
    static time_t last_time = 0;
    time_t current_time = time(NULL);
    if (current_time - last_time >= 1)
    {
        send_heartbeat(socket_fd, src_addr, src_addr_len);
        last_time = current_time;
    }
}

void send_heartbeat(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len)
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
    // ret = sendto(socket_fd, messageken, message_len, 0, (const struct sockaddr *)src_addr, src_addr_len);

    ret = sendto(socket_fd, buffer, len, 0, (const struct sockaddr*)src_addr, src_addr_len);
    if (ret != message_len)
    // if (ret != len)
    {
        printf("sendto ret=%d message_len=%d error: %s --len=%d\n", ret, message_len, strerror(errno), len);
    }
    else
    {
        printf("Sent heartbeat\n");
    }
}

void send_image_start_capture(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len)
{
    mavlink_message_t message;

    // Set parameters for the command
    const uint8_t system_id = 42;
    uint8_t component_id = MAV_COMP_ID_CAMERA;

    uint8_t target_system = 1;
    uint8_t target_component = 1;
    uint32_t interval = 1000;  // Capture interval in ms
    uint32_t total_images = 0; // 0 means continuous capture
    uint8_t seq_number = 0;    // Start sequence number

    // Pack the command
    mavlink_msg_command_long_pack(
        system_id, component_id, &message,
        target_system, target_component,
        MAV_CMD_IMAGE_START_CAPTURE,
        0, interval, total_images, seq_number, 0, 0, 0, 0);

    // Send the packed command
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buffer, &message);
    int ret = sendto(socket_fd, buffer, len, 0, (const struct sockaddr *)src_addr, src_addr_len);

    if (ret != len)
    {
        printf("sendto failed for IMAGE_START_CAPTURE: %s\n", strerror(errno));
    }
    else
    {
        printf("Sent IMAGE_START_CAPTURE\n");
    }
}
