# mymuduo



### 简介

- 仿照muduo库实现了EventLoop、EventLoopThread、EventLoopThreadPool、TcpConnection、TcpServer等muduo库中的网络IO核心类，muduo库中的网络IO核心类的关系如图所示

  ![muduo](.\image\muduo.png)

  

  - TcpServer类中包含EventLoopThreadPool，EventLoopThreadPool包含多个EventLoopThread（数量由numThreads指定），muduo库为多线程多reactor模型，每个EventLoop就对应一个reactor，每个EventLoopThread就对应一个线程（该线程包含一个EventLoop）

  - MainLoop负责接收新连接，当新连接到达时，MainLoop中的Acceptor会通过调用accept将sockfd封装成TcpConnection，并使用轮询的方式交给EventLoopThread中的IOLoop，IOLoop将相应sockfd注册到Poller中，Poller类中使用epoll_wait/poll监听

  - Channel类可以认为是对sockfd以及其对应事件的封装，当Poller中有读写事件发生时，由Channel调用相应sockfd的回调函数。

    

- 仿照muduo库日志实现日志系统，实现了异步日志、日志滚动功能。

  

  ![Logger](.\image\Logger.png)

  

  - 日志系统实际上是一个多生产者单消费者模型，日志系统包括两个主要类：Logger以及AsyncLogThread，工作线程中调用Logger类将日志输入到streamBuffer中，通过析构函数，将streamBuffer中的日志输入到AsynLogThread中的currentBuffer中。
  - AsynLogThread使用双缓冲区currentBuffer、nextBuffer，不断接收工作线程中的日志buffer，之后输出到buffers中
  - 最后在buffers中的所有日志通过LogFile类写入log文件。



- 基于重写的muduo库实现了http服务器，提供了http报文解析功能，并支持POST请求
- 基于重写的muduo库实现了简单的跳表服务器以及简单的跳表服务器客户端，可以实现增删改查



### 安装

```shell
sudo ./build.sh
```



### 运行

- HttpServer

  ```shell
  cd bin
  sudo ./HttpServer
  ```

- SkipList

  ```shell
  cd bin
  sudo ./SkipListServer
  
  sudo ./SkipListClient
  ```



### Http服务器压力测试

- 使用webbench工具进行http压力测试工具进行测试，将Http服务器部署到云服务器上（2核，低带宽），本地机器使用以下指令

  ```shell
  webbench -c 1000 -t 60 http://host/
  ```

- 测试结果

  ```
  1000 clients, cunning 60 sec.
  
  Speed=8709 pages/min, 12773 bytes/sec.
  Requests: 8709 susceed, 0 failed
  ```

- 结果较差原因

  - 测试的服务器只支持短链接，频繁建立与撤销TCP连接造成了性能下降

  - 云服务器性能较低：核心数较少，带宽较低

  - 没有针对http做特殊优化

    

### Reference



- https://github.com/chenshuo/muduo
- https://github.com/linyacool/WebServer
- https://github.com/youngyangyang04/Skiplist-CPP