/*
  serial.c - 为了通过串口接收或发送字节提供底层函数
  Grbl 的一部分

  版权所有 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  版权所有 2009-2011 Simen Svale Skogsrud
  
  Grbl 是自由软件：你可以在自由软件基金会的GNU 普通公共许可(GPL v3+)条款下发行，或修改它。
  Grbl的发布是希望它能有用，但没有任何保证;甚至没有隐含的保证适销性或适合某一特定目的。
  更多详细信息，请参阅GNU通用公共许可证。

  您应该已经收到GNU通用公共许可证的副本和Grbl一起。如果没有，请参阅<http://www.gnu.org/licenses/>。
*/

#include "grbl.h"

#define RX_RING_BUFFER (RX_BUFFER_SIZE+1) // 定义接收缓冲区环形队列长度
#define TX_RING_BUFFER (TX_BUFFER_SIZE+1) // 定义发送缓冲区队列长度

uint8_t serial_rx_buffer[RX_RING_BUFFER]; // 定义串口接收环形队列
data uint8_t serial_rx_buffer_head = 0; // 定义串口接收环形队列头指针
data volatile uint8_t serial_rx_buffer_tail = 0; // 定义串口接收环形队列尾指针

uint8_t serial_tx_buffer[TX_RING_BUFFER]; // 定义串口发送环形队列
data uint8_t serial_tx_buffer_head = 0; // 定义串口发送环形队列头指针
data volatile uint8_t serial_tx_buffer_tail = 0; // 定义串口发送环形队列尾指针
bit tx_busy;


// 返回串口读缓冲区可用字节数。
uint8_t serial_get_rx_buffer_available()
{
  uint8_t rtail = serial_rx_buffer_tail; // 临时变量暂存尾指针优化volatile
  if (serial_rx_buffer_head >= rtail) { return(RX_BUFFER_SIZE - (serial_rx_buffer_head-rtail)); }
  return((rtail-serial_rx_buffer_head-1));
}


// 返回串口读缓冲区已用的字节数。
// 注意：已废弃。不再被使用除非在config.h中开启了经典状态报告。
uint8_t serial_get_rx_buffer_count()
{
  uint8_t rtail = serial_rx_buffer_tail; // 临时变量暂存尾指针优化volatile
  if (serial_rx_buffer_head >= rtail) { return(serial_rx_buffer_head-rtail); }
  return (RX_BUFFER_SIZE - (rtail-serial_rx_buffer_head));
}


// 返回串口发送缓冲区已用的字节数。
// 注意：没有用到除非为了调试和保证串口发送缓冲区没有瓶颈。
uint8_t serial_get_tx_buffer_count()
{
  uint8_t ttail = serial_tx_buffer_tail; // Copy to limit multiple calls to volatile
  if (serial_tx_buffer_head >= ttail) { return(serial_tx_buffer_head-ttail); }
  return (TX_RING_BUFFER - (ttail-serial_tx_buffer_head));
}

// 串口初始化
void serial_init()
{
  SCON = 0x50;    //8位数据,可变波特率，允许串口接收数据
  AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器时钟1T模式
  T2L = BRT;      //设置定时初始值
  T2H = BRT >> 8; //设置定时初始值
  AUXR |= 0x10;		//定时器2开始计时
	ES = 1;         //使能串口中断
	tx_busy = false;
}

void usb_init()
{
    P3M0 &= ~0x03;
    P3M1 |= 0x03;
    
    IRC48MCR = 0x80;
    while (!(IRC48MCR & 0x01));
    
    USBCLK = 0x00;
    USBCON = 0x90;

    usb_write_reg(FADDR, 0x00);
    usb_write_reg(POWER, 0x08);
    usb_write_reg(INTRIN1E, 0x3f);
    usb_write_reg(INTROUT1E, 0x3f);
    usb_write_reg(INTRUSBE, 0x07);
    usb_write_reg(POWER, 0x00);

    DeviceState = DEVSTATE_DEFAULT;
    Ep0State.bState = EPSTATE_IDLE;
    InEpState = 0x00;
    OutEpState = 0x00;

    UsbInBusy = 0;
    UsbOutBusy = 0;

    IE2 |= 0x80;    //EUSB = 1;
}


// 写入一个字节到串口发送缓冲区。被主程序调用。
void serial_write(uint8_t _data) {
  // 计算下一个头指针，如果已经到达最大值，移到开始，形成环形
  uint8_t next_head = serial_tx_buffer_head + 1;
  if (next_head == TX_RING_BUFFER) { next_head = 0; }

  // 等待，直到缓冲区有空间
  while (next_head == serial_tx_buffer_tail) {
    // 代办：重构st_prep_tx_buffer()调用，在长打印期间在这里执行。
    if (sys_rt_exec_state & EXEC_RESET) { return; } // 只检查终止防止死循环。
  }

  // 储存数据并向前移动头指针
  serial_tx_buffer[serial_tx_buffer_head] = _data;
  serial_tx_buffer_head = next_head;
	// 如果发送队列不为空，启动发送
  if(!tx_busy){
    TI = 1;
    tx_busy = true;
  }
}

char putchar(char c) {
  serial_write(c);
  return c;
}

// 数据寄存器为空的中断处理
void SERIAL_TX_ISR()
{
	uint8_t tail;
  
  // 由于环形队列尾指针中断和主程序都会使用，有可能导致数据读取时，指针已经发生了变化，
  // 存在不稳定性，所以要用临时变量暂存，增加读取时的稳定性。
  tail = serial_tx_buffer_tail; // 临时变量暂存 serial_tx_buffer_tail (为volatile优化)
  // 从缓冲区发送一个字节到串口
  SBUF = serial_tx_buffer[tail];

  // 更新尾指针位置，如果已经到达顶端，返回初始位置，形成环形
  tail++;
  if (tail == TX_RING_BUFFER) { tail = 0; }

  serial_tx_buffer_tail = tail;
}


