#!/bin/bash

set -eux

sudo apt-get update
sudo apt-get install -y software-properties-common debian-archive-keyring dirmngr
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 04EE7237B7D453EC
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 648ACFD622F3D138
sudo add-apt-repository "deb http://ftp.us.debian.org/debian sid main"
sudo apt-get update
