//---------------------------------------------------------------------
// ATtiny26 8MHz
// https://nicotak.com 
// by Takuya Matsubara / NICO Corp.2008
//
// PS/2Keyboard(Japanese) to X68000 connector

// program internal 8MHz
//avrsp -FL11100100

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

void kb_decode(void);

char bitcount=11;
unsigned char tempdata;// Holds the received scan code

char kb_buffcnt=0;
unsigned char kb_buffer;
char kb_bcntisup=0;
unsigned char kb_bufisup;

#define FLAG_ISUP  (1<<0)
#define FLAG_EZERO (1<<1)

char kb_flag;


//*********************
// Pin definition file
//*********************
// Keyboard connections

#define LED_MASK	(1<<6)

#define LED_PORT PORTB
#define LED_DDR  DDRB

#define PS2_DATAPIN  PINA
#define PS2_DATAPORT PORTA
#define PS2_DATADDR  DDRA
#define PS2_DATABIT  (1<<0)

#define PS2_CLKPIN   PINA
#define PS2_CLKPORT  PORTA
#define PS2_CLKDDR   DDRA
#define PS2_CLKBIT   (1<<7)


#define PS2INTR_ENABLE	GIMSK|=(1<<PCIE1)	// 割り込み許可
#define PS2INTR_DISABLE	GIMSK&=~(1<<PCIE1)	// 割り込み禁止


void mouse_sendbyte(unsigned char data);
unsigned char mouse_recvbyte(void);
void ps2_clkwait(void);


// (Japanese Keyboard)
// Unshifted characters
PROGMEM unsigned char keytable[] = {
	0        //0x00
	,0x6b    //0x01 f9 
	,0       //0x02   
	,0x67    //0x03 f5 
	,0x65    //0x04 f3 
	,0x63    //0x05 f1 
	,0x64    //0x06 f2 
	,0x73    //0x07 f12 ==> OPT.2
	,0x69    //0x08 f7  
	,0x6c    //0x09 f10 
	,0x6a    //0x0a f8  
	,0x68    //0x0b f6 
	,0x66    //0x0c f4 
	,0x10    //0x0d tab 
	,0x60    //0x0e zenkaku 
	,0       //0x0f    
	,0       //0x10    
	,0x55    //0x11 alt ==> XF1 
	,0x70    //0x12 shift 
	,0x5f    //0x13
	,0x71    //0x14 ctrl
	,0x11    //0x15 q 
	,0x02    //0x16 1 
	,0       //0x17
	,0       //0x18
	,0       //0x19
	,0x2a    //0x1a z 
	,0x1f    //0x1b s 
	,0x1e    //0x1c a 
	,0x12    //0x1d w 
	,0x03    //0x1e 2 
	,0       //0x1f windows
	,0       //0x20  
	,0x2c    //0x21 c 
	,0x2b    //0x22 x 
	,0x20    //0x23 d 
	,0x13    //0x24 e 
	,0x05    //0x25 4 
	,0x04    //0x26 3 
	,0       //0x27
	,0       //0x28
	,0x35    //0x29 space  
	,0x2d    //0x2a v 
	,0x21    //0x2b f 
	,0x15    //0x2c t 
	,0x14    //0x2d r 
	,0x06    //0x2e 5 
	,0       //0x2f
	,0       //0x30  
	,0x2f    //0x31 n 
	,0x2e    //0x32 b 
	,0x23    //0x33 h 
	,0x22    //0x34 g 
	,0x16    //0x35 y 
	,0x07    //0x36 6 
	,0       //0x37
	,0       //0x38
	,0x31    //0x39 ,    
	,0x30    //0x3a m 
	,0x24    //0x3b j 
	,0x17    //0x3c u 
	,0x08    //0x3d 7 
	,0x09    //0x3e 8 
	,0       //0x3f
	,0       //0x40
	,0x31    //0x41 ,    
	,0x25    //0x42 k 
	,0x18    //0x43 i 
	,0x19    //0x44 o 
	,0x0b    //0x45 zero 
	,0x0a    //0x46 9 
	,0       //0x47
	,0       //0x48
	,0x32    //0x49 . 
	,0x33    //0x4a / 
	,0x26    //0x4b L 
	,0x27    //0x4c ; 
	,0x1a    //0x4d p 
	,0x0c    //0x4e - 
	,0       //0x4f
	,0       //0x50
	,0x34    //0x51 yen_
	,0x28    //0x52 : 
	,0       //0x53   
	,0x1b    //0x54 @ 
	,0x0d    //0x55 ^ 
	,0       //0x56
	,0       //0x57
	,0x5d    //0x58 caps
	,0x70    //0x59 shift 
	,0x1d    //0x5a ENTER 
	,0x1c    //0x5b [
	,0       //0x5c
	,0x29    //0x5d ]  
	,0       //0x5e
	,0       //0x5f
	,0       //0x60
	,0       //0x61
	,0       //0x62
	,0       //0x63
	,0x56    //0x64 henkan ==>XF2
	,0       //0x65
	,0x0f    //0x66 BS 
	,0x57    //0x67 muhenkan ==> XF3
	,0       //0x68
	,0x4b    //0x69 1 
	,0x0e    //0x6a yen| 
	,0x47    //0x6b 4 
	,0x43    //0x6c 7 
	,0       //0x6d
	,0       //0x6e
	,0       //0x6f
	,0x4f    //0x70 0 
	,0x51    //0x71 . 
	,0x4c    //0x72 2 
	,0x48    //0x73 5 
	,0x49    //0x74 6 
	,0x44    //0x75 8 
	,0x01    //0x76 ESC 
	,0       //0x77 num 
	,0x72    //0x78 F11 ==> OPT.1
	,0x46    //0x79 +   
	,0x4d    //0x7a 3   
	,0x42    //0x7b -   
	,0x41    //0x7c *   
	,0x45    //0x7d 9   
};


