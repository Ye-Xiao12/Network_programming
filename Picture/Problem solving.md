## 解决github中无法显示图片的问题   
github将存储用户上传的素材文件放在raw.githubusercontent.com中，避免跟主服务抢占负载。由于存在DNS污染的问题，可能会把网络中的计算机引导到错误的服务器或服务器的网址。   
 - 解决方法： 1. 网上搜raw.githubusercontent.com 对应的IP(不同时间可能会变)，结果为：185.199.108.133（2021.2.115）  
2.修改windows电脑上的C:\Windows\System32\drivers\etc\hosts文件，在最后添加一行：185.199.108.133 raw.githubusercontent.com（需要修改文件读写权限）。这样默认计算机访问这个域名不用到DNS服务器查询对应IP,而使用默认IP。