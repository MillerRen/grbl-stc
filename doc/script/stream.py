#!/usr/bin/env python
"""\

grbl控制器的G代码流

这个脚本和simple_stream.py脚本的不同点在于：它追踪grbl串口读缓冲区
中字符数。这允许grbl从串口缓冲区直接拉取下一行并且不需要从上位机等待响应。
这有效地增加了另一个缓冲层，以防止缓冲空闲。

改动:
- 20170531: 1.0秒间隔的状态报告反馈。
    可配置的波特率和报告间隔。缺陷修复。
- 20161212: 为简单流添加了推送消息反馈
- 20140714: 更新波特率到115200，添加了设置。
  通过简单的流式传输方法实现写入模式。 MIT授权.

代办: 
- 添加在流期间的实时控制命令。

---------------------
The MIT License (MIT)

Copyright (c) 2012-2017 Sungeun K. Jeon

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
---------------------
"""

import serial
import re
import time
import sys
import argparse
import threading

RX_BUFFER_SIZE = 128
BAUD_RATE = 115200
ENABLE_STATUS_REPORTS = True
REPORT_INTERVAL = 1.0 # 单位为秒

is_run = True # 控制查询计时器

# 定义命令行参数接口
parser = argparse.ArgumentParser(description='Stream g-code file to grbl. (pySerial and argparse libraries required)')
parser.add_argument('gcode_file', type=argparse.FileType('r'),
        help='g-code filename to be streamed')
parser.add_argument('device_file',
        help='serial device path')
parser.add_argument('-q','--quiet',action='store_true', default=False, 
        help='suppress output text')
parser.add_argument('-s','--settings',action='store_true', default=False, 
        help='settings write mode')        
parser.add_argument('-c','--check',action='store_true', default=False,
        help='stream in check mode')
args = parser.parse_args()

# 用于状态报告的周期性定时器。
# TODO: Need to track down why this doesn't restart consistently before a release.
def send_status_query():
    s.write('?')
    
def periodic_timer() :
    while is_run:
      send_status_query()
      time.sleep(REPORT_INTERVAL)
  

# 初始化
s = serial.Serial(args.device_file,BAUD_RATE)
f = args.gcode_file
verbose = True
if args.quiet : verbose = False
settings_mode = False
if args.settings : settings_mode = True
check_mode = False
if args.check : check_mode = True

# 唤醒grbl
print "Initializing Grbl..."
s.write("\r\n\r\n")

# 等待grbl初始化并刷新串口启动文本
time.sleep(2)
s.flushInput()

if check_mode :
    print "Enabling Grbl Check-Mode: SND: [$C]",
    s.write("$C\n")
    while 1:
        grbl_out = s.readline().strip() # 等待grbl返回回车。
        if grbl_out.find('error') >= 0 :
            print "REC:",grbl_out
            print "  Failed to set Grbl check-mode. Aborting..."
            quit()
        elif grbl_out.find('ok') >= 0 :
            if verbose: print 'REC:',grbl_out
            break

start_time = time.time();

# 启动状态报告查询定时器
if ENABLE_STATUS_REPORTS :
    timerThread = threading.Thread(target=periodic_timer)
    timerThread.daemon = True
    timerThread.start()

# grbl的G代码流
l_count = 0
error_count = 0
if settings_mode:
    # 通关简单的请求-响应流方法发送配置文件。 
    # 由于EEPROM访问期间会关闭串口中断，所以配置必须以这种方式进行流式传输。
    print "SETTINGS MODE: Streaming", args.gcode_file.name, " to ", args.device_file
    for line in f:
        l_count += 1 # 迭代行计数器   
        # l_block = re.sub('\s|\(.*?\)','',line).upper() # Strip comments/spaces/new line and capitalize
        l_block = line.strip() # 去掉EOL字符以保证一致性。
        if verbose: print "SND>"+str(l_count)+": \"" + l_block + "\""
        s.write(l_block + '\n') # 发送G代码块给grbl
        while 1:
            grbl_out = s.readline().strip() # 等待grbl返回回车。
            if grbl_out.find('ok') >= 0 :
                if verbose: print "  REC<"+str(l_count)+": \""+grbl_out+"\""
                break
            elif grbl_out.find('error') >= 0 :
                if verbose: print "  REC<"+str(l_count)+": \""+grbl_out+"\""
                error_count += 1
                break
            else:
                print "    MSG: \""+grbl_out+"\""
else:    
    # 通过改进的流协议发送g代码程序，该协议强制字符进入Grbl的串行读取缓冲区，
    # 以确保Grbl能够立即访问下一个g代码命令，而不是等待调用响应串行协议完成。 
    # 这是通过仔细计算流发送到Grbl的字符数并跟踪Grbl响应来完成的，
    # 这样我们就不会溢出Grbl串行读取缓冲区。 
    g_count = 0
    c_line = []
    for line in f:
        l_count += 1 # Iterate line counter
        l_block = re.sub('\s|\(.*?\)','',line).upper() # 移除注释空行换行并大写
        # l_block = line.strip()
        c_line.append(len(l_block)+1) # 跟踪grbl串口读缓冲区的字符数
        grbl_out = '' 
        while sum(c_line) >= RX_BUFFER_SIZE-1 | s.inWaiting() :
            out_temp = s.readline().strip() # 等待响应
            if out_temp.find('ok') < 0 and out_temp.find('error') < 0 :
                print "    MSG: \""+out_temp+"\"" # 调试响应
            else :
                if out_temp.find('error') >= 0 : error_count += 1
                g_count += 1 # 迭代G代码计数器
                if verbose: print "  REC<"+str(g_count)+": \""+out_temp+"\""
                del c_line[0] # 删除与最后一个“ok”对应的块字符计数
        s.write(l_block + '\n') # 发送G代码块到grbl
        if verbose: print "SND>"+str(l_count)+": \"" + l_block + "\""
    # 等待直到所有响应都被接收了。
    while l_count > g_count :
        out_temp = s.readline().strip() # 等待grbl响应
        if out_temp.find('ok') < 0 and out_temp.find('error') < 0 :
            print "    MSG: \""+out_temp+"\"" # 调试响应
        else :
            if out_temp.find('error') >= 0 : error_count += 1
            g_count += 1 # 迭代G代码计数器
            del c_line[0] # 删除最有一个"ok"对应的字符计数
            if verbose: print "  REC<"+str(g_count)+": \""+out_temp + "\""

# 流完成后等待用户输入
print "\nG-code streaming finished!"
end_time = time.time();
is_run = False;
print " Time elapsed: ",end_time-start_time,"\n"
if check_mode :
    if error_count > 0 :
        print "CHECK FAILED:",error_count,"errors found! See output for details.\n"
    else :
        print "CHECK PASSED: No errors found in g-code program.\n"
else :
   print "WARNING: Wait until Grbl completes buffered g-code blocks before exiting."
   raw_input("  Press <Enter> to exit and disable Grbl.") 

# 关闭文件和串口端口
f.close()
s.close()
