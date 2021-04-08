#! /bin/bash

sudo apt install -y virtualbox
sudo apt install -y nfs-common nfs-kernel-server

curl -O https://releases.hashicorp.com/vagrant/2.2.7/vagrant_2.2.7_x86_64.deb
sudo apt install ./vagrant_2.2.7_x86_64.deb

# vagrant plugin install vagrant-cachier
vagrant plugin install vagrant-disksize
vagrant plugin install vagrant-scp
vagrant plugin install vagrant-faster

git clone https://github.com/uruzahe/veins.git
