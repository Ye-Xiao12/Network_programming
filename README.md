# c++高并发网络编程  项目笔记  
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
