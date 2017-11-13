#!/system/bin/sh
factory=`getprop persist.sys.usb.factory`
configfs=`getprop sys.usb.configfs`
controller=`getprop sys.usb.controller`
config="$1"

case $configfs in
    "0")
        case $factory in
            "0")
                case $config in
                    "nubia")
                        echo 0 > /sys/class/android_usb/android0/enable
                        echo 19D2 > /sys/class/android_usb/android0/idVendor
                        echo FFCC > /sys/class/android_usb/android0/idProduct
                        echo mass_storage > /sys/class/android_usb/android0/functions
                        echo 1 > /sys/class/android_usb/android0/enable
                    ;;
                    "nubia_adb" | "")
                        echo 0 > /sys/class/android_usb/android0/enable
                        echo 19D2 > /sys/class/android_usb/android0/idVendor
                        echo FFCD > /sys/class/android_usb/android0/idProduct
                        echo mass_storage,adb > /sys/class/android_usb/android0/functions
                        echo 1 > /sys/class/android_usb/android0/enable
                    ;;
                    *)
                        echo "$0: Invalid argument ($config)"
                    ;;
                esac
            ;;
            *)
                case $config in
                    "nubia")
                        echo 0 > /sys/class/android_usb/android0/enable
                        echo 19D2 > /sys/class/android_usb/android0/idVendor
                        echo FFAF > /sys/class/android_usb/android0/idProduct
                        echo smd > /sys/class/android_usb/android0/f_serial/transports
                        echo diag > /sys/class/android_usb/android0/f_diag/clients
                        echo diag,serial,mass_storage > /sys/class/android_usb/android0/functions
                        echo 1 > /sys/class/android_usb/android0/enable
                    ;;
                    "nubia_adb" | "")
                        echo 0 > /sys/class/android_usb/android0/enable
                        echo 19D2 > /sys/class/android_usb/android0/idVendor
                        echo FFB0 > /sys/class/android_usb/android0/idProduct
                        echo smd > /sys/class/android_usb/android0/f_serial/transports
                        echo diag > /sys/class/android_usb/android0/f_diag/clients
                        echo diag,serial,mass_storage,adb > /sys/class/android_usb/android0/functions
                        echo 1 > /sys/class/android_usb/android0/enable
                    ;;
                    *)
                        echo "$0: Invalid argument ($config)"
                    ;;
                esac
            ;;
        esac
    ;;
    *)
        case $factory in
            "0")
                case $config in
                    "nubia")
		        echo msc > /config/usb_gadget/g1/configs/b.1/strings/0x409/configuration
		        rm /config/usb_gadget/g1/configs/b.1/f1
		        rm /config/usb_gadget/g1/configs/b.1/f2
		        rm /config/usb_gadget/g1/configs/b.1/f3
		        rm /config/usb_gadget/g1/configs/b.1/f4
		        rm /config/usb_gadget/g1/configs/b.1/f5
		        echo 0x19D2 > /config/usb_gadget/g1/idVendor 
		        echo 0xFFCC > /config/usb_gadget/g1/idProduct 
		        ln -s /config/usb_gadget/g1/functions/mass_storage.0 /config/usb_gadget/g1/configs/b.1/f1
                        echo $controller > /config/usb_gadget/g1/UDC
			setprop sys.usb.state mass_storage
                    ;;
                    "nubia_adb" | "")
		        echo adb_msc > /config/usb_gadget/g1/configs/b.1/strings/0x409/configuration
		        rm /config/usb_gadget/g1/configs/b.1/f1
		        rm /config/usb_gadget/g1/configs/b.1/f2
		        rm /config/usb_gadget/g1/configs/b.1/f3
		        rm /config/usb_gadget/g1/configs/b.1/f4
		        rm /config/usb_gadget/g1/configs/b.1/f5
		        echo 0x19D2 > /config/usb_gadget/g1/idVendor
		        echo 0xFFCD > /config/usb_gadget/g1/idProduct
		        ln -s /config/usb_gadget/g1/functions/mass_storage.0 /config/usb_gadget/g1/configs/b.1/f1
		        ln -s /config/usb_gadget/g1/functions/ffs.adb /config/usb_gadget/g1/configs/b.1/f2
		        echo $controller >  /config/usb_gadget/g1/UDC
		        setprop sys.usb.state mass_storage,adb
                    ;;
                    *)
                        echo "$0: Invalid argument ($config)"
                    ;;
                esac
            ;;
            *)
                case $config in
                    "nubia")
		        echo dsm > /config/usb_gadget/g1/configs/b.1/strings/0x409/configuration
		        rm /config/usb_gadget/g1/configs/b.1/f1
		        rm /config/usb_gadget/g1/configs/b.1/f2
		        rm /config/usb_gadget/g1/configs/b.1/f3
		        rm /config/usb_gadget/g1/configs/b.1/f4
		        rm /config/usb_gadget/g1/configs/b.1/f5
		        echo 0x19D2 > /config/usb_gadget/g1/idVendor 
		        echo 0xFFAF > /config/usb_gadget/g1/idProduct
#			echo $cdromname > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/file
#			echo CD-ROM > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/cdrom
#			echo WRITE(10,12) > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/nofua
#			echo read-only > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/ro
#			echo removable > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/removable
			ln -s /config/usb_gadget/g1/functions/diag.diag /config/usb_gadget/g1/configs/b.1/f1
		        ln -s /config/usb_gadget/g1/functions/cser.dun.0 /config/usb_gadget/g1/configs/b.1/f2
		        ln -s /config/usb_gadget/g1/functions/mass_storage.0 /config/usb_gadget/g1/configs/b.1/f3
		        echo $controller > /config/usb_gadget/g1/UDC
			setprop sys.usb.state diag,serial,mass_storage
                    ;;
                    "nubia_adb" | "")
		        echo dsma > /config/usb_gadget/g1/configs/b.1/strings/0x409/configuration
		        rm /config/usb_gadget/g1/configs/b.1/f1
		        rm /config/usb_gadget/g1/configs/b.1/f2
		        rm /config/usb_gadget/g1/configs/b.1/f3
		        rm /config/usb_gadget/g1/configs/b.1/f4
		        rm /config/usb_gadget/g1/configs/b.1/f5
		        echo 0x19D2 > /config/usb_gadget/g1/idVendor 
		        echo 0xFFB0 > /config/usb_gadget/g1/idProduct
#			echo $cdromname > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/file
#			echo CD-ROM > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/cdrom
#			echo WRITE(10,12) > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/nofua
#			echo read-only > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/ro
#			echo removable > /config/usb_gadget/g1/functions/mass_storage.0/lun.0/removable
			ln -s /config/usb_gadget/g1/functions/diag.diag /config/usb_gadget/g1/configs/b.1/f1
		        ln -s /config/usb_gadget/g1/functions/cser.dun.0 /config/usb_gadget/g1/configs/b.1/f2
		        ln -s /config/usb_gadget/g1/functions/mass_storage.0 /config/usb_gadget/g1/configs/b.1/f3
		        ln -s /config/usb_gadget/g1/functions/ffs.adb /config/usb_gadget/g1/configs/b.1/f4
                        echo $controller > /config/usb_gadget/g1/UDC
			setprop sys.usb.state diag,serial,mass_storage,adb
                    ;;
                    *)
                        echo "$0: Invalid argument ($config)"
                    ;;
                esac
            ;;
        esac
    ;;
esac
