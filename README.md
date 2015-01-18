**本人编写了一个简易的网络接口测试工具，用于弥补nc测试工具的不足之处，欢迎大家使用~**

### TC使用方法

为了提高效率，方便大家测试自己的程序接口。我从程序员和管理员使用的角度出发开发出了一个测试客户端TC(Test Client)。TC主要用于测试网络接口，支持TCP和UDP测试。现对TC特色功能做一下介绍。

* 支持超时设置
* 支持时间显示（显示每次传输时延）
* 支持自动重连（只有TCP）
* 简化重复输入

现对TC具体使用方法做详细说明。

  Usage:tc -s <SERVER_IP> -p <SERVER_PORT> [-w <wait time>] [-r <repeat times>] [-u] [-c] [-t]
-
* -s 服务器端IP地址
* -p 连接端口
* -w 超时时间（单位：秒）
* -r 重复次数
* -u 开启UDP
* -c 自动重连，只有TCP支持
* -t 显示每次接受时延

**示例：tc -s 192.168.1.101 -p 8888 -r 3 -c -t**

连接192.168.1.101的8888端口（TCP连接），重复三次（只需输入一次命令），开启自动重连和时间显示。

#### 1.1版新增功能：

新增服务器监听模式，用于测试客户端程序。

   Usage:tc [-i <LISTEN_IP>] -p <LISTEN_PORT> -l

#### TC不适用一下情况：
* 文件传输测试
* 非一应一答模式
* 性能测试
 
下载链接：(http://pan.baidu.com/share/link?shareid=30287&uk=1761640494)

另外：本人没有对输入参数做严格检查，望见谅。如有发现BUG，请发送邮件至mymonkey110@163.com。
