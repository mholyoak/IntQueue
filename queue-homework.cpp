// queue-homework.cpp
//
//
//  running in gcc docker container 
//
//  docker run -it --rm -v <path-to-local-source-directory>:/queue-homework gcc /bin/bash
//  cd /queue-homework/
//  g++ queue-homework.cpp  -lpthread -o hw
//  ./hw



#include <stdio.h>
#include <assert.h>
#include <memory>
#include <thread>
#include <iostream>
#include <mutex>

//base queue interface
class IIntQueue
{
public:
    virtual ~IIntQueue() = default;

    // Add an item to the queue
    virtual void Add(int item) = 0;

    // Remove the first item and return it
    virtual int Remove() = 0;

    // return the count of items
    virtual int GetCount() = 0;
};

//
// prototypes
//
void TestQueueProc(void* lpParameter, int id);
void TestQueue(IIntQueue *pQueue);


// TODO
// Add the implementation to IntQueue class so the program can run without
// any asserts

// Queue class implementation 
class IntQueue : public IIntQueue
{
public:
    IntQueue() 
    {
        _count = 0;
        _head = _tail;
    };

    virtual void Add(int item)
    {
        std::unique_lock<std::mutex> lock (_mutex);

        if (_head == nullptr)
        {
            _head = std::make_shared<IntQueueItem>(item);
            _tail = _head;
        }
        else
        {
            auto newItem = std::make_shared<IntQueueItem>(item);
            _tail->_tail = newItem;
            _tail = newItem;
        }
        _count++;
    };

    virtual int Remove()
    {
        std::unique_lock<std::mutex> lock (_mutex);

        int item = 0;

        assert(_head != nullptr);
        if (_head != nullptr)
        {
            item = _head->_value;
            _head = _head->_tail;
            _count--;
        }

        return item;
    };

    virtual int GetCount() 
    {
        std::unique_lock<std::mutex> lock (_mutex);
        /*
        int count = 0;

        std::shared_ptr<IntQueueItem> curItem = _head;
        while (curItem != nullptr)
        {
            count++;
            curItem = curItem->_tail;
        }

        return count;
        */
        return _count;
    };

 private:

    struct IntQueueItem
    {
        IntQueueItem(){}
        IntQueueItem(int value) :
            _value(value)
        {}

        IntQueueItem(int value, std::shared_ptr<IntQueueItem>& tail):
            _value(value),
            _tail(tail)
        {}

        int _value;
        std::shared_ptr<IntQueueItem> _tail;
    };

    std::mutex _mutex;
    int _count;
    std::shared_ptr<IntQueueItem> _head;
    std::shared_ptr<IntQueueItem> _tail;
};


// Test code below

struct TestQueueData
{
    IIntQueue* pQueue;
    int count;
    bool run;
};

// test queues thread function
void  TestQueueProc( void * lpParameter, int id)
{

    TestQueueData* pData = (TestQueueData*)lpParameter;

    // Wait until thread is told to start 
    while (!pData->run) 
    {
    }   
 
    // Add count items to the queue.  Each item is the id
    for (int i = 0; i < pData->count; i++)
    {
        pData->pQueue->Add(id);
    }

}

void TestQueue(IIntQueue *pQueue)
{
    // create threads to update the queue
    
    TestQueueData data;

    data.count = 20000;
    data.pQueue = pQueue;
    data.run = false;

    // Create test threads
    
    std::thread thread1(TestQueueProc, &data, 1);
    std::thread thread2(TestQueueProc, &data, 2);

    std::this_thread::sleep_for(std::chrono::seconds(1));
 
    //Notify threads
    data.run = true;    

    thread1.join();
    thread2.join();

    int countThread1 = 0;
    int countThread2 = 0;

    // check the data 
    printf("dump:  count=%d\n", pQueue->GetCount());

    assert(data.count * 2 == pQueue->GetCount());

    while (pQueue->GetCount() > 0)
    {
        int item = pQueue->Remove();

        if (item == 1)
        {
            countThread1++;
        }
        else if (item == 2)
        {
            countThread2++;
        }
        else
        {
            assert(0);
        }
    }

    printf("dump:  count=%d  CountID1=%d  CountID2=%d\n", pQueue->GetCount(), countThread1, countThread2);

    assert(countThread1 == data.count);
    assert(countThread2 == data.count);
}


int main(int argc, char** argv)
{
    // Test the queue implementation 
    IntQueue queue1;
    TestQueue(&queue1);

    return 0;
}

