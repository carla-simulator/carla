#! /bin/bash

sudo apt install virtualbox

curl -O https://releases.hashicorp.com/vagrant/2.2.7/vagrant_2.2.7_x86_64.deb
sudo apt install ./vagrant_2.2.7_x86_64.deb
vagrant plugin install vagrant-disksize
vagrant plugin install vagrant-scp

git clone https://github.com/uruzahe/veins.git

