#ifndef BLUETOOTH_ADAPTER
#define BLUETOOTH_ADAPTER

#include <unordered_map>
#include <memory>
#include <list>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <string>
#include <thread>
#include <functional>
#include <dbus/dbus.h>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <ostream>
#include <cstring>
#include <condition_variable>
#include "GlobalVariable.h"

class NetworkProvider;
class BluetoothAdapter;

class BluetoothDevice
{
    friend class BluetoothAdapter;
    public:

        std::string getDeviceName() const;
        std::string getDeviceAddress() const;
        std::string getDevicePath() const;

        int getState() const;
        std::vector<std::string> getUUIDs() const;

        void createBond();
        void destroyBond();
        void connectProfile(const std::string& profile);

    private:
        BluetoothDevice(BluetoothAdapter& adapter, const std::string& deviceName, const std::string& deviceAddress, const std::string& devicePath ,const std::vector<std::string>& uuids);

        BluetoothAdapter& mAdapter;
        std::vector<std::string> mUUIDs;
        std::string mDeviceName;
        std::string mDeviceAddress;
        std::string mDevicePath;
        int mState;
};

class BluetoothAdapter
{
    friend class NetworkProvider;
    friend class BluetoothDevice;
    public:

        static std::unordered_map<std::string, std::string> gProfileMap;
        static BluetoothAdapter& initialize(NetworkProvider& network);
        static BluetoothAdapter& getInstance();
        static std::string getProfile(const char* uuid);

        void startDiscovery();
        void stopDiscovery();

        void toggleBluetoothPower();

        bool getBluetoothPower() const;

        std::string getBluetoothName() const;
        std::string getBluetoothAddress() const;

        std::vector<std::shared_ptr<BluetoothDevice>> getBondedDevices() const;
        
    private:
        BluetoothAdapter(NetworkProvider& network);
        void discoveringHandler();

        std::unordered_map<std::string, std::shared_ptr<BluetoothDevice>> mDevices;
        NetworkProvider& mNetwork;
        std::string mBluetoothName;
        std::string mBluetoothAddress;
        std::shared_mutex mMutex;
        std::mutex mDiscoveringMutex;
        std::condition_variable mDiscoveringCV;
        bool mDiscovering;
        std::thread* mDiscoveringThread;

};
#endif