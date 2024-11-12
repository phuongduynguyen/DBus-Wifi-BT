#include "BluetoothManager.h"
#include "NetworkProvider.h"
#include <locale>
#include <regex>
#include <unistd.h>
#include <variant> 

static BluetoothAdapter* gInstance = nullptr;

std::unordered_map<std::string, std::string> BluetoothAdapter::gProfileMap = {
                                                                                {"00001200-0000-1000-8000-00805f9b34fb", "PnP"}, // Plug and Play
                                                                                {"0000111f-0000-1000-8000-00805f9b34fb", "HFP" }, // Handfree profile
                                                                                {"0000112f-0000-1000-8000-00805f9b34fb", "PBAP"}, // Phone Book Access Profile
                                                                                {"0000110a-0000-1000-8000-00805f9b34fb", "Audio-Source"},
                                                                                {"0000110b-0000-1000-8000-00805f9b34fb", "Audio-Sink"},
                                                                                {"0000110e-0000-1000-8000-00805f9b34fb", "A2DP-Source"},
                                                                                {"0000110c-0000-1000-8000-00805f9b34fb", "A2DP-Sink"},
                                                                                {"0000110d-0000-1000-8000-00805f9b34fb", "A2DP"}, // Advanced Audio Distribution Profile
                                                                                {"00001116-0000-1000-8000-00805f9b34fb", "HSP"}, // Headset Profile
                                                                                {"00001132-0000-1000-8000-00805f9b34fb", "MAP"}, // Message Access Profile
                                                                                {"00001801-0000-1000-8000-00805f9b34fb", "GATT"}, // Generic Attribute Profile
                                                                                {"00000000-deca-fade-deca-deafdecacafe", "Custom-Profile"},
                                                                                {"02030302-1d19-415f-86f2-22a2106a0a77", "Custom-UUID-1"},
                                                                                {"2d8d2466-e14d-451c-88bc-7301abea291a", "Custom-UUID-2"},
                                                                                {"00000000-deca-fade-deca-deafdecacafe", "iAP"}, // Apple's Internet Accessory Protocol
                                                                                {"2d8d2466-e14d-451c-88bc-7301abea291a", "Carplay"},
                                                                                {"02030302-1d19-415f-86f2-22a2106a0a77", "iAP-v2"},
                                                                                {"02030302-1d19-415f-86f2-22a2106a0a77", "iAP-v2"},
                                                                                {"1ff31936-572e-4b36-a2bf-b2409b1aa6f4", "MAP-Apple"},
                                                                                {"00001000-0000-1000-8000-00805f9b34fb", "Discovery-Server-Service-Class-ID"},
                                                                                {"00001800-0000-1000-8000-00805f9b34fb", "GAP"}, // Generic Access Profile
                                                                                {"0000180a-0000-1000-8000-00805f9b34fb", "DIS"}, // Device Information Service
                                                                                {"9fa480e0-4967-4542-9390-d343dc5d04ae", "TDS"}, // Transport Discovery Service
                                                                                {"d0611e78-bbb4-4591-a5f8-487910ae4366", "AMS"}, // Apple Media Service 
                                                                            };


BluetoothAdapter& BluetoothAdapter::initialize(NetworkProvider& network)
{
    if (nullptr == gInstance) {
        gInstance = new BluetoothAdapter(network);
    }
    return *gInstance;
}

BluetoothAdapter& BluetoothAdapter::getInstance()
{
    if (nullptr == gInstance) {
        throw std::runtime_error("BluetoothAdapter must initialize first");
    }
    return *gInstance;
}

std::string BluetoothAdapter::getProfile(const char* uuid)
{
    std::string ret = std::string(uuid);
    std::unordered_map<std::string, std::string>::iterator foundItem = gProfileMap.begin();
    while (foundItem != gProfileMap.end())
    {
        if (std::string(uuid) == foundItem->first)  {
            ret = foundItem->second;
            break;
        }
        foundItem++;
    }
    return ret;
}

BluetoothAdapter::BluetoothAdapter(NetworkProvider& network) : mNetwork(network), mDiscovering(false), mDiscoveringThread(nullptr)
{
    mDiscoveringThread = new std::thread(std::bind(&BluetoothAdapter::discoveringHandler, this));
    
}

