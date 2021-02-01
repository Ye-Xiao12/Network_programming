# c++高并发网络编程  项目笔记  
## 1.Socket 编程  
- 服务端：1.创建socket()  2.绑定端口 bind()  3.监听端口 listen()  4.接受连接 accept()  5.接收数据 recv()  6.发送数据 send()  7.关闭端口 close();  
- 客户端：1.创建socket()  2.连接服务器 connect()  3.接收数据 recv()  4.发送数据 send()  5.关闭连接 close()  

## 2.非阻塞网络模式 select模式  
四个基本函数：  
 - FD_SET(_sock,&fdReads):将_sock加入待观察数组fdReads  
 - FD_CLR(_sock,&fdReads):将_sock从待观察数组fdReads中移除  
 - FD_ISSET(_sock,&fdReads):判断_sock是否有待处理时间  
 - FD_ZERO(&fdReads):将待观察数组fdReads清空
fdReads数组实现方式在windows和linux中有所区别，windows是由数组中元素个count及数组array组成，linux中由指针实现  
### select(max_sock+1,&fdReads,&fdWrites,&fdExcept,flag)原理：  
  轮询检查对应数组中的文件是否有待处理文件，有则将对应文件标识符值为true，从而实现非阻塞网络模式。在windows中最大文件数为64，在linux中最大文件树1024，这也是select模式中网络的最大连接数。
  
## 3.粘包，少包：高吞吐量下服务器缓存已经占满，服务器处理速度跟不上接收信息速度  
  解决方案：增加二级缓存
