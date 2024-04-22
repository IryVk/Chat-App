/**
 * @file common/thread_list.cpp
 * @date 2024-04-22
 * @author Arwa Essam Abdelaziz
 * @brief This file contains the implementation of the ThreadList class
 * 
 * This file contains the implementation of the ThreadList class, which is used to manage a list of std::thread objects. It provides methods to add threads to the list, join all threads in the list, and clear the list.
*/

#include "common/thread_list.h"

/**
 * @brief Construct a new ThreadList::Node object
 * 
 * Initialize a new ThreadList::Node object with the specified thread.
 * 
 * @param newThread The thread to store in the node
 * 
 * @return ThreadList::Node object
*/
ThreadList::Node::Node(std::thread newThread) : thread(std::move(newThread)), next(nullptr) {}

/**
 * @brief Construct a new ThreadList object
 * 
 * Initialize a new ThreadList object with an empty list.
 * 
 * @return ThreadList object
*/
ThreadList::ThreadList() : head(nullptr) {}

/**
 * @brief Destroy the ThreadList object
 * 
 * Destroy the ThreadList object and free any allocated resources.
 * 
 * @return void
*/
ThreadList::~ThreadList() {
    clear();  // make sure all threads are joined before destroying the list
}

/**
 * @brief Add a new thread to the list
 * 
 * Add a new thread to the list by creating a new node and moving the thread into it.
 * 
 * @param newThread The thread to add to the list
 * 
 * @return void
*/
void ThreadList::addThread(std::thread newThread) {
    auto newNode = std::make_unique<Node>(std::move(newThread));
    if (head != nullptr) {
        newNode->next = std::move(head);
    }
    head = std::move(newNode);
}

/**
 * @brief Join all threads in the list
 * 
 * Join all threads in the list by iterating over the nodes and calling join() on each thread.
 * 
 * @return void
*/
void ThreadList::joinAll() {
    Node* current = head.get();
    while (current != nullptr) {
        if (current->thread.joinable()) {
            current->thread.join();
        }
        current = current->next.get();
    }
}

/**
 * @brief Clear the list
 * 
 * Clear the list by iterating over the nodes, joining them, and moving the next node into the head.
 * 
 * @return void
*/
void ThreadList::clear() {
    while (head != nullptr) {
        if (head->thread.joinable()) {
            head->thread.join();
        }
        head = std::move(head->next);
    }
}

/**
 * @brief Check if the list is empty
 * 
 * Check if the list is empty by checking if the head pointer is null.
 * 
 * @return bool True if the list is empty, false otherwise
*/
bool ThreadList::isEmpty() const {
    return head == nullptr;
}
