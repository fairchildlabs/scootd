```
$      - copy and paste into SSH or PI terminal (only the charecters after the $) 
PI>    - means PI terminal ONLY (Touchscreen or VNC)
//This is a comment in code, for information only, don't type
LABPC>  0This means type this into the cmd window on your labpc
$$$ This means the lines that follow can be copied and pasted all at once (multiple lines)
2$  Means 2nd SSH Terminal 
```

# Lab3

## 3.1 Enable the Wired Network interface on your PI

```
$ sudo nano /etc/network/interfaces.d/eth0

** paste these lines

allow-hotplug eth0
iface eth0 inet static
address 11.11.11.1/24
netmask 255.255.255.0
gateway 11.11.11.1

<CTRL><X> then "Y" to save

$ sudo nano reboot

```

## 3.2 Use Ethernet (Network) Cable to attach LABPC to PI

Disable Network Connection on PI (DEMO)
Start Putty normally. (Alternate, use 11.11.11.1 IP address)

## 3.3 Create /mnt direcotry for USB

```
$ sudo nano /etc/fstab
** This makes sure that the usb drive is always at /mnt
** Paste this line
/dev/sda1	/mnt	auto	defaults,nofail,sync	0	0
<CTRL><X> and "Y" to exit

$ mount -a

** This creates a link so you can view files on the USB drive from browser
$ sudo mkdir /var/www/html/usb
$ ln -s /mnt /var/www/html/usb
```


## 3.4 
```
$ git clone https://github.com/fairchildlabs/Lab3.git
$ cd Lab3/code
$ make
$ make scootdtest

** cleanup the old videos

$ rm /mnt/*

** Start the test
$ ./scootd

** Open a second SSH session to the PI (mtputty)
** In MTPUTTY, click on the PI name again to open a second window

$2> cd Lab3/code
$2> ./testdrive.sh

```

















