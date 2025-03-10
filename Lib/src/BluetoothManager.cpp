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
                                                                                {"0000110e-0000-1000-8000-00805f9b34fb", "A2DP-Source"}, // A/V Remote Control
                                                                                {"0000110c-0000-1000-8000-00805f9b34fb", "A2DP-Sink"}, // A/V Remote Control Target
                                                                                {"0000110d-0000-1000-8000-00805f9b34fb", "A2DP"}, // Advanced Audio Distribution Profile
                                                                                {"00001116-0000-1000-8000-00805f9b34fb", "HSP"}, // Headset Profile
                                                                                {"00001108-0000-1000-8000-00805f9b34fb", "HSP-Audio-Headphones"},
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
                                                                                {"00001105-0000-1000-8000-00805f9b34fb", "OPP"}, // OBEX Object Push Profile
                                                                                {"00001112-0000-1000-8000-00805f9b34fb","HSP-AG"}, // Headset Profile (HSP) - Audio Gateway (AG)
                                                                                {"00001115-0000-1000-8000-00805f9b34fb", "PAN"}, // Personal Area Networking
                                                                                {"0000FE03-0000-1000-8000-00805F9B34FB", "VendorId - Baidu"},
                                                                                {"0000FDDF-0000-1000-8000-00805F9B34FB", "VendorId - Amazon"},
                                                                                {"00001203-0000-1000-8000-00805F9B34FB", "GAVDP"}, // eneric Audio/Video Distribution Profile
                                                                                {"0000110F-0000-1000-8000-00805F9B34FB", "AVRCP"}, // Audio/Video Remote Control Profile
                                                                                {"00001101-0000-1000-8000-00805f9b34fb", "Serial-Port"},
                                                                                {"00001124-0000-1000-8000-00805f9b34fb", "HID"}, // Human Interface Device
                                                                                {"0000180f-0000-1000-8000-00805f9b34fb", "BS"}, // Battery Service
                                                                                {"0000fd72-0000-1000-8000-00805f9b34fb", "LIS"}, // Logitech International SA
                                                                                {"00010000-0000-1000-8000-011f2000046d", "Logitech-Vendor-Id"},
                                                                                {"0000111e-0000-1000-8000-00805f9b34fb", "HFP-HV-SK579BT"},
                                                                                {"00001812-0000-1000-8000-00805f9b34fb","HID-Logitech"},
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

