### 在使用此文件前请先根据同目录下的` 关于flink_kafka的安装.md `文件进行环境配置和相应程序的安装
# 实时到课率的Flink实现
### 原作者：杨宇凡、沈涛 检查、修改、完善：高梓骐
____
### 一、数据传输路径
数据流向：10.20.253.8（数据源） ---> 10.21.44.35 kafka (topic “SendToFlink") ---> Flink集群（10.21.44.35 & 10.21.44.38) ---> 10.21.44.35 kafka (topic "SendToMySQL"） ---> 10.21.44.35 （MySQL 数据库）
</br>  
即先提交任务`course_id_transport.py`至Flink集群，开始准备处理数据流。在此之后，使用`TestSendKafka.py`或手动输入Wifi日志将数据传入到Kafka的SendToFlink Topic，
这些被输入的数据会传输至Flink中进行处理，最后我们可以在SendToKafka Topic中得到Flink处理后的数据，运行`SendToMySQL.py`将最终数据发送到MySQL即可。

### 二、Kafka的相关设置
#### 1. Zookeeper启动

启动zookeeper （集群各台机器都需要启动）
```shell
sudo /opt/zookeeper/apache-zookeeper-3.6.1-bin/bin/zkServer.sh start 
```
查看zookeeper 状态 
``` shell
sudo /opt/zookeeper/apache-zookeeper-3.6.1-bin/bin/zkServer.sh status 
```
（结束计算后关闭zookeeper: sudo /opt/zookeeper/apache-zookeeper-3.6.1-bin/bin/zkServer.sh stop）

#### 2. 关于kafka的topic的操作  

创建Kafka Topics:  
```shell
sudo /opt/Kafka/kafka_2.12-2.5.0/bin/kafka-topics.sh --create --zookeeper 10.20.253.8:2181 (任意一个Zookeeper集群Ip，在zookeeper已启动的状态下可以填写任意一个ip即可) --partitions 3 --replication-factor 3 --topic SendToFlink
sudo /opt/Kafka/kafka_2.12-2.5.0/bin/kafka-topics.sh --create --zookeeper 10.20.253.8:2181 (任意一个Zookeeper集群Ip) --partitions 3 --replication-factor 3 --topic SendToKafka

```
（结束计算后关闭Kafka: sudo /opt/Kafka/kafka_2.12-2.5.0/bin/kafka-server-stop.sh ）

启动Kafka Producer Console: 
```shell
sudo /opt/Kafka/kafka_2.12-2.5.0/bin/kafka-console-producer.sh --bootstrap-server 10.20.253.8:9092 --topic SendToFlink 
```
启动Kafka Consumer Console来监测: 
```shell
sudo /opt/Kafka/kafka_2.12-2.5.0/bin/kafka-console-consumer.sh --bootstrap-server 10.20.253.8:9092 --topic SendToFlink --from-beginning 
```

以上操作的topic名字视具体任务而变动

#### 3. 输送数据：

*  通过打开Kafka Producer Console直接传输json数据流：  
格式如下：'{"currentapname": "AZ-1F-16", "apname": "AZ-1F-14", "ssid": "CUHK(SZ)", "mac": "60-8c-4a-57-12-4d", "connecttime": "2019-09-02 15:23:02", "connectresult": "Success", "uppacketnum": 5565, "upbyte": 2070823, "downpacketnum": 3852, "downbyte": 2168987}'  

*  通过运行 /realtime2DB/Test/TestSendKafka.py 实现数据传输 (注意修改TestSendKafka.py的kafka设置，wifi记录位置，log写入位置)




### 三、基于Flink集群进行数据转换
#### 1. 启动集群：
```shell
cd /opt/Flink-1.11.2
./bin/start-cluster.sh
```
(如要关闭集群：./bin/stop-cluster.sh)  
集群的运行与设置以及任务的运行可以通过浏览器访问 10.21.44.35:8081（你的虚拟机IP） 中查看  

#### 2. Python环境确认 (所有Flink集群下的机器都要进行下面的安装步骤)
``` shell
#首先安装pipenv来管理第三方包 （建议设置虚拟环境，否则请注意Flink默认使用~/.local/lib/python3.x/site-packages的python环境）
pip install pipenv
#从GitLab上克隆下project
git clone 
#用pipenv建立虚拟环境
pipenv --three
#激活虚拟环境
pipenv shell
#安装相关的gcc等编译器及python相关头文件
yum group install "Development Tools"
yum -y install python36-devel
#用pipenv安装第三方包
pip3 install --upgrade pip  
python3 -m pip install --upgrade setuptools  
pipenv install -r requirements/common.txt
#如果上面这个指令无效，用pip安装
pip install -r requirements/common.txt
#安装pyflink （时间较长且需要安装很多whl，需要上面安装的C和C++的编译工具）
pip install apache-flink -i https://pypi.tuna.tsinghua.edu.cn/simple --default-timeout=1000
```


#### 3. 上载任务flink-kafka: 任务内容为数据流的查询与转换 (建立接收用数据库和输出用数据库以及相关处理函数)

启动前修改course_id_transport中的集群地址，log位置，并确认`course_id_transport.py`中的jar包路径与服务器中一致。
```shell
./bin/flink run -py /realtimeDB/course_id_transport.py
```

如果未设置并进入虚拟环境，此处不对flink进行额外设置会出现python版本启动错误的情况，可以进行如下操作进行处理（虚拟环境的具体设置待更新）:
```shell
#备份python软链接
mv /usr/bin/python /usr/bin/python.bak

#创建python3软链接
ln -s /usr/bin/python3 /usr/bin/python

#然后使用命令`python --version`即可看到当前python命令为python3  
#创建pip3软链接
ln -s /usr/bin/python3/bin/pip3 /usr/bin/pip3  
```
修改相关依赖python2的文件 (`/usr/bin/yum`, `/usr/libexec/urlgrabber-ext-down`)
将第一行的声明从`#! /usr/bin/python`改为 `#! /usr/bin/python2`


#### 4. 提交任务后，若有数据传入kafka topic “SendToFlink"，可以在topic "SendToMySQL"中查看输出的json数据
根据服务器IP地址登录相应端口（如Flink的JobManager在192.168.0.1则可以通过http://192.168.0.1:8081进入Flink的HTTP Dashboard页面），在Running Jobs中看到如下信息,且shell中会给出如下信息
`Job has been submitted with JobID xxxxxxxxxxxxxxx`则提交成功。   
如在执行过程中出现错误**shell中不会显示错误信息**，请到flink的log目录中的`flink-flink-client-flink1.log`文件查看情况。
![](./img/4.png)

### 四、结合MySQL实现到课率计算
#### 1. 将kafka (topic "SendToMySQL"）的数据流传输至mysql数据库
```shell
/realtime2DB/SendToMySQL.py
```

#### 2. 存储完毕后,可以通过利用
```shell
/realtime2DB/course_attendance_calculate.py
```
脚本中的两个函数可以得到到课率或者到课率字典：  
函数crssearch，可以输入课程代码和时间查看当天的到课率，返回到课率  
函数timesearch，可以输入时间（目前仅支持固定字符串形式的日期），返回当天所有课程的到课率的字典。  
例子:  
```python
print(crssearch('ERG',2050,1,'2019-09-02'))-> output: 0.04%

```
```python
print(timesearch('2019-09-02'))-> output: {'ACT5005':{1:'7.23%'},'ERG2050‘: {1:'3.57%'}, 'TRA5201': {1: '5.88%'}}

```
