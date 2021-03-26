#!/bin/bash

set -eux

# ssh key
mkdir -p ~/.ssh/
echo "-----BEGIN OPENSSH PRIVATE KEY-----" > ~/.ssh/id_rsa
echo $sshkey >> ~/.ssh/id_rsa
echo "-----END OPENSSH PRIVATE KEY-----" >> ~/.ssh/id_rsa
wc -l ~/.ssh/id_rsa
chmod -R 700 ~/.ssh/id_rsa
ssh-keygen -p -m PEM -f ~/.ssh/id_rsa
ssh-keygen -y -f ~/.ssh/id_rsa > ~/.ssh/id_rsa.pub
cat ~/.ssh/id_rsa.pub
cat << EOF | tee ~/.ssh/config
Host aur.archlinux.org
    IdentityFile ~/.ssh/id_rsa
    User aur
EOF
ssh-keyscan -H aur.archlinux.org | tee -a ~/.ssh/known_hosts

# git
git config --global user.email "aur-tcg@emailaddress.biz"
git config --global user.name "tcg"