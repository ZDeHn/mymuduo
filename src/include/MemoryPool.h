#pragma once

#include <mutex>

namespace muduozdh{

template<class C, int sizeofElement,int numElement>
class MemoryPool{

public:
    
    class Slot{
    public:
        char data[sizeofElement];
        bool used;
        Slot* next;
        Slot():used(false) { }
    };

    class Block{
    public:
        int currentSlot;
        Slot slots[numElement];
        Block *next;

        Block():currentSlot(0), next(nullptr) { }
        ~Block() {

            for(int i = 0;i < currentSlot; ++i){

                Slot* data = slots + i;
                if(data->used){
                    reinterpret_cast<C*>(data)->~C();
                }
            }
        }
    };

    MemoryPool();

    ~MemoryPool();

    C* allocate();
    void deallocate(C* c);

    template<class ... Args>
    void construct(C* c, Args&&... args);
    
    template<class ... Args>
    C* newData(Args&&... args);
    void deleteData(C* c);

private:

    Block* blockHead_;
    Slot* freeSlotHead_;
  
    std::mutex mutex_;

};



template<class C, int sizeofElement, int numElement>
MemoryPool<C, sizeofElement, numElement>::MemoryPool(): freeSlotHead_(nullptr), blockHead_(nullptr) { }

template<class C, int sizeofElement, int numElement>
MemoryPool<C, sizeofElement ,numElement>::~MemoryPool(){

    Block* currentBlock = blockHead_;
    while(currentBlock){
        Block* ptr = currentBlock;
        currentBlock=ptr->next;
        delete(ptr);
    }

}

template<class C, int sizeofElement, int numElement>
C* MemoryPool<C, sizeofElement, numElement>::allocate(){

    std::unique_lock<std::mutex> lock(mutex_);

    if(freeSlotHead_){
        freeSlotHead_->used = true;
        C* result = reinterpret_cast<C*>(freeSlotHead_);
        freeSlotHead_ = freeSlotHead_->next;
        return result;
    }
    else{
        if(!blockHead_){
            Block* newBlock = new Block();
            blockHead_ = newBlock;
        }
        else if(blockHead_->currentSlot >= numElement){
            Block* newBlock = new Block();
            newBlock->next = blockHead_;
            blockHead_ = newBlock;
        
        }
        Slot* data = blockHead_->slots + blockHead_->currentSlot++;
        data->used = true;
        return reinterpret_cast<C*>(data);
        
    }
}

template<class C, int sizeofElement, int numElement>
void MemoryPool<C, sizeofElement, numElement>::deallocate(C* c){

    std::unique_lock<std::mutex> lock(mutex_);

    if(c){
        reinterpret_cast<Slot*>(c)->next =  freeSlotHead_;
        reinterpret_cast<Slot*>(c)->used = false;
        freeSlotHead_ = reinterpret_cast<Slot*>(c);
    }
}


template<class C, int sizeofElement, int numElement>
template<class ... Args>
void MemoryPool<C, sizeofElement, numElement>::construct(C *c, Args&&... args){
    new (c)C(std::forward<Args>(args)...);
}


template<class C, int sizeofElement, int numElement>
template<class ... Args>
C* MemoryPool<C, sizeofElement, numElement>::newData(Args&&... args){

    C* data = allocate();
    construct(data, std::forward<Args>(args)...);
    return data;
}

template<class C, int sizeofElement, int numElement>
void MemoryPool<C, sizeofElement, numElement>::deleteData(C* c){

    if(c){
        c->~C();
        deallocate(c);
    }

}

}









