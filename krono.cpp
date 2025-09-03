#include "misc/base.hpp"
#include "process/proc.hpp"
#include "tcp/server.hpp"
#include "tcp/client.hpp"
#include "tcp/tcp_map.hpp"
#include "panel/paneldef.hpp"
#include "panel/registerpanels.hpp"

void client_display(std::string,bool);

int main(int argc, char** argv) {
    args = ArgParser(argv,argc);
    args.parse();
    auto v = std::getenv("KRONO_DEBUG");
    if (v){
        is_debug = std::string(v) == std::string("yes");
    }
    logger = std::make_shared<Logger>("Krono.log");
    int port = (args.has("port") ? std::stoi(args.values["port"][0]) : 8080);

    if (args.has("panel")){
        signal(SIGWINCH, [](int){ get_terminal_size(); });

        get_terminal_size();

        PanelManager pm;
        

        auto panelname = args.values["panel"][0];
        register_panels(&pm,port,panelname);
        std::vector<std::string> allowedpanels{"list","disasm","view"};
        if (in(panelname,allowedpanels)){
            pm.run();
            
        }

    }

    else if (args.has("server")){
        register_endpoints(server);
        server->port = port;
        server->start();
    } else if (args.has("client")){
        std::string input;
        std::string command;
        std::string args;
        std::cout << "Client Repl\n";
        TCPClient client("127.0.0.1",port);
        auto update = std::make_shared<bool>(false);
        client.setLPAD_cb(client_display);
        client.connectToServer();
        client.sendCommand("iamclient");

        while (command != "stop"){
            std::cout << "\u001b[1m" << "krono" << "\033[0m" <<"▶ ";
            std::getline(std::cin, input);
            if (input == "disconnect"){
                client.disconnect();
                exit(0);
            }
        


            size_t pos = input.find(' ');


            if (pos != std::string::npos) {

                command = input.substr(0, pos);
                args = input.substr(pos + 1);
            } else {
                command = input;
                args = "_";
            }
            client_display(client.sendCommand(command,args),false);
        }
        client.disconnect();

        //client.sendCommand("stop");
    }



    


    return 0;
}

void client_display(std::string dcontent,bool isinput=false){
    if (dcontent == "startloop" || dcontent == "endloop") return;
    dcontent = replace(replace(dcontent,"<thinline>",""),"<fill>","");
    std::cout << dcontent << (isinput ? "" : (dcontent.find("\n") == std::string::npos ? "\n" : ""));
    
    
    
}
