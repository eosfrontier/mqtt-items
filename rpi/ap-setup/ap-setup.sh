#!/bin/sh
sudo apt-get -q -y update
sudo apt-get -q -y install dnsmasq hostapd
sudo cp 70-persistent-net.rules /etc/udev/rules.d/
sudo cp dnsmasq.conf /etc/
sudo cp hostapd.conf /etc/hostapd/
sudo cp hostapd.default /etc/default/hostapd
sudo cp wpa_supplicant.conf /etc/wpa_supplicant/
sudo cp interfaces /etc/network/
sudo cp iptables.up.rules /etc/
sudo cp iptables /etc/network/if-pre-up.d
sudo cp ipforward.conf /etc/sysctl.d/
