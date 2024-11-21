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
        else if(input == "startscan") {
            NetworkProvider::getInstance().setScanMode(true);
        }
        else if(input == "stopscan") {
            NetworkProvider::getInstance().setScanMode(false);
        }
        else if(input == "name") {
            std::cout << "Central name: " << NetworkProvider::getInstance().getBluetoothName() << "\n";
        }
        else if(input == "address") {
            std::cout << "Central address: " << NetworkProvider::getInstance().getBluetoothAddress() << "\n";
        }
        else if(input == "dump"){
            NetworkProvider::getInstance().dumpBluetoothDevices();
        }
        else {

        }
    }
}