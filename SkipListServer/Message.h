#pragma once

#include <string>

class Message{
    
public:
        
    Message(int operation, std::string key, std::string value = std::string())
        : operation_(operation), key_(key), value_(value) { }
    
    std::string key() { return key_; }
    std::string value() { return value_; }
    int operation() {return operation_; }

private:
    int operation_;
    std::string key_;
    std::string value_;

};