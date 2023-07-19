#include "Node.h"

#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <string>

template<typename K, typename V>
class SkipList{

public:

    SkipList(int);
    ~SkipList();

    int getRandomLevel();

    Node<K,V>* createNode(K,V,int);
    Node<K,V>* createNode(K,V);

    bool insertElement(K,V);
    bool searchElement(K, Node<K,V>*);
    bool deleteElement(K);
    
    void dumpFile();
    void loadFile();

    int size();

    void displayList();

private:

    void getKeyValueFromString(const std::string& str, std::string* key, std::string* value);
    bool isValidString(const std::string& std);

    int maxLevel_;

    int skipListLevel_;
    Node<K,V> *header_;

    std::ofstream fileWriter_;
    std::ifstream fileReader_;

    int elementCount_;

    std::mutex mutex_;

    std::string filePath_;

};


#include <muduozdh/Logging.h>

template<typename K, typename V> 
SkipList<K, V>::SkipList(int maxLevel) 
    :maxLevel_(maxLevel), skipListLevel_(0), elementCount_(0), filePath_("./skiplist"), mutex_(){

    K k;
    V v;
    this->header_ = new Node<K, V>(k, v, maxLevel_);
}

template<typename K, typename V> 
SkipList<K, V>::~SkipList(){

    if(fileWriter_.is_open()) {
        fileWriter_.close();
    }
    if(fileReader_.is_open()){
        fileReader_.close();
    }

    delete header_;
}

template<typename K, typename V> 
int SkipList<K, V>::getRandomLevel(){

    int k = 1;
    while(rand() % 2) {
        k++;
    }

    k = (k < maxLevel_) ? k : maxLevel_;
    return k;

}

template<typename K, typename V> 
Node<K, V> *SkipList<K, V>::createNode(const K k, const V v , int level){

    Node<K, V> *node = new Node<K, V>(k, v, level);
    return node;
}

template<typename K, typename V> 
Node<K, V> *SkipList<K, V>::createNode(const K k, const V v){

    int level = getRandomLevel();
    Node<K, V> *node = new Node<K, V>(k, v, level);
    return node;
}

template<typename K, typename V> 
bool SkipList<K, V>::insertElement(const K key, const V value){

    std::unique_lock<std::mutex>(mutex_);

    Node<K, V> *current = this->header_;

    Node<K, V> *update[maxLevel_ + 1];
    memset(update, 0, sizeof(Node<K, V>*)*(maxLevel_ + 1));

    for (int i = skipListLevel_; i >= 0 ;i--){
        while(current->next()[i] && current->next()[i]->key() < key){
            current = current->next()[i]; 
        }
        update[i] = current;

    }

    current = current->next()[0];

    if(current && current->key() == key){
        LOG << "key: " << key << " exists";
        return false;
    }

    if(!current || current->key() != key){

        int randomLevel = getRandomLevel();
        if(randomLevel > skipListLevel_){
            for(int i = skipListLevel_ + 1; i < randomLevel + 1; i++){
                update[i] = header_;
            }
            skipListLevel_ = randomLevel;
        }

        Node<K, V>* insertNode = createNode(key, value, randomLevel);

        for(int i = 0;i <= randomLevel; i++){
            insertNode->next()[i] = update[i]->next()[i];
            update[i]->next()[i] = insertNode;
        }

        LOG << "Insert Successfully: key: " << key;
        elementCount_++;
    }
    return true;
}

template<typename K, typename V> 
bool SkipList<K, V>::searchElement(K key, Node<K,V>* returnNode){

    std::unique_lock<std::mutex> lock(mutex_);

    LOG<< "Search Element "  << key; 
    Node<K, V> *current = header_;

    if(elementCount_ <= 0) {
        return false;
    }

    for(int i = skipListLevel_; i >= 0 ;i--){
        while(current->next()[i] && current->next()[i]->key() < key){
            current = current->next()[i];
        }
    }
    
    current = current->next()[0];

    if(current and current->key() == key){
        LOG << "Found key: " << key << ", value: " << current->value();
        returnNode->setKey(current->key());
        returnNode->setValue(current->value());
        return true;
    }

    LOG << "Not Found Key: " << key;
    return false; 

}

template<typename K, typename V> 
bool SkipList<K, V>::deleteElement(K key){

    std::unique_lock<std::mutex>(mutex_);

    Node<K, V> *current = header_;
    Node<K, V> *update[maxLevel_ + 1];
    memset(update, 0 , sizeof(Node<K, V>*)*(maxLevel_ + 1));

    for(int i = skipListLevel_ ; i >= 0 ; i--){
        while(current->next()[i] && current->next()[i]->key() < key){
            current = current->next()[i];
        }
        update[i] = current;
    }

    current = current->next()[0];

    if(current && current->key() == key){

        for (int i = 0;i<= skipListLevel_;i++){
            if(update[i]->next()[i] != current) {
                break;
            }
            update[i]->next()[i] = current->next()[i];
        }

        while(skipListLevel_ > 0 && header_->next()[skipListLevel_] == 0){
            skipListLevel_--;
        }
        elementCount_--;
        LOG << "Delete Successfully: key: " <<  key ;
        return true;
    }

    return false;

}

template<typename K, typename V>
void SkipList<K, V>::loadFile(){

    fileReader_.open(filePath_);
    LOG << "Load File---------------";
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();

    while(getline(fileReader_, line)){
        getKeyValueFromString(line, key, value);
        if(key->empty() || value->empty()){
            continue;
        }
        insertElement(*key, *value);

        LOG << "Load File " << *key << " : " << value;
    }

    fileReader_.close();
}


template<typename K, typename V>
void SkipList<K, V>::dumpFile(){

    LOG << "Dump File---------------";
    fileWriter_.open(filePath_);
    Node<K, V> *current = header_->next()[0];
    
    while(current){
        fileWriter_ << current->key() << ":" << current->value << "\n";
        LOG << "Dump File " << current->key() << " : " << current->value();
        current = current->next()[0];
    }
    fileWriter_.flush();
    fileWriter_.close();
    return ;

}

template<typename K, typename V>
int SkipList<K, V>::size(){
    return elementCount_;
}

template<typename K, typename V>
void SkipList<K, V>::displayList(){

    std::cout<< "*****************Skip List*****************" << std::endl;

    for(int i = 0;i<=skipListLevel_;i++){
        Node<K, V> *node = this->header_->next()[i];
        std::cout << "Level " << i << ": ";
        while(node != NULL){
            std::cout << node->key() << ":" << node->value() << ";";
            node = node->next()[i];
        } 
        std::cout << std::endl;
    }

}


template<typename K, typename V> 
bool SkipList<K, V>::isValidString(const std::string& str){

    if (str.empty()){
        return false;
    }
    if (str.find(':') == std::string::npos){
        return false;
    }
    return true;
}

template<typename K, typename V> 
void SkipList<K, V>::getKeyValueFromString(const std::string& str, std::string* key, std::string* value){

    if(!isValidString(str)){
        return;
    }    
    *key = str.substr(0, str.find(':'));
    *value = str.substr(str.find(':') + 1, str.length());
}



