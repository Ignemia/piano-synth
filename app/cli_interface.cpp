#include <iostream>
#include <string>

class CliInterface {
public:
    void initialize() {
        std::cout << "CLI interface initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "CLI interface shutdown" << std::endl;
    }
    
    void processCommand(const std::string& command) {
        std::cout << "Processing CLI command: " << command << std::endl;
    }
};
