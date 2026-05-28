# Linux DVB driver for SMIT iCast DTMB/DVBC dual mode demodulator USB dongle

#### ubuntu上使用openwrt SDK交叉编译

```shell
#下载SDK并解压(相应固件的SDK包或者交叉编译链)
wget https://downloads.immortalwrt.org/releases/23.05.2/targets/x86/64/immortalwrt-sdk-23.05.2-x86-64_gcc-12.3.0_musl.Linux-x86_64.tar.xz
tar xvf immortalwrt-sdk-23.05.2-x86-64_gcc-12.3.0_musl.Linux-x86_64.tar.xz

# 下载源码
git clone https://github.com/nxdong520/iCast.git
cd iCast

# 设置环境变量(X86_64架构)
export ARCH=x86
export STAGING_DIR=../immortalwrt-sdk-23.05.2-x86-64_gcc-12.3.0_musl.Linux-x86_64/staging_dir/toolchain-x86_64_gcc-12.3.0_musl/bin/
export KERNEL_DIR=../immortalwrt-sdk-23.05.2-x86-64_gcc-12.3.0_musl.Linux-x86_64/build_dir/target-x86_64_musl/linux-x86_64/linux-5.15.150
export TOOLCHAIN="../immortalwrt-sdk-23.05.2-x86-64_gcc-12.3.0_musl.Linux-x86_64/staging_dir/toolchain-x86_64_gcc-12.3.0_musl/bin/x86_64-openwrt-linux-"

# 编译准备
make prepare

#开始编译
make
```

#### 将生成的内核驱动程序ko文件复制到目标机器
```shell
#加载驱动程序
insmod dvb-core.ko
insmod dvb-usb.ko
insmod dvb-usb-icast-fe.ko
insmod dvb-usb-icast-smit.ko

root@ImmortalWrt:~# dmesg
[11892.415906] dvb-usb: found a 'iCast DTMB/DVBC USB adapter' in warm state.
[11892.416553] dvb-usb: will pass the complete MPEG2 transport stream to the software demuxer.
[11892.417884] dvbdev: DVB: registering new adapter (iCast DTMB/DVBC USB adapter)
[11892.418970] usb 1-1: DVB: registering adapter 0 frontend 0 (iCast DTMB/DVBC USB demodulator)...
[11892.420001] dvb-usb: iCast DTMB/DVBC USB adapter successfully initialized and connected.
[11892.420933] usbcore: registered new interface driver dvb_usb_icast_smit

root@ImmortalWrt:~# ls /dev/dvb/adapter0/ -al
drwxr-xr-x    2 root     root           120 May 28 14:42 .
drwxr-xr-x    3 root     root            60 May 28 11:26 ..
crw-------    1 root     root      212,   4 May 28 14:42 demux0
crw-------    1 root     root      212,   5 May 28 14:42 dvr0
crw-------    1 root     root      212,   3 May 28 14:42 frontend0
crw-------    1 root     root      212,   7 May 28 14:42 net0
```

#### 说明

支持“iCast融合电视伴侣” DTMB/DVC 双模电视棒   
操作系统需要先加载rc-core.ko内核模块，可以到相应的固件源中安装或另外单独编译一个   
驱动成功加载后出现/dev/dvb/adapterx/目录   
提供给所有支持标准linux dvb api的软件使用   
本源码通讯协议通过usb抓包取得，并参考逆向分析由网友提供的闭源驱动程序部分流程，感谢提供者！   
