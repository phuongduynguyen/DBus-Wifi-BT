#ifndef NETWORK_PROVIDER
#define NETWORK_PROVIDER

#include <dbus/dbus.h>
#include <iostream>
#include <string>

class NetworkProvider
{
    public:
        enum class NetworkType
        {
            Wifi,
            Bluetooth
        };

        static NetworkProvider& initialize();
        static NetworkProvider& getInstance();
        void toggleNetWork(const NetworkType& type);

    private:
        NetworkProvider();
        bool doInit();

        DBusMessage* createMethod(const char* serviceName, const char* objectPath, const char* method);
        DBusMessage* invokeMethod(DBusMessage* messageSend, const char* interface, const char* property, bool value = false);

        bool getWiFiStatus();
        bool getBTStatus();

        DBusConnection* mConnection = nullptr;
    };

#endif // NETWORK_PROVIDER