#include <string.h>
#include "websocket/private-libwebsockets.h"
#include "../example.h"

#if (EXAMPLE_USE_DEMO==WEBSOCKET_TEST_DEMO)

static unsigned int opts;
static int was_closed;
static struct libwebsocket *wsi_mirror;
static int mirror_lifetime = 0;
static volatile int force_exit = 0;
static int longlived = 0;
char f_test_web_sockets = 0;

#define HFGPIO_F_WEBSOCKET_DATA			(HFGPIO_F_USER_DEFINE+0)
#define HFGPIO_F_WEBSOCKET_START			(HFGPIO_F_USER_DEFINE+1)

const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE]=
{
	HF_M_PIN(2),	//HFGPIO_F_JTAG_TCK
	HFM_NOPIN,	//HFGPIO_F_JTAG_TDO
	HFM_NOPIN,	//HFGPIO_F_JTAG_TDI
	HF_M_PIN(5),	//HFGPIO_F_JTAG_TMS
	HFM_NOPIN,		//HFGPIO_F_USBDP
	HFM_NOPIN,		//HFGPIO_F_USBDM
	HF_M_PIN(39),	//HFGPIO_F_UART0_TX
	HF_M_PIN(40),	//HFGPIO_F_UART0_RTS
	HF_M_PIN(41),	//HFGPIO_F_UART0_RX
	HF_M_PIN(42),	//HFGPIO_F_UART0_CTS
	
	HF_M_PIN(27),	//HFGPIO_F_SPI_MISO
	HF_M_PIN(28),	//HFGPIO_F_SPI_CLK
	HF_M_PIN(29),	//HFGPIO_F_SPI_CS
	HF_M_PIN(30),	//HFGPIO_F_SPI_MOSI
	
	HFM_NOPIN,	//HFGPIO_F_UART1_TX,
	HFM_NOPIN,	//HFGPIO_F_UART1_RTS,
	HFM_NOPIN,	//HFGPIO_F_UART1_RX,
	HFM_NOPIN,	//HFGPIO_F_UART1_CTS,
	
	HF_M_PIN(43),	//HFGPIO_F_NLINK
	HF_M_PIN(44),	//HFGPIO_F_NREADY
	HFM_NOPIN,	//HFGPIO_F_NRELOAD:45
	HF_M_PIN(7),	//HFGPIO_F_SLEEP_RQ
	HFM_NOPIN,	//HFGPIO_F_SLEEP_ON:8
		
	HF_M_PIN(15),		//HFGPIO_F_WPS
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HF_M_PIN(8),	//HFGPIO_F_USER_DEFINE
	HF_M_PIN(45),
};

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{NULL,NULL,NULL,NULL} //the last item must be null
};

/*
 * This demo shows how to connect multiple websockets simultaneously to a
 * websocket server (there is no restriction on their having to be the same
 * server just it simplifies the demo).
 *
 *  dumb-increment-protocol:  we connect to the server and print the number
 *				we are given
 *
 *  lws-mirror-protocol: draws random circles, which are mirrored on to every
 *				client (see them being drawn in every browser
 *				session also using the test server)
 */

enum demo_protocols {
	/*HF IOT Test*/
	PROTOCOL_HFIOT_TEST,

	PROTOCOL_DUMB_INCREMENT,
	PROTOCOL_LWS_MIRROR,

	/* always last */
	DEMO_PROTOCOL_COUNT
};

static void USER_FUNC do_websocket_data(void);

/* dumb_increment protocol */

