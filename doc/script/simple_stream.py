#!/usr/bin/env python
"""\
grbl的简单G代码流脚本

作为grbl基本通信接口的示例。当grbl完成了解析G代码块后，将会返回'ok'或'error'。
当规划器缓冲区满了时，grbl将不会发送返回信息知道规划器缓冲区腾出空间。

G02/03 圆弧指令是个例外，它们直接注入短线段到规划器。因此在圆弧期间可能没有返回。

---------------------
The MIT License (MIT)

Copyright (c) 2012 Sungeun K. Jeon

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
import time

# 打开grbl串口端口
s = serial.Serial('/dev/tty.usbmodem1811',115200)

# 代开G代码文件
f = open('grbl.gcode','r');

# 唤醒grbl
s.write("\r\n\r\n")
time.sleep(2)   # 等待grbl初始化
s.flushInput()  # 刷新串行端口的启动文本

# G代码流给grbl
for line in f:
    l = line.strip() # 去除所有EOL字符以保证一致性
    print 'Sending: ' + l,
    s.write(l + '\n') # 发送G代码块给grbl
    grbl_out = s.readline() # 等待grbl回车响应
    print ' : ' + grbl_out.strip()

# 等待直到grbl完成，关闭串口端口和文件
raw_input("  Press <Enter> to exit and disable grbl.") 

# 关闭文件和串口端口
f.close()
s.close()    