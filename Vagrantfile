Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/xenial32"
  config.vm.synced_folder ".", "/home/ubuntu/linux"

  config.vm.provider "virtualbox" do |vb|
    vb.name = 'linux-0.99p12 on Ubuntu xenial-xerus'
    vb.gui = false
    vb.memory = "2048"
    vb.cpus = 4
  end

  config.vm.provision "shell", inline: <<-SHELL
    cd /etc/apt
    sudo apt-get update
    sudo apt-get install make gcc bin86 g++ gdb qemu-system-i386 -y
  SHELL
end
