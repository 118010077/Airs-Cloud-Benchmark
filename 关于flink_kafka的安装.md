
# 关于Flink和kafka的安装
### 原作者：杨宇凡、沈涛 检查、修改、完善：高梓骐
### 一、安装Flink
1. 基本配置：安装并查看java的版本号，推荐使用java8。最好也安装scala2.11。  
2. 打开apache Flink官网（https://flink.apache.org/zh/downloads.html#apache-flink-1112）下载最新版本的Flink，并解压到本地地址（例如：/usr/local）
We highly recommend all users to upgrade to Flink 1.11.2  
updated maven dependency：
```
<dependency>
  <groupId>org.apache.flink</groupId>
  <artifactId>flink-java</artifactId>
  <version>1.11.2</version>
</dependency>
<dependency>
  <groupId>org.apache.flink</groupId>
  <artifactId>flink-streaming-java_2.11</artifactId>
  <version>1.11.2</version>
</dependency>
<dependency>
  <groupId>org.apache.flink</groupId>
  <artifactId>flink-clients_2.11</artifactId>
  <version>1.11.2</version>
</dependency>
```
设置环境变量vi /etc/profile（对全体用户和shell有效，不建议修改）或是修改~/.bashrc （仅对当前用户和交互式shell有效）
```
#flink
export FLINK_HOME=/usr/local/flink-1.11.1
export PATH=$FLINK_HOME/bin:$PATH
```
3.  找到conf/flink-conf.yaml.配置文件，配置集群环境. 将 jobmanager.rpc.address key 配置为你的master node，与此同时定义环境中的memroy。（Flink is allowed to allocate on each node by setting the jobmanager.heap.size and taskmanager.memory.process.size keys. These values are given in MB.）
```
# JobManager runs.
jobmanager.rpc.address: cdh1
# The RPC port where the JobManager is reachable.
jobmanager.rpc.port: 6123
# The heap size for the JobManager JVM
jobmanager.heap.size: 1024m
# The heap size for the TaskManager JVM
taskmanager.heap.size: 1024m
# The number of task slots that each TaskManager offers. Each slot runs one parallel pipeline.
taskmanager.numberOfTaskSlots: 1
# The parallelism used for programs that did not specify and other parallelism.
parallelism.default: 1
#配置是否在Flink集群启动时候给TaskManager分配内存，默认不进行预分配，这样在我们不适用flink集群时候不会占用集群资源
taskmanager.memory.preallocate: false
# 用于未指定的程序的并行性和其他并行性，默认并行度 
parallelism.default: 2 
 #指定JobManger的可视化端口，尽量配置一个不容易冲突的端口
 jobmanager.web.port: 5566
 ```
再编辑conf/masters 输入jobmanager的IP/host。  
接下来要配置几个节点作为worker nodes. 编辑文件 conf/workers 然后输入 IP/host 名称，这些节点将作为TaskManager运行程序。

接下来举一个 三个工作节点的例子(IP 地址 from 10.0.0.1 to 10.0.0.3 ；他们的hostnames分别为 master, worker1, worker2) ：   
![p1](./img/1.png)

将flink安装所有信息已经环境信息同步到其他机器上面，这里有几台机器就要执行几次
(以下命令是一个例子，具体路径需要按照配置的路径进行修改,注意保证flink在各台机器上的绝对路径一致)
```
scp /etc/profile root@cdh3:etc/profile
scp -r ./flink-1.11.1 root@cdh3:/usr/local

source /etc/proflie （各个机器均需该指令激活新的环境变量）
```
4. 进入flink环境目录下，启动命令
```
./bin/start-cluster.sh
```
可以在localhost（ip名称）：8081中查看集群状态与各类信息
然后jps查看一下进程，分别可以看到JobManager和TaskManager的2个进程：  
```
[root@cdh1 bin]$ jps
3876 StandaloneSessionClusterEntrypoint
[root@cdh2 ~]$ jps
3544 TaskManagerRunner
```

5.如果需要修改配置或者修改集群的环境，需要在集群路径下退出集群：

> ./bin/stop-cluster.sh  

然后通过如下命令使得环境变量生效： 

> source /etc/profle   

最后在配置完成后重新进入集群

### 二、Zookeeper安装和配置

