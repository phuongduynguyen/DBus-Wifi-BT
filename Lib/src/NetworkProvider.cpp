#include "NetworkProvider.h"

static void dumpDbusMessage(DBusMessage* msg) {
    if (!msg) {
        std::cerr << "Message is null." << std::endl;
        return;
    }

    // Get message type
    int type = dbus_message_get_type(msg);
    std::cout << "Message Type: " << type << std::endl;

    // Get object path
    const char* path = dbus_message_get_path(msg);
    if (nullptr != path) {
        std::cout << "Object Path: " << path << std::endl;
    } 
    else {
        std::cout << "Object Path is NULL" << std::endl;
    }

    // Get interface
    const char* iface = dbus_message_get_interface(msg);
    if (nullptr != iface) {
        std::cout << "Interface: " << iface << std::endl;
    } 
    else {
        std::cout << "Interface is NULL" << std::endl;
    }

    // Get method/member
    const char* member = dbus_message_get_member(msg);
    if (nullptr != member) {
        std::cout << "Member: " << member << std::endl;
    } 
    else {
        std::cout << "Member is NULL" << std::endl;
    }

    // Get sender
    const char* sender = dbus_message_get_sender(msg);
    if (nullptr != sender) {
        std::cout << "Sender: " << sender << std::endl;
    } 
    else {
        std::cout << "Sender is NULL" << std::endl;
    }

    // Get destination
    const char* destination = dbus_message_get_destination(msg);
    if (nullptr != destination) {
        std::cout << "Destination: " << destination << std::endl;
    } 
    else {
        std::cout << "Destination is NULL" << std::endl;
    }

    // Get serial number of the message
    unsigned int serial = dbus_message_get_serial(msg);
    std::cout << "Serial: " << serial << std::endl;

    // Get reply serial if it's a reply message
    unsigned int reply_serial = dbus_message_get_reply_serial(msg);
    if (reply_serial != 0) {
        std::cout << "Reply Serial: " << reply_serial << std::endl;
    } 
    else {
        std::cout << "Reply Serial is not applicable." << std::endl;
    }

    // Get error name (for error messages)
    const char* errorName = dbus_message_get_error_name(msg);
    if (nullptr != errorName) {
        std::cout << "Error Name: " << errorName << std::endl;
    } 
    else {
        std::cout << "Error Name is not applicable." << std::endl;
    }

    // Get signature of the message
    const char* signature = dbus_message_get_signature(msg);
    if (nullptr != signature) {
        std::cout << "Signature: " << signature << std::endl;
    } 
    else {
        std::cout << "Signature is NULL" << std::endl;
    }

    // Dump message arguments
    DBusMessageIter args;
    if (dbus_message_iter_init(msg, &args)) {
        std::cout << "Arguments: ";
        do {
            int argType = dbus_message_iter_get_arg_type(&args);
            if (argType == DBUS_TYPE_STRING) {
                const char* arg;
                dbus_message_iter_get_basic(&args, &arg);
                std::cout << arg << " ";
            } 
            else if (argType == DBUS_TYPE_INT32) {
                int arg;
                dbus_message_iter_get_basic(&args, &arg);
                std::cout << arg << " ";
            } 
            else if (argType == DBUS_TYPE_BOOLEAN) {
                dbus_bool_t arg;
                dbus_message_iter_get_basic(&args, &arg);
                std::cout << (arg ? "true" : "false") << " ";
            }
            else if (argType == DBUS_TYPE_ARRAY) {
                DBusMessageIter sub_iter;
                dbus_message_iter_recurse(&args, &sub_iter);
                std::cout << "[ ";
                while (dbus_message_iter_get_arg_type(&sub_iter) != DBUS_TYPE_INVALID) {
                    int arrayElemType = dbus_message_iter_get_arg_type(&sub_iter);
                    if (arrayElemType == DBUS_TYPE_STRING) {
                        const char* str_value;
                        dbus_message_iter_get_basic(&sub_iter, &str_value);
                        std::cout << str_value << " ";
                    } 
                    else if (arrayElemType == DBUS_TYPE_INT32) {
                        int int_value;
                        dbus_message_iter_get_basic(&sub_iter, &int_value);
                        std::cout << int_value << " ";
                    } 
                    else if (arrayElemType == DBUS_TYPE_BOOLEAN) {
                        dbus_bool_t bool_value;
                        dbus_message_iter_get_basic(&sub_iter, &bool_value);
                        std::cout << (bool_value ? "true" : "false") << " ";
                    } 
                    else {
                        std::cout << "Unsupported element type in array: " << arrayElemType << " ";
                    }
                    dbus_message_iter_next(&sub_iter);
                }
                std::cout << "] ";
            }
            else {
                std::cout << "Unsupported argument type: " << argType << " ";
            }
        } while (dbus_message_iter_next(&args));
        std::cout << std::endl;
    } else {
        std::cout << "No arguments in message." << std::endl;
    }
    std::cout << "\n=============================================================\n";
}

