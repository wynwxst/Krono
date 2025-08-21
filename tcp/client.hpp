#include <iostream>
#include <cstring>
#ifndef CLIENT
#define CLIENT
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

class TCPClient {
public:
    std::function<void(std::string,bool)> lpad_cb = nullptr;
    TCPClient(const std::string& ip, int port) : ip(ip), port(port), sock(-1) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
#endif
    }

    ~TCPClient() {
        disconnect();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    

    void connectToServer() {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) throw std::runtime_error("Socket creation error");

        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
            throw std::runtime_error("Invalid address");

        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
            throw std::runtime_error("Connection Failed");

        std::cout << "Connected to server.\n";
    }

    void sendMessage(const std::string& msg) {
        std::string padding = (std::to_string(msg.size()).size() < 8 ? std::string(8 - std::to_string(msg.size()).size(),'0') : "" );
        std::string len_str = "len:" + padding +  std::to_string(msg.size());
        send(sock,len_str.c_str(),(int)len_str.size(),0);
        std::string sanity = receive();
        if (sanity != "ok"){
            throw std::runtime_error("Bad len\nReq:\n" + len_str + "\nsanity: " + sanity);
        }
        send(sock, msg.c_str(), (int)msg.size(), 0);
    }

    std::string _receive(int size=1024) {
        char* buffer = (char*)malloc(size);
        #ifdef _WIN32
                int valread = recv(sock, buffer, size, 0);
        #else
                int valread = read(sock, buffer, size);
        #endif
        return std::string(buffer, valread > 0 ? valread : 0);
    }

    std::string receive(){
        std::string len_str = _receive(4+8);
        std::string int_str = "";
        uint64_t len = 1024;
        if (len_str.substr(0,4) == "len:"){
            for (int i = 4; i < len_str.size(); i++ ){
                if (isdigit(len_str[i]) == 0){
                    break;
                } else {
                    int_str += len_str[i];
                }
            }
            if (int_str != ""){
                char* endptr;
                len = std::strtoull(int_str.c_str(),&endptr,10);
            }
            //sendMessage("ok");
            std::string result = _receive(len);
            //std::cout << "returning amount, len: " << len << "\nres: " << result << std::endl;
            auto trimmed = result;
            trimmed.erase(trimmed.find_last_not_of(" \n\r\t")+1);
            if (wait_for.find(trimmed) != wait_for.end()){
                std::cout << "got update!" << std::endl;
                *wait_for[result] = true;
            }
            return result;
        } else {
            return "invalid format";
        }

    }


    std::string sendNrecv(std::string send){
        sendMessage(send);
        return receive();
    }

    void setLPAD_cb(std::function<void(std::string,bool)> cb){
        lpad_cb = cb;
    }

    std::string sendCommand(std::string command, std::string args="_"){
        auto result = sendNrecv(command);
        if (result == "ok"){
            return sendNrecv(args);
        } 
        else if (result == "LPAD"){
            if (lpad_cb){
                lpad_cb("startloop",false);
            }
            std::string inp = sendNrecv(args);
            while (inp != "endloop"){
                if (inp != "input_required"){
                    display(inp);
                } else {
                    std::string prompt = receive(); // begin_prompt
                    while (prompt != "end_prompt"){
                        if (prompt != "end_prompt" && prompt != "begin_prompt"){
                            display(prompt,true);
                        }
                        prompt = receive();
                    }
                    std::string input;
                    std::getline(std::cin, input);
                    
                    std::string sanity = sendCommand("setinput",input);
                    if (sanity != "received"){
                        display("Failed to send input to process: " + sanity);
                    }

                    //display("Sending free input");
                    sendCommand("free_input");
                    //display("sent");
                    
                }
                inp = receive();
            }
            if (lpad_cb){
                lpad_cb("endloop",false);
            }
            return "";

        }
        else {
            return result;
        }
    }

    void display(std::string content,bool isinput=false){
        if (lpad_cb){
            lpad_cb(content,isinput);
        } else {
            std::cout << content << std::endl;
        }
    }

    void disconnect() {
        std::string discreq =  sendCommand("disconnect");

#ifdef _WIN32
        if (sock != -1) closesocket(sock);
#else
        if (sock != -1) close(sock);
#endif
        sock = -1;
    }

    void register_wait(std::string wfor, std::shared_ptr<bool> ref){
        wait_for[wfor] = ref;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<bool>> wait_for;
    std::string ip;
    int port;
    int sock;
};

#endif

