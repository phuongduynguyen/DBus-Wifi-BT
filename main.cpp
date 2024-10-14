#include <iostream>
#include "NetworkProvider.h"

int main(void)
{
    NetworkProvider::initialize();
    std::string input;
    while (true)
    {
        std::cout << "Enter Message: ";
        std::getline(std::cin, input);
        if (input == "bt") {
            NetworkProvider::getInstance().toggleNetWork(NetworkProvider::NetworkType::Bluetooth);
        } 
        else if(input == "wf") {
            NetworkProvider::getInstance().toggleNetWork(NetworkProvider::NetworkType::Wifi);
        }
        else {

        }
        
    }
}