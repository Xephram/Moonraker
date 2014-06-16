if [ ! -d "install" ]; then
mkdir install
fi
cp moonraker.ko install
echo installing moonraker...
insmod install/moonraker.ko
echo dmesg entry:
dmesg | grep Moonraker
echo lsmod entry:
lsmod | grep moonraker
sleep 10
echo removing moonraker...
rmmod -v moonraker
echo removing install directory + module
rm -rf install
echo done!