void BluetoothAdapter::startDiscovery()
{
    DBusMessage *message = nullptr;
    DBusMessage *reply = nullptr;
    DBusError err;

    if (nullptr == mNetwork.mConnection) {
        std::cout << "startDiscovery but not establish connection\n";
        return;
    }

    if (mDiscovering) {
        std::cout << "startDiscovery but already scan\n";
        return;
    }
    
    {
        std::unique_lock<std::shared_mutex> lock(mMutex);
        {
            std::lock_guard<std::mutex> lock(mDiscoveringMutex);
            mDiscovering = true;
            mDiscoveringCV.notify_all();
        }
        dbus_error_init(&err);
        message = mNetwork.createMethod(G_BT_SERVICE_NAME,G_BT_OBJECT_PATH, G_BT_ADAPTER_INTERFACE, G_METHOD_START_DISCOVERY);

        if (nullptr == message) {
            std::cerr << "Message is NULL\n";
            return;
        }

        reply = dbus_connection_send_with_reply_and_block(mNetwork.mConnection, message, -1, &err);
        dbus_message_unref(message);

        if (dbus_error_is_set(&err)) {
            std::cerr << "Error starting discovery: " << err.message << std::endl;
            dbus_error_free(&err);
            return;
        }
        dbus_message_unref(reply);
    }

    std::cout << "Started Bluetooth discovery..." << std::endl;
}

void BluetoothAdapter::stopDiscovery()
{
    DBusMessage *message = nullptr;
    DBusMessage *reply = nullptr;
    DBusError err;

    if (nullptr == mNetwork.mConnection) {
        std::cout << "stopDiscovery but not establish connection\n";
        return;
    }

    if (!mDiscovering) {
        std::cout << "stopDiscovery but already stopped\n";
        return;
    }

    {
        std::unique_lock<std::shared_mutex> lock(mMutex);
        {
            std::lock_guard<std::mutex> lock(mDiscoveringMutex);
            mDiscovering = false;
            mDiscoveringCV.notify_all();
        }

        dbus_error_init(&err);
        message = mNetwork.createMethod(G_BT_SERVICE_NAME,G_BT_OBJECT_PATH, G_BT_ADAPTER_INTERFACE, G_METHOD_STOP_DISCOVERY);
        if (nullptr == message) {
            std::cerr << "Message is NULL\n";
            return;
        }

        reply = dbus_connection_send_with_reply_and_block(mNetwork.mConnection, message, -1, &err);
        dbus_message_unref(message);

        if (dbus_error_is_set(&err)) {
            std::cerr << "Error starting discovery: " << err.message << std::endl;
            dbus_error_free(&err);
            return;
        }

        dbus_message_unref(reply);
    }
    std::cout << "Stop Bluetooth discovery\n";
}

void BluetoothAdapter::toggleBluetoothPower()
{
    DBusMessage* messageSend = nullptr;
    DBusMessage* messageReply = nullptr;
    bool networkStatus = getBluetoothPower();
    messageSend = mNetwork.createMethod(G_BT_SERVICE_NAME, G_BT_OBJECT_PATH, G_INTERFACE_DBUS_PROP , G_METHOD_SET);
    messageReply = mNetwork.invokeMethod(messageSend, G_BT_ADAPTER_INTERFACE, G_METHOD_POWERED_PROP, !networkStatus);
    if (nullptr != messageReply) {
        dbus_message_unref(messageReply);
    }
    if (nullptr != messageSend) {
        dbus_message_unref(messageSend);
    }   
}

