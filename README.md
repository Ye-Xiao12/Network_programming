# c++高并发网络编程  项目笔记    
## 高性能服务器基本框架
![image](https://github.com/Ye-Xiao12/Network_programming/blob/master/picture/%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E6%A1%86%E6%9E%B6.svg)  
## 1.Socket 编程  
- 服务端：1.创建socket()  2.绑定端口 bind()  3.监听端口 listen()  4.接受连接 accept()  5.接收数据 recv()  6.发送数据 send()  7.关闭端口 close();  
- 客户端：1.创建socket()  2.连接服务器 connect()  3.接收数据 recv()  4.发送数据 send()  5.关闭连接 close()  

## 2.非阻塞网络模式 select模式  
### 四个基本函数：  
 - FD_SET(_sock,&fdReads):将_sock加入待观察数组fdReads  
 - FD_CLR(_sock,&fdReads):将_sock从待观察数组fdReads中移除  
 - FD_ISSET(_sock,&fdReads):判断_sock是否有待处理时间  
 - FD_ZERO(&fdReads):将待观察数组fdReads清空
fdReads数组实现方式在windows和linux中有所区别，windows是由数组中元素个count及数组array组成，linux中由指针实现  
### select模式原理：  
  函数参数：select(max_sock+1,&fdReads,&fdWrites,&fdExcept,flag)。轮询检查对应数组中的文件是否有待处理文件，有则将对应文件标识符值为true，从而实现非阻塞网络模式。在windows中最大文件数为64(可通过修改FD_SETSIZE这个宏定义来修改可支持的最大文件数)，在linux中最大文件数1024（无法修改，若要突破这个值，只能使用epoll模式），这也是select模式中网络的最大连接数。
  
## 3.粘包，少包  
 - 产生原因：高吞吐量下服务器缓存已经占满，服务器处理速度跟不上接收信息速度。  
 - 解决方案：增加二级缓存。  
  
## 4.多线程基本概念
 - 临界资源 锁的概念 c++中 mutex 库  .lock() 和 .unlock成员函数锁定了临界区域
	mutex库中lock_guard类已经将上述两个成员函数写进构造和析构函数，只要将临界区域的代码写在一个{}域中，推出时能自动调用析构函数
 - 原子操作：cpu运算中不可分割的操作，一代开始除非全部完成操作，否则在cpu中不会中断来执行其他线程，其中原子操作的cpu消耗比加锁要低  
	c++中 atomic 库    
 - 线程休眠函数：std::chrono::milliseconds t(1)    :先定义chrono库中的休眠时间  
	            std::this_thread::sleep_for(t)     :该线程休眠t时间
  
## 5.C++一些基本概念    
- 三大特性：封装，继承，多态
- 虚函数：虚函数可以继承（除构造函数外），虚函数使得基类的指针可以调用派生类的成员函数（前提：1.当基类指针指向派生类对象时 2该成员函数继承自基类，其重载了基类的成员函数。）  
	纯虚函数：void fun() = 0; 不具备函数的功能，不能被调用，它只是通知编译器这里声明一个纯虚函数，留待派生类中定义  
  
## 6.git的一些操作  
### git的一些基本操作  
 - git add filename : 将某个文件提交到暂存区  
 - git commit -m "修改注释"  : 将修改代码提交到本地仓库  
 - git push : 将代码提交到远程仓库  
 - git log : 显示所有修改的版本记录  
 - git reset -- hard 版本号 : 代码回退到某个版本号  git reset --hard HEAD^ : 回退到上个版本的代码
### git分支管理  
 - git branch : 显示所有当前分支  
 - git branch dev : 创建该名字的分支  
 - git push origin dev 或者 git branch --set-upstream-to=origin/dev : 将当前新创建的分支更新到远程仓库  
 - git branch -d dev : 删除dev分支  
 - git push origin :dev : 删除dev分支，前面的冒号表示删除
 - git checkout dev : 将当前默认分支转换到新的分支  
 - git merge dev : 前提是先切换到主分支，再将某个分支的代码合并到主分支  
 ## 7.TCP三次握手和四次挥手  
 ![image](https://github.com/Ye-Xiao12/network_programming/blob/master/picture/%E4%B8%89%E6%AC%A1%E6%8F%A1%E6%89%8B%E5%9B%9B%E6%AC%A1%E6%8C%A5%E6%89%8B.svg)  
### 基本概念  
- 2个序号：  
1.seq(sequence number) : 顺序号  
2.ack(acknowledge number) : 确认号，响应前面的seq，为接收的seq值+1。  
  
 - 6个标记位：  
1.URG(urgent) : 紧急标志  
2.ACK(acknowledgement) : 确认标志  
3.PSH(push) : 表示推送操作  
4.RST(reset) : 重置复位标志  
5.SYN(synchronous) : 发送/同步标志  
6.FIN (finish): 结束标志  
  
- Dos攻击  
客户端与服务端建立连接要发送三个报文，如果客户端通过伪造不存在的IP作为源地址向服务器发送SYN连接请求报文，服务端收到后回应一个SYN+ACK报文并在自己的半连接队列中为收到的SYN报文创建一个条目并等待客户端的回应。但客户端采取了IP欺骗，服务器发送的SYN-ACK报文根本得不到回应，这时服务器会不断等待、重传直至重传次数超过系统规定的最大重传次数才停止，并将这个SYN项目从半连接队列中删除。SYN泛洪攻击就是在短时间内伪造大量不存在的IP地址并快速发送大量这样的SYN报文给攻击目标计算机，使其半连接队列被阻塞，正常的SYN请求反而被丢弃，同时还要不断对这个庞大的半连接队列中所有项目进行SYN+ACK的重试，系统可用资源急剧减少，系统运行缓慢，严重者会引起网络堵塞甚至系统瘫痪。  
 - 为什么TCP连接要三次握手而关闭要四次挥手？  
在关闭连接时，收到主动方的报文只表示对方没有数据发送给你了，但是可能被动关闭连接的一方任然存在要发送给对方的数据，这时候被动关闭的一方会马上发送一条ACK数据向对方确认已经收到FIN报文，然后等待数据发送完成后再发送SYN同步报文。而TCP连接建立的过程中并不存在这样的情况，相当于服务器端一次性将ACK确认信息和SYN同步信息放在了同一个报文中发送给了对方。  
 