// 获取串口接收缓冲区的第一个字节。被主程序调用。
uint8_t serial_read()
{
  uint8_t tail = serial_rx_buffer_tail; // 临时变量暂存 serial_rx_buffer_tail (优化volatile)
  if (serial_rx_buffer_head == tail) { // 如果接收环形队列为空，则设置结束符号
    return SERIAL_NO_DATA;
  } else {
    uint8_t _data = serial_rx_buffer[tail]; // 从接受环形队列取一个字节

    tail++; // 更新尾指针
    if (tail == RX_RING_BUFFER) { tail = 0; } // 环形
    serial_rx_buffer_tail = tail;

    return _data;
  }
}

// 串口数据接收中断处理
void SERIAL_RX_ISR()
{
  uint8_t _data = SBUF; // 从串口数据寄存器取出数据
  uint8_t next_head; // 初始化下一个头指针

  // 直接从串行流中选取实时命令字符。这些字符不被传递到主缓冲区，但是它们设置了实时执行的系统状态标志位。
  switch (_data) {
    case CMD_RESET:         mc_reset(); break; // 调用运动控制重置程序
    case CMD_STATUS_REPORT: system_set_exec_state_flag(EXEC_STATUS_REPORT); break; // 设置为 true
    case CMD_CYCLE_START:   system_set_exec_state_flag(EXEC_CYCLE_START); break; // 设置为 true
    case CMD_FEED_HOLD:     system_set_exec_state_flag(EXEC_FEED_HOLD); break; // 设置为 true
    default :
      if (_data > 0x7F) { // 实时控制都是扩展的ASCII字符
        switch(_data) {
          case CMD_SAFETY_DOOR:   system_set_exec_state_flag(EXEC_SAFETY_DOOR); break; // 设置为 true
          case CMD_JOG_CANCEL:   
            if (sys.state & STATE_JOG) { // 阻止所有其他状态，调用运动取消。
              system_set_exec_state_flag(EXEC_MOTION_CANCEL); 
            }
            break; 
          #ifdef DEBUG
            case CMD_DEBUG_REPORT: {uint8_t sreg = SREG; cli(); bit_true(sys_rt_exec_debug,EXEC_DEBUG_REPORT); SREG = sreg;} break;
          #endif
          // 以下为实时覆盖命令
          case CMD_FEED_OVR_RESET: system_set_exec_motion_override_flag(EXEC_FEED_OVR_RESET); break;
          case CMD_FEED_OVR_COARSE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_PLUS); break;
          case CMD_FEED_OVR_COARSE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_MINUS); break;
          case CMD_FEED_OVR_FINE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_PLUS); break;
          case CMD_FEED_OVR_FINE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_MINUS); break;
          case CMD_RAPID_OVR_RESET: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_RESET); break;
          case CMD_RAPID_OVR_MEDIUM: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_MEDIUM); break;
          case CMD_RAPID_OVR_LOW: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_LOW); break;
          case CMD_SPINDLE_OVR_RESET: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_RESET); break;
          case CMD_SPINDLE_OVR_COARSE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_PLUS); break;
          case CMD_SPINDLE_OVR_COARSE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_MINUS); break;
          case CMD_SPINDLE_OVR_FINE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_PLUS); break;
          case CMD_SPINDLE_OVR_FINE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_MINUS); break;
          case CMD_SPINDLE_OVR_STOP: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_STOP); break;
          case CMD_COOLANT_FLOOD_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_FLOOD_OVR_TOGGLE); break;
          #ifdef ENABLE_M7
            case CMD_COOLANT_MIST_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_MIST_OVR_TOGGLE); break;
          #endif
        }
        // 除了上面已知的实时命令，其他的ASCII扩展字符都被丢掉
      } else { // 其他的字符被认为都是G代码，会被写入到主缓冲区
        next_head = serial_rx_buffer_head + 1; // 更新临时头指针
        if (next_head == RX_RING_BUFFER) { next_head = 0; }

        // 写入到接收缓冲区，直到它满了为止。
        if (next_head != serial_rx_buffer_tail) {
          serial_rx_buffer[serial_rx_buffer_head] = _data;
          serial_rx_buffer_head = next_head;
        }
      }
  }
}

// 串口中断响应
void serial_isr () interrupt 4 {
	if(TI) {
		TI = 0;
    // 环形队列不为空就处理
    if(serial_tx_buffer_tail != serial_tx_buffer_head) { 
      SERIAL_TX_ISR();
    } else {
      tx_busy = false;
    }
    
	}

	if(RI) {
		RI = 0;
		SERIAL_RX_ISR();
	}
}

// 重置并清空串口读缓冲区数据。用于急停和重置。
void serial_reset_read_buffer()
{
  serial_rx_buffer_tail = serial_rx_buffer_head;
}


void usb_isr() interrupt 25
{
    uint8_t intrusb;
    uint8_t intrin;
    uint8_t introut;

    intrusb = usb_read_reg(INTRUSB);
    intrin = usb_read_reg(INTRIN1);
    introut = usb_read_reg(INTROUT1);

    if (intrusb & RSUIF) usb_resume();
    if (intrusb & RSTIF) usb_reset();

    if (intrin & EP0IF) usb_setup();

#ifdef EN_EP1IN
    if (intrin & EP1INIF) usb_in_ep1();
#endif
#ifdef EN_EP2IN
    if (intrin & EP2INIF) usb_in_ep2();
#endif

#ifdef EN_EP1OUT
    if (introut & EP1OUTIF) usb_out_ep1();
#endif

    if (intrusb & SUSIF) usb_suspend();
}
