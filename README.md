概述
=========================

铜锁（Tongsuo）是一个提供现代密码学算法和安全通信协议的开源基础密码库，为存储、网络、密钥管理、隐私计算等诸多业务场景提供底层的密码学基础能力，实现数据在传输、使用、存储等过程中的私密性、完整性和可认证性，为数据生命周期中的隐私和安全提供保护能力。

铜锁开源密码库的前身是BabaSSL开源项目。BabaSSL于2022年更名为铜锁。（TODO：后续这里放一个铜锁的logo）

特性
=========================

铜锁提供如下主要的功能特性：

  * 技术合规能力
    * 铜锁开源密码库（原BabaSSL）正在取得国家密码管理局商用密码检测中心授予的”软件密码模块安全一级“资质，详见：TBD
  * 密码学算法
    * 中国商用密码算法：SM2、SM3、SM4、祖冲之等
    * 国际主流算法：ECDSA、RSA、AES、SHA等
    * 同态加密算法：EC-ElGamal、Paillier等
    * 后量子密码学：LAC、NTRU、Saber、Dilithium等
  * 安全通信协议
    * 支持GB/T 38636-2020 TLCP标准，即双证书国密通信协议
    * 支持[RFC 8998](https://datatracker.ietf.org/doc/html/rfc8998)，即TLS 1.3 + 国密单证书
    * 支持[QUIC](https://datatracker.ietf.org/doc/html/rfc9000) API
    * 支持Delegated Credentials功能，基于[draft-ietf-tls-subcerts-10](https://www.ietf.org/archive/id/draft-ietf-tls-subcerts-10.txt)
    * 支持TLS证书压缩
    * 支持紧凑TLS协议

文档
=========================

铜锁的相关文档组织在 [铜锁文档网站](https://babassl.readthedocs.io/) 上。

报告安全缺陷
=========================

铜锁目前使用蚂蚁集团的威胁搜集系统，请访问如下地址进行安全缺陷的报告：

 * [https://security.alipay.com/](https://security.alipay.com/)

注意：对于非安全相关的Bug，请使用Gitte的Issues进行提交。
