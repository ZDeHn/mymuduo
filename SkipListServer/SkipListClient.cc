#include "Message.h"

#include <string>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <algorithm>
#include <cctype>
#include <iostream>

void handle(Message message,int clientFd){

    char responseHeader[8];

    int operation = message.operation();

    int keyLength = message.key().size();
    int valueLength = message.value().size();

    memcpy(responseHeader, &operation, 1);
    memcpy(responseHeader + 1, &keyLength, 3);
    memcpy(responseHeader + 4, &valueLength, 4);
    
    std::string request(responseHeader, 8);

    request.append(message.key());
    
    if(!message.value().empty()){
        request.append(message.value());
    }

    // std::cout << "keyLength: " << keyLength << " valueLength: " << valueLength << std::endl;
 
    if(-1 == send(clientFd, request.c_str(), request.size(), 0)){
        std::cout << "Send Message Error!" << std::endl;
        ::close(clientFd);
        exit(-1);
    }

    // std::cout << "send message ok" << std::endl;


    char buf[1024];
    int recvLength;

    

    if(-1 != (recvLength = recv(clientFd ,buf, 1024 ,0))){

        if(recvLength < 8){
            std::cout << "Resposne Error!" <<  std::endl;
            ::close(clientFd);
            exit(1);
        }

        std::string responseBuffer(buf, recvLength);
        
        if(recvLength != 0){

            int operation;
            int returnKeyLength;
            int returnValueLength;
            bool result;

            memcpy(&operation, responseBuffer.c_str(), 1);
            memcpy(&result, responseBuffer.c_str() + 1, 1);
            memcpy(&returnKeyLength, responseBuffer.c_str() + 2, 2);
            memcpy(&returnValueLength, responseBuffer.c_str() + 4, 4);

            // std::cout << "operation: " << operation << std::endl;
            // std::cout << "result length " << recvLength << std::endl;

            // std::cout << "result: " << result<<std::endl;
            // std::cout << "resultKeyLength" << returnKeyLength << std::endl;
            // std::cout << "resultValueLength" << returnValueLength << std::endl;

            if(operation != message.operation() || responseBuffer.size() < 8 + returnKeyLength + returnValueLength){
                std::cout << "Resposne Error!" <<  std::endl;
                ::close(clientFd);
                exit(1);
            }

            std::string returnKey(responseBuffer.c_str() + 8, responseBuffer.c_str() + 8 + returnKeyLength);
            std::string returnValue(responseBuffer.c_str() + 8 + returnKeyLength, responseBuffer.c_str() + 8 + returnKeyLength + returnValueLength);


            if(result){
                switch (operation)
                {
                case 0:
                    std::cout << "Insert Success! Key: " << returnKey << std::endl;
                    break;
                case 1:
                    std::cout << "Delete Success! Key: " << returnKey << std::endl;
                    break;
                case 2:
                    std::cout << "Find Success! Key: " << returnKey << " Value: " << returnValue << std::endl;
                    break;
                }
            }
            else{
                switch (operation)
                {
                case 0:
                    std::cout << "Insert Fail! Key: " << returnKey << " is Exists! "<<std::endl;
                    break;
                case 1:
                    std::cout << "Delete Fail! Key: " << returnKey << " is not Found!" << std::endl;
                    break;
                case 2:
                    std::cout << "Find Fail! Key: " << returnKey << " is not Found!" << returnValue << std::endl;
                    break;
                }
            }

        }

    }

}


int main(){

    int clientFd = ::socket(AF_INET, SOCK_STREAM, 0);

    std::string ip = "127.0.0.1";
    int port = 8080;

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(clientFd, (struct sockaddr*)&server_addr, sizeof(server_addr))){

        ::close(clientFd);
        std::cout << "Skiplist Service Connection Error ! Please Try Again..." << std::endl;
        exit(1);
    }

    std::cout << "Connected Server!" << std::endl;

    std::string input;
    
    while(std::getline(std::cin , input)){

        const char *space = std::find(input.c_str(), input.c_str() + input.size() , ' ');
        
        if(space == input.c_str() + input.size()){

            std::transform(input.begin(), input.end(), input.begin() , tolower);
            if(input == "exit"){
                ::close(clientFd);
                std:: cout << "Client Exit!" << std::endl;
                break;
            }
            else{
                std::cout << "Input Error! Please Try Again" << std::endl;
                continue;
            }
        }

        std::string operation(input.c_str(),space);
        std::transform(operation.begin(), operation.end() , operation.begin(), tolower);


        if(operation == "insert"){

            const char *secondSpace = std::find(space + 1, input.c_str() + input.size(), ' ');
            
            if(secondSpace == input.c_str() + input.size()){
                std::cout << "Insert Command Error: " << std::string(space + 1,input.c_str() + input.size()) << std::endl;
                continue ;
            }
            
            std::string key(space + 1, secondSpace);
            std::string value(secondSpace + 1, input.c_str() + input.size());
            if(value.size() == 0 || key.size() > 512 || key.size() == 0 || value.size() > 2147483647){

                std::cout << "Insert Command Error: " << std::string(space + 1,input.c_str() + input.size()) << std::endl;
                continue ;
            }

            Message message(0,key,value);
            handle(message, clientFd);
        }

        else if(operation == "delete") {

            const char *secondSpace = std::find(space + 1, input.c_str() + input.size(), ' ');

            if(secondSpace != input.c_str() + input.size()){
                std::cout << "Delete Command Error !" << std::endl;
                continue;
            }

            std::string key(space + 1, input.c_str() + input.size());
            
            if(key.size() == 0 || key.size() > 512){

                std::cout << "Delete Command Error !" << std::endl;
                continue;
            }
            Message message(1,key);
            handle(message, clientFd);
        }

        else if(operation == "find"){

            const char *secondSpace = std::find(space + 1, input.c_str() + input.size(), ' ');

            if(secondSpace != input.c_str() + input.size()){
                std::cout << "Find Command Error !" << std::endl;
                continue;
            }

            std::string key(space + 1, input.c_str() + input.size());
            
            if(key.size() == 0 || key.size() > 512){

                std::cout << "Find Command Error !" << std::endl;
                continue;
            }
            Message message(2,key);
            handle(message, clientFd);
        }

        else{

            std::cout << "Unknown Operation " <<  operation  << std::endl;
            std::cout << "Input Operation : 1. insert 2. delete 3. find" << std::endl;
            continue;
        }

    }

}

