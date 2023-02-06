---
layout: post
title: 在 Docker 上运行 Presto 和 Kafka
date: 2023-01-19
mathjax: false
---

## 创建虚拟网桥

在 Docker 中，不同网络中的容器无法互相通信。默认情况下，`docker run` 创建的容器会连接到 Docker 的默认网桥上，`docker-compose` 则会新建一个网桥。为了保证 Presto 和 Kafka 的容器能够互通，最简单的方法就是让它们处于同一网络之下。

```bash
docker network create --driver=bridge --subnet=172.18.0.0/16 docker-br0
```

## 安装 Presto

Docker Hub 上有一些开箱即用的 Presto 镜像，官方推荐的 Ahana 的镜像集成了命令行客户端和一些 catalogs。Starburst 的镜像比较旧，缺少对 ClickHouse 等数据库和一些新语法的支持。

{% link 'https://hub.docker.com/r/ahanaio/prestodb-sandbox' 
ahanaio/prestodb-sandbox %}

{% link 'https://hub.docker.com/r/starburstdata/presto/' starburstdata/presto %}

我们也可以自己打包一个新的镜像：

```dockerfile Dockerfile
FROM openjdk:8-jre

LABEL os="debian"
LABEL app="presto"
LABEL version="0.1"
LABEL maintainer="raincandyame@github"

# Presto version will be passed in at build time
ARG PRESTO_VERSION

RUN sed -i 's/deb.debian.org/mirrors.ustc.edu.cn/g' /etc/apt/sources.list

RUN java -version

# Install python
RUN apt-get update
RUN apt-get install -y python2.7
RUN mv /usr/bin/python2.7 /usr/bin/python
RUN python --version

ENV PRESTO_DIR /usr/lib/presto
ENV PRESTO_ETC_DIR /usr/lib/presto/etc
ENV PRESTO_DATA_DIR /data/presto

RUN mkdir -p ${PRESTO_DIR} ${PRESTO_ETC_DIR}/catalog \
 && curl -s https://repo1.maven.org/maven2/com/facebook/presto/presto-server/${PRESTO_VERSION}/presto-server-${PRESTO_VERSION}.tar.gz \
 | tar --strip 1 -vxzC ${PRESTO_DIR}

WORKDIR ${PRESTO_DIR}
RUN pwd

# Config node.properties
RUN echo "node.environment=ci\n\
node.id=faaaafffffff-ffff-ffff-ffff-ffffffffffff\n\
node.data-dir=${PRESTO_DATA_DIR}\n"\ > ${PRESTO_ETC_DIR}/node.properties

# Config jvm.config
RUN echo '-server\n\
-Xmx1G\n\
-XX:+UseG1GC\n\
-XX:G1HeapRegionSize=32M\n\
-XX:+UseGCOverheadLimit\n\
-XX:+ExplicitGCInvokesConcurrent\n\
-XX:+HeapDumpOnOutOfMemoryError\n\
-XX:+ExitOnOutOfMemoryError\n'\ > ${PRESTO_ETC_DIR}/jvm.config

# Config log.properties
RUN echo 'coordinator=true\n\
node-scheduler.include-coordinator=true\n\
http-server.http.port=8080\n\
query.max-memory=0.4GB\n\
query.max-memory-per-node=0.2GB\n\
discovery-server.enabled=true\n\
discovery.uri=http://127.0.0.1:8080\n'\ > ${PRESTO_ETC_DIR}/config.properties

# Config log.properties
RUN echo 'com.facebook.presto=WARN\n'\ > ${PRESTO_ETC_DIR}/log.properties

# Specify the entrypoint to start
ENTRYPOINT ${PRESTO_DIR}/bin/launcher run
```

```bash
docker build --build-arg PRESTO_VERSION=0.278.1 --tag presto:0.1 .
```

建议将 Presto 的配置文件、日志和插件目录复制出来，在宿主机上做持久化。