1. zookeeper 集群通常是用来对用户的分布式应用程序提供协调服务的，为了保证数据的一致性，对 zookeeper 集群进行了这样三种角色划分：leader、follower、observer分别对应着总统、议员和观察者。 
总统（leader）：负责进行投票的发起和决议，更新系统状态。 
议员（follower）：用于接收客户端请求并向客户端返回结果以及在选举过程中参与投票。 
观察者（observer）：也可以接收客户端连接，将写请求转发给leader节点，但是不参与投票过程，只同步leader的状态。通常对查询操作做负载。 
下载 zookeeper 
官网下载地址：http://mirror.bit.edu.cn/apache/zookeeper/ 

2. 安装JDK 由于zookeeper集群的运行需要Java运行环境，所以需要首先安装 JDK，关于安装步骤，我在前面博客介绍过：https://www.cnblogs.com/ysocean/p/6952166.html 

3. 解压 zookeeper 
在 /usr/local 目录下新建 software 目录，然后将 zookeeper 压缩文件上传到该目录中，然后通过如下命令解压。 
```
tar -zxvf zookeeper-3.3.6.tar.gz 
```

4. 修改配置文件 zoo.cfg 
将zookeeper压缩文件解压后，我们进入到 conf 目录, 将 zoo_sample.cfg 文件复制并重命名为 zoo.cfg 文件。 
```
cp zoo_sample.cfg zoo.cfg 
```
然后通过 vi zoo.cfg 命令对该文件进行修改： 
参见/img/3.png

图中红色框住的内容即我们修改的内容： 

*  tickTime：基本事件单元，这个时间是作为Zookeeper服务器之间或客户端与服务器之间维持心跳的时间间隔，每隔tickTime时间就会发送一个心跳；最小 的session过期时间为2倍tickTime 

*  dataDir：存储内存中数据库快照的位置，除非另有说明，否则指向数据库更新的事务日志。注意：应该谨慎的选择日志存放的位置，使用专用的日志存储设备能够大大提高系统的性能，如果将日志存储在比较繁忙的存储设备上，那么将会很大程度上影像系统性能。 

*  client：监听客户端连接的端口。 

*  initLimit：允许follower连接并同步到Leader的初始化连接时间，以tickTime为单位。当初始化连接时间超过该值，则表示连接失败。 
*  syncLimit：表示Leader与Follower之间发送消息时，请求和应答时间长度。如果follower在设置时间内不能与leader通信，那么此follower将会被丢弃。 

*  server.A=B:C:D   
A：其中 A 是一个数字，表示这个是服务器的编号；   
B：是这个服务器的 ip 地址；   
C：Leader选举的端口；   
D：Zookeeper服务器之间的通信端口。   
我们需要修改的第一个是 dataDir ,在指定的位置处创建好目录。 
第二个需要新增的是 server.A=B:C:D 配置，其中 A 对应下面我们即将介绍的myid 文件。B是集群的各个IP地址，C:D 是端口配置。 

5. 创建 myid 文件 
在 上一步 dataDir 指定的目录下，创建 myid 文件。 
然后在该文件添加上一步 server 配置的对应 A 数字。 
比如我们在上面所配置好的`zoo.cfg`中配置了dataDir和server信息： 
```
dataDir=/usr/local/software/zookeeper-3.3.6/data   
server.0=192.168.146.200:2888:3888   
server.1=192.168.146.201:2888:3888   
server.2=192.168.146.202:2888:3888   
```
那么就必须在192.168.146.200, 201，和202 机器的的 /usr/local/software/zookeeper-3.3.6/data 目录下分别创建 myid 文件，并分别写入 0, 1, 2 即可。

6.  配置环境变量（可选）
为了能够在任意目录启动zookeeper集群，我们需要配置环境变量。 
首先修改到 /etc/profile（对全体用户和shell有效，不建议修改）或是修改~/.bashrc （仅对当前用户和交互式shell有效），添加相应的配置信息： 
```
#set zookeeper environment 
export ZK_HOME=/usr/local/software/zookeeper-3.3.6 
export PATH=$PATH:$ZK_HOME/bin 
```
然后通过如下命令使得环境变量生效： 

> source /etc/profle 


7. 启动zookeeper服务（配置环境变量后使用，否则请进入到zookeeper目录下的bin目录后再操作）
启动命令： 

> zkServer.sh start 

