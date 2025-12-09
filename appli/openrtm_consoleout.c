/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2022 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 * 
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  $Id: sample1.c 1728 2022-11-20 11:47:31Z ertl-hiro $
 */

/* 
 *  サンプルプログラム(1)の本体
 *
 *  ASPカーネルの基本的な動作を確認するためのサンプルプログラム．
 *
 *  プログラムの概要:
 *
 *  ユーザインタフェースを受け持つメインタスク（MAIN_TASK）と，3つの並
 *  行実行されるタスク（TASK1〜TASK3），例外処理タスク（EXC_TASK）の5
 *  つのタスクを用いる．これらの他に，システムログタスクが動作する．ま
 *  た，周期ハンドラ，アラームハンドラ，割込みサービスルーチン，CPU例
 *  外ハンドラをそれぞれ1つ用いる．
 *
 *  並行実行されるタスクは，task_loop回のループを実行する度に，タスク
 *  が実行中であることをあらわすメッセージを表示する．ループを実行する
 *  のは，プログラムの動作を確認しやすくするためである．また，低速なシ
 *  リアルポートを用いてメッセージを出力する場合に，すべてのメッセージ
 *  が出力できるように，メッセージの量を制限するという理由もある．
 *
 *  周期ハンドラ，アラームハンドラ，割込みサービスルーチンは，3つの優
 *  先度（HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY）のレディキューを
 *  回転させる．周期ハンドラは，プログラムの起動直後は停止状態になって
 *  いる．
 *
 *  CPU例外ハンドラは，CPU例外からの復帰が可能な場合には，例外処理タス
 *  クを起動する．例外処理タスクは，CPU例外を起こしたタスクに対して，
 *  終了要求を行う．
 *
 *  メインタスクは，シリアルポートからの文字入力を行い（文字入力を待っ
 *  ている間は，並行実行されるタスクが実行されている），入力された文字
 *  に対応した処理を実行する．入力された文字と処理の関係は次の通り．
 *  Control-Cまたは'Q'が入力されると，プログラムを終了する．
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "openrtm_consoleout.h"
#include "xil_printf.h"
#include "tlsf.h"
#include "lwip/dhcp.h"
#include <RtORB/corba.h>
#include "CosName/CosNaming.h"


#include "lwip/api.h"
#include "lwip/sys.h"

extern void echo_application_thread();
extern void openrtm_consoleout_thread(EXINF exinf);
/**
 * netifの確保
 */
struct netif server_netif;
struct netif *echo_netif;

/**
 * C++グローバルインスタンス初期化
 */
void init_cpp(void);
/**
 * 可変長メモリプールの確保
 */
#define MEMORY_POOL_SIZE	TOPPERS_ROUND_SZ(1024*65536, sizeof(intptr_t))
							/* 10*1024の部分は，適切なサイズに変更する */
intptr_t memory_pool[MEMORY_POOL_SIZE / sizeof(intptr_t)];
void *__dso_handle = 0;
void _fini(void)
{
	
}
void _exit (sint32 status)
{
	dly_tsk(100);
}
/*
 *  サービスコールのエラーのログ出力
 */
Inline void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}
#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))

void outbyte(char c)
{
	serial_wri_dat(TASK_PORTID,&c,1);
}

char inbyte()
{
	char c;
	return serial_rea_dat(TASK_PORTID,&c,1);
}

