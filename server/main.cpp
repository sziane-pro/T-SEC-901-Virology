#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
using namespace std;

ix::SocketTLSOptions makeServerTLSOptions(const string& dir)
{
    ix::SocketTLSOptions opts;
    opts.tls      = true;
    opts.certFile = dir + "/cert.pem";
    opts.keyFile  = dir + "/key.pem";
    opts.caFile   = "NONE";
    return opts;
}

int main(int argc, char** argv){

    ix::initNetSystem();

    auto tls = makeServerTLSOptions(argc > 1 ? argv[1] : "certs");
    if (!tls.isValid())
    {
        cerr << "TLS: " << tls.getErrorMsg() << endl;
        return 1;
    }

    int port = 9090;
    string host("0.0.0.0");
    ix::WebSocketServer server(port, host);
    server.setTLSOptions(tls);

    server.setOnClientMessageCallback([](shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket & webSocket, const ix::WebSocketMessagePtr & msg) {
        switch(msg->type){
            case ix::WebSocketMessageType::Open:
                cout << "New connection" << endl;
                break;
            case ix::WebSocketMessageType::Message:
                cout << "Received" << msg->str << endl;
                webSocket.send("Hi client", false);
                break;
            default:
                break;
        }
    });

    auto res = server.listen();

    if (!res.first)
    {
        cerr << "Failed to start server: "
                  << res.second << endl;

        return 1;
    }

    server.start();

    cout << "Server listening on port 9090..." << endl;

    string ligne;

    while(std::getline(std::cin, ligne) && ligne != "quit"){
        auto clients = server.getClients();
        if(!clients.empty()){
            for (auto& client : clients) {
                client->send(ligne);
            }
        }else{
            cout << "Any client connected" << endl;
        }
    }

    server.stop();

    ix::uninitNetSystem();

    return 0;
}