static int
callback_dumb_increment(struct libwebsocket_context *this,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	//lwsl_notice("callback_dumb_increment get called with reason %d\n", reason);
	
	switch (reason) {

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lwsl_info("callback_dumb_increment: LWS_CALLBACK_CLIENT_ESTABLISHED\n");
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_info("LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
		was_closed = 1;
		break;

	case LWS_CALLBACK_CLOSED:
		lwsl_info("LWS_CALLBACK_CLOSED\n");
		was_closed = 1;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		((char *)in)[len] = '\0';
		lwsl_info("rx %d '%s'\n", (int)len, (char *)in);
		do_websocket_data();
		break;

	default:
		break;
	}

	return 0;
}


/* lws-mirror_protocol */


static int
callback_lws_mirror(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	int l = 0;
	int n;
	unsigned char lws_mirror_buf[LWS_SEND_BUFFER_PRE_PADDING + 80 +  LWS_SEND_BUFFER_POST_PADDING];

	switch (reason) {

	case LWS_CALLBACK_CLIENT_ESTABLISHED:

		mirror_lifetime = 10 + (rand() & 1023);
		/* useful to test single connection stability */
		if (longlived)
			mirror_lifetime += 50000;

		lwsl_notice( "opened mirror connection with %d lifetime\n", mirror_lifetime);

		/*
		 * mirror_lifetime is decremented each send, when it reaches
		 * zero the connection is closed in the send callback.
		 * When the close callback comes, wsi_mirror is set to NULL
		 * so a new connection will be opened
		 */

		/*
		 * start the ball rolling,
		 * LWS_CALLBACK_CLIENT_WRITEABLE will come next service
		 */

		libwebsocket_callback_on_writable(context, wsi);
		break;

	case LWS_CALLBACK_CLOSED:
		lwsl_info("mirror: LWS_CALLBACK_CLOSED mirror_lifetime=%d\n", mirror_lifetime);
		wsi_mirror = NULL;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		//lwsl_info( "rx %d '%s'\n", (int)len, (char *)in);
		do_websocket_data();
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:

		for (n = 0; n < 1; n++)
			l += sprintf((char *)&lws_mirror_buf[LWS_SEND_BUFFER_PRE_PADDING + l],
					"c #%06X %d %d %d;",
					(int)rand() & 0xffffff,
					(int)rand() % 500,
					(int)rand() % 250,
					(int)rand() % 24);

		n = libwebsocket_write(wsi,
		   &lws_mirror_buf[LWS_SEND_BUFFER_PRE_PADDING], l, opts | LWS_WRITE_TEXT);

		if (n < 0)
			return -1;
		if (n < l) {
			lwsl_err("Partial write LWS_CALLBACK_CLIENT_WRITEABLE\n");
			return -1;
		}

		mirror_lifetime--;
		if (!mirror_lifetime) {
			lwsl_err("closing mirror session\n");
			return -1;
		} else
			/* get notified as soon as we can write again */
			libwebsocket_callback_on_writable(context, wsi);
		do_websocket_data();
		break;

	default:
		break;
	}

	return 0;
}

/* hfiot-test_protocol */
#define MAX_HFIOT_WAIT_CYCLE 20000
int wait_cycle;

static int
callback_hfiot_test(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	int l = 0;
	int n;
	unsigned char hfiot_test_buf[LWS_SEND_BUFFER_PRE_PADDING + 128 +  LWS_SEND_BUFFER_POST_PADDING];

	switch (reason) {

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		wait_cycle = MAX_HFIOT_WAIT_CYCLE;
		libwebsocket_callback_on_writable(context, wsi);
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_info("LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
		was_closed = 1;
		break;

	case LWS_CALLBACK_CLOSED:
		lwsl_info("LWS_CALLBACK_CLOSED\n");
		was_closed = 1;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		lwsl_notice( "rx %d: '%s'\n", (int)len, (char *)in);
		do_websocket_data();
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		if(--wait_cycle<=0){
			wait_cycle = MAX_HFIOT_WAIT_CYCLE;
			for (n = 0; n < 1; n++)
				l += sprintf((char *)&hfiot_test_buf[LWS_SEND_BUFFER_PRE_PADDING + l],
						"{\"CID\":1}");

			n = libwebsocket_write(wsi,
		   			&hfiot_test_buf[LWS_SEND_BUFFER_PRE_PADDING], l, opts | LWS_WRITE_TEXT);

			if (n < 0)
				return -1;
			if (n < l) {
				lwsl_err("Partial write LWS_CALLBACK_CLIENT_WRITEABLE\n");
				return -1;
			}
			do_websocket_data();
		}
		libwebsocket_callback_on_writable(context, wsi);
		break;

	default:
		break;
	}

	return 0;
}

/* list of supported protocols and callbacks */

static struct libwebsocket_protocols protocols[] = {
	{
		NULL,//"lws-hfito-protocol",
		callback_hfiot_test,
		0,
		128,
	},
	{
		"dumb-increment-protocol,fake-nonexistant-protocol",
		callback_dumb_increment,
		0,
		20,
	},
	{
		"fake-nonexistant-protocol,lws-mirror-protocol",
		callback_lws_mirror,
		0,
		128,
	},
	{ NULL, NULL, 0, 0 } /* end */
};

char test_server_address[]="192.168.2.103";
int test_port=7681; //8443; //8080;

int test_websocket_client()
{
	int n = 0;
	int ret = 0;
	int port = test_port;
	int use_ssl;
	struct libwebsocket_context *context;
	const char *address;
	struct libwebsocket *wsi_dumb, *wsi_hfiot;
	int ietf_version = -1; /* latest */
	struct lws_context_creation_info info;

	lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE);

	memset(&info, 0, sizeof info);

	use_ssl = 0;
	ietf_version = 13/*atoi(optarg)*/;
	longlived = 1;
	address = test_server_address/*argv[optind]*/;

	/*
	 * create the websockets context.  This tracks open connections and
	 * knows how to route any traffic and which protocol version to use,
	 * and if each connection is client or server side.
	 *
	 * For this client-only demo, we tell it to not listen on any port.
	 */

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;

	context = libwebsocket_create_context(&info);
	if (context == NULL) {
		lwsl_err("Creating libwebsocket context failed\n");
		return 1;
	}
#if 1
	/* create a client websocket using dumb increment protocol */
	wsi_dumb = libwebsocket_client_connect(context, address, port, use_ssl,
				"/", address, address,
				 protocols[PROTOCOL_DUMB_INCREMENT].name, ietf_version);
	
	if (wsi_dumb == NULL) {
		lwsl_err("libwebsocket connect failed\n");
		ret = 1;
		goto bail;
	}
	
	lwsl_info("Waiting for connect...\n");
#endif

#if 0
	/* create a client websocket using mirror protocol */
	wsi_hfiot = libwebsocket_client_connect(context,	address, port, use_ssl, 
					"/svc", address, address,
					protocols[PROTOCOL_HFIOT_TEST].name, ietf_version);
		
	if (wsi_hfiot == NULL) {
		lwsl_err("libwebsocket hfiot connect failed\n");
		ret = 1;
		goto bail;
	}
#endif

	/*
	 * sit there servicing the websocket context to handle incoming
	 * packets, and drawing random circles on the mirror protocol websocket
	 * nothing happens until the client websocket connection is
	 * asynchronously established
	 */

	n = 0;
	while (n >= 0 && !was_closed && !force_exit) {
		n = libwebsocket_service(context, 10);

		if (n < 0)
			continue;

		if (wsi_mirror)
			continue;
#if 1
		/* create a client websocket using mirror protocol */
		wsi_mirror = libwebsocket_client_connect(context,	address, port, use_ssl, 
					"/", address, address,
					protocols[PROTOCOL_LWS_MIRROR].name, ietf_version);
		
		if (wsi_mirror == NULL) {
			lwsl_err("libwebsocket mirror connect failed\n");
			ret = 1;
			goto bail;
		}
#endif
	}

bail:

	libwebsocket_context_destroy(context);

	return ret;

}

static void USER_FUNC do_websocket_data(void)
{
	if(hfgpio_fpin_is_high(HFGPIO_F_WEBSOCKET_DATA))
		hfgpio_fset_out_low(HFGPIO_F_WEBSOCKET_DATA);
	else
		hfgpio_fset_out_high(HFGPIO_F_WEBSOCKET_DATA);
}

static void USER_FUNC do_websocket_test(uint32_t arg1,uint32_t arg2)
{
	if(!hfgpio_fpin_is_high(HFGPIO_F_WEBSOCKET_START))
	{
		f_test_web_sockets=1;
		u_printf("Press the reload button and start websocket test\n");
	}
}

static void USER_FUNC set_test_gpio()
{
	hfgpio_fset_out_high(HFGPIO_F_WEBSOCKET_DATA);
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_WEBSOCKET_START,HFPIO_IT_EDGE,do_websocket_test,1)!=HF_SUCCESS)
	{
		u_printf("configure HFGPIO_F_WEBSOCKET_START fail\n");
		return;
	}
}

void Websockets(void)
{

	while(1){
		if(!f_test_web_sockets)
		{
			msleep(10000);
			continue;
		}
		test_websocket_client();
		f_test_web_sockets=0;
	}
}

int USER_FUNC app_main (void)
{
	if(hfgpio_fmap_check()!=0)
	{
		while(1)
		{
			HF_Debug(DEBUG_ERROR,"gpio map file error\n");
			msleep(1000);
		}
		return 0;
	}

	while(!hfnet_wifi_is_active())
	{
		msleep(50);
	}

	if(hfnet_start_uart(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)NULL)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start uart fail!\n");
	}
	
	set_test_gpio();

	hfthread_create((PHFTHREAD_START_ROUTINE)Websockets,"Websockets",768,NULL,3,NULL,NULL);

	return 1;
}

#endif