// Shifted characters
PROGMEM unsigned char keytable2[] = {
	0x61     // 0x69 end ==> BREAK
	,0    // 0x6a 
	,0x3b // 0x6b LEFT
	,0x36 // 0x6c home
	,0    // 0x6d
	,0    // 0x6e
	,0    // 0x6f
	,0x5e // 0x70 ins
	,0x37 // 0x71 del    
	,0x3e // 0x72 DOWN
	,0    // 0x73
	,0x3d // 0x74 RIGHT
	,0x3c // 0x75 UP
	,0    // 0x76
	,0    // 0x77
	,0    // 0x78
	,0    // 0x79
	,0x39 // 0x7a pagedown
	,0    // 0x7b
	,0    // 0x7c
	,0x38 // 0x7d pageup
};

void ps2_clk_high(void)
{
	PS2_CLKPORT |= PS2_CLKBIT;//pullup(ハード的にプルアップしてるなら不要)
	PS2_CLKDDR &= ~PS2_CLKBIT;
}

void ps2_clk_low(void)
{
	PS2_CLKPORT &= ~PS2_CLKBIT;
	PS2_CLKDDR |= PS2_CLKBIT;
}

void ps2_data_high(void)
{
	PS2_DATAPORT |= PS2_DATABIT;//pullup(ハード的にプルアップしてるなら不要)
	PS2_DATADDR &= ~PS2_DATABIT;
}

void ps2_data_low(void)
{
	PS2_DATAPORT &= ~PS2_DATABIT;
	PS2_DATADDR |= PS2_DATABIT;
}

//----------keyboard init(init_kb)
void kb_init(void)
{
	ps2_data_high();
	ps2_clk_high();

//init intr.
	ACSR |= (1<<ACD);	//Analog Comparator Disable
	PS2INTR_ENABLE;	// 割り込み許可
	GIFR|=(1<<PCIF);
}

//----------keyboard Clock intrrupt
// clk割り込み
SIGNAL (SIG_PIN_CHANGE)
{
// Routine entered at falling edge
	unsigned char datapin=0;

//	PS2INTR_ENABLE;	// 割り込み許可
//	GIFR|=(1<<PCIF);

	if(PS2_CLKPIN & PS2_CLKBIT)
		return;	// 立ち上がりなら不要

//	LED_PORT |= LED_MASK;	//LED on

	if(PS2_DATAPIN & PS2_DATABIT)
		datapin =0x80;

	if(bitcount==11){//startbit
		if(datapin){// DATA==HIGH error
			return;
		}
	}else if(bitcount == 2){//Parity bit
		//
	}else if(bitcount == 1){//stopbit
		bitcount = 11;
		if(datapin==0){//DATA==LOW error
			return;
		}
		kb_decode();
		return;
	}else{//data bit
		tempdata >>= 1;
		tempdata |= datapin;// Store a '1'
	}
	bitcount--;
}

//----------keyboard decode(decode)
void kb_decode(void)
{
	PGM_P p = (PGM_P)keytable;
	unsigned char kb_keycode;

	if(tempdata==0xe0){
		kb_flag |= FLAG_EZERO;
		return;
	}
	if(tempdata==0xf0){// The up-key identifier
		kb_flag |= FLAG_ISUP;	//sw ON-->OFF
		return;
	}

	if(kb_flag & FLAG_EZERO){
		if(tempdata >= 0x69){
			tempdata -= 0x69;
			p = (PGM_P)keytable2;
		}
	}
	kb_keycode = pgm_read_byte(p + tempdata);
	if(kb_keycode){
		if(kb_flag & FLAG_ISUP){
			kb_bufisup = kb_keycode;
			kb_bcntisup = 1;
		}else{
			kb_buffer = kb_keycode;
			kb_buffcnt = 1;
		}
	}
	kb_flag &= ~(FLAG_EZERO | FLAG_ISUP);
}



