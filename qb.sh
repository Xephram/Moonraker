make
sudo cp moonraker.ko /lib/modules/$(uname -r)
sudo chown root:root /lib/modules/$(uname -r)/moonraker.ko
sudo chmod +x /lib/modules/$(uname -r)/moonraker.ko