```bash
docker run --name presto-tmp -d presto:0.1
docker cp presto-tmp:/usr/lib/presto/etc /home/ubuntu/docker/presto/
docker cp presto-tmp:/data/presto/var /home/ubuntu/docker/presto/
docker cp presto-tmp:/usr/lib/presto/plugin /home/ubuntu/docker/presto/
docker stop presto-tmp
docker rm presto-tmp
docker run --name presto \
	-p 18080:8080 \
	-v /home/ubuntu/docker/presto/etc:/usr/lib/presto/etc \
	-v /home/ubuntu/docker/presto/var:/data/presto/var \
	-v /home/ubuntu/docker/presto/plugin:/usr/lib/presto/plugin \
	--network=docker-br0 \
	-d presto:0.1
```

由于没有设置账号密码，使用 JDBC 连接时用户名可以随便填写，密码留空。

如果需要命令行客户端，可以在官网直接下载。

## 安装 Zookeeper + Kafka

在 Docker 上运行 Kafka 有两个问题：

* Kafka 无法脱离 Zookeeper 独立运行；

* Kafka 为了避免中间人攻击，会将客户端的请求与 Broker 上报给 Zookeeper 的 hostname 等信息进行比较。

为了让宿主机和 Presto 容器都能正常访问到容器部署的 Kafka，除了指定 Zookeeper 和 Kafka 的网络，还需要配置 Kafka 的监听地址（`KAFKA_ADVERTISED_LISTENERS` 和 `KAFKA_LISTENERS`）。


```yaml docker-compose.yml
services:
  zookeeper:
    image: zookeeper:latest
      container_name: zookeeper
      ports:
        - "2181:2181"
      expose:
    	- "2181"
      environment:
        - ZOO_MY_ID=1
      networks:
        docker-br0:
          ipv4_address: 172.18.1.0
  kafka:
      image: wurstmeister/kafka:latest
      container_name: kafka
      environment:
        - KAFKA_BROKER_ID=1
        - KAFKA_ZOOKEEPER_CONNECT=zookeeper:2181
        - KAFKA_MESSAGE_MAX_BYTES=2000000
        - KAFKA_LISTENER_SECURITY_PROTOCOL_MAP=INSIDE:PLAINTEXT,OUTSIDE:PLAINTEXT
		# 发布到 zookeeper 供客户端使用的服务地址，容器网络内可通过容器名称（kafka）访问
        - KAFKA_ADVERTISED_LISTENERS=INSIDE://kafka:9093,OUTSIDE://localhost:9092
		# kafka 服务监听地址
        - KAFKA_LISTENERS=INSIDE://0.0.0.0:9093,OUTSIDE://0.0.0.0:9092
        - KAFKA_INTER_BROKER_LISTENER_NAME=INSIDE
      ports:
        - "9092:9092"
      expose:
        - "9093"
      depends_on:
        - zookeeper
      networks:
        docker-br0:
          ipv4_address: 172.18.1.1
networks:
  docker-br0:
    external: true
```

## Presto 连接 Kafka

在 `etc/catalog` 创建配置文件 `kafka.properties`，重启容器后生效。

```text
connector.name=kafka
kafka.nodes=kafka:9093
kafka.table-names=testdb.customer
kafka.hide-internal-columns=false
kafka.table-description-dir=/usr/lib/presto/etc/kafka
```

使用方法可以参考[官方文档](https://prestodb.io/docs/current/connector/kafka-tutorial.html)。

## 参考

[手把手教你制作一个 Presto 的 Docker 镜像](https://www.jianshu.com/p/bb5181008cd7)

[解决 Docker 容器连接 Kafka 连接失败问题](https://www.cnblogs.com/hellxz/p/why_cnnect_to_kafka_always_failure.html)

[使用 Docker 部署 Kafka 时的网络应该如何配置](https://www.jianshu.com/p/52a505354bbc)

