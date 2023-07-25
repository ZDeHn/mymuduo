# mymuduo



### 简介

- 仿照muduo库实现了EventLoop、EventLoopThread、EventLoopThreadPool、TcpConnection、TcpServer等muduo库中的网络IO核心类，muduo库中的网络IO核心类的关系如图所示

  ![muduo](https://github.com/ZDeHn/mymuduo/blob/master/image/muduo.png)

  

  - TcpServer类中包含EventLoopThreadPool，EventLoopThreadPool包含多个EventLoopThread（数量由numThreads指定），muduo库为多线程多reactor模型，每个EventLoop就对应一个reactor，每个EventLoopThread就对应一个线程（该线程包含一个EventLoop）

  - MainLoop负责接收新连接，当新连接到达时，MainLoop中的Acceptor会通过调用accept将sockfd封装成TcpConnection，并使用轮询的方式交给EventLoopThread中的IOLoop，IOLoop将相应sockfd注册到Poller中，Poller类中使用epoll_wait/poll监听

  - Channel类可以认为是对sockfd以及其对应事件的封装，当Poller中有读写事件发生时，由Channel调用相应sockfd的回调函数。

    

- 仿照muduo库日志实现日志系统，实现了异步日志、日志滚动功能。

  

  ![Logger](https://github.com/ZDeHn/mymuduo/blob/master/image/Log.png)

  

  - 日志系统实际上是一个多生产者单消费者模型，日志系统包括两个主要类：Logger以及AsyncLogThread，工作线程中调用Logger类将日志输入到streamBuffer中，通过析构函数，将streamBuffer中的日志输入到AsynLogThread中的currentBuffer中。
  - AsynLogThread使用双缓冲区currentBuffer、nextBuffer，不断接收工作线程中的日志buffer，之后输出到buffers中
  - 最后在buffers中的所有日志通过LogFile类写入log文件。



- 基于重写的muduo库实现了http服务器，提供了http报文解析功能，并支持POST请求。新增内存池优化http
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

- 使用apachbench工具进行http压力测试工具进行测试，将Http服务器部署到云服务器上（2核，2G），本地机器使用以下指令

  ```shell
  ab -n 100000 -c 1000 -k http://host/
  ```

- 测试结果

  - 未加内存池

    ```
    Concurrency Level:      1000
    Time taken for tests:   33.566 seconds
    Complete requests:      100000
    Failed requests:        0
    Non-2xx responses:      100000
    Keep-Alive requests:    100000
    Total transferred:      10800000 bytes
    HTML transferred:       500000 bytes
    Requests per second:    2979.23 [#/sec] (mean)
    Time per request:       335.657 [ms] (mean)
    Time per request:       0.336 [ms] (mean, across all concurrent requests)
    Transfer rate:          314.22 [Kbytes/sec] received
    
    Connection Times (ms)
                  min  mean[+/-sd] median   max
    Connect:        0    2 268.4      0   31548
    Processing:    10   49 127.0     13    6928
    Waiting:       10   49 127.0     13    6928
    Total:         10   52 297.1     13   31777
    
    Percentage of the requests served within a certain time (ms)
      50%     13
      66%     14
      75%     14
      80%     15
      90%    226
      95%    233
      98%    445
      99%    673
     100%  31777 (longest request)
    ```

  - TcpServer以及HttpServer增加TcpConnection和HttpContext内存池

    ```
    Concurrency Level:      1000
    Time taken for tests:   32.936 seconds
    Complete requests:      100000
    Failed requests:        0
    Non-2xx responses:      100000
    Keep-Alive requests:    100000
    Total transferred:      10800000 bytes
    HTML transferred:       500000 bytes
    Requests per second:    3036.17 [#/sec] (mean)
    Time per request:       329.362 [ms] (mean)
    Time per request:       0.329 [ms] (mean, across all concurrent requests)
    Transfer rate:          320.22 [Kbytes/sec] received
    
    Connection Times (ms)
                  min  mean[+/-sd] median   max
    Connect:        0    0  22.7      0    7167
    Processing:    10   43 111.4     12    6768
    Waiting:       10   43 111.4     12    6768
    Total:         10   43 113.6     12    7185
    
    Percentage of the requests served within a certain time (ms)
      50%     12
      66%     13
      75%     14
      80%     14
      90%    224
      95%    232
      98%    244
      99%    453
     100%   7185 (longest request)
    ```

    增加内存池提升效果并不明显，原因可能是限制其速度的最根本原因是服务器带宽，并不是本机内存分配时消耗的时间，如果增加网络带宽，内存池的效果会更明显

  

### Reference



- https://github.com/chenshuo/muduo
- https://github.com/cacay/MemoryPool
- https://github.com/linyacool/WebServer
- https://github.com/youngyangyang04/Skiplist-CPP