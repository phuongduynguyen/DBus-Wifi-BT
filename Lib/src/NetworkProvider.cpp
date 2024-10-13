#include "NetworkProvider.h"


const char* NM_DBUS_SERVICE = "org.freedesktop.NetworkManager";
const char* NM_DBUS_PATH = "/org/freedesktop/NetworkManager";
const char* NM_DBUS_INTERFACE = "org.freedesktop.NetworkManager";
const char* INTERFACE_DBUS_PROP = "org.freedesktop.DBus.Properties";
const char* METHOD_GET = "Get";
const char* METHOD_SET = "Set";
const char* METHOD_SET_PROP = "SetProperty";
const char* METHOD_POWERED_PROP = "Powered";

const char* BT_SERVICE_NAME = "org.bluez";
const char* BT_OBJECT_PATH = "/org/bluez/hci0";
const char* BT_ADAPTER_INTERFACE = "org.bluez.Adapter1";



static NetworkProvider* gInstance = nullptr;

NetworkProvider& NetworkProvider::initialize() {
    if (gInstance == nullptr) {
        gInstance = new NetworkProvider();
    }
    return *gInstance;    
}

NetworkProvider& NetworkProvider::getInstance() {
    if (gInstance == nullptr) {
        throw std::runtime_error("Must initialize first");
    }
    return *gInstance;    
}

NetworkProvider::NetworkProvider() {
    if(!doInit()) {
        throw std::runtime_error("Initialize failed");
    }
}

bool NetworkProvider::doInit()
{
    bool ret = true;
    do
    {
        DBusError err;
        dbus_error_init(&err);
        mConnection = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
        if (dbus_error_is_set(&err)) {
            std::cerr << "Connection Error: " << err.message << std::endl;
            dbus_error_free(&err);
            ret = false;
            break;
        }
        if (nullptr == mConnection) {
            std::cerr << "Failed to connect to the D-Bus system bus." << std::endl;
            ret = false;
            break;        
        }
        
    } while (0);
    return ret;
}

DBusMessage* NetworkProvider::createMethod(const char* serviceName, const char* objectPath, const char* method)
{
    DBusMessage* message = nullptr;
    do
    {
        if ((nullptr == serviceName) || (nullptr == objectPath) || (nullptr == method)) {
            std::cout << "createMethod but empty parameter\n";
            break;
        }
        message = dbus_message_new_method_call(
            serviceName,                        // Service name
            objectPath,                         // object path 
            INTERFACE_DBUS_PROP,                // Interface Dbus property
            method                              // Method name
        );

    } while (0);
    
    return message;
}

DBusMessage* NetworkProvider::invokeMethod(DBusMessage* messageSend, const char* interface, const char* property, bool value)
{
    DBusMessageIter iter;
    DBusMessageIter variant;
    DBusError error;
    DBusMessage* message = nullptr;
    bool valueSend = value;
    do
    {
        if (nullptr == messageSend) {
            std::cout << "createMethod but empty parameter\n";
            break;
        }
        if (nullptr == mConnection) {
            std::cout << "createMethod but empty connection\n";
            break;
        }

        dbus_error_init(&error);

        dbus_message_iter_init_append(messageSend, &iter);
        if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface)) {
            std::cerr << "createMethod but out of memory for interface!\n";
            break;
        }

        if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property)) {
            std::cerr << "createMethod but out of memory for property!\n";
            break;
        }
        
        if (dbus_message_is_method_call(messageSend,INTERFACE_DBUS_PROP,METHOD_SET)) {  
            std::cout << "[dbus_message_is_method_call true\n";
            dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "b", &variant);
            if (!dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &valueSend)) {
                std::cerr << "Out of memory!" << std::endl;
                break;
            }
            dbus_message_iter_close_container(&iter, &variant);
        }
        else {
            std::cout << "[dbus_message_is_method_call false\n";
        }

        message = dbus_connection_send_with_reply_and_block(mConnection, messageSend, -1, &error);
        if (dbus_error_is_set(&error)) {
            std::cerr << "Error in reply: " << error.message << std::endl;
            dbus_error_free(&error);
            break;
        }
    } while (0);
    return message;
}

void NetworkProvider::toggleNetWork(const NetworkType& type)
{
    std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
    bool networkStatus = false;
    DBusMessage* messageSend = nullptr;
    DBusMessage* messageReply = nullptr;

    switch (type)
    {
        case NetworkType::Wifi: {

            break;
        }

        case NetworkType::Bluetooth: {
            networkStatus = getBTStatus();
            std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
            messageSend = createMethod(BT_SERVICE_NAME, BT_OBJECT_PATH, METHOD_SET);
            std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
            messageReply = invokeMethod(messageSend,BT_ADAPTER_INTERFACE, METHOD_POWERED_PROP, !networkStatus);
            std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
            break;
        }

        default: {
            return;
        }
    }
    if (nullptr != messageReply) {
        dbus_message_unref(messageReply);
    }
    if (nullptr != messageSend) {
        dbus_message_unref(messageSend);
    }   
}

bool NetworkProvider::getWiFiStatus()
{
    return false;
}

bool NetworkProvider::getBTStatus()
{
    std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
    DBusMessageIter iter;
    DBusMessageIter variant;
    DBusMessage* messageSend = nullptr;
    DBusMessage* messageReply = nullptr;   
    bool ret = false;
    
    do
    {
        std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
        if (nullptr == mConnection) {
            std::cout << "getBTStatus but empty connection\n";
            break;
        }
        messageSend = createMethod(BT_SERVICE_NAME, BT_OBJECT_PATH, METHOD_GET);
        if (nullptr == messageSend) {
            std::cerr << "getBTStatus but messageSend creation failed\n" ;
            break;
        }
        std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
        messageReply = invokeMethod(messageSend,BT_ADAPTER_INTERFACE,METHOD_POWERED_PROP);
        std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
        if (!dbus_message_iter_init(messageReply, &iter)) {
            std::cerr << "getBTStatus but init message reply failed\n" ;
            break;
        }
        std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
        if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_VARIANT) {
            dbus_message_iter_recurse(&iter, &variant);
            if (dbus_message_iter_get_arg_type(&variant) == DBUS_TYPE_BOOLEAN) {
                dbus_message_iter_get_basic(&variant, &ret);
            }
        }

    } while (0);
    std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
    // if (nullptr != messageSend) {
    //     dbus_message_unref(messageSend);
    // }
    std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
    // if (nullptr != messageReply) {
    //     dbus_message_unref(messageReply);
    // }
    std::cout << "[" << __PRETTY_FUNCTION__ << "] - " << " line: " << __LINE__ << "\n";
    return ret;
}





