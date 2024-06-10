---
layout: post
title: 封装忽略 HTTPS 证书认证的 HttpClient 
date: 2023-02-03
mathjax: false
---

使用 HttpClient 直接访问某些 HTTPS 网站时，可能会遇到 SSL 证书认证失败的问题。可以自定义 SSLConnectionSocketFactory 的证书信任策略，忽略证书认证。

```pom.xml
<dependency>
    <groupId>org.apache.httpcomponents</groupId>
    <artifactId>httpclient</artifactId>
    <version>4.5.14</version>
</dependency>
```

```java
 public static CloseableHttpClient createHttpsClient()
        throws NoSuchAlgorithmException, KeyManagementException {
	// 实现证书信任管理器
    X509TrustManager x509mgr = new X509TrustManager() {
		// 检查客户端证书
        public void checkClientTrusted(X509Certificate[] arg0, String arg1)
            throws CertificateException {}
		// 检查服务端证书
        public void checkServerTrusted(X509Certificate[] arg0, String arg1)
            throws CertificateException {}
		// 返回受信任的证书
        public X509Certificate[] getAcceptedIssuers() {
            return null;
        }
    };

    SSLContext sslContext = SSLContext.getInstance("TLS");
    sslContext.init(null, new TrustManager[] {x509mgr}, null);

    SSLConnectionSocketFactory factory =
            new SSLConnectionSocketFactory(sslContext, NoopHostnameVerifier.INSTANCE);

    return HttpClients.custom().setSSLSocketFactory(factory).build();
}
```