static void dumDbusMessageIter(DBusMessageIter* iter) {
    if (!iter) {
        std::cerr << "DBusMessageIter is null." << std::endl;
        return;
    }

    int arg_type;
    while ((arg_type = dbus_message_iter_get_arg_type(iter)) != DBUS_TYPE_INVALID) {
        switch (arg_type) {
            case DBUS_TYPE_STRING: {
                const char* str_value;
                dbus_message_iter_get_basic(iter, &str_value);
                std::cout << "String: " << str_value << std::endl;
                break;
            }
            case DBUS_TYPE_INT32: {
                int32_t int_value;
                dbus_message_iter_get_basic(iter, &int_value);
                std::cout << "Int32: " << int_value << std::endl;
                break;
            }
            case DBUS_TYPE_UINT32: {
                uint32_t uint_value;
                dbus_message_iter_get_basic(iter, &uint_value);
                std::cout << "UInt32: " << uint_value << std::endl;
                break;
            }
            case DBUS_TYPE_BOOLEAN: {
                dbus_bool_t bool_value;
                dbus_message_iter_get_basic(iter, &bool_value);
                std::cout << "Boolean: " << (bool_value ? "true" : "false") << std::endl;
                break;
            }
            case DBUS_TYPE_DOUBLE: {
                double double_value;
                dbus_message_iter_get_basic(iter, &double_value);
                std::cout << "Double: " << double_value << std::endl;
                break;
            }
            case DBUS_TYPE_ARRAY: {
                DBusMessageIter sub_iter;
                dbus_message_iter_recurse(iter, &sub_iter);
                std::cout << "Array: [" << std::endl;
                dumDbusMessageIter(&sub_iter);
                std::cout << "]" << std::endl;
                break;
            }
            case DBUS_TYPE_STRUCT: {
                DBusMessageIter sub_iter;
                dbus_message_iter_recurse(iter, &sub_iter);
                std::cout << "Struct: {" << std::endl;
                dumDbusMessageIter(&sub_iter);
                std::cout << "}" << std::endl;
                break;
            }
            case DBUS_TYPE_VARIANT: {
                DBusMessageIter sub_iter;
                dbus_message_iter_recurse(iter, &sub_iter);
                std::cout << "Variant: (" << std::endl;
                dumDbusMessageIter(&sub_iter);
                std::cout << ")" << std::endl;
                break;
            }
            case DBUS_TYPE_DICT_ENTRY: {
                DBusMessageIter dict_iter;
                dbus_message_iter_recurse(iter, &dict_iter);
                std::cout << "Dict Entry: {" << std::endl;
                
                int key_type = dbus_message_iter_get_arg_type(&dict_iter);
                if (key_type != DBUS_TYPE_INVALID) {
                    std::cout << "  Key: ";
                    dumDbusMessageIter(&dict_iter);
                }

                if (dbus_message_iter_next(&dict_iter)) {
                    std::cout << "  Value: ";
                    dumDbusMessageIter(&dict_iter);
                }
                
                std::cout << "}" << std::endl;
                break;
            }
            default:
                std::cout << "Unsupported type: " << arg_type << std::endl;
                break;
        }
        dbus_message_iter_next(iter);  // Chuyển sang tham số tiếp theo
    }
}

