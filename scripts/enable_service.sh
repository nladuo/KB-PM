#!/bin/bash
#Description: Add kbpm to system service
#Author: Kalen Blue

sudo cp kbpm /etc/init.d/

# for centos
if [ -f /etc/redhat-release ]; then
    sudo chkconfig --add kbpm
fi

# for ubuntu
if [ -f /etc/lsb-release ]; then
    sudo update-rc.d kbpm defaults
fi
