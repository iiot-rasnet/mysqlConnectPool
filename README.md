# 数据库连接池，可以减少那一部分的性能损耗：
1. 建立通信连接的TCP三次握手
2. 数据库服务器的连接认证
3. 数据库服务器关闭连接时的资源回收
4. 断开通信连接的TCP四次挥手
   
# 连接池的设计
1. 连接池只需要一个实例，所以连接池类应该是一个单例模式的类
2. 所有的数据库连接应该维护到一个安全队列中
   
  a. 使用队列的目的是方便连接的添加和删除

  b. 所谓的安全指的是线程安全，也就是说需要使用互斥锁来保护队列数据的读写。	
3. 在需要的时候可以从连接池中得到一个或多个可用的数据库连接

  a. 如果有可用连接，直接取出
  
  b. 如果没有可用连接，阻塞等待一定时长然后在重试
  
4. 如果队列中没有可用连接，需要动态的创建新连接
5. 如果队列中空闲的连接太多，需要动态的销毁一部分
6. 数据库操作完毕，需要将连接归还到连接池中。

# 主要使用的技术：
  1.多线程编程
  2.线程同步（互斥锁的使用）
  3.处理时间和日期的chrono库
  4.条件变量
  5.智能指针（帮我们释放一块内存）
  6.lambda表达式
  7.使用=delete删除函数（在实现数据库连接池的时候，需要创建一个单例模式的类，某些函数是不允许存在的，比如说拷贝构造函数，还有=赋值操作符重载函数，可以用=delete删除，它们也就不允许使用了。如果不使用=delete删除，访问权限设置为私有也可以）

# 其他知识点：
  1.MYSQL数据库查询，主要是官方API的封装和使用
  2.单例模式
  3.STL容器
  4.生产者和消费者模型
  5.Jsoncpp库的使用（使用Cmake解析配置文件中的数据库相关信息）
![图片1](https://github.com/iiot-rasnet/mysqlConnectPool/assets/45915062/14cd8fa4-1fa7-46cd-9a83-0e89bef232e0)


# 连接不上linux上的mysql服务器?
第一步：通过root登录到MySQL：

mysql -u root -p

执行赋权命令

MySQL> GRANT ALL PRIVILEGES ON *.* TO 'root'@'%'IDENTIFIED BY 'password' WITH GRANT OPTION;

MySQL> flush privileges;

password为MySQL的root用户对应的密码

第二步：修改MySQL的配置文件：

vi /etc/mysqld.cnf

将监听的地址改为bind-address=0.0.0.0

第三步：关闭防火墙或者让防火墙开放3306端口

systemctl stop firewalld

firewall-cmd --zone=public --add-port=3306/tcp --permanent
