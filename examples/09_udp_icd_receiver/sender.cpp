// Example 09 - UDP sender.
//
// Emits 4 copies of the same 8-byte ICD record at 127.0.0.1:5555.
// Record: seq (2 bytes LE uint16) | temp_raw (2 bytes LE uint16) | pad (4 bytes).

#ifdef _WIN32
    #define NOMINMAX  // windows.h otherwise defines min/max macros that break std::min/max
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    static int close_socket(SOCKET s) { return closesocket(s); }
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <unistd.h>
    using SOCKET = int;
    static int close_socket(SOCKET s) { return ::close(s); }
#endif

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>

int main()
{
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port   = htons(5555);
    inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

    for (uint16_t seq = 1; seq <= 4; ++seq) {
        char buf[8] = {};
        std::memcpy(buf + 0, &seq, 2);
        uint16_t temp_raw = static_cast<uint16_t>(200 + seq * 5); // 205, 210, 215, 220
        std::memcpy(buf + 2, &temp_raw, 2);

        sendto(s, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&dest), sizeof(dest));
        std::cout << "sent seq=" << seq << " temp_raw=" << temp_raw << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    close_socket(s);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
