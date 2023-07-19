#pragma once

template<typename K, typename V> 
class Node {

public:
    
    Node(){};
    Node(K k, V v, int); 
    ~Node();

    K key() const;
    void setKey(K);

    V value() const;
    void setValue(V);

    int level() const;

    Node<K, V> **next() const;

private:
    
    K key_;
    V value_;
    int level_;

    Node<K, V> **next_;


};


template<typename K, typename V> 
Node<K, V>::Node(const K k, const V v, int level)
    : key_(k), value_(v), level_(level) , next_(new Node<K, V>*[level + 1]){
    
    memset(this->next_, 0, sizeof(Node<K, V>*)*(level + 1));
};

template<typename K, typename V>
Node<K, V>::~Node(){

    delete[] next_;
}

template<typename K, typename V>
K Node<K, V>::key() const{
    return key_;
}

template<typename K, typename V>
void Node<K, V>::setKey(K k){
    key_ = k;
}

template<typename K, typename V>
V Node<K, V>::value() const {
    return value_;
}

template<typename K, typename V>
void Node<K, V>::setValue(V v){
    value_ = v;
}

template<typename K, typename V>
Node<K, V> **Node<K, V>::next() const{
    return next_;
}
