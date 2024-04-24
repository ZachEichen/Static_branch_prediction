# make a local bin directory 
mkdir -p ~/bin
wget https://github.com/sharkdp/hyperfine/releases/download/v1.16.1/hyperfine_1.16.1_amd64.deb -O ~/hyperfine.deb
dpkg -x ~/hyperfine.deb ~/bin/

# update bashrc 
echo 'export PATH=$PATH:~/bin/usr/bin' >> ~/.bashrc 
echo 'export PATH=$PATH:~/bin/usr/bin' >> ~/.zshrc 

source ~/.bashrc

echo  'hyperfine has been installed locally. run `source ~/.bashrc` to get access to it.' 

