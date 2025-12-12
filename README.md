---
title: TOPPERS版OpenRTMの最新版対応
subtitle: asp3_3.7 OpenRTM-aist 2.0 RTC作成手順
author: 伊藤和宣
keywords: ["Rev1.0"]
---

# asp3_3.7 OpenRTM-aist 2.0 RTC作成手順

zybo-z7で動作するtoppers/asp上にLWIP、RtORB、OpenRTM-aist 2.0を移植した。  
ビルド方法、ユーザーが作成したRTCをbuildする方法について記載する。  

<br>

## build方法（ConsoleOutComp）  
サンプルのRTCとして、ConsoleOutCompが移植されている。  
asp3_3.7フォルダ下で以下の操作を実行することでConsoleOutCompが生成される。  
```
mkdir obj
cd obj
../build.sh
```

<br>

## build前に準備するライブラリ  

下記ライブラリを事前にOpenRTM-aist 2.0 、RtORBをビルドして作成しておく。  
LWIPについてはaspビルド時に同時にビルドされる。  

<br>

|フレームワーク名|ライブラリ名|
|:----|:---|
|OpenRTM-aist 2.0| libRTC2-static.a
|OpenRTM-aist 2.0| libOpenrtmNamesPlugin-static.a
|RtORB| libRtORB_cpp.a
|RtORB| libRtORB.a

<br>

## ライブラリの指定方法  

今回、aspconfig.rb（場所、名称はビルド時に指定するので特に制限はない）を配置しその中にプロジェクト情報を記述している。  
ライブラリの指定はファイル内で、LIBS変数にビルドを実行するフォルダからの相対パスを含んだライブラリ名を指定若しくは-l[library]の形で記述する。  

```    
LIBS     = <<LIBS_END
        ../../OpenRTM-aist/build_arm/src/lib/rtm/libRTC2-static.a
        ../../OpenRTM-aist/build_arm/utils/openrtmNames/libOpenrtmNamesPlugin-static.a
        ../../RtORB/lib/CXX/libRtORB_cpp.a
        ../../RtORB/lib/libRtORB.a
        -lstdc++
        -lm
LIBS_END

```

<div style="page-break-before：always"></div>


## ユーザーアプリの作成方法  

appli/下にRTCソースファイル（idl-compilerによって生成されたソースも含む。）を追加する。  
そのうえで、追加したソースファイルをコンフィグファイルにビルド対象として追加する。  

```
    SRC_FILES = <<SRC_FILES
    #APPL
        .
        .
        .
        .
        .
        ../appli/ConsoleOut.cpp             <<< ConsoleOut用に追加
        ../appli/ConsoleOutComp.cpp         <<< ConsoleOut用に追加
```

<br>

## サンプルRTCファイル構成  

```
        appli/openrtm_consoleout.c          <<<[コンフィグファイルのAPPLNAMEで指定した名前].c
        appli/openrtm_consoleout.cfg        <<<[コンフィグファイルのAPPLNAMEで指定した名前].cfg
        appli/openrtm_consoleout.cdl        <<<[コンフィグファイルのAPPLNAMEで指定した名前].cdl
        appli/opnertm_consoleout.h          <<<[コンフィグファイルのAPPLNAMEで指定した名前].h
        appli/ConsoleOut.cpp
        appli/ConsoleOutComp.cpp
```

<br>

<div style="page-break-before：always"></div>

## サンプルRTCの記述方法  

通常のRTCと違い。  
Toppersのメインタスクから、RTCのタスクを起動して実行する。  
また、C++のグローバルインスタンスの初期化を事前に実行する。  
今回はRTCソース内に初期化関数を定義し、Toppersのメインタスクから呼び出している。  
通常のRTCと違いmain関数ではなくToppersのタスクとしてmain処理を実装する。  

<br>

```
openrtm_consoleout.c:615 C++のグローバルインスタンス初期化
	cpp_init();

openrtm_consoleout.c:621 ConsoleOutスレッドの起動
 	sys_thread_new("openrtm_consoleout", (void(*)(void*))openrtm_consoleout_thread, 10,
            STACK_SIZE,
            DEFAULT_THREAD_PRIO);

OpenConsoleOutComp.cpp:162 ConsoleOutCompメイン処理
static char *argv[]={"Server","-i","-f","none","-o","logger.stream:cout","-o","corba.nameservers:10.0.2.2:2809","-o","manager.components.preactivation: ConsoleOut0",NULL};
static int argc = 10;
extern "C"{
void openrtm_consoleout_thread(void* exinf)
{
  RTC::Manager* manager;
  std::cout << "ConsoleOut Starting...\n";
  manager = RTC::Manager::init(argc, argv);
  manager->addRtcLifecycleActionListener(new OverwriteInstanceName(argc, argv), true);
  .
  .
  .
```

<br>

<div style="page-break-before：always"></div>

## OpenRTM-aist用に準備したサービスAPI  

今回、OpenRTM-aist 2.0をToppersに移植するにあたり、次にあげる関数を用意した。  
Toppersに依存する部分はLWIPのポーティングレイヤーで吸収する方針としている。  
LWIPを移植した系では今回追加したAPIを追加することでOpenRTM-aistを移植することができる。  

