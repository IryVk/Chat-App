#ifndef THREADLIST_H
#define THREADLIST_H

#include <thread>
#include <memory>
#include <utility>  

// linked list of threads
class ThreadList {
public:
    ThreadList();
    ~ThreadList();

    // non-copyable and non-movable
    ThreadList(const ThreadList&) = delete;
    ThreadList& operator=(const ThreadList&) = delete;
    ThreadList(ThreadList&&) = delete;
    ThreadList& operator=(ThreadList&&) = delete;

    void addThread(std::thread newThread); // add a new thread to the list
    void joinAll(); // join all threads in the list
    void clear(); // clear all nodes from the list (automatically joins threads)
    bool isEmpty() const; // check if the list is empty

private:
    struct Node {
        std::thread thread;
        std::unique_ptr<Node> next;
        Node(std::thread newThread);
    };

    // head of the list, using unique_ptr to manage memory
    std::unique_ptr<Node> head;
};

#endif // THREADLIST_H