/* 
在UDP通信中，客户端和服务端之间的交互模式可以根据应用需求而定。UDP是一种无连接的协议，这意味着每个数据包的发送都是独立的，发送方不会等待接收方的确认。
这种特性使得UDP非常适合那些对实时性要求高、可以容忍丢包的应用场景，如流媒体传输、在线游戏等。 
*/

/*服务端行为：
    接收后发送响应：服务端在接收到客户端的消息后，可以发送一个响应消息回客户端。这种模式适用于需要交互确认的场景。
    仅接收处理：服务端也可以仅仅接收客户端的消息并处理，而不发送任何响应。这种模式适用于数据收集或广播通知等场景。 

 */
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifdef _WIN32
#include <Windows.h>
#define close_socket(s) closesocket(s)
#else
#include <unistd.h>
#define close_socket(s) close(s)
#endif

#include <mavlink/common/mavlink.h>
#include "udp_socket.h"


void receive_some(int socket_fd, struct sockaddr_in *src_addr, socklen_t *src_addr_len, bool *src_addr_set);
void handle_heartbeat(const mavlink_message_t *message);

void send_some(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len);
void send_heartbeat(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len);


const int PORT = 12345;
const char *SERVER_IP = "10.66.30.49"; //如果其他设备想连接服务器，服务器的 IP 地址应该设置为其他设备可以访问的IP地址,不能选择本地回环地址 
// const char *SERVER_IP = "127.0.0.1"; // IP地址是 127.0.0.1（本地回环地址）时，只有本地设备可以连接到服务器,同一台电脑自发自收
int main() {
    // 创建 UDP 套接字
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // 绑定 IP 地址和端口号
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP); // 你想要绑定的特定 IP 地址 可以正常运行
  //serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 使用任意可用的IP地址 可以正常运行(sin_addr设置为INADDR_ANY表示可以和任何的主机通信)
  //在网络上面有着许多类型的机器,这些机器在表示数据的字节顺序是不同的, 比如i386芯片是低字节在内存地址的低端,高字节在高端,而alpha芯片却相反. 为了统一起来,在Linux下面,有专门的字节转换函数,其中htonl作用是将本机器上的long数据转化为网络上的long(h 代表host, l 代表long)
  
    //同一个电脑实现自发自收服务端和客户端也要相同的端口号
    serverAddr.sin_port = htons(PORT); // 端口号 12345

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    printf("UDPserver SERVER_IP=%s PORT=%d start,waitting for udpclient...\n", SERVER_IP, PORT);

    // 设置接收超时
    struct timeval tv;
    tv.tv_sec = 10; // 设置超时时间为10秒 客户端接收超时设置（仅适用于Linux）recv和recvfrom函数会阻塞，阻塞10s后返回错误码
    tv.tv_usec = 0;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    while (true) {
        // 接收消息
        char buffer[1024];
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        /*如果没有消息到达，recvfrom函数将会阻塞程序，直到有消息到达为止。具体来说，recvfrom函数会一直等待，直到有数据包到达服务器的UDP套接字。
        如果在等待过程中没有数据包到达，该函数将阻塞程序的执行，直到收到数据或发生错误为止。*/
        int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientAddrSize);
        if (bytesReceived < 0) {
            std::cerr << "Receive failed\n";
            return 1;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Received message from " << inet_ntoa(clientAddr.sin_addr) << ": " << buffer << std::endl;

        // 发送响应
        std::string response = "Message received! from:server";
        bool src_addr_set = true;
        receive_some(serverSocket, &clientAddr, &clientAddrSize, &src_addr_set);

        usleep(1000*1000); // 延时 1 秒
    }

    // 关闭套接字（永远不会执行到这里）
    close_socket(serverSocket);

    return 0;
}
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

            // printf(
            //     "Received message %d from %d/%d\n",
            //     message.msgid, message.sysid, message.compid);

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
        // printf("===ggg FILE= %-40s FUNC= %-15s LINE= %-4d ===ret=%d\n",
        //        __FILE__, __FUNCTION__, __LINE__, ret);
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
    

    // if (/* condition to send MAV_CMD_IMAGE_START_CAPTURE */)
    {
        // send_image_start_capture(socket_fd, src_addr, src_addr_len);
    }
    // else if (/* condition to send MAV_CMD_IMAGE_STOP_CAPTURE */)
    // {
    //     send_image_stop_capture(socket_fd, src_addr, src_addr_len);
    // }
    // else if (/* condition to send MAV_CMD_VIDEO_START_CAPTURE */)
    // {
    //     send_video_start_capture(socket_fd, src_addr, src_addr_len);
    // }
    // else if (/* condition to send MAV_CMD_VIDEO_STOP_CAPTURE */)
    // {
    //     send_video_stop_capture(socket_fd, src_addr, src_addr_len);
    // }
    // else if (/* condition to send MAV_CMD_SET_CAMERA_MODE */)
    // {
    //     send_set_camera_mode(socket_fd, src_addr, src_addr_len);
    // }
    // else if (/* condition to send MAV_CMD_SET_CAMERA_ZOOM */)
    // {
    //     send_set_camera_zoom(socket_fd, src_addr, src_addr_len);
    // }
    // else if (/* condition to send MAV_CMD_SET_CAMERA_FOCUS */)
    // {
    //     send_set_camera_focus(socket_fd, src_addr, src_addr_len);
    // }
    }
}
// void send_image_start_capture(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len)
// {
//     // Create MAVLink message for MAV_CMD_IMAGE_START_CAPTURE
//     const uint8_t system_id = 42;
//     // const uint8_t base_mode = 0;
//     // const uint8_t custom_mode = 0;
//     mavlink_message_t message;
//     mavlink_msg_command_long_pack_chan(
//         system_id,
//         MAV_COMP_ID_PERIPHERAL,
//         MAVLINK_COMM_0,
//         &message,
//         MAV_CMD_IMAGE_START_CAPTURE,
//         0, // param1
//         0, // param2
//         0, // param3
//         0 // param4
//     );

//     // Send the message
//     uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
//     const int len = mavlink_msg_to_send_buffer(buffer, &message);
//     int ret = sendto(socket_fd, buffer, len, 0, (const struct sockaddr *)src_addr, src_addr_len);

//     // Handle errors
// }
void send_video_start_capture(int socket_fd, const struct sockaddr_in *src_addr, socklen_t src_addr_len)
{
    mavlink_message_t message;

    // Set parameters for the command
    uint8_t target_system = 1;
    uint8_t target_component = 1;
    uint32_t duration = 0;  // Duration in seconds, 0 means continuous capture
    uint8_t frequency = 30; // Frame rate in Hz
    const uint8_t system_id = 42;

    // Pack the command
    mavlink_msg_command_long_pack(
        system_id, MAV_COMP_ID_CAMERA, &message,
        target_system, target_component,
        MAV_CMD_VIDEO_START_CAPTURE,
        0, duration, frequency, 0, 0, 0, 0, 0);

    // Send the packed command
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buffer, &message);
    int ret = sendto(socket_fd, buffer, len, 0, (const struct sockaddr *)src_addr, src_addr_len);

    if (ret != len)
    {
        printf("sendto failed for VIDEO_START_CAPTURE: %s\n", strerror(errno));
    }
    else
    {
        printf("Sent VIDEO_START_CAPTURE\n");
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

    ret = sendto(socket_fd, buffer, len, 0, (const struct sockaddr *)src_addr, src_addr_len);
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