# Access #

一个可远程控制的基于树莓派的门禁系统

</br>

## 编译 ##

在使用之前需要先从c语言的源码编译出可执行文件才能使用

编译所需的条件：Linux系统，系统中安装了gcc

**服务器端：**
命令行下cd到源码的目录，输入make server，接着在源码的目录下就会编译出一个名为access_server的可执行文件


**树莓派端：**
命令行下cd到源码的目录，输入make client，接着在源码的目录下就会编译出一个名为access_client的可执行文件

</br>

## 运行 ##

编译出了可执行文件之后就可以使用这个可执行文件了

**服务器端：**
命令行下cd到access_server这个文件所在的目录（如果你编译后没有移动这个文件的话，那这个文件所在的目录就是源码的目录），输入./access_server port key，其中port和key是两个参数

参数解释：

> port 程序用来和树莓派通信的端口
> 
> key 通信中用来校验树莓派身份的密钥


**树莓派端：**
同上，在命令行下cd到access_client的目录，输入./access_client address key

参数解释：

> address 服务器的地址，格式是IP地址:端口号，这里的端口号要和服务器端运行程序时的port参数一致

> key 校验的密钥，要和服务器的key一致才能校验成功

<br/>

## 整个程序的原理 ##

在树莓派端，树莓派通过access_client这个程序调用一些树莓派提供的接口从而实现控制门禁的功能，同时access_client这个程序还会创建一个基于TCP的网络socket来和服务器通信

而在服务器端，为了能够远程控制门禁，access_server这个程序需要能够和树莓派进行通信，因此access_server也会创建一个基于TCP的socket并与树莓派的socket连接来实现二者的通信

然而仅仅这样是不够的，如果我们想要让微信能够控制门禁呢？想要让微信控制门禁的话，服务器上的微信程序并不需要直接和树莓派通信，微信程序只要和本地的access_server程序进行互相通信即可，这样就微信程序就可以通过access_server来间接地控制树莓派了

微信程序和access_server的通信是使用Unix域的流式(SOCK_STREAM)socket来实现

<br/>

## 与access_server通信文档 ##

access_server程序运行之后，会在它所在的目录创建一个名为access_socket的文件，而这个文件就是用来进行Unix域socket通信的通信地址，只要用这个地址创建了流式的Unix域socket并连接上，就可以开始通信了。

连接上之后，接着微信程序就可以向socket发送字符串来控制门禁，这里我们将发送的字符串称为命令，每一个命令可以实现一个控制门禁的功能（例如发送"openDoor"就是开门），在发送了命令之后，access_server程序也会返回来一个字符串，来告诉微信程序命令执行的结果，acccess_server返回的字符串格式如下：

"success/fail:返回信息"

解释：（注意返回的字符串中并不包含双引号，这里用双引号是为了说明这是一个字符串）返回的字符串一开头是success或者fail，success代表本次命令执行成功，fail代表失败。紧接着是一个冒号，在紧着是返回信息。但也有例外，那就是返回的字符串中有可能不包含冒号和返回信息，只有一个success或fail

下面是详细的通信文档：

**通信方式：**Unix域的流式socket

**通信地址：**access_server文件所在的目录/access_socket

**命令文档**：

1. openDoor：打开门禁

    返回信息："success"或者"fail:失败原因"

    返回示例："success"，"fail"，"fail:cannot open door"


2. closeDoor:关上门禁 

    返回信息："success"或者"fail:失败原因"

    返回示例："success"，"fail"，"fail:cannot close door"

3. getState：获取当前门的状态

    返回信息："success:closed/opened"或者"fail:失败原因"

    返回示例："success:closed"，"success:opened"，"fail:cannot get state"