//----------keyboard get buffer
unsigned char kb_getbuff(void)
{
	if(kb_buffcnt==0) return(0);

	kb_buffcnt--;
	return(kb_buffer);
}

//----------keyboard get buffer
unsigned char kb_getbuffisup(void)
{
	if(kb_bcntisup==0) return(0);

	kb_bcntisup--;
	return(kb_bufisup);
}



#ifndef F_CPU
#define   F_CPU   8000000    //CPUクロック周波数 8MHz
#endif

//#define BAUDRATE 9600        //ボーレート
#define BAUDRATE 2400        //ボーレート

#if BAUDRATE==2400
	#define PRESCALE  16        //プリスケーラ値
	#define PRESCALECR 5        //プリスケーラ設定値
#endif
#if BAUDRATE==9600
	#define PRESCALE  4         //プリスケーラ値
	#define PRESCALECR 3        //プリスケーラ設定値
#endif

#define SIO_PORT PORTB      //ISP用ポート
#define SIO_PIN  PINB       //ISP用ポート入力
#define SIO_DDR  DDRB       //ISP用ポート入出力設定
#define SIO_TXMASK   (1<<5) //送信ビット


#define  SIO_TIFR    TIFR   //タイマ1フラグ
#define  SIO_MAXCNT  0x100  //タイマ最大値

#define SIO_TCNT    (SIO_MAXCNT-((F_CPU/PRESCALE)/BAUDRATE))
#define TCNT_100US  (SIO_MAXCNT-((F_CPU/PRESCALE)/(1000000/100)))


//void sio_bitwait(unsigned char dat);
void sio_tx(char sio_txdat);
void timer_wait(unsigned char cnt);


//-----------------------------------------------------------------------
// ウエイト
void timer_wait(unsigned char cnt)
{
	TCNT1 = cnt;
	SIO_TIFR |= (1 << TOV1);  // TIFRのビットをクリア

	while(!(SIO_TIFR & (1 << TOV1)));
	// TOV1ビットが1になるまで
}

void sio_bitwait(void)
{
	while(!(SIO_TIFR & (1 << TOV1)));
	// TOV1ビットが1になるまで

	TCNT1 = SIO_TCNT;
	SIO_TIFR |= (1 << TOV1);  // TIFRのビットをクリア
}

//-----------------------------------------------------------------------
// 1バイト送信
//引数 sio_txdat：送信データ0x00-0xff

//#define SIO_MARK	SIO_PORT&=~SIO_TXMASK	// 無通信状態
//#define SIO_SPACE	SIO_PORT|=SIO_TXMASK	// 0

#define SIO_MARK	SIO_PORT|=SIO_TXMASK	// 無通信状態
#define SIO_SPACE	SIO_PORT&=~SIO_TXMASK	// 0

void sio_tx(char sio_txdat)
{
	unsigned char mask = 0x01;

	//スタートビット
	SIO_SPACE;
	TCNT1 = SIO_TCNT;
	SIO_TIFR |= (1 << TOV1);  // TIFRのビットをクリア
	sio_bitwait();

	//データビット0-7(下位から送信)
	while(mask != 0){	
		if(sio_txdat & mask)
			SIO_MARK;
		else
			SIO_SPACE;

		mask <<= 1;
		sio_bitwait();
	}
	//ストップビット
	SIO_MARK;
	sio_bitwait();
}


void wait_msec(int i)
{
	int z;

	while(i--){
		for(z=0; z<10; z++){
			timer_wait(TCNT_100US);
		}
	}
}


int main(void)
{
	unsigned char keycode;

	PORTB = 0xff;           //Pullup
	PORTA = 0xff;           //Pullup
	LED_DDR |= LED_MASK;
	LED_PORT |= LED_MASK;   //LED on

	// シリアルポート初期化
	SIO_DDR |= SIO_TXMASK;
	SIO_MARK;	//無通信状態

	TCCR1A= 0;              // タイマ1 モード 
	TCCR1B= PRESCALECR;     // タイマ1 プリスケーラ設定

	wait_msec(1000);        //約1000msec
	kb_init();
	LED_PORT &= ~LED_MASK;  //LED off

	sei();
	//debug
//	while(1)
//	{
//		LED_PORT |= LED_MASK;	//LED on
//		sio_tx('A');
//		LED_PORT &= ~LED_MASK;	//LED off
//		wait_msec(1000);	//約1000msec
//	}

	while(1)
	{
		keycode = kb_getbuffisup();
		if(keycode != 0 ){	//is up
			keycode |= 0x80;
			LED_PORT &= ~LED_MASK;	//LED off
			sio_tx(keycode);
		}
		keycode = kb_getbuff();
		if(keycode != 0 ){		//is down
			LED_PORT |= LED_MASK;	//LED on
			sio_tx(keycode);
		}
	}
	return(0);
}

