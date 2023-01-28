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

Starburst 提供了一个开箱即用的 Presto 镜像：

{% link 'https://hub.docker.com/r/starburstdata/presto/' starburstdata/presto %}

这里把 Presto 配置文件和日志都做了持久化。

```bash
docker run --name presto \
	-p 18080:8080 \
	-v /home/ubuntu/docker/presto/etc:/usr/lib/presto/etc \
	-v /home/ubuntu/docker/presto/var:/data/presto/var \
	--network=docker-br0 \
	--restart=no \
	-d starburstdata/presto:350-e.18
```

## 安装 Zookeeper + Kafka

在 Docker 上运行 Kafka 有两个问题：

* Kafka 无法脱离 Zookeeper 独立运行。

* Kafka 为了避免中间人攻击，会将客户端的请求与 Broker 上报给 Zookeeper 的 hostname 等信息进行比较。

为了让宿主机和 Presto 容器都能正常访问到容器部署的 Kafka，除了指定 Zookeeper 和 Kafka 的网络，还需要配置 Kafka 的监听地址（`KAFKA_ADVERTISED_LISTENERS` 和 `KAFKA_LISTENERS`）。


```yaml
# docker-compose.yml
services:
  zookeeper:
    image: zookeeper:latest
    container_name: zookeeper
    ports:
	  - "2181:2181"
    expose:
      - "2181"
    environment:
      ZOO_MY_ID: 1
    networks:
      docker-br0:
        ipv4_address: 172.18.1.0

  kafka:
    image: wurstmeister/kafka:latest
    container_name: kafka
    environment:
      KAFKA_BROKER_ID: 1
      KAFKA_ZOOKEEPER_CONNECT: zookeeper:2181
      KAFKA_MESSAGE_MAX_BYTES: 2000000
      KAFKA_LISTENER_SECURITY_PROTOCOL_MAP: INSIDE:PLAINTEXT,OUTSIDE:PLAINTEXT
	  # 发布到 zookeeper 供客户端使用的服务地址，容器网络内可通过容器名称（kafka）访问
      KAFKA_ADVERTISED_LISTENERS: INSIDE://kafka:9093,OUTSIDE://localhost:9092
	  # kafka 服务监听地址
      KAFKA_LISTENERS: INSIDE://0.0.0.0:9093,OUTSIDE://0.0.0.0:9092
      KAFKA_INTER_BROKER_LISTENER_NAME: INSIDE
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

Presto 单节点配置（`etc/node.properties`)：

```text
node.environment=docker
node.data-dir=/data/presto
plugin.dir=/usr/lib/presto/plugin
```

在 `etc/catalog` 下创建配置文件 `kafka.properties`，重启容器后生效。查询方法可以参考[官方文档](https://prestodb.io/docs/current/connector/kafka-tutorial.html)。

```text
connector.name=kafka
kafka.nodes=kafka:9093
kafka.table-names=testdb.customer
kafka.hide-internal-columns=false
kafka.table-description-dir=/usr/lib/presto/etc/kafka
```

## 参考

[解决 Docker 容器连接 Kafka 连接失败问题](https://www.cnblogs.com/hellxz/p/why_cnnect_to_kafka_always_failure.html)

[使用 Docker 部署 Kafka 时的网络应该如何配置](https://www.jianshu.com/p/52a505354bbc)

