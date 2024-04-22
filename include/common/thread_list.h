#ifndef THREADLIST_H
#define THREADLIST_H

#include <thread>
#include <memory>
#include <utility>  

/**
 * @brief A class to manage a list of threads
 * 
 * This class is used to manage a list of threads. It provides methods to add threads to the list, join all threads in the list, clear the list, and check if the list is empty.
 * 
 * The class is non-copyable and non-movable.
*/
class ThreadList {
public:
    ThreadList();
    ~ThreadList();

    // Non-copyable and non-movable
    ThreadList(const ThreadList&) = delete;
    ThreadList& operator=(const ThreadList&) = delete;
    ThreadList(ThreadList&&) = delete;
    ThreadList& operator=(ThreadList&&) = delete;

    void addThread(std::thread newThread); // Add a new thread to the list
    void joinAll(); // Join all threads in the list
    void clear(); // Clear all nodes from the list (automatically joins threads)
    bool isEmpty() const; // Check if the list is empty

private:
    struct Node {
        std::thread thread;
        std::unique_ptr<Node> next;
        Node(std::thread newThread);
    };

    // Head of the list, using unique_ptr to manage memory
    std::unique_ptr<Node> head;
};

#endif // THREADLIST_H