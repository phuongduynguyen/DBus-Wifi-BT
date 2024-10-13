#include <iostream>
#include "NetworkProvider.h"

int main(void)
{
    NetworkProvider::initialize().toggleNetWork(NetworkProvider::NetworkType::Bluetooth);
}