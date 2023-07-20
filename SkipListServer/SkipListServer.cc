#include "SkipList.h"
#include "Message.h"

#include "TcpServer.h"
#include "InetAddress.h"
#include "Logging.h"
#include "Buffer.h"

#include <string>
#include <functional>
#include <cstring>

class SkipListServer{

public:

    SkipListServer(muduozdh::EventLoop *loop, const muduozdh::InetAddress &addr, const std::string &name, int skipListMaxLevel, muduozdh::TcpServer::Option option)
        :server_(loop, addr, name, option), skiplist_(skipListMaxLevel){

        server_.setConnectionCallback(std::bind(&SkipListServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&SkipListServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        server_.setThreadNum(3);
    }

    void start(){
        LOG << "start" ;
        LOG << "SkipListServer starts listening on " << server_.ipPort();
        server_.start();
    }

private:

    muduozdh::TcpServer server_;
    
    static const int headerLength = 8;
    static const int headerOperationLength = 1;
    static const int headerKeyLength = 3;
    static const int headerValueLength = 4;    
    std::string skipListSavePath_;

    SkipList<std::string, std::string> skiplist_;

    void onConnection(const muduozdh::TcpConnectionPtr &conn){

        if(conn->connected()){
            LOG << "new Connection arrived";
        }
        else{
            LOG << "Connection closed";
        }

    }

    void onMessage(const muduozdh::TcpConnectionPtr &conn, muduozdh::Buffer *buf, muduozdh::Timestamp time){

        size_t readable = buf->readableBytes();
        char* currentBufferBegin;
        
        LOG << " received :" << readable;

        if(readable < headerLength){
            LOG << "read incomplete";
            return ;
        }
        else{

            int operation;
            int keyLength;
            int valueLength;

            memcpy(&operation, buf->peek(), headerOperationLength);
            memcpy(&keyLength, buf->peek() + headerOperationLength, headerKeyLength);
            memcpy(&valueLength, buf->peek() + headerOperationLength + headerKeyLength, headerValueLength);

            int currentMessageLength = keyLength + valueLength + headerLength;

            if(readable < currentMessageLength){
                LOG << "read incomplete";
                return ;
            }

            if(operation >= 0 && operation <= 2){

                std::string key(buf->peek() + headerLength, buf->peek() + headerLength + keyLength );
                std::string value(buf->peek() + headerLength + keyLength, buf->peek() + headerLength + keyLength + valueLength);
                
                LOG << "get request, key: " << key << " value: " << value;

                Message message(operation, key ,value);
                std::string response = handle(message);
            
                buf->retrieveAll();
                conn->send(response);

            }
            else{
                LOG << "operation error";
                buf->retrieveUntil(buf->peek() + 1);
                return ;
            }

            // if(buf->readableBytes()){
            //     onMessage(conn, buf, time);
            // }

        }
    }

    std::string handle(Message message){

        bool result;
        std::string response;

        switch (message.operation())
        {
        case 0:
            result = skiplist_.insertElement(message.key(), message.value());
            response = getResponse(message,result);
            break;
        case 1:
            result = skiplist_.deleteElement(message.key());
            response = getResponse(message,result);
            break;
        case 2:
            // Node<std::string,std::string> node;         调用析构函数内存泄漏 

            Node<std::string ,std::string> *node = (Node<std::string, std::string> *)malloc(sizeof(Node<std::string, std::string>));
            
            result = skiplist_.searchElement(message.key(), node);
            if(result){
                Message returnMessage(2 ,node->key(), node->value());
                free(node);
                response = getResponse(returnMessage,result);
            }
            else{
                response = getResponse(message,result);
            }
            break;
        }
        return response;

    }


    std::string getResponse(Message &message, bool result){

        int operation = message.operation();
        int keyLength = message.key().size();

        int valueLength = operation == 2 ? message.value().size() : 0;

        char returnHeader[8];
        
        LOG << "operation: " << operation;

        memcpy(returnHeader, &operation, 1);
        memcpy(returnHeader + 1, &result, 1);
        memcpy(returnHeader + 2, &keyLength, 2);
        memcpy(returnHeader + 4, &valueLength, 4);
        
        std::string response(returnHeader, 8);

        response.append(message.key());

        if(operation == 2 && result){
            response.append(message.value());
        }

        LOG << "response size: " << response.size();

        return response;
    }

};


int main(){

    muduozdh::Logger::setLogFileName("./SkipListServer");

    muduozdh::EventLoop loop;
    muduozdh::InetAddress addr(8080);

    int skipListMaxLevel = 6;

    SkipListServer server(&loop, addr, "skipListServer", skipListMaxLevel, muduozdh::TcpServer::kReusePort );

    server.start();
    loop.loop();

}