static NetworkProvider* gInstance = nullptr;

NetworkProvider& NetworkProvider::initialize() {
    if (nullptr == gInstance) {
        gInstance = new NetworkProvider();
    }
    return *gInstance;    
}

NetworkProvider& NetworkProvider::getInstance() {
    if (nullptr == gInstance) {
        throw std::runtime_error("NetworkProvider must initialize first");
    }
    return *gInstance;    
}

NetworkProvider::NetworkProvider() {
    if(!doInit()) {
        throw std::runtime_error("Initialize failed");
    }
}

NetworkProvider::~NetworkProvider()
{
    if (nullptr == mWorkerThread) {
        mWorkerThread->join();
        delete mWorkerThread;
        mWorkerThread = nullptr;
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

        // mWorkerThread = new std::thread(std::bind(&NetworkProvider::signalHandler, this));
        BluetoothAdapter::initialize(*this);

    } while (0);

    return ret;
}

void NetworkProvider::signalHandler()
{
    std::string introspect = "type='signal',interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'";
    DBusError error;
    DBusMessage *message = nullptr;

    dbus_error_init(&error);
    dbus_bus_add_match(mConnection, introspect.c_str(), &error);
    dbus_connection_flush(mConnection);

    if (dbus_error_is_set(&error)) {
        std::cerr << "Error adding match: " << error.message << std::endl;
        dbus_error_free(&error);
        return;
    }
    std::cout << "Listening for Bluetooth power changes..." << std::endl;

    while (true)
    {
        dbus_connection_read_write(mConnection, 0);
        message = dbus_connection_pop_message(mConnection);
        if (nullptr == message) {
            continue;
        }

        dumpDbusMessage(message);

        if (!dbus_message_is_signal(message, G_INTERFACE_DBUS_PROP, G_SIGNAL_PROPERTIES_CHANGED)) {
            continue;
        }
        
        DBusMessageIter args;
        dbus_message_iter_init(message, &args);
        const char *interface;

        if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) {
            continue;
        }
                
        dbus_message_iter_get_basic(&args, &interface);

        if (std::string(interface) != std::string(G_BT_ADAPTER_INTERFACE)) {
            continue;
        }

        dbus_message_iter_next(&args);

        if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY) {
            continue;
        }

        DBusMessageIter dictIter;
        dbus_message_iter_recurse(&args, &dictIter);
        while (dbus_message_iter_get_arg_type(&dictIter) == DBUS_TYPE_DICT_ENTRY) 
        {

            DBusMessageIter entryIter;
            dbus_message_iter_recurse(&dictIter, &entryIter);
            const char *propertyName;

            if (dbus_message_iter_get_arg_type(&entryIter) != DBUS_TYPE_STRING) {
                goto NextIter;
            }
                
            dbus_message_iter_get_basic(&entryIter, &propertyName);

            if (dbus_message_iter_get_arg_type(&entryIter) != DBUS_TYPE_STRING) {
                goto NextIter;
            }

            if (std::string(propertyName) != std::string(G_METHOD_POWERED_PROP)) {
                goto NextIter;
            }
                    
            dbus_message_iter_next(&entryIter);


            if (dbus_message_iter_get_arg_type(&entryIter) != DBUS_TYPE_VARIANT) {
                goto NextIter;
            }

            DBusMessageIter valueIter;
            dbus_message_iter_recurse(&entryIter, &valueIter);
            bool powered;

            if (dbus_message_iter_get_arg_type(&valueIter) != DBUS_TYPE_BOOLEAN) {
                goto NextIter;
            }

            dbus_message_iter_get_basic(&valueIter, &powered);
            std::cout << "\nBluetooth state: " << (powered ? "ON" : "OFF") << std::endl;

NextIter:
            dbus_message_iter_next(&dictIter);

        }
    }

    if (nullptr == message) {
        dbus_message_unref(message);
    }
}

