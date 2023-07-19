#include <unordered_map>
#include <string>

class MimeType {

public:
    static std::string getMime(const std::string &suffix);

private:

    static void init();
    static std::unordered_map<std::string, std::string> mime;
    MimeType();
    MimeType(const MimeType &m);
    static pthread_once_t once_control;

};