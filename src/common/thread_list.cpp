#include "common/thread_list.h"

// node constructor
ThreadList::Node::Node(std::thread newThread) : thread(std::move(newThread)), next(nullptr) {}

// threadList constructor
ThreadList::ThreadList() : head(nullptr) {}

// threadList destructor
ThreadList::~ThreadList() {
    joinAll();  // make sure all threads are joined before destroying the list
}

// add a new thread to the list
void ThreadList::addThread(std::thread newThread) {
    auto newNode = std::make_unique<Node>(std::move(newThread));
    if (head != nullptr) {
        newNode->next = std::move(head);
    }
    head = std::move(newNode);
}

// join all threads in the list
void ThreadList::joinAll() {
    Node* current = head.get();
    while (current != nullptr) {
        if (current->thread.joinable()) {
            current->thread.join();
        }
        current = current->next.get();
    }
}

// clear all nodes from the list (automatically joins threads)
void ThreadList::clear() {
    while (head != nullptr) {
        if (head->thread.joinable()) {
            head->thread.join();
        }
        head = std::move(head->next);
    }
}
