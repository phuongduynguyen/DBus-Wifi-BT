#ifndef GLOBAL_VARIABLE
#define GLOBAL_VARIABLE

    static const int G_STATE_UNPAIRED = 0x0000000a;
    static const int G_STATE_PAIRED = 0x0000000b;
    static const int G_STATE_DISCONNECT = 0x0000000c;
    static const int G_STATE_CONNECTED = 0x0000000d;

    static constexpr const char* G_BT_SERVICE_NAME = "org.bluez";
    static constexpr const char* G_BT_OBJECT_PATH = "/org/bluez/hci0";
    static constexpr const char* G_BT_ADAPTER_INTERFACE = "org.bluez.Adapter1";
    static constexpr const char* G_BT_INTERFACE_DEVICE1 = "org.bluez.Device1";
    static constexpr const char* G_METHOD_POWERED_PROP = "Powered";
    static constexpr const char* G_METHOD_START_DISCOVERY = "StartDiscovery";
    static constexpr const char* G_METHOD_GET_ADDRESS = "Address";
    static constexpr const char* G_METHOD_STOP_DISCOVERY = "StopDiscovery";
    static constexpr const char* G_ALIAS = "Alias";

    static constexpr const char* G_INTERFACE_DBUS_PROP = "org.freedesktop.DBus.Properties";
    static constexpr const char* G_METHOD_GET = "Get";
    static constexpr const char* G_METHOD_GET_ALL = "GetAll";
    static constexpr const char* G_METHOD_SET = "Set";

    static constexpr const char* G_NM_DBUS_SERVICE = "org.freedesktop.NetworkManager";
    static constexpr const char* G_NM_DBUS_PATH = "/org/freedesktop/NetworkManager";
    static constexpr const char* G_NM_DBUS_INTERFACE = "org.freedesktop.NetworkManager";
    static constexpr const char* G_SIGNAL_PROPERTIES_CHANGED = "PropertiesChanged";
    static constexpr const char* G_METHOD_WIRELESS_ENABLED = "WirelessEnabled";

#endif