---
title: OpenRTM-aist TOPPERS 版のOpenRTM-aist-2.0 対応に向けた更新作業
subtitle: 試験仕様書及び試験結果報告書
author: 伊藤和宣
keywords: ["Rev1.0"]
---

# 試験手順  
## OpenRTM-aist サンプル ConsoleOutComp 試験手順 
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
$ export ORBtraceLevel=40
$ export ORBtraceFile=trace_omniNames.txt
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

rtm2-nameingのログ 抜粋
ConsoleOut0が登録されていることを確認。
```
omniORB: (4) 2025-12-12 17:11:53.494877: 
4749 4f50 0102 0103 1a00 0000 0400 0000 GIOP............
0000 0000 0e00 0000 ff00 2ace 3b69 0100 ..........*.;i..
f3c9 0000 0001                          ......
omniORB: (4) 2025-12-12 17:11:53.494888: Handling a GIOP LOCATE_REQUEST.
omniORB: (4) 2025-12-12 17:11:53.494893: sendChunk: to giop:tcp:[::ffff:10.0.2.15]:52435 20 bytes
omniORB: (4) 2025-12-12 17:11:53.494895: 
4749 4f50 0102 0104 0800 0000 0400 0000 GIOP............
0100 0000                               ....
omniORB: (4) 2025-12-12 17:11:53.496203: inputMessage: from giop:tcp:[::ffff:10.0.2.15]:52435 280 bytes
omniORB: (4) 2025-12-12 17:11:53.496210: 
4749 4f50 0102 0100 0c01 0000 0500 0000 GIOP............
0100 0000 0000 0000 0e00 0000 ff00 2ace ..............*.
3b69 0100 f3c9 0000 0001 0000 0700 0000 ;i..............
7265 6269 6e64 006e 0000 0000 0000 0000 rebind.n........
0100 0000 0c00 0000 436f 6e73 6f6c 654f ........ConsoleO
7574 3000 0400 0000 7274 6300 3500 0000 ut0.....rtc.5...
4944 4c3a 6f70 656e 7274 6d2e 6169 7374 IDL:openrtm.aist
2e67 6f2e 6a70 2f4f 7065 6e52 544d 2f44 .go.jp/OpenRTM/D
6174 6146 6c6f 7743 6f6d 706f 6e65 6e74 ataFlowComponent
3a31 2e30 0000 0000 0100 0000 0000 0000 :1.0............
7400 0000 0101 0200 0a00 0000 3130 2e30 t...........10.0
2e32 2e31 3500 d0cc 2100 0000 5274 4f52 .2.15...!...RtOR
4230 3030 4133 3530 3030 3130 3030 3030 B000A35000100000
3030 3030 3030 3042 3543 3936 0000 0000 0000000B5C96....
0200 0000 0000 0000 0800 0000 0100 0000 ................
004d 5452 0100 0000 1c00 0000 0100 0000 .MTR............
0100 0100 0100 0000 0100 0105 0901 0100 ................
0100 0000 0901 0100                     ........
omniORB: (4) 2025-12-12 17:11:53.496243: Creating ref to remote: key<RtORB000A350001000000000000B5C96.>
 target id      : IDL:omg.org/CORBA/Object:1.0
```
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

その後、ConsoleIn上で数値入力して動作確認を実施。

<div style="page-break-before：always"></div>

ConsoleIn　画面表示
```

------------------------------
Please input number: 1
------------------------------
Data Listener: ON_SEND
Profile::name: ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:   6d408751-565f-4382-b01f-4ade81b82979
Data:          1
------------------------------
------------------------------
Data Listener: ON_RECEIVED
Profile::name: ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:   6d408751-565f-4382-b01f-4ade81b82979
Data:          1
------------------------------
Please input number: 2
------------------------------
Data Listener: ON_SEND
Profile::name: ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:   6d408751-565f-4382-b01f-4ade81b82979
Data:          2
------------------------------
------------------------------
Data Listener: ON_RECEIVED
Profile::name: ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:   6d408751-565f-4382-b01f-4ade81b82979
Data:          2
```




ConsoleOut 画面表示
```
------------------------------
Data Listener: ON_RECEIVED(OutPort)
Profile::name: ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:   6d408751-565f-4382-b01f-4ade81b82979
Data:          1
------------------------------
------------------------------
Data Listener: ON_BUFFER_WRITE(OutPort)
Profile::name: ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:   6d408751-565f-4382-b01f-4ade81b82979
Data:          1
------------------------------
------------------------------
Data Listener: ON_BUFFER_READ(OutPort)
Profile::name: ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:   6d408751-565f-4382-b01f-4ade81b82979
Data:          1
------------------------------
Received: 1
TimeStamp: 0[s] 0[ns]
------------------------------
Data Listener: ON_RECEIVED(OutPort)
Profile::name: ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:   6d408751-565f-4382-b01f-4ade81b82979
Data:          2
------------------------------
------------------------------
Data Listener: ON_BUFFER_WRITE(OutPort)
Profile::name: ConsoleIn0.out?port=rtcname://localhost:2809/*/ConsoleOut0.in
Profile::id:   6d408751-565f-4382-b01f-4ade81b82979
Data:          2
------------------------------
```


<br>

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