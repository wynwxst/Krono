
#include "misc/base.hpp"
#ifndef SERVER
#define SERVER
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif
std::shared_ptr<std::vector<int>> _client_sockets = std::make_shared<std::vector<int>>();
std::shared_ptr<std::string> _specific_update = std::make_shared<std::string>();
std::shared_ptr<std::vector<int>> _pending_updates = std::make_shared<std::vector<int>>();;
class TCPServer {
public:
    //std::mutex clients_mutex;
    std::shared_ptr<std::vector<int>> client_sockets;
    std::shared_ptr<std::vector<int>> pending_updates;
    std::shared_ptr<std::string> specific_update;

    TCPServer(int port) : port(port), server_fd(-1), client_fd(-1) {
        client_sockets = _client_sockets;
        pending_updates = _pending_updates;
        specific_update = _specific_update;
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
            throw std::runtime_error("WSAStartup failed");
#endif
    }

    ~TCPServer() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }
    void add_map(std::string map, void* to){ mappings[map] = to; }

    bool sigstop = false;
    std::unordered_map<std::string, void*> mappings;

    void start() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) throw std::runtime_error("Socket creation failed");

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        int opt = 1;
#ifdef _WIN32
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
            throw std::runtime_error("Bind failed");

        if (listen(server_fd, 3) < 0)
            throw std::runtime_error("Listen failed");

        std::cout << "Server listening on port " << port << "...\n";

        while (!sigstop) {
            sockaddr_in client_addr{};
            socklen_t addrlen = sizeof(client_addr);
            int sock = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
            if (sock < 0) continue;

            {
                client_sockets->push_back(sock);
            }

            if (client_fd == -1) {

                client_fd = sock;
                std::thread(&TCPServer::handle_client, this, sock, true).detach();
            } else {

                std::thread(&TCPServer::handle_client, this, sock, false).detach();
            }

            std::cout << "Client connected.\n";
        }
    }


    std::string _receive(uint64_t size = 1024, int sock = -1, bool first_client = true) {
        if (first_client && sock == -1) sock = client_fd;

        char* buffer = (char*)malloc(size);
#ifdef _WIN32
        int valread = recv(sock, buffer, size, 0);
#else
        int valread = read(sock, buffer, size);
#endif

        std::string result = std::string(buffer, valread > 0 ? valread : 0);
        if (result == "stop") sigstop = true;
        

        free(buffer);
        return result;
    }

    std::string receive(int sock = -1, bool first_client = true) {
        std::string len_str = _receive(12, sock, first_client);
        // << "recv: " << len_str << std::endl;

        std::string int_str = "";
        uint64_t len = 1024;
        if (len_str.substr(0, 4) == "len:") {
            for (int i = 4; i < len_str.size(); i++) {
                if (!isdigit(len_str[i])) break;
                int_str += len_str[i];
            }
            if (!int_str.empty()) {
                len = std::strtoull(int_str.c_str(), nullptr, 10);
            }
            sendMessage("ok", sock, first_client);
            return _receive(len, sock, first_client);
        } else {
            return "invalid format";
        }
    }

    void sendMessage(const std::string& msg, int sock = -1, bool first_client = true) {
        if (first_client && sock == -1) sock = client_fd;

        std::string padding = (std::to_string(msg.size()).size() < 8
                                ? std::string(8 - std::to_string(msg.size()).size(), '0')
                                : "");
        std::string len_str = "len:" + padding + std::to_string(msg.size());

#ifdef _WIN32
        send(sock, len_str.c_str(), (int)len_str.size(), 0);
        send(sock, msg.c_str(), (int)msg.size(), 0);
#else
        write(sock, len_str.c_str(), (int)len_str.size());
        write(sock, msg.c_str(), (int)msg.size());
#endif
    }

    void sendToAll(const std::string& msg) {
            for (int sock : *client_sockets) {
                sendMessage(msg,sock,false);
            }
    }

    void BroadcastUpdate(){
        pending_updates->clear();
        *specific_update = "";
        for (int sock : *client_sockets) {
            if (sock != -1){
                //sendMessage("update",sock,false);
                if (inT(sock,*pending_updates) == false){
                    pending_updates->push_back(sock);
                }
                
            }
        }
    }


    void BroadcastSpecificUpdate(std::string type){
        //pending_updates.clear();
        *specific_update = type;
    }

    void RemovePending(int cfd){
        for (int i = 0; i < pending_updates->size();i++){
            if (pending_updates->at(i) == cfd){
                pending_updates->at(i) = 0;
                break;
            }
        }
    }


    void removeClient(int cfd){
        for (int i = 0; i < client_sockets->size();i++){
            if (client_sockets->at(i) == cfd){
                client_sockets->at(i) = -1;
                break;
            }
        }
    }

    bool inPending(int cfd){
        for (int i = 0; i < pending_updates->size();i++){
            if (cfd == pending_updates->at(i) && cfd != 0){
                return true;
            }

        }
        return false;
    }

    int server_fd;
    int client_fd;
    bool targetset;

    bool isClientsEmpty(){
        for (auto sock : *client_sockets){
            if (sock != -1){
                return false;
            }

        }
        return true;
    }

    void handle_client(int sock, bool first_client = false) {
        while (true) {
            std::string command = receive(sock, !first_client ? false : true);
            if (command.empty()) break;

            int8_t opt = handle_request(command, sock, !first_client ? false : true);
            if (opt == -2){
                break;
            }
            if (sigstop && isClientsEmpty()){
                stop();
                break;
            }
        }

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        if (first_client) client_fd = -1;
        std::cout << "Client disconnected.\n";
    }

    int handle_request(const std::string& command, int sock = -1, bool first_client = true) {
        if (sigstop && command != "disconnect"){
            sendMessage("stop_client");
        }
        if (command == "stop") {
            sendMessage("Stopping...", sock, first_client);
            BroadcastUpdate();
            return -1;
        }
        if (command == "iamclient") {
            client_fd = sock;
            sendMessage("received", sock, first_client);
            return -1;
        }
        if (command == "disconnect") {
            sendMessage("received", sock, first_client);
            removeClient(sock);
            RemovePending(sock);
            return -2;
        }
        if (command == "updated") {
            sendMessage("gotit", sock, first_client);
            RemovePending(sock);
            return -1;
        }

        if (command == "pendingupdate") {
            std::string pending = inPending(sock) ? "yes" : "no";
            if (*specific_update != "" && pending == "no"){
                sendMessage("specific", sock, first_client);
                sendMessage(*specific_update, sock, first_client);
                if (receive(sock) == "yes"){
                    *specific_update = "";
                }


            } else {
                sendMessage(pending, sock, first_client);
            }
            return -1;
        }

        if (command != "target" && targetset == false){
            sendMessage("Target not initialized", sock, first_client);
            return -1;
        }

        auto it = mappings.find(command);
        if (it == mappings.end()) {
            sendMessage("Unknown command", sock, first_client);
            return -1;
        }

        if (command == "launch" || command == "continue")
            sendMessage("LPAD", sock, first_client);
        else
            sendMessage("ok", sock, first_client);

        auto func = (std::string (*)(std::string))mappings[command];
        std::string arg = receive(sock, first_client);
        std::string result = func(arg);

        sendMessage(result, sock, first_client);
        return 0;
    }
    void stop() {
        #ifdef _WIN32
                if (client_fd != -1) closesocket(client_fd);
                if (server_fd != -1) closesocket(server_fd);
        #else
                if (client_fd != -1) close(client_fd);
                if (server_fd != -1) close(server_fd);
        #endif
        client_fd = -1;
        server_fd = -1;
        //std::cout << "Stopped Server" << std::endl;
    }



    int port;
};

auto server = std::make_shared<TCPServer>(8000);
#endif