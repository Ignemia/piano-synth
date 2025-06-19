#include <iostream>

class EventProcessor {
public:
    void initialize() {
        std::cout << "Event processor initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "Event processor shutdown" << std::endl;
    }
    
    void processEvent() {
        std::cout << "Processing event..." << std::endl;
    }
};
