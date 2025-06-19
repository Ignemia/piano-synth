#include <iostream>

class TimingProcessor {
public:
    void initialize() {
        std::cout << "Timing processor initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "Timing processor shutdown" << std::endl;
    }
    
    void processTimestamp(double timestamp) {
        std::cout << "Processing timestamp: " << timestamp << std::endl;
    }
};