DBusMessage* NetworkProvider::createMethod(const char* serviceName, const char* objectPath, const char* interface, const char* method)
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
            interface,                          // Interface Dbus
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

        if (std::string(dbus_message_get_interface(messageSend)) == G_BT_ADAPTER_INTERFACE) {
            goto Sending;
        }
        
        dbus_message_iter_init_append(messageSend, &iter);
        if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface)) {
            std::cerr << "createMethod but out of memory for interface!\n";
            break;
        }

        if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property)) {
            std::cerr << "createMethod but out of memory for property!\n";
            break;
        }
        
        if (dbus_message_is_method_call(messageSend,G_INTERFACE_DBUS_PROP,G_METHOD_SET)) {  
            dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "b", &variant);
            if (!dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &valueSend)) {
                std::cerr << "Out of memory!" << std::endl;
                break;
            }
            dbus_message_iter_close_container(&iter, &variant);
        }

Sending:
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
    bool networkStatus = false;
    DBusMessage* messageSend = nullptr;
    DBusMessage* messageReply = nullptr;

    switch (type)
    {
        case NetworkType::Wifi: {
            networkStatus = getWiFiStatus();
            messageSend = createMethod(G_NM_DBUS_SERVICE, G_NM_DBUS_PATH, G_INTERFACE_DBUS_PROP, G_METHOD_SET);
            messageReply = invokeMethod(messageSend, G_NM_DBUS_INTERFACE, G_METHOD_WIRELESS_ENABLED, !networkStatus);
            break;
        }

        case NetworkType::Bluetooth: {
            BluetoothAdapter::getInstance().toggleBluetoothPower();
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

void NetworkProvider::setScanMode(bool isScan)
{
    if (isScan) {
        BluetoothAdapter::getInstance().startDiscovery();
    }
    else {
        BluetoothAdapter::getInstance().stopDiscovery();
    } 
}

bool NetworkProvider::getWiFiStatus()
{
    DBusMessageIter iter;
    DBusMessageIter variant;
    DBusMessage* messageSend = nullptr;
    DBusMessage* messageReply = nullptr;

    /**
     * Note: Do not use bool type, DBus writes 4 bytes to a bool, 
     *       which only expects 1 byte. This can overwrite adjacent memory, 
     *       leading to corruption and eventually a crash.
     */
    dbus_bool_t ret = FALSE;
    do
    {
        if (nullptr == mConnection) {
            std::cout << "getWiFiStatus but empty connection\n";
            break;
        }
        messageSend = createMethod(G_NM_DBUS_SERVICE, G_NM_DBUS_PATH, G_INTERFACE_DBUS_PROP, G_METHOD_GET);
        if (nullptr == messageSend) {
            std::cerr << "getWiFiStatus but messageSend creation failed\n" ;
            break;
        }
        messageReply = invokeMethod(messageSend,G_NM_DBUS_INTERFACE,G_METHOD_WIRELESS_ENABLED);
        if (!dbus_message_iter_init(messageReply, &iter)) {
            std::cerr << "getWiFiStatus but init message reply failed\n" ;
            break;
        }
        if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_VARIANT) {
            dbus_message_iter_recurse(&iter, &variant);
            if (dbus_message_iter_get_arg_type(&variant) == DBUS_TYPE_BOOLEAN) {
                dbus_message_iter_get_basic(&variant, &ret);
            }
        }

    } while (0);

    if (nullptr != messageSend) {
        dbus_message_unref(messageSend);
    }

    if (nullptr != messageReply) {
        dbus_message_unref(messageReply);
    }
    return static_cast<bool>(ret);
}

bool NetworkProvider::getBTStatus()
{
    return BluetoothAdapter::getInstance().getBluetoothPower();
}

std::string NetworkProvider::getBluetoothName() const
{
    return BluetoothAdapter::getInstance().getBluetoothName();
}

std::string NetworkProvider::getBluetoothAddress() const
{
    return BluetoothAdapter::getInstance().getBluetoothAddress();
}

void NetworkProvider::dumpBluetoothDevices()
{
    BluetoothAdapter::getInstance().dumpDevicesUnpaired();
}