<br>

|関数|内容|備考|
|:---|:---|:---|
|void sys_arch_version(int *major,int *minor, int　*patch)|バージョン情報|OpenRTM用に追加
|void sys_thread_sleep()|実行中のタスクをスリープに移行|OpenRTM用に追加
|void sys_thread_wakeup(sys_thread_t *tskid)|指定したタスクを実行状態に遷移|OpenRTM用に追加
|uint64_t sys_arch_system_clock()|μs単位のカウンタ値を取得|OpenRTM用に追加
|void sys_arch_currentid(sys_thread_t *thread)|現在のタスクIDを取得|OpenRTM用に追加 
|int sys_arch_delay(unsigned int miliseconds)|指定したms間スリープ|OpenRTM用に追加 
|int sys_arch_usleep(unsigned long int us)|指定したus間スリープ|OpenRTM用に追加 
|char *get_ifaddr()|ifaddrを取得|OpenRTM用に追加
|void sys_arch_mutex_lock()|セマフォ排他アクセス用ロック|OpenRTM用に追加 
|void sys_arch_mutex_unlock()|セマフォ排他アクセス用アンロック|OpenRTM用に追加 
|void sys_init(void)|初期化|LWIPの標準ポーティングレイヤー
|u32_t sys_now()|ms単位のカウンタの取得|LWIPの標準ポーティングレイヤー
|err_t sys_mutex_new(sys_mutex_t *mutex)|mutexの生成|LWIPの標準ポーティングレイヤー
|void sys_mutex_lock(sys_mutex_t *mutex)|mutexのロック|LWIPの標準ポーティングレイヤー

<div style="page-break-before：always"></div>

|関数|内容|備考|
|:---|:---|:---|
|void sys_mutex_unlock(sys_mutex_t *mutex)|mutexのアンロック|LWIPの標準ポーティングレイヤー
|void sys_mutex_free(sys_mutex_t *mutex)|mutexの解放|LWIPの標準ポーティングレイヤー
|err_t sys_sem_new(sys_sem_t *sem,u8_t count)|セマフォの生成|LWIPの標準ポーティングレイヤー
|void sys_sem_free(sys_sem_t *sem)|セマフォの解放|LWIPの標準ポーティングレイヤー
|void sys_sem_signal(sys_sem_t *sem)|セマフォの資源増加|LWIPの標準ポーティングレイヤー
|u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)|セマフォの資源取得|LWIPの標準ポーティングレイヤー
|sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread,void *arg, int stacksize, int prio) |タスクの生成 stacksize無効|LWIPの標準ポーティングレイヤー

<div style="page-break-before：always"></div>

## 今回用意した下層の関数  

|分類|関数名|ファイル名|機能|備考|
|:---|:---|:---|:---|:---|
|BSP|outbyte|openrtm_consoleout.c|UART1に1byte出力|BSP内の_writeから呼び出し
|BSP|inbyte|openrtm_consoleout.c|UART1に1byte出力|BSP内の_writeから呼び出し
|BSP|_fini|openrtm_consoleout.c|処理なし|共有ライブラリの終了処理用に呼び出し
|BSP|_exit|openrtm_consoleout.c|ビジーループにならないように<br>dly_tskを呼び出す。|exitから呼ばれる。<br>基本的にはexitは呼び出されないが、<br>移植対象によっては呼び出される。
|RtORB|gettimeofday|ftl_gettimeofday.c|timevalを返す|BSPで用意されていればそちらを使用でも良い。
|RtORB|tlsf_malloc|tlsf.c|RtORB_mallocの置き換え|USE_TLSFを有効に設定が必要。
|RtORB|tlsf_realloc|tlsf.c|RtORB_reallocの置き換え|USE_TLSFを有効に設定が必要。
|RtORB|tlsf_calloc|tlsf.c|RtORB_callocの置き換え|USE_TLSFを有効に設定が必要。
|RtORB|strdup|ftl_string.c|tlsfを用いて提供|USE_TLSFを有効に設定が必要。
|RtORB|strndup|ftl_string.c|tlsfを用いて提供|USE_TLSFを有効に設定が必要。
|RtORB|tlsf_free|tlsf.c|RtORB_freeの置き換え|USE_TLSFを有効に設定が必要。
|OpenRTM-aist|_malloc_r|ftl_malloc.c|c++の可変長メモリプール用に提供|tlsfを用いて提供<br>
|OpenRTM-aist|_free_r|ftl_malloc.c|c++の可変長メモリプール用に提供<br>|tlsfを用いて提供<br>

<div style="page-break-before：always"></div>

|分類|関数名|ファイル名|機能|備考|
|:---|:---|:---|:---|:---|
|OpenRTM-aist|_realloc_r|ftl_malloc.c|c++の可変長メモリプール用に提供<br>|tlsfを用いて提供<br>
|OpenRTM-aist|_calloc_r|ftl_malloc.c|c++の可変長メモリプール用に提供<br>|tlsfを用いて提供<br>
|OpenRTM-aist|ftl_getopt|ftl_getopt.c|オプション文字列解析|-
    

<div style="page-break-before：always"></div>
