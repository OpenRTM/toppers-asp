---
title: TOPPERS版OpenRTMの最新版対応
subtitle: 動作確認手順
author: 伊藤和宣
keywords: ["Rev1.0"]
---

# 動作確認手順  
## OpenRTM-aist サンプル ConsoleOutComp 動作確認手順  
NamingServiceが10.0.2.2:2809で動作している前提で作成されている。  
ソース中の定義修正で変更可能。  

<br>

## 必要なアプリ  
動作環境には事前にOpenRTM-aist 2.0の実行環境をインストールする。  

- rtm2Naming 
- ConsoleInComp 
- qemu-system-arm

<br>

## 仮想ネットワークの構築  
```
sudo ip tuntap add dev tap0 mode tap
sudo ip link set tap0 up
sudo ip addr add 10.0.2.2/24 dev tap0
sudo dnsmasq --interface=tap0 --bind-interfaces --except-interface=eth0 --except-interface=lo --dhcp-range=10.0.2.15,10.0.2.15,12h 
```

<div style="page-break-before：always"></div>

## qemuの起動  
下記内容のシェルスクリプト(exec_qemu)を作成してtoppersバイナリを指定して実行  
```
~/bin/qemu-system-arm -M xilinx-zynq-a9 \
-serial /dev/null \
-serial mon:stdio \
-device loader,addr=0xf8000008,data=0xDF0D,data-len=4 \
-device loader,addr=0xf8000140,data=0x00500801,data-len=4 \
-device loader,addr=0xf800012c,data=0x1ed044d,data-len=4 \
-device loader,addr=0xf8000108,data=0x0001e008,data-len=4 \
-device loader,addr=0xF800025C,data=0x00000005,data-len=4 \
-device loader,addr=0xF8000240,data=0x00000000,data-len=4 \
-boot mode=5 \
-nic tap,ifname=tap0,script=no,downscript=no,model=cadence_gem \
-kernel $@ 
```

<br>

ターミナルからqemuを用いてConsoleOutCompを起動する。  

```
$ rtm2Naming     # NamingServiceの起動
$ exec_qemu asp  # gdbでデバッグしたい場合には -S -gdb tcp:localhost:port 等として起動する。

起動したら以下のメッセージが表示される。  
TOPPERS/ASP3 Kernel Release 3.7.1 for ZYBO <Zynq-7000, Cortex-A9> (Dec 11 2025, 18:39:52)
Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
                            Toyohashi Univ. of Technology, JAPAN
Copyright (C) 2004-2023 by Embedded and Real-Time Systems Laboratory
            Graduate School of Information Science, Nagoya Univ., JAPAN
Copyright (C) 1977-2025 by Future Technology Laboratories.

System logging task is started.
```

<div style="page-break-before：always"></div>

```
-----lwIP Socket Mode Echo server Demo Application ------
Start PHY autonegotiation 
Waiting for PHY to complete autonegotiation.
autonegotiation complete 
Using default Speed from design
link spSample program starts (exinf = 0).
eed for phy address 7: 100
DHCP request success
Board IP: 10.0.2.15
Netmask : 255.255.255.0
Gateway : 10.0.2.2
         echo server      7 $ telnet <board_ip> 7

ConsoleOut Starting...
Configuration file: none not found.
Configuration file: none not found.
：
：
：
=================================================
Port0 (name): ConsoleOut0.in
-------------------------------------------------
- properties -
port.port_type: DataInPort
dataport.data_type: IDL:RTC/TimedLong:1.0
dataport.subscription_type: Any
dataport.marshaling_types: cdr
dataport.dataflow_type: push,pull
dataport.interface_type: corba_cdr,data_service,direct
dataport.interface_option.corba_cdr: 
dataport.interface_option.data_service: 
dataport.interface_option.direct: 
data_type: IDL:RTC/TimedLong:1.0
-------------------------------------------------

```

今回のサンプルはConsoleOutCompの機能以外に7番ポートで echo serverが立ち上がっており、telnet等で7番ポートに接続することで通信の成立を確認することができる。  

<div style="page-break-before：always"></div>

別のターミナルでConsoleInCompを起動して接続  
```
$ ConsoleInComp \
-o "manager.components.preconnect:ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in" \
-o "manager.components.preactivation:rtcname://localhost:2809/*/ConsoleOut0,ConsoleIn0"

接続が成功したら下記メッセージが表示される。
------------------------------
Connector Listener: ON_CONNECT
Profile::name:      ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:        b900dfcc-ae2d-40d6-a39d-0812ce8609e0
Profile::properties: 
- data_type: IDL:RTC/TimedLong:1.0
- allow_dup_connection: 
- dataflow_type: push
- interface_type: corba_cdr
- serializer
  - cdr
    - endian: little,big
- outport: 
- inport: 
- provider: 
```

<br>>

## qemuのビルド  
https://github.com/xilinx/qemuからレポジトリをクローンする。  
今回のaspを動作させるためには以下のパッチを適用する必要がある。  
パッチ0001-FIX-read-to-clear-write-1-clear-read-only.patchを適用しビルドを実行する。  
パッチ適応後ビルドを実行する。  
```
$ git clone https://github.com/xilinx/qemu qemu
$ cd qemu
$ git am 0001-FIX-read-to-clear-write-1-clear-read-only.patch
$ mkdir build
$ cd build
$ ../configure
$ make
```

<div style="page-break-before：always"></div>