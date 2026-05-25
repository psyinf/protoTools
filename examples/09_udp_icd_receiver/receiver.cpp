// Example 09 - UDP receiver feeding a dissector + interpreter.
//
// Binds 127.0.0.1:5555, receives 4 datagrams, dissects each, interprets each.
// Demonstrates that protos' dissector/interpreter are transport-agnostic: UDP
// arrives as full datagrams via recvfrom(), which we hand straight to
// GenericDissector::addBytes().

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

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/interpretation/GenericPacketInterpreter.hpp>
#include <protos/interpretation/TypeFormatter.hpp>

#include <cstddef>
#include <cstring>
#include <iostream>
#include <span>
#include <vector>

using namespace protos::dissector;
using namespace protos::interpreter;
using namespace protos::value;

int main()
{
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in bind_addr{};
    bind_addr.sin_family      = AF_INET;
    bind_addr.sin_port        = htons(5555);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ::bind(s, reinterpret_cast<sockaddr*>(&bind_addr), sizeof(bind_addr));

    PacketDescriptor tmpl;
    tmpl.add({.name = "seq",      .size = 2});
    tmpl.add({.name = "temp_raw", .size = 2});
    tmpl.add({.name = "pad",      .size = 4});

    GenericPacketInterpreter interp;
    interp.addField({.name = "seq",      .type = Type::UNSIGNED_INTEGER, .format = "{}"});
    interp.addField({.name = "temp_raw", .type = Type::UNSIGNED_INTEGER,
                     .mapper = [](const Variant& v) -> InterpretationResults {
                         auto raw = std::get<uint64_t>(v);
                         return {{{"temp_c", Variant{static_cast<double>(raw) * 0.1}}, "{:.1f}", false}};
                     }});

    std::cout << "listening on 127.0.0.1:5555...\n";
    char buf[512];
    for (int i = 0; i < 4; ++i) {
        sockaddr_in from{}; socklen_t from_len = sizeof(from);
        auto n = recvfrom(s, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&from), &from_len);
        if (n <= 0) break;

        // Fresh dissector per datagram (each datagram is a whole packet).
        GenericDissector diss(tmpl);
        auto bytes = std::span<const std::byte>(reinterpret_cast<const std::byte*>(buf), static_cast<size_t>(n));
        auto packet = diss.addBytes(bytes);
        if (!packet) { std::cerr << "incomplete datagram\n"; continue; }

        auto results = interp.interpretPacketData(*packet);
        std::cout << "datagram " << (i + 1) << ":";
        for (const auto& r : results) {
            std::cout << "  " << r.name << "=" << variant_to_formatted_string(r.value, r.format);
        }
        std::cout << "\n";
    }

    close_socket(s);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
