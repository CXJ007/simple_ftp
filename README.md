# 一个没什么用的simple_ftp
A simple ftp server implemented with sockets in the Linux C language

servet:实现了一些简单的功能可以同时连接多个client，可以自动获取本机ip和随机port，也可以主动输入，其他的可以通过输入help查看帮助  
![](https://raw.githubusercontent.com/CXJ007/simple_ftp/master/picture/QQ%E6%88%AA%E5%9B%BE20220228142919.png)  
client:实现了一些常见功能比如get,put, ls,cd ,pwd,和本地的 ls,cd ,pwd也可以通过输入help查看帮助  
下面是一个很抽象的手绘流程图  
![](https://raw.githubusercontent.com/CXJ007/simple_ftp/master/picture/QQ%E6%88%AA%E5%9B%BE20220228140730.png)  
最后应该也不会有正经人平时用这个传文件FileZallia不好吗？主要最近刚学习了linux应用层用来练练手,这个simple_ftp连续肝了两三天左手小拇指都在痛
