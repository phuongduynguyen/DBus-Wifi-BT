#ifndef GLOBAL_VARIABLE
#define GLOBAL_VARIABLE
    enum class Status
    {
        Unpaired,
        Disconnected,
        Connected
    } ;

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

    inline std::ostream& operator<<(std::ostream& strm, const Status& value)
    {
        std::ostream *ptr = &strm;
        static const char *valueTbl[] = {
            "Unpaired",
            "Disconnected",
            "Connected"
        };

        if (static_cast<uint8_t>(value) < 3) {
            strm << valueTbl[static_cast<uint8_t>(value)];
        }
        else {
            strm << "unknown";
        }
        return *ptr;
    }

#endif