#if LWIP_IPV6==1
void print_ip6(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf(" %x:%x:%x:%x:%x:%x:%x:%x\n\r",
			IP6_ADDR_BLOCK1(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK2(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK3(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK4(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK5(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK6(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK7(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK8(&ip->u_addr.ip6));
}
#else
void
print_ip(char *msg, ip_addr_t *ip)
{
	xil_printf(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

void
print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}
#endif

/*
 *  プロセッサ時間の消費
 *
 *  ループによりプロセッサ時間を消費する．最適化ができないように，ルー
 *  プ内でvolatile変数を読み込む．
 */
static volatile long_t	volatile_var;

static void
consume_time(ulong_t ctime)
{
	ulong_t		i;

	for (i = 0; i < ctime; i++) {
		(void) volatile_var;
	}
}

/*
 *  並行実行されるタスクへのメッセージ領域
 */
char	message[3];

/*
 *  ループ回数
 */
ulong_t	task_loop;		/* タスク内でのループ回数 */

/*
 *  割込みサービスルーチン
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
#ifdef INTNO1

void
intno1_isr(EXINF exinf)
{
	intno1_clear();
	SVC_PERROR(rot_rdq(HIGH_PRIORITY));
	SVC_PERROR(rot_rdq(MID_PRIORITY));
	SVC_PERROR(rot_rdq(LOW_PRIORITY));
}

#endif /* INTNO1 */

/*
 *  CPU例外ハンドラ
 */
ID	cpuexc_tskid;		/* CPU例外を起こしたタスクのID */

#ifdef CPUEXC1

void
cpuexc_handler(void *p_excinf)
{
	syslog(LOG_NOTICE, "CPU exception handler (p_excinf = %08p).", p_excinf);
	if (sns_ctx() != true) {
		syslog(LOG_WARNING,
					"sns_ctx() is not true in CPU exception handler.");
	}
	if (sns_dpn() != true) {
		syslog(LOG_WARNING,
					"sns_dpn() is not true in CPU exception handler.");
	}
	syslog(LOG_INFO, "sns_loc = %d, sns_dsp = %d, xsns_dpn = %d",
								sns_loc(), sns_dsp(), xsns_dpn(p_excinf));

	if (xsns_dpn(p_excinf)) {
		syslog(LOG_NOTICE, "Sample program ends with exception.");
		SVC_PERROR(ext_ker());
		assert(0);
	}

	SVC_PERROR(get_tid(&cpuexc_tskid));
	SVC_PERROR(act_tsk(EXC_TASK));
}

#endif /* CPUEXC1 */

/*
 *  周期ハンドラ
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
void
cyclic_handler(EXINF exinf)
{
	SVC_PERROR(rot_rdq(HIGH_PRIORITY));
	SVC_PERROR(rot_rdq(MID_PRIORITY));
	SVC_PERROR(rot_rdq(LOW_PRIORITY));
}

/*
 *  アラームハンドラ
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
void
alarm_handler(EXINF exinf)
{
	SVC_PERROR(rot_rdq(HIGH_PRIORITY));
	SVC_PERROR(rot_rdq(MID_PRIORITY));
	SVC_PERROR(rot_rdq(LOW_PRIORITY));
}

/*
 *  例外処理タスク
 */
void
exc_task(EXINF exinf)
{
	SVC_PERROR(ras_ter(cpuexc_tskid));
}

/**
 * CORBA バインディング
 */

 CORBA_boolean
bindObjectToName(CORBA_ORB orb, CORBA_Object obj, CORBA_Environment *env){
  CosNaming_NamingContext rootContext;
  CosNaming_Name *contextName;
  CosNaming_NamingContext testContext;
  CosNaming_Name *objectName;

  rootContext = CORBA_ORB_resolve_initial_references(orb, "NameService", env);
  catchDefaultException(env);

  contextName = CosNaming_Name__alloc();
  contextName->_buffer = CosNaming_Name_allocbuf(1);
  contextName->_length = contextName->_maximum = 1;

  contextName->_buffer[0].id = "test";
  contextName->_buffer[0].kind = "my_context";

  testContext = CosNaming_NamingContext_bind_new_context(CORBA_Object_dup(rootContext), contextName, env);

  if(catchException(env, ex_CosNaming_NamingContext_AlreadyBound) ){
    fprintf(stderr, "!!!! Already Bound %s, %s \n",
	    contextName->_buffer[0].id ,contextName->_buffer[0].kind ); 
    testContext = CosNaming_NamingContext_resolve(rootContext, contextName, env);
    if(catchException(env, NULL) ){
      return 0;
    }
  }else if(catchException(env, "SystemException")){
    fprintf(stderr, "SystemException  \n"); 
    return 0;
  }else if(env->_major) return 0;


  objectName = CosNaming_Name__alloc();
  objectName->_buffer = CosNaming_Name_allocbuf(1);
  objectName->_length = objectName->_maximum = 1;

  objectName->_buffer[0].id = "Echo";
  objectName->_buffer[0].kind = "Object";

  CosNaming_NamingContext_bind(testContext, objectName, obj, env);

  if(catchException(env, ex_CosNaming_NamingContext_AlreadyBound) ){
    fprintf(stderr, "Sorry, Already_Bound  \n"); 
    CosNaming_NamingContext_rebind(testContext, objectName, obj, env);
    if(catchException(env, NULL) ){
      fprintf(stderr, "Error, %s \n", env->_repo_id ); 
      return 0;
    }
  }else if(catchException(env, "SystemException")){
    fprintf(stderr, "SystemException  \n"); 
    return 0;
  }else if(catchException(env, NULL) ){
    fprintf(stderr, "Error, %s \n", env->_repo_id ); 
    return 0;
  }

  return 1;
}
static char *argv[]={"Server","-ORBInitRef", "NameService=corbaloc::10.0.2.2:2809/NameService",NULL};
static int argc = 3;

/*
 *  ネットワークタスク
 */
void network_thread(EXINF extinf)
{
    struct netif *netif;
    /* the mac address of the board. this should be unique per board */
    unsigned char mac_ethernet_address[] = { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };
#if LWIP_IPV6==0
    ip_addr_t ipaddr, netmask, gw;
#if LWIP_DHCP==1
 volatile   int mscnt = 0;
#endif
#endif
 	netif = &server_netif;

	xil_printf("\r\n\r\n");
    xil_printf("-----lwIP Socket Mode Echo server Demo Application ------\r\n");


#if LWIP_IPV6==0
#if LWIP_DHCP==0
    /* initialize IP addresses to be used */
    IP4_ADDR(&ipaddr,  192, 168, 137, 10);
    IP4_ADDR(&netmask, 255, 255, 255,  0);
    IP4_ADDR(&gw,      192, 168, 137, 1);
#endif

    /* print out IP settings of the board */

#if LWIP_DHCP==0
    print_ip_settings(&ipaddr, &netmask, &gw);
    /* print all application headers */
#endif

#if LWIP_DHCP==1
	ipaddr.addr = 0;
	gw.addr = 0;
	netmask.addr = 0;
#endif
#endif

#if LWIP_IPV6==0
    /* Add network interface to the netif_list, and set it as default */
    if (!xemac_add(netif, &ipaddr, &netmask, &gw, mac_ethernet_address, PLATFORM_EMAC_BASEADDR)) {
	xil_printf("Error adding N/W interface\r\n");
    }
#else
    /* Add network interface to the netif_list, and set it as default */
    if (!xemac_add(netif, NULL, NULL, NULL, mac_ethernet_address, PLATFORM_EMAC_BASEADDR)) {
	xil_printf("Error adding N/W interface\r\n");
	return;
    }

    netif->ip6_autoconfig_enabled = 1;

    netif_create_ip6_linklocal_address(netif, 1);
    netif_ip6_addr_set_state(netif, 0, IP6_ADDR_VALID);

    print_ip6("\n\rBoard IPv6 address ", &netif->ip6_addr[0].u_addr.ip6);
#endif


    netif_set_default(netif);

    /* specify that the network if is up */
    netif_set_up(netif);
    sys_thread_new("xemacif_input_thread", (void(*)(void*))xemacif_input_thread, netif,
            STACK_SIZE,
            DEFAULT_THREAD_PRIO);
	
#if LWIP_IPV6==0
#if LWIP_DHCP==1
    dhcp_start(netif);
    while (1) {
		dly_tsk(DHCP_FINE_TIMER_MSECS * 1000); //to unit us;

		dhcp_fine_tmr();
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) {
			dhcp_coarse_tmr();
			mscnt = 0;
		}
	}
#else
    xil_printf("\r\n");
    xil_printf("%20s %6s %s\r\n", "Server", "Port", "Connect With..");
    xil_printf("%20s %6s %s\r\n", "--------------------", "------", "--------------------");

    print_echo_app_header();
    xil_printf("\r\n");
    sys_thread_new("echod", echo_application_thread, 0,
		THREAD_STACKSIZE,
		DEFAULT_THREAD_PRIO);
    vTaskDelete(NULL);
#endif
#else
    print_echo_app_header();
  	xil_printf("\r\n");
    sys_thread_new("echod",echo_application_thread, 0,
		STACK_SIZE,
		DEFAULT_THREAD_PRIO);
#endif

}

/*
 *  メインタスク
 */
void
main_task(EXINF exinf)
{
	char	c;
	int_t	tskno = 1;
	ER_UINT	ercd;
	PRI		tskpri;
#ifndef TASK_LOOP
	SYSTIM	stime1, stime2;
#endif /* TASK_LOOP */
	HRTCNT	hrtcnt1, hrtcnt2;
#if LWIP_DHCP==1
 volatile   int mscnt = 0;
#endif
	/**
	 * 可変長プールの初期化
	 */
	init_memory_pool(MEMORY_POOL_SIZE,memory_pool);

	SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
	syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

	/*
	 *  シリアルポートの初期化
	 *
	 *  システムログタスクと同じシリアルポートを使う場合など，シリアル
	 *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
	 *  ない．
	 */
	ercd = serial_opn_por(TASK_PORTID);
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.",
									itron_strerror(ercd), SERCD(ercd));
	}
	SVC_PERROR(serial_ctl_por(TASK_PORTID,
							(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));

	/*
 	 *  ループ回数の設定
	 *
	 *  並行実行されるタスク内でのループの回数（task_loop）は，ループ
	 *  の実行時間が約0.4秒になるように設定する．この設定のために，
	 *  LOOP_REF回のループの実行時間を，その前後でget_timを呼ぶことで
	 *  測定し，その測定結果から空ループの実行時間が0.4秒になるループ
	 *  回数を求め，task_loopに設定する．
	 *
	 *  LOOP_REFは，デフォルトでは1,000,000に設定しているが，想定した
	 *  より遅いプロセッサでは，サンプルプログラムの実行開始に時間がか
	 *  かりすぎるという問題を生じる．逆に想定したより速いプロセッサで
	 *  は，LOOP_REF回のループの実行時間が短くなり，task_loopに設定す
	 *  る値の誤差が大きくなるという問題がある．そこで，そのようなター
	 *  ゲットでは，target_test.hで，LOOP_REFを適切な値に定義すること
	 *  とする．
	 *
	 *  また，task_loopの値を固定したい場合には，その値をTASK_LOOPにマ
	 *  クロ定義する．TASK_LOOPがマクロ定義されている場合，上記の測定
	 *  を行わずに，TASK_LOOPに定義された値をループの回数とする．
	 *
	 *  ターゲットによっては，ループの実行時間の1回目の測定で，本来より
	 *  も長めになるものがある．このようなターゲットでは，MEASURE_TWICE
	 *  をマクロ定義することで，1回目の測定結果を捨てて，2回目の測定結
	 *  果を使う．
	 */
#ifdef TASK_LOOP
	task_loop = TASK_LOOP;
#else /* TASK_LOOP */

#ifdef MEASURE_TWICE
	SVC_PERROR(get_tim(&stime1));
	consume_time(LOOP_REF);
	SVC_PERROR(get_tim(&stime2));
#endif /* MEASURE_TWICE */

	SVC_PERROR(get_tim(&stime1));
	consume_time(LOOP_REF);
	SVC_PERROR(get_tim(&stime2));
	task_loop = LOOP_REF * 400LU / (ulong_t)(stime2 - stime1) * 1000LU;

#endif /* TASK_LOOP */

	
	/**
	 * lwipの起動
	 */
	tcpip_init(NULL,NULL);
	    sys_thread_new("NW_THRD", network_thread, NULL,
		STACK_SIZE,
            DEFAULT_THREAD_PRIO);

#if LWIP_IPV6==0
#if LWIP_DHCP==1
while (1) {
		dly_tsk(DHCP_FINE_TIMER_MSECS*1000);
		if (server_netif.ip_addr.addr) {
			xil_printf("DHCP request success\r\n");
			print_ip_settings(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
			print_echo_app_header();
			xil_printf("\r\n");
			sys_thread_new("echod", echo_application_thread, 0,
					STACK_SIZE,
					DEFAULT_THREAD_PRIO);
			break;
		}
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS * 500) {
			xil_printf("ERROR: DHCP request timed out\r\n");
			xil_printf("Configuring default IP of 192.168.137.10\r\n");
			IP4_ADDR(&(server_netif.ip_addr),  192, 168, 137, 10);
			IP4_ADDR(&(server_netif.netmask), 255, 255, 255,  0);
			IP4_ADDR(&(server_netif.gw),  192, 168, 137, 1);
			print_ip_settings(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
			/* print all application headers */
			xil_printf("\r\n");
			xil_printf("%20s %6s %s\r\n", "Server", "Port", "Connect With..");
			xil_printf("%20s %6s %s\r\n", "--------------------", "------", "--------------------");

			print_echo_app_header();
			xil_printf("\r\n");
			sys_thread_new("echod", echo_application_thread, 0,
					STACK_SIZE,
					DEFAULT_THREAD_PRIO);
			break;
		}
	}
#else
    xil_printf("\r\n");
    xil_printf("%20s %6s %s\r\n", "Server", "Port", "Connect With..");
    xil_printf("%20s %6s %s\r\n", "--------------------", "------", "--------------------");

    print_echo_app_header();
    xil_printf("\r\n");
    sys_thread_new("echod", echo_application_thread, 0,
		THREAD_STACKSIZE,
		DEFAULT_THREAD_PRIO);
#endif
#else
    print_echo_app_header();
    xil_printf("\r\n");
    sys_thread_new("echod",echo_application_thread, 0,
		STACK_SIZE,
		DEFAULT_THREAD_PRIO);
#endif

	/**
	 * C++グローバルインスタンス初期化
	 */
	cpp_init();

	/**
	 * corba task 開始
	 */
 			
	sys_thread_new("openrtm_consoleout", (void(*)(void*))openrtm_consoleout_thread, 10,
            STACK_SIZE,
            DEFAULT_THREAD_PRIO);

		
	slp_tsk();
	SVC_PERROR(ext_ker());
	assert(0);
}
