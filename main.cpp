#include <iostream>
#include "NetworkProvider.h"
#include <dlfcn.h>
#include <string.h>

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
    else if(input == "connect") {
        std::string address;
        std::string profile;
        std::cout << "\nEnter Address: ";
        std::getline(std::cin, address);
        std::cout << "\nEnter profile: ";
        std::getline(std::cin, profile);
        NetworkProvider::getInstance().connectProfile(address, profile);
    }
    else if(input == "disconnectprofile") {
        std::string address;
        std::string profile;
        std::cout << "\nEnter Address: ";
        std::getline(std::cin, address);
        std::cout << "\nEnter profile: ";
        std::getline(std::cin, profile);
        NetworkProvider::getInstance().disconnectProfile(address, profile);
    }
    else if (input == "disconnect") {
        std::string address;
        std::cout << "\nEnter Address: ";
        std::getline(std::cin, address);
        NetworkProvider::getInstance().disconnectBluetoothDevice(address);
    }
    else {

    }   
  }
}