停止命令： 

> zkServer.sh stop 

重启命令： 

> zkServer.sh restart 

查看集群节点状态： 

> zkServer.sh status 

我们分别对集群三台机器执行启动命令（都要手动启动）。执行完毕后，分别查看集群节点状态： 
出现如下即是集群搭建成功： 
```
Mode:leader 
Mode:follower
Mode:follower
```
三台机器，slave1 成功的通过了选举称为了leader,而剩下的两台成为了 follower。这时候，如果你将slave1关掉，会发现剩下两台又会有一台变成了 leader节点。 


### 三、Kafka安装和配置
1. 下载 kafka 压缩包 
官网下载地址：http://kafka.apache.org/downloads 
解压 kafka   
将下载的 kafka 压缩文件上传到集群中的每台机器相应目录，执行如下命令进行解压。 

> tar -zxf kafka_2.12-2.0.0.tgz 

2. 修改Kafka安装目录下的配置文件config/server.properties   
```
1 broker.id=0 
2 listeners=PLAINTEXT://192.168.146.200:9092 
3 zookeeper.connect=192.168.146.200:2181,192.168.146.201:2181,192.168.146.202:2181
```
第一个 broker.id 后面的值和搭建 zookeeper 集群中 myid 一样，是一个集群中唯一的数，要求是正数。需要保证kafka集群中设置的都不一样。 
第二个设置监听器，后面的 IP 地址对应当前的 ip 地址。 
第三个是配置 zookeeper 集群的 IP 地址。 

3. 启动 kafka （注意将下面的地址修改成环境中的地址）

> /usr/local/software/kafka_2.12-2.0.0/bin/kafka-server-start.sh /usr/local/software/kafka_2.12-2.0.0/config/server.properties & 

该命令虽然是后台启动服务，但是日志仍然会打印到控制台。 
想要完全后台启动，执行如下命令： 
```shell
/usr/local/software/kafka_2.12-2.0.0/bin/kafka-server-start.sh /usr/local/software/kafka_2.12-2.0.0/config/server.properties 1>/dev/null 2>&1 & 
```
或使用如下命令:
```shell
sudo /usr/local/software/kafka_2.12-2.0.0/bin/kafka-server-start.sh -daemon /usr/local/software/kafka_2.12-2.0.0/config/server.properties
```

其中1>/dev/null  2>&1 是将命令产生的输入和错误都输入到空设备，也就是不输出的意思。/dev/null代表空设备。 
执行完毕后，输入 jps ，出现 kafka 的进程，则证明启动成功。 

4. 创建topic并连入Producer和Consumer 
集群启动成功后，我们通过创建一个名字为 test，partitions为3，replication为3的topic。 
进入到bin 目录下，执行如下命令： 
```shell
./kafka-topics.sh --create --zookeeper 192.168.146.200:2181,192.168.146.201:2181,192.168.146.202:2181 --partitions 3 --replication-factor 3 --topic test 
```
向 topic 发送消息 
进入到 bin 目录下，执行如下命令： 
```shell
./kafka-console-producer.sh --broker-list 192.168.146.200:9092,192.168.146.201:9092,192.168.146.202:9092 --topic test   
```
例如：输入 hello kafka ，然后 enter 键，即向名为 test 的topic 发送了一条消息：hello kafka 
同时在另一个终端运行如下命令连接一个consumer：
```shell
./kafka-console-consumer.sh --bootstrap-list 192.168.146.200:9092 --topic test  
```

随后我们在producer输入信息后便可在consumer处获取到相应的信息。  

5. 删除topic  

由于Kafka特性的原因使用如下指令并不会彻底删除Topic而是添加删除标记:    
```shell
sudo /opt/Kafka/kafka_2.12-2.5.0/bin/kafka-topics.sh --delete --topic test  
```

若要彻底删除topic可以使用如下方法：  
```shell
zkCli.sh  #进入zookeeper客户端  
deleteall /brokers/topics/[Topic名称]  
quit #退出zookeeper客户端  
```

6. 查看Kafka Topics:  
```shell
sudo /opt/Kafka/kafka_2.12-2.5.0/bin/kafka-topics.sh --list --zookeeper 10.21.44.35:2181
```
进行到此处则Kafka, Flink的环境基本配置完成，请参考`README.md`继续操作.

