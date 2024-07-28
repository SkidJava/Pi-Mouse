GADGET="/sys/kernel/config/usb_gadget"
VID="0x056E"                        # Elecom
PID="0x0057"                        # Multifunction Composite Gadget
DEVICE="0x0100"                     # v1.0.0
USB_TYPE="0x0200"                   # USB2
SERIAL_NUMBER="35081301"            # serial number
MANUFACTURER="Elecom"        # manufacturer code
PRODUCT_NAME="Mouse Device"    # product name
PROTOCOL="1"                        # USB protocol
SUBCLASS="0"                        # USB subclass
REPORT_LENGTH="8"                   # USB report length
REPORT_DESCRIPTOR="05010902a1010901a100050919012903150025019503750181029501750581010501093009311581257f750895028106c0c0"
DEVICE_NO="usb0"
CONFIG_NO="1"
BM_ATTRIBUTES="0x80"
MAX_POWER="250"

case "$1" in
    start)
        echo "Creating the USB gadget"
        modprobe libcomposite

        echo "Creating gadget directory"
        cd $GADGET

        # USBマウスとして認識されるデバイスの作成
        mkdir -p g1
        cd g1

        echo "Setting ID's"
        echo $VID > idVendor  # Linux Foundation
        echo $PID > idProduct # Multifunction Composite Gadget
        echo $DEVICE > bcdDevice # v1.0.0
        echo $USB_TYPE > bcdUSB    # USB2

        echo "Creating strings"
        # USBデバイスに関する設定
        mkdir -p strings/0x409
        echo $SERIAL_NUMBER > strings/0x409/serialnumber
        echo $MANUFACTURER > strings/0x409/manufacturer
        echo $PRODUCT_NAME > strings/0x409/product

        echo "Creating the functions"
        # インターフェースの設定
        mkdir -p functions/hid.$DEVICE_NO
        echo $PROTOCOL > functions/hid.$DEVICE_NO/protocol
        echo $SUBCLASS > functions/hid.$DEVICE_NO/subclass
        echo $REPORT_LENGTH > functions/hid.$DEVICE_NO/report_length
        echo "05010902A1010901A10005091901290515002501950575018102950175038103050109300931093816018026FF7F751095038106C0C0" | xxd -r -ps > functions/hid.$DEVICE_NO/report_desc

        echo "Creating the configurations"
        # コンフィグレーションの設定
        mkdir -p configs/c.$CONFIG_NO
        /usr/bin/mkdir configs/c.$CONFIG_NO/strings/0x409
        echo $BM_ATTRIBUTES > configs/c.$CONFIG_NO/bmAttributes
        echo $MAX_POWER > configs/c.$CONFIG_NO/MaxPower

        echo "Associating the functions with their configurations"
        ln -s functions/hid.$DEVICE_NO configs/c.$CONFIG_NO/

        echo "Enabling the USB gadget"
        # USBデバイスのエミュレーションを有効にする
        ls /sys/class/udc > UDC
        chmod 777 /dev/hidg0
        echo "OK"

        ;;
    stop)
        echo "Stopping the USB gadget"

        echo "Disabling the USB gadget"
        cd $GADGET/g1
        echo "" > UDC

        echo "Cleaning up"
        rm configs/c.$CONFIG_NO/hid.$DEVICE_NO
        rmdir functions/hid.$DEVICE_NO

        echo "Cleaning up configuration"
        rmdir configs/c.$CONFIG_NO/strings/0x409
        rmdir configs/c.$CONFIG_NO

        echo "Clearing strings"
        rmdir strings/0x409

        echo "Removing gadget directory"
        cd $GADGET
        rmdir g1
        cd /

        # modprobe -r libcomposite    # Remove composite module
        echo "OK"
    ;;
    *)
        echo "Usage : $0 {start vid pid serialnumber manufacture product|stop}"
        ;;
esac