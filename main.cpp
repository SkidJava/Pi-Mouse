#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include "mouse.h"

Mouse* mouse;

void socket_thread() {
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // ソケットの作成
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return;
    }

    // サーバーアドレスの設定
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // バインド
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error binding");
        close(serverSocket);
        return;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    // メッセージの受信と送信
    MouseData receiveData;
    while (true) {
        ssize_t bytesRead = recvfrom(serverSocket, &receiveData, sizeof(MouseData), 0,
                                     (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesRead <= 0) {
            break;
        }

        // マウスを更新
        //std::cout << mouse->aimbotkey << std::endl;
        if (mouse->get_data().tip & (uint8_t)ClickType::FORWARD) {
            std::cout << "Received: " << mouse->get_data().tip << std::endl;
            mouse->move(receiveData);
        }

        // 受信したメッセージをクライアントに返信
        ssize_t bytesSent = sendto(serverSocket, &receiveData, sizeof(MouseData), 0,
                                    (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        if (bytesSent == -1) {
            perror("Error sending message");
            break;
        }
    }

    // ソケットのクローズ
    close(serverSocket);
}

int main() {
    const char *gadget_path = "/dev/hidg0";
    int gadget_fd = open(gadget_path, O_WRONLY);
    if (gadget_fd == -1) {
        std::cerr << "Error opening " << gadget_path << std::endl;
        return 1;
    }

    const char *input_path = "/dev/input/event0";
    int input_fd = open(input_path, O_RDONLY);
    if (input_fd == -1) {
        std::cerr << "Error opening " << input_path << std::endl;
        return 1;
    }

    mouse = new Mouse(gadget_fd, input_fd);

    std::thread thread1(socket_thread);

    while(true) {
        if (mouse->update()) {
            mouse->move(mouse->get_data());
        }
    }
    
    close(gadget_fd);
    close(input_fd);

    thread1.join();

    return 0;
}