bool BluetoothAdapter::getBluetoothPower() const
{
    DBusMessageIter iter;
    DBusMessageIter variant;
    DBusMessage* messageSend = nullptr;
    DBusMessage* messageReply = nullptr;   
    dbus_bool_t ret = FALSE;

    do
    {
        if (nullptr == mNetwork.mConnection) {
            std::cout << "getBTStatus but empty connection\n";
            break;
        }
        messageSend = mNetwork.createMethod(G_BT_SERVICE_NAME, G_BT_OBJECT_PATH, G_INTERFACE_DBUS_PROP, G_METHOD_GET);
        if (nullptr == messageSend) {
            std::cerr << "getBTStatus but messageSend creation failed\n" ;
            break;
        }
        messageReply = mNetwork.invokeMethod(messageSend,G_BT_ADAPTER_INTERFACE,G_METHOD_POWERED_PROP);
        if (!dbus_message_iter_init(messageReply, &iter)) {
            std::cerr << "getBTStatus but init message reply failed\n" ;
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

std::string BluetoothAdapter::getBluetoothName() const
{
    DBusError error;
    dbus_error_init(&error);

    DBusMessage* message = mNetwork.createMethod(G_BT_SERVICE_NAME, G_BT_OBJECT_PATH, G_INTERFACE_DBUS_PROP, G_METHOD_GET);
    
    if (nullptr == message) {
        std::cerr << "Message is NULL\n";
        return "";
    }

    dbus_message_append_args(message,
                             DBUS_TYPE_STRING, &G_BT_ADAPTER_INTERFACE,
                             DBUS_TYPE_STRING, &G_ALIAS,
                             DBUS_TYPE_INVALID);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(mNetwork.mConnection, message, -1, &error);
    dbus_message_unref(message);
        
    DBusMessageIter args;
    if (!dbus_message_iter_init(reply, &args)) {
        std::cerr << "Reply has no arguments." << std::endl;
    } else if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_VARIANT) {
        DBusMessageIter variant;
        dbus_message_iter_recurse(&args, &variant);

        const char* bluetoothName = nullptr;
        if (dbus_message_iter_get_arg_type(&variant) == DBUS_TYPE_STRING) {
            dbus_message_iter_get_basic(&variant, &bluetoothName);
        }

        dbus_message_unref(reply);
        return bluetoothName ? std::string(bluetoothName) : "";
    }

    dbus_message_unref(reply);
    return "";

}

std::string BluetoothAdapter::getBluetoothAddress() const
{
    DBusMessage *message = nullptr;
    DBusMessage *reply = nullptr;
    DBusError error;
    if (nullptr == mNetwork.mConnection) {
        std::cout << "getBluetoothAddress but not establish connection\n";
        return "";
    }

    {
        std::unique_lock<std::shared_mutex> lock(mMutex);
        dbus_error_init(&error);
        DBusMessage* message = mNetwork.createMethod(G_BT_SERVICE_NAME, G_BT_OBJECT_PATH, G_INTERFACE_DBUS_PROP, G_METHOD_GET);
        
        if (nullptr == message) {
            std::cerr << "Message is NULL\n";
            return "";
        }
        
        dbus_message_append_args(message,
                                DBUS_TYPE_STRING, &G_BT_ADAPTER_INTERFACE,
                                DBUS_TYPE_STRING, &G_METHOD_GET_ADDRESS,
                                DBUS_TYPE_INVALID);

        DBusMessage* reply = dbus_connection_send_with_reply_and_block(mNetwork.mConnection, message, -1, &error);
        dbus_message_unref(message);

        if (dbus_error_is_set(&error)) {
            std::cerr << "Error getBluetoothAddress: " << error.message << std::endl;
            dbus_error_free(&error);
            return "";
        }
        
        DBusMessageIter args;
        if (!dbus_message_iter_init(reply, &args)) {
            std::cerr << "Reply has no arguments." << std::endl;
        } else if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_VARIANT) {
            DBusMessageIter variant;
            dbus_message_iter_recurse(&args, &variant);

            const char* bluetoothAddress = nullptr;
            if (dbus_message_iter_get_arg_type(&variant) == DBUS_TYPE_STRING) {
                dbus_message_iter_get_basic(&variant, &bluetoothAddress);
            }

            dbus_message_unref(reply);
            return bluetoothAddress ? std::string(bluetoothAddress) : "";
        }

        dbus_message_unref(reply);
        return "";
    }
}


std::vector<std::shared_ptr<BluetoothDevice>> BluetoothAdapter::getBondedDevices() const
{
    std::vector<std::shared_ptr<BluetoothDevice>> ret;
    return ret;
}

void BluetoothAdapter::discoveringHandler()
{

    static std::function<void(DBusConnection*, const char*)> getDeviceInfo = [this](DBusConnection *conn, const char* device_path) {
        DBusMessage* message;
        DBusMessage* reply;
        DBusError err;
        std::string ret = "";
        DBusMessageIter iter, dict_entry_iter, dict_value_iter;
        
        dbus_error_init(&err);

        message = mNetwork.createMethod(G_BT_SERVICE_NAME, device_path, G_INTERFACE_DBUS_PROP, G_METHOD_GET_ALL);

        dbus_message_append_args(message, DBUS_TYPE_STRING, &G_BT_INTERFACE_DEVICE1, DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(conn, message, -1, &err);
        dbus_message_unref(message);

        if (dbus_error_is_set(&err)) {
            std::cerr << "Error getting properties: " << err.message << std::endl;
            dbus_error_free(&err);
            return;
        }

        if (nullptr != reply) {
            if (dbus_message_iter_init(reply, &iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY) {
                dbus_message_iter_recurse(&iter, &dict_entry_iter);
                while (dbus_message_iter_get_arg_type(&dict_entry_iter) == DBUS_TYPE_DICT_ENTRY) {
                    DBusMessageIter dict_entry_key_iter, dict_entry_value_iter;
                    dbus_message_iter_recurse(&dict_entry_iter, &dict_entry_key_iter);
                    const char* property_name;
                    dbus_message_iter_get_basic(&dict_entry_key_iter, &property_name);
                    // std::cout << "- Property : " << std::string(property_name) << "\n";
                    if (std::string(property_name) == "Name") {
                        dbus_message_iter_next(&dict_entry_key_iter);
                        dbus_message_iter_recurse(&dict_entry_key_iter, &dict_entry_value_iter);

                        const char* device_name;
                        dbus_message_iter_get_basic(&dict_entry_value_iter, &device_name);
                        std::cout << "Device Name: " << device_name << std::endl;
                        // break;
                    }
                    else if (std::string(property_name) == "Address") {
                        dbus_message_iter_next(&dict_entry_key_iter);
                        dbus_message_iter_recurse(&dict_entry_key_iter, &dict_entry_value_iter);

                        const char* device_name;
                        dbus_message_iter_get_basic(&dict_entry_value_iter, &device_name);
                        std::cout << "Device address: " << device_name << std::endl;
                    }
                    else if (std::string(property_name) == "Paired")
                    {
                        dbus_bool_t ret = FALSE;
                        dbus_message_iter_next(&dict_entry_key_iter);
                        dbus_message_iter_recurse(&dict_entry_key_iter, &dict_entry_value_iter);

                        dbus_message_iter_get_basic(&dict_entry_value_iter, &ret);
                        std::cout << "Device Paired: " << ret << std::endl;
                    }
                    else if (std::string(property_name) == "Bonded")
                    {
                        dbus_bool_t ret = FALSE;
                        dbus_message_iter_next(&dict_entry_key_iter);
                        dbus_message_iter_recurse(&dict_entry_key_iter, &dict_entry_value_iter);

                        dbus_message_iter_get_basic(&dict_entry_value_iter, &ret);
                        std::cout << "Device Bonded: " << ret << std::endl;
                    }
                    else if (std::string(property_name) == "Connected")
                    {
                        dbus_bool_t ret = FALSE;
                        dbus_message_iter_next(&dict_entry_key_iter);
                        dbus_message_iter_recurse(&dict_entry_key_iter, &dict_entry_value_iter);

                        dbus_message_iter_get_basic(&dict_entry_value_iter, &ret);
                        std::cout << "Device Connected: " << ret << std::endl;
                    }
                    else if (std::string(property_name) == "UUIDs")
                    {
                        dbus_message_iter_next(&dict_entry_key_iter);
                        dbus_message_iter_recurse(&dict_entry_key_iter, &dict_entry_value_iter);
                        
                        if (dbus_message_iter_get_arg_type(&dict_entry_value_iter) == DBUS_TYPE_ARRAY) {
                            DBusMessageIter uuid_iter;
                            dbus_message_iter_recurse(&dict_entry_value_iter, &uuid_iter);
                            std::cout << "UUIDs: " ;
                            while (dbus_message_iter_get_arg_type(&uuid_iter) == DBUS_TYPE_STRING) {
                                const char* uuid;
                                dbus_message_iter_get_basic(&uuid_iter, &uuid);
                                std::cout << BluetoothAdapter::getProfile(uuid) << " | ";
                                dbus_message_iter_next(&uuid_iter);
                            }
                        }       
                        std::cout << std::endl;
                    }            
                    else if (std::string(property_name) == "Adapter") {
                        dbus_message_iter_next(&dict_entry_key_iter);
                        dbus_message_iter_recurse(&dict_entry_key_iter, &dict_entry_value_iter);
                        const char* adapter_path;
                        dbus_message_iter_get_basic(&dict_entry_value_iter, &adapter_path);
                        std::cout << "Adapter Path: " << adapter_path << std::endl;
                    }
                    else {

                    }
                    dbus_message_iter_next(&dict_entry_iter);
                }
            } else {
                std::cerr << "Failed to initialize iterator or invalid response format\n";
            }
            dbus_message_unref(reply);
        } else {
            std::cerr << "Failed to get properties\n";
        }
    };

    DBusMessage *message;
    DBusError err;
    dbus_error_init(&err);
    dbus_bus_add_match(mNetwork.mConnection, "type='signal',interface='org.freedesktop.DBus.ObjectManager',member='InterfacesAdded'", &err);
    dbus_bus_add_match(mNetwork.mConnection, "type='signal',interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'", &err);
    dbus_connection_flush(mNetwork.mConnection);
    
    if (dbus_error_is_set(&err)) {
        std::cerr << "Match rule error: " << err.message << std::endl;
        dbus_error_free(&err);
        return;
    }

    while (true) {
        {
            std::unique_lock<std::mutex> lock(mDiscoveringMutex);
            mDiscoveringCV.wait(lock, [&]{
                return mDiscovering;
            });
        }

        dbus_connection_read_write(mNetwork.mConnection, 0);
        message = dbus_connection_pop_message(mNetwork.mConnection);

        if (message != nullptr) {
            if (dbus_message_is_signal(message, "org.freedesktop.DBus.ObjectManager", "InterfacesAdded")) {
                DBusMessageIter args;
                dbus_message_iter_init(message, &args);

                if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_OBJECT_PATH) {
                    const char *device_path;
                    dbus_message_iter_get_basic(&args, &device_path);
                    std::cout << "\nDevice found: " << device_path << std::endl;
                    static std::regex pattern("/org/bluez/");
                    if (!std::regex_search(std::string(device_path), pattern)) {
                        std::cout << "\nDevice found: " << device_path << " not Bluetooth device "<< std::endl;
                        goto Unref;
                    }

                    getDeviceInfo(mNetwork.mConnection, device_path);
                }
            } else if (dbus_message_is_signal(message, G_INTERFACE_DBUS_PROP, G_SIGNAL_PROPERTIES_CHANGED)) {
                DBusMessageIter args;
                dbus_message_iter_init(message, &args);

                const char *interface_name;
                dbus_message_iter_get_basic(&args, &interface_name);

                if (0 == strcmp(interface_name, G_BT_INTERFACE_DEVICE1)) {
                    std::cout << "Device properties changed!" << std::endl;
                    const char *device_path = dbus_message_get_path(message);
                    getDeviceInfo(mNetwork.mConnection, device_path);
                }
            }
Unref:
            dbus_message_unref(message);

        } else {
            usleep(200000);
        }
    }
}

/*========================================================================================================*/

BluetoothDevice::BluetoothDevice(BluetoothAdapter& adapter, const std::string& deviceName, const std::string& deviceAddress, const std::string& devicePath ,const std::vector<std::string>& uuids) : mAdapter(adapter), 
                                                                                                                                                                                                     mDeviceName(deviceName),
                                                                                                                                                                                                     mDeviceAddress(deviceAddress),
                                                                                                                                                                                                     mDevicePath(devicePath),
                                                                                                                                                                                                     mUUIDs(uuids)
{

}

std::string BluetoothDevice::getDeviceName() const
{
    return "";
}

std::string BluetoothDevice::getDeviceAddress() const
{
    return "";
}

std::string BluetoothDevice::getDevicePath() const
{
    return "";
}

int BluetoothDevice::getState() const
{
    return 0;
}

std::vector<std::string> BluetoothDevice::getUUIDs() const
{
    std::vector<std::string> ret;
    return ret;
};

void BluetoothDevice::createBond()
{

}

void BluetoothDevice::destroyBond()
{

}

void BluetoothDevice::connectProfile(const std::string& profile)
{

}