template<typename T>
std::string BluetoothAdapter::getProfile(const T& uuid)
{
    std::string ret;
    if constexpr (std::is_same_v<T, std::string>) {
        ret = uuid;
    }
    else if constexpr (std::is_same_v<T, const char*>) {
        ret = std::string(uuid);
    }
    else {
        // Do nothing
        return "";
    }

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

BluetoothAdapter::BluetoothAdapter(NetworkProvider& network) : mNetwork(network), mDiscovering(false), mDiscoveringThread(nullptr), mBluetoothActionThread(nullptr)
{
    std::function<std::optional<std::vector<std::string>>(DBusConnection*)> getPairedDevicesPath = [](DBusConnection* connection) -> std::optional<std::vector<std::string>> {
        std::vector<std::string> devicePaths;
        DBusError error;
        DBusMessage* message = dbus_message_new_method_call(
            "org.bluez",                  // Service name
            "/",                          // Object path
            "org.freedesktop.DBus.ObjectManager", // Interface
            "GetManagedObjects"           // Method
        );

        if (nullptr == message) {
            std::cerr << "Failed to create D-Bus message.\n";
            return {};
        }

        DBusMessage* reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (nullptr == reply) {
            std::cerr << "Failed to send D-Bus message.\n";
            return {};
        }
        
        DBusMessageIter iter;
        if (dbus_message_iter_init(reply, &iter)) {
            DBusMessageIter dict_entry;
            dbus_message_iter_recurse(&iter, &dict_entry);

            while (dbus_message_iter_get_arg_type(&dict_entry) != DBUS_TYPE_INVALID) {
                const char* objectPath;
                dbus_message_iter_recurse(&dict_entry, &iter);
                dbus_message_iter_get_basic(&iter, &objectPath);
                static std::regex pattern("^/org/bluez/hci0/dev_[^/]*$");
                if (std::regex_search(std::string(objectPath), pattern)) {
                    devicePaths.emplace_back(objectPath);
                }
                dbus_message_iter_next(&dict_entry);
            }
        }
        dbus_message_unref(message);
        dbus_message_unref(reply);
        return devicePaths;
    };

    std::function<std::shared_ptr<BluetoothDevice>(DBusConnection*,const std::string&)> getDevice = [this](DBusConnection* connection, const std::string& devicePath) -> std::shared_ptr<BluetoothDevice> {
        DBusError error;
        dbus_error_init(&error);

        DBusMessage* message = dbus_message_new_method_call(
            "org.bluez",                  // Service name
            devicePath.c_str(),          // Object path
            "org.freedesktop.DBus.Properties", // Interface
            "GetAll"                      // Method
        );

        if (nullptr == message) {
            std::cerr << "Failed to create D-Bus message.\n";
            return nullptr;
        }

        const char* interface_name = "org.bluez.Device1";
        dbus_message_append_args(message, DBUS_TYPE_STRING, &interface_name, DBUS_TYPE_INVALID);

        DBusMessage* reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (dbus_error_is_set(&error)) {
            std::cerr << "Error getting device info: " << error.message << std::endl;
            dbus_error_free(&error);
            return nullptr;
        }
        std::string devName = "";
        std::string devAddress = "";
        std::vector<std::string> uuids;
        bool isConnected = false;
        bool isPaired = false;

        DBusMessageIter iter;
        if (dbus_message_iter_init(reply, &iter)) {
            DBusMessageIter dict;
            dbus_message_iter_recurse(&iter, &dict);

            while (dbus_message_iter_get_arg_type(&dict) != DBUS_TYPE_INVALID) {
                const char* key;
                dbus_message_iter_recurse(&dict, &iter);
                dbus_message_iter_get_basic(&iter, &key);
                dbus_message_iter_next(&iter);
                if (strcmp(key, "Name") == 0) {
                    const char* name;
                    dbus_message_iter_recurse(&iter, &iter);
                    dbus_message_iter_get_basic(&iter, &name);
                    devName = name ? std::string(name) : "Unknown";
                } else if (strcmp(key, "Address") == 0) {
                    const char* address;
                    dbus_message_iter_recurse(&iter, &iter);
                    dbus_message_iter_get_basic(&iter, &address);
                    devAddress = address ? std::string(address) : "Unknown";
                } else if (strcmp(key, "UUIDs") == 0) {
                    DBusMessageIter array;
                    dbus_message_iter_recurse(&iter, &array);
                    if (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_ARRAY) {
                        DBusMessageIter uuid_iter;
                        dbus_message_iter_recurse(&array, &uuid_iter);
                        while (dbus_message_iter_get_arg_type(&uuid_iter) == DBUS_TYPE_STRING) {
                            const char* uuid;
                            dbus_message_iter_get_basic(&uuid_iter, &uuid);
                            uuids.emplace_back(std::string(uuid));
                            dbus_message_iter_next(&uuid_iter);
                        }
                    }
                } else if (strcmp(key, "Connected") == 0) {
                    bool connected = false;
                    dbus_message_iter_recurse(&iter, &iter);
                    dbus_message_iter_get_basic(&iter, &connected);
                    isConnected = connected;
                }
                else if (strcmp(key, "Paired") == 0) {
                    bool connected = false;
                    dbus_message_iter_recurse(&iter, &iter);
                    dbus_message_iter_get_basic(&iter, &connected);
                }
                else {
                    // Do nothing
                }
                dbus_message_iter_next(&dict);
            }
        }
        
        std::shared_ptr<BluetoothDevice> device(new BluetoothDevice(*this,devName, devAddress, std::string(devicePath), uuids));
        device->setStatus((isConnected == true ? Status::Connected : Status::Disconnected));
        dbus_message_unref(message);
        dbus_message_unref(reply);
        return device;
    };

    std::once_flag init;
    std::call_once(init, [this, getPairedDevicesPath, getDevice](){
        std::optional<std::vector<std::string>> devicesPathOpt = getPairedDevicesPath(mNetwork.mConnection);
        if (devicesPathOpt.has_value()) {
            std::vector<std::string> devicesPath = devicesPathOpt.value();
            for (int i = 0; i < devicesPath.size(); i++) {
                mDevicesTable.emplace(devicesPath[i],getDevice(mNetwork.mConnection,devicesPath[i]));
            }
        }
        dumpDevicesPaired();
        mDiscoveringThread = new std::thread(std::bind(&BluetoothAdapter::discoveringHandler, this));
        mBluetoothActionThread = new std::thread(std::bind(&BluetoothAdapter::bluetoothActionHandler, this));
    });
}

BluetoothAdapter::~BluetoothAdapter()
{
    if (nullptr != mDiscoveringThread) {
        mDiscoveringThread->join();
        delete mDiscoveringThread;
        mDiscoveringThread = nullptr;
    }
    if (nullptr != mBluetoothActionThread) {
        mBluetoothActionThread->join();
        delete mBluetoothActionThread;
        mBluetoothActionThread = nullptr;
    }

    {
        std::lock_guard<std::shared_mutex> lock(mMutex);
        mDeviceInfosQueues.clear();
    }
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

bool BluetoothAdapter::existsPaired(const std::string& devicePath)
{
    bool ret = false;
    std::unordered_map<std::string, std::shared_ptr<BluetoothDevice>>::iterator foundedItem = mDevicesTable.begin();
    while (foundedItem != mDevicesTable.end())
    {
        if (foundedItem->first == devicePath) {
            ret = true;
            break;
        }
        foundedItem++;
    }
    return ret;
}

void BluetoothAdapter::bluetoothActionHandler()
{
    while (true)
    {
        std::list<std::tuple<std::string, std::string, std::string, std::vector<std::string>>>  deviceInfosQueues;
        {
            std::unique_lock<std::mutex> cvLock(mBluetoothActionMutex);
            if (deviceInfosQueues.empty()) {
                mBluetoothActionCV.wait(cvLock);
            }

            deviceInfosQueues = mDeviceInfosQueues;
            mDeviceInfosQueues.clear();
        }
        while(!deviceInfosQueues.empty()) {
            std::tuple<std::string, std::string, std::string, std::vector<std::string>> deviceInfo = deviceInfosQueues.front();
            deviceInfosQueues.pop_front();
            std::cout << "\nDevice found: \n" << " - Name: " << std::get<1>(deviceInfo) << "\n - Address: " << std::get<2>(deviceInfo) << "\n - DevicePath: " << std::get<0>(deviceInfo);
            std::vector<std::string> uuids = std::get<3>(deviceInfo);
            std::cout << "\n UUIDs: ";
            for (int i = 0; i < uuids.size(); i++)
            {
                std::cout << getProfile(uuids[i]) << " | " ;
            }
            if (!existsPaired(std::get<0>(deviceInfo))) {
                mDevicesTable.emplace(std::get<0>(deviceInfo),new BluetoothDevice(*this, std::get<1>(deviceInfo),std::get<2>(deviceInfo),std::get<0>(deviceInfo), uuids));
            }
        }
    }
}

void BluetoothAdapter::discoveringHandler()
{
    std::function<void(DBusConnection*, const char*)> getDeviceInfo = [this](DBusConnection *conn, const char* device_path) {
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
        std::string deviceName;
        std::string deviceAddress;
        std::vector<std::string> uuids;
        if (nullptr != reply) {
            if (dbus_message_iter_init(reply, &iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY) {
                dbus_message_iter_recurse(&iter, &dict_entry_iter);
                while (dbus_message_iter_get_arg_type(&dict_entry_iter) == DBUS_TYPE_DICT_ENTRY) {

                    DBusMessageIter dict_entry_key_iter, dict_entry_value_iter;
                    dbus_message_iter_recurse(&dict_entry_iter, &dict_entry_key_iter);
                    const char* property_name;
                    dbus_message_iter_get_basic(&dict_entry_key_iter, &property_name);

                    if (std::string(property_name) == "Name") {
                        dbus_message_iter_next(&dict_entry_key_iter);
                        dbus_message_iter_recurse(&dict_entry_key_iter, &dict_entry_value_iter);

                        const char* device_name;
                        dbus_message_iter_get_basic(&dict_entry_value_iter, &device_name);
                        std::cout << "Device Name: " << device_name << std::endl;
                        deviceName = std::string(device_name);
                    }
                    else if (std::string(property_name) == "Address") {
                        dbus_message_iter_next(&dict_entry_key_iter);
                        dbus_message_iter_recurse(&dict_entry_key_iter, &dict_entry_value_iter);

                        const char* device_address;
                        dbus_message_iter_get_basic(&dict_entry_value_iter, &device_address);
                        std::cout << "Device address: " << device_address << std::endl;
                        deviceAddress = std::string(device_address);
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
                                uuids.emplace_back(std::string(uuid));
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

                mDeviceInfosQueues.emplace_back(std::make_tuple(std::string(device_path),deviceName,deviceAddress, uuids));
                mBluetoothActionCV.notify_all();
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
                    static std::regex pattern("/org/bluez/");
                    if (!std::regex_search(std::string(device_path), pattern)) {
                        std::cout << "\nDevice found: " << device_path << " not Bluetooth device "<< std::endl;
                        goto Unref;
                    }
                    {
                        std::lock_guard<std::mutex> lock(mBluetoothActionMutex);
                        getDeviceInfo(mNetwork.mConnection, device_path);
                    }
                }
            } else if (dbus_message_is_signal(message, G_INTERFACE_DBUS_PROP, G_SIGNAL_PROPERTIES_CHANGED)) {
                DBusMessageIter args;
                dbus_message_iter_init(message, &args);

                const char *interface_name;
                dbus_message_iter_get_basic(&args, &interface_name);

                if (0 == strcmp(interface_name, G_BT_INTERFACE_DEVICE1)) {
                    std::cout << "\nDevice properties changed!" << std::endl;
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

void BluetoothAdapter::dumpDevicesUnpaired()
{
    std::unordered_map<std::string, std::shared_ptr<BluetoothDevice>>::iterator item = mDevicesTable.begin();
    while (item != mDevicesTable.end())
    {
        if (item->second->getStatus() == Status::Unpaired) {
            item->second->dump();
        }
        item++;
    }    
}

void BluetoothAdapter::dumpDevicesPaired()
{
    std::unordered_map<std::string, std::shared_ptr<BluetoothDevice>>::iterator item = mDevicesTable.begin();
    while (item != mDevicesTable.end())
    {
        if (item->second->getStatus() >= Status::Disconnected) {
            item->second->dump();
        }
        item++;
    }    
}

std::shared_ptr<BluetoothDevice> BluetoothAdapter::getBluetoothDevice(const std::string& address)
{
    std::unordered_map<std::string, std::shared_ptr<BluetoothDevice>>::iterator foundedItem = mDevicesTable.begin();
    while (foundedItem != mDevicesTable.end())
    {
        if (foundedItem->second->getDeviceAddress() == address) {
            return foundedItem->second;
        }
        foundedItem++;
    }
    return nullptr;
}

void BluetoothAdapter::disconnectBluetooth(const std::string& address)
{
    std::shared_ptr<BluetoothDevice> device = getBluetoothDevice(address);
    if (nullptr == device) {
        std::cout << "Not found device : " << address << '\n';
        return;
    }
    device->disconnect();
}

void BluetoothAdapter::connectProfile(const std::string& address, const std::string& profile)
{
    std::shared_ptr<BluetoothDevice> device = getBluetoothDevice(address);
    if (nullptr == device) {
        std::cout << "Not found device : " << address << '\n';
        return;
    }
    device->connectProfile(profile);
}

void BluetoothAdapter::disconnectProfile(const std::string& address, const std::string& profile)
{
    std::shared_ptr<BluetoothDevice> device = getBluetoothDevice(address);
    if (nullptr == device) {
        std::cout << "Not found device : " << address << '\n';
        return;
    }
    device->disconnectProfile(profile);
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
    std::lock_guard<std::shared_mutex> lock(mMutex);
    return mDeviceName;
}

std::string BluetoothDevice::getDeviceAddress() const
{
    std::lock_guard<std::shared_mutex> lock(mMutex);
    return mDeviceAddress;
}

std::string BluetoothDevice::getDevicePath() const
{
    std::lock_guard<std::shared_mutex> lock(mMutex);
    return mDevicePath;
}

Status BluetoothDevice::getStatus() const
{
    std::lock_guard<std::shared_mutex> lock(mMutex);
    return mState;
}

std::vector<std::string> BluetoothDevice::getUUIDs() const
{
    std::lock_guard<std::shared_mutex> lock(mMutex);
    return mUUIDs;
};

void BluetoothDevice::createBond()
{

}

void BluetoothDevice::destroyBond()
{

}

void BluetoothDevice::connectProfile(const std::string& profile)
{
    DBusError err;
    std::string uuid = "";
    static std::function<std::string(std::string)> upperCase = [](std::string letter) -> std::string {
        std::transform(letter.begin(), letter.end(), letter.begin(), [](unsigned char c){
            if (c >= 'a' && c <= 'z') {
                return std::toupper(static_cast<int>(c));
            }
            return static_cast<int>(c);
        });
        return letter;
    };
    std::unordered_map<std::string, std::string>::iterator foundedItem = BluetoothAdapter::gProfileMap.begin();
    while (foundedItem != BluetoothAdapter::gProfileMap.end())
    {
        if (upperCase(foundedItem->second) == upperCase(profile)) {
            uuid = foundedItem->first;
            break;
        }
        foundedItem ++;
    }
    
    if (uuid.empty()) {
        std::cout << "Invalid profile request\n";
        return;
    }
  
    DBusMessage *message = dbus_message_new_method_call(
        "org.bluez",
        mDevicePath.c_str(),
        "org.bluez.Device1",
        "ConnectProfile"
    );

    if (nullptr == message) {
        std::cerr << "Failed to create DBus message." << std::endl;
        return;
    }

    DBusMessageIter args;
    dbus_message_iter_init_append(message, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &uuid)) {
        std::cerr << "Failed to append UUID to DBus message." << std::endl;
        dbus_message_unref(message);
        return;
    }

    DBusPendingCall *pending = nullptr;
    if (!dbus_connection_send_with_reply(mAdapter.mNetwork.mConnection, message, &pending, -1)) {
        std::cerr << "Failed to send DBus message." << std::endl;
        dbus_message_unref(message);
        return;
    }

    if (nullptr == pending) {
        std::cerr << "Failed to create pending call for DBus message." << std::endl;
        dbus_message_unref(message);
        return;
    }


    dbus_connection_flush(mAdapter.mNetwork.mConnection);
    dbus_message_unref(message);

    dbus_pending_call_block(pending);
    DBusMessage *reply = dbus_pending_call_steal_reply(pending);
    dbus_pending_call_unref(pending);

    if (nullptr == reply) {
        std::cerr << "Failed to get reply from DBus." << std::endl;
        return;
    }

    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
        std::cerr << "Error in DBus reply: " << dbus_message_get_error_name(reply) << std::endl;
        dbus_message_unref(reply);
        return;
    }

    std::cout << "Connected to Bluetooth profile successfully!" << std::endl;
    dbus_message_unref(reply);
    return;
}

void BluetoothDevice::disconnectProfile(const std::string& profile)
{
    DBusError err;
    dbus_error_init(&err);
    std::string uuid = "";
    static std::function<std::string(std::string)> upperCase = [](std::string letter) -> std::string {
        std::transform(letter.begin(), letter.end(), letter.begin(), [](unsigned char c){
            if (c >= 'a' && c <= 'z') {
                return std::toupper(static_cast<int>(c));
            }
            return static_cast<int>(c);
        });
        return letter;
    };

    std::unordered_map<std::string, std::string>::iterator foundedItem = BluetoothAdapter::gProfileMap.begin();
    while (foundedItem != BluetoothAdapter::gProfileMap.end())
    {
        if (upperCase(foundedItem->second) == upperCase(profile)) {
            uuid = foundedItem->first;
            break;
        }
        foundedItem ++;
    }
    
    if (uuid.empty()) {
        std::cout << "Invalid profile request\n";
        return;
    }
    
    
    DBusMessage *message = dbus_message_new_method_call(
        "org.bluez",
        mDevicePath.c_str(),
        "org.bluez.Device1",
        "DisconnectProfile"
    );

    if (nullptr == message) {
        std::cerr << "Failed to create DBus message." << std::endl;
        return;
    }

    DBusMessageIter args;
    dbus_message_iter_init_append(message, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &uuid)) {
        std::cerr << "Failed to append UUID to DBus message." << std::endl;
        dbus_message_unref(message);
        return;
    }

    DBusPendingCall *pending = nullptr;
    if (!dbus_connection_send_with_reply(mAdapter.mNetwork.mConnection, message, &pending, -1)) {
        std::cerr << "Failed to send DBus message." << std::endl;
        dbus_message_unref(message);
        return;
    }
    if (nullptr == pending) {
        std::cerr << "Failed to create pending call for DBus message." << std::endl;
        dbus_message_unref(message);
        return;
    }


    dbus_connection_flush(mAdapter.mNetwork.mConnection);
    dbus_message_unref(message);

    dbus_pending_call_block(pending);
    DBusMessage *reply = dbus_pending_call_steal_reply(pending);
    dbus_pending_call_unref(pending);

    if (nullptr == reply) {
        std::cerr << "Failed to get reply from DBus." << std::endl;
        return;
    }

    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
        std::cerr << "Error in DBus reply: " << dbus_message_get_error_name(reply) << std::endl;
        dbus_message_unref(reply);
        return;
    }

    std::cout << "Disconnected to Bluetooth profile successfully!" << std::endl;
    dbus_message_unref(reply);
    return;
}

void BluetoothDevice::disconnect()
{
    DBusError err;
    dbus_error_init(&err);
   
    DBusMessage *message = dbus_message_new_method_call(
        "org.bluez",
        mDevicePath.c_str(),
        "org.bluez.Device1",
        "Disconnect"
    );

    if (nullptr == message) {
        std::cerr << "Failed to create DBus message." << std::endl;
        return;
    }

    DBusPendingCall *pending = nullptr;
    if (!dbus_connection_send_with_reply(mAdapter.mNetwork.mConnection, message, &pending, -1)) {
        std::cerr << "Failed to send DBus message." << std::endl;
        dbus_message_unref(message);
        return;
    }
    if (nullptr == pending) {
        std::cerr << "Failed to create pending call for DBus message." << std::endl;
        dbus_message_unref(message);
        return;
    }


    dbus_connection_flush(mAdapter.mNetwork.mConnection);
    dbus_message_unref(message);

    dbus_pending_call_block(pending);
    DBusMessage *reply = dbus_pending_call_steal_reply(pending);
    dbus_pending_call_unref(pending);

    if (nullptr == reply) {
        std::cerr << "Failed to get reply from DBus." << std::endl;
        return;
    }

    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
        std::cerr << "Error in DBus reply: " << dbus_message_get_error_name(reply) << std::endl;
        dbus_message_unref(reply);
        return;
    }

    std::cout << "Disconnected to Bluetooth profile successfully!" << std::endl;
    dbus_message_unref(reply);
    return;
}

void BluetoothDevice::dump()
{
    std::cout << "\n Device : " << mDeviceAddress << "\n\t Name: " << mDeviceName 
                                                  << "\n\t DevicePath: " << mDevicePath
                                                  << "\n\t Status: " << mState
                                                  << "\n\t UUIDs: ";
    for (int i = 0; i < mUUIDs.size(); i++) {
        std::cout << BluetoothAdapter::getProfile(mUUIDs[i]) << " | " ;
    }
}

void BluetoothDevice::setStatus(const Status& state)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mState = state;
}
