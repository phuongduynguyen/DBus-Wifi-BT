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
#include <utility>
#include <tuple>
#include <type_traits>
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
        void dump();

    protected:
        BluetoothDevice(BluetoothAdapter& adapter, const std::string& deviceName, const std::string& deviceAddress, const std::string& devicePath ,const std::vector<std::string>& uuids);

        BluetoothAdapter& mAdapter;
        mutable std::shared_mutex mMutex;
        std::vector<std::string> mUUIDs;
        std::string mDeviceName;
        std::string mDeviceAddress;
        std::string mDevicePath;
        int mState;
};

class BluetoothAdapter
{
    template<typename... Args>
    friend std::shared_ptr<BluetoothDevice> std::make_shared(Args&&... args);
    friend class NetworkProvider;
    friend class BluetoothDevice;
    public:

        static std::unordered_map<std::string, std::string> gProfileMap;
        static BluetoothAdapter& initialize(NetworkProvider& network);
        static BluetoothAdapter& getInstance();

        template<typename T>
        static std::string getProfile(const T& uuid);

        void startDiscovery();
        void stopDiscovery();
        void toggleBluetoothPower();
        void dumpDevicesUnpaired();
        void dumpDevicesPaired();
        bool getBluetoothPower() const;

        std::string getBluetoothName() const;
        std::string getBluetoothAddress() const;
        std::vector<std::shared_ptr<BluetoothDevice>> getBondedDevices() const;
        

    private:
        BluetoothAdapter(NetworkProvider& network);
        ~BluetoothAdapter();
        void discoveringHandler();
        void bluetoothActionHandler();
        bool existsPaired(const std::string& devicePath);

        std::unordered_map<std::string, std::shared_ptr<BluetoothDevice>> mDevicesTable;
        std::list<std::tuple<std::string, std::string, std::string, std::vector<std::string>>> mDeviceInfosQueues;
        NetworkProvider& mNetwork;
        std::string mBluetoothName;
        std::string mBluetoothAddress;
        mutable std::shared_mutex mMutex;
        std::mutex mDiscoveringMutex;
        std::condition_variable mDiscoveringCV;
        std::mutex mBluetoothActionMutex;
        std::condition_variable mBluetoothActionCV;
        bool mDiscovering;
        std::thread* mDiscoveringThread;
        std::thread* mBluetoothActionThread;


};
#endif