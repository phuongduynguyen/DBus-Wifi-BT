# Control Wifi / Bluetooth via DBus on system-bus
command control bluetoothd
  bluetoothctl
  power on
  discoverable on
  show
