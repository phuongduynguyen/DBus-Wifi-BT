#ifndef NETWORK_PROVIDER
#define NETWORK_PROVIDER

#include <dbus/dbus.h>
#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include "BluetoothManager.h"
class NetworkProvider
{
    friend class BluetoothAdapter;
    public:
        enum class NetworkType
        {
            Wifi,
            Bluetooth
        };

        static NetworkProvider& initialize();
        static NetworkProvider& getInstance();
        void toggleNetWork(const NetworkType& type);
        void setScanMode(bool isScan);
        std::string getBluetoothName() const;
        std::string getBluetoothAddress() const;
        void dumpBluetoothDevices();
        
    private:
        NetworkProvider();
        ~NetworkProvider();
        bool doInit();
        void signalHandler();

        DBusMessage* createMethod(const char* serviceName, const char* objectPath, const char* interface, const char* method);
        DBusMessage* invokeMethod(DBusMessage* messageSend, const char* interface, const char* property, bool value = false);

        bool getWiFiStatus();
        bool getBTStatus();

        DBusConnection* mConnection = nullptr;
        std::thread* mWorkerThread = nullptr;
    };

#endif // NETWORK_PROVIDER