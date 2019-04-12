# TRF7960-13-56Mhz-code
基于TRF7960的相关代码与资料
**跟TRF7960“纠缠”了许久，官网和一些论坛上面的代码大多是用MSP340作为主控，但是我用的是c8051f340，所以一些设置上有差别。通过基本的代码，已经能实现15693协议的所有功能，但14443A/B和TAG-IT的最基本的防碰撞读UID都没有实现，但是已经根据15693实现的流程进行了修改。**
------------------------------------------
# 背景说明
通过TI的TRF7960a与C8051f340对13.56MHz进行项目开发，实现15693协议，14443A/B协议，Tag-it协议的读写操作。

-----------------------------
# 项目经过
1. 了解15693，14443A/B，Tag-it的协议内容，熟悉各个协议的工作流程，尤其指令的含义与功能。
2. 了解C8051f340的操作，并进行demo的编写
3. 收集相关开发文档和程序源码，在其他大佬的工作基础上进行快捷开发。
4. 编写初步demo，开始为15693，接着为14443a/b，tag-it
5. 对代码进行debug

----------------------

# 目前进度
实现了15693的防碰撞以及其他读写功能，而其他协议射频芯片对PICC没有响应。

---------------------
# 问题说明
1. 在**anticollision.c**中的RequestCommand函数中，多次对i_reg赋值为1，而程序中有多个while(i_reg==1)的死循环，导致程序卡死。

![在这里插入图片描述](https://img-blog.csdnimg.cn/2019040321251918.png)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190403212557688.png)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190403212612845.png)
对于这个问题，我思考如下：RequestCommand函数在15693协议中也用到了。在debug之后发现，但是在函数中，while语句之前程序进入了Port_0函数（即外部中断函数），中断函数中有一个中断处理函数InterruptHandlerReader（），在这个函数中i_reg值进行改变了。因此对于14443A/B和Tag-it来说，问题应该在于中断函数。在进行debug后又发现，中断函数处理第一次发送指令给TRF7960进行了响应，所以i_reg的值变为了0x00。但是之后，程序对i_reg进行赋值为1，在while之前就没有产生中断。于是就不可能改变i_reg的值。
我尝试将赋值语句注释掉，但是后面又无法进入AnticollisionLoopA（）函数，于是程序陷入死局。

2. 在上一个问题的基础上，尝试探究为什么TRF7960对14443A/B的PICC没有响应。
首先就是对于参数设置的再次对应，发现这没有问题。然后又考虑了时钟频率的问题，这部分可能真的有问题，因为c8051内部为12M晶振，这个频率的晶振时间最不准确，因此问题可能就是出在这。还考虑过C8051对TRF7960a指令没有写入，但是读取buff里面数据后发现是成功写入了TRF7960的，说明TRF7960没有执行指令，或是执行指令但是PICC没有响应。这个问题尝试过查找资料。
3. 之前由于不知道TI公司没有被“墙”，所以在CSDN上傻乎乎的花了很多积分下载相关资料，而且许多还是重复。在[TI的资料](http://www.ti.com.cn/product/cn/TRF7960A/description)中提到可以支持15693,14443A/B，tag-it等13.56MHz协议，但是提供的资料只有15693和14443b。![在这里插入图片描述](https://img-blog.csdnimg.cn/2019040321243163.png)
且提供的源码为TRF7970的
![在这里插入图片描述](https://img-blog.csdnimg.cn/2019040321273444.png)
接着在国内搜索相关问题和类似用TRF7960进行读取的案例，资料很少，而且基本上都没有用c8051作为主控。然后在国外进行搜索，刚好有类似的帖子


![在这里插入图片描述](https://img-blog.csdnimg.cn/20190403213402393.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0Nhb3lhbmdfSGU=,size_16,color_FFFFFF,t_70)
但是这个问题也没有解决，网友提供的解决方案也都试过，都没有用。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190403213602815.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0Nhb3lhbmdfSGU=,size_16,color_FFFFFF,t_70)
还有一些大佬做过类似工作，但是也只完成了15693协议部分

4. 还有的问题就比较奇诡了，例如开始说道15693协议 也用到了RequestCommand()函数，但是我在这个函数中，断点位置不同，到某个地方的值就完全不同，而且不同的值导致程序运行的结果完全不同，一个正常运行，一个就陷入循环。![在这里插入图片描述](https://img-blog.csdnimg.cn/20190403214338613.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0Nhb3lhbmdfSGU=,size_16,color_FFFFFF,t_70)
5. 还有的问题就是对于串口发送和接收的问题，即C8051能通过sendchar（）函数发送出去字符，但是没有办法通过==void RXhandler (void) interrupt 4==接收到数据。而且sendchar函数的构造也很奇诡

```c
sendchar(char TXchar)
{
     
      if (TXchar == '\n')  {                // check for newline character
         while (!TI0);                 // wait until UART0 is ready to transmit
         TI0 = 0;                      // clear interrupt flag
         SBUF0 = 0x0d;                 // output carriage return command
      }
      while (!TI0);                    // wait until UART0 is ready to transmit
      TI0 = 0;
	  SBUF0 = TXchar;                         // clear interrupt flag
      return (SBUF0);              // output <c> using UART 0
}

```




