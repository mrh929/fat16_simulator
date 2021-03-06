## 参考资料：

##### https://blog.csdn.net/sikuon/article/details/75939434

https://blog.csdn.net/cassie_huang/article/details/62227023

https://blog.csdn.net/yeruby/article/details/41978199

最详细：https://blog.csdn.net/mj5742356/article/details/20125511

长短文件名 https://wenku.baidu.com/view/d729d62a680203d8cf2f2474.html








# FAT16

##### 一、关于需要掌握的基础知识

​	1.文件读写与文件指针的移动操作

​	2.十六进制计算

​	3.链表

​	4.小端序



##### 二、FAT16文件系统的结构

​	**1.**FAT16第一个扇区是DBR，是对整个分区的格式及参数进行说明的部分，从中可以读取到FAT所占扇区数、每簇扇区数、每个扇区的字节数、保留扇区数…… 

​	系统通过这些参数可以计算出FAT1、FAT2、根目录、数据区的位置，其中，保留扇区之后的即为FAT区，FAT区由两个完全相同的FAT1、FAT2构成，FAT2作为FAT1的一个备份，保障数据安全。

​	**2.**FAT1中存放了每个簇之间的对应关系以及自身的属性。

​	FAT2是FAT1的一个备份，以便于磁盘意外损坏的恢复

​	通过查表我们可以了解到这个簇到底是未被使用还是已被使用，以及这个簇后面还有没有链接到下一个簇。从而将连续的数据离散储存。

​	**3.**FAT2后的是根目录，根目录占用32个扇区，当中储存了根节点下面所拥有的所有文件和文件夹（文件夹可以当作一种特殊的文件），每个文件占用32个字节（当文件名过长的时候可能会占用更多空间），这32个字节分别对应的是这个文件对应的各种属性以及簇号。

​	我们可以通过寻找该簇来读取该文件的具体内容，如果是文件夹，这个簇中储存的是文件夹内的所有子目录元素（与根目录的作用类似）。

​	文件夹创建的时候会生成两个32字节的项

​		一个是  "." ，其中储存的是文件夹自身的信息

​		另一个是  ".."， 储存了这个文件夹的父目录的信息（方便返回上一级）





##### 三、删除文件（夹）的过程

​	找到描述该文件的那个32字节的项，将偏移值为0h的数据改为 E5，以说明该数据被删除。如果是文件夹的话，还应递归删除其目录下的所有文件。



##### 四、创建文件（夹）的过程

​	在当前目录的簇下寻找一个空位置（32字节） 将新文件（夹）的信息写入，并且在磁盘中寻找一个未使用的新簇，将其分配给创建的文件（夹）



##### 五、读取文件（夹）的过程

​	读取文件（夹）所对应的32字节数据，从而读取到整个文件的信息尤其是簇号，根据这个簇号去磁盘中的相应区域读取这个文件（夹）所储存的信息（文件）



##### 六、写入文件的过程

​	类似于读取文件



##### 七、遇到当前簇满的处理措施

​	在磁盘中寻找一个新簇，将原来的簇指向新簇，从而将两个簇连接在一起，形成一个连续的数据区域



**八、注意事项**

​	**1.文件系统中的数据以小端序保存（数字的低位 存在 地址的低位上，这与普通人的思维有所区别）**

​		e.g.  数据1234h，  用两个字节进行存储，则其在内存中的存储形式为：  34 12

​	**2.DBR(DOS boot record) 占一个扇区，里面储存了整个文件系统的各种配置信息（如每个扇区的字节数，每个簇的扇区数，FAT区的个数，每个FAT所占扇区数等等）**

​	**3.FAT表中每个记录的位数成为FAT大小（fat16就代表一个簇的簇号所占空间为十六位，两个字节），这意味着一个fat16磁盘最多能储存20000h个簇，也就限制了fat16的一个分区的大小**





##### 八、相关图表



对于FAT16的一个分区：

每簇占多少个扇区：64

DBR（保留扇区数）0h

FAT1（FAT扇区数）：标记某个簇的状态（已使用 未使用 坏簇），以两个字节为一个单位。  1000h

FAT2（FAT扇区数）：与FAT1完全一样，作为FAT1的备份  1D800h

根目录（32个扇区）    3C000h

数据区（第二个簇）  40000h



第五个簇58000h

第六个簇60000h











### DBR数据对照表

![img](https://img-blog.csdn.net/20170318145547933?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvY2Fzc2llX2h1YW5n/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)



**注意：1Ch:  20为文件 10为文件夹**





### 文件信息对照表

![img](https://img-blog.csdn.net/20170318155723701?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvY2Fzc2llX2h1YW5n/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

![img](https://img-blog.csdn.net/20170318174247672?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvY2Fzc2llX2h1YW5n/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)



### **系统时间与文件系统中的时间的换算**

```
1.0x16~0x17中的时间=小时*2048+分钟*32+秒/2。得出的结果换算成16进制填入即可。也就是：0x16字节的0~4位是以2秒为单位的量值；0x16字节的5~7位和0x17字节的0~2位是分钟；0x17字节的3~7位是小时。
2.0x18~0x19中的日期=(年份-1980)*512+月份*32+日。得出的结果换算成16进制填入即可。也就是：0x18字节0~4位是日期数；0x18字节5~7位和0x19字节0位是月份；0x19字节的1~7位为年号，原定义中0~119分别代表1980~2099，目前高版本的Windows允许取0~127，即年号最大可以到2107年。
```





### FAT16簇空间大小与分区空间大小的关系

| 表9  FAT16分区大小与对因簇大小 |              |            |
| ------------------------------ | ------------ | ---------- |
| 分区空间大小                   | 每个簇的扇区 | 簇空间大小 |
| 0MB-32MB                       | 1            | 512个字节  |
| 33MB-64MB                      | 2            | 1k         |
| 65MB-128MB                     | 4            | 2k         |
| 129MB-225MB                    | 8            | 4k         |
| 256MB-511MB                    | 16           | 8k         |
| 512MB-1023MB                   | 32           | 16k        |
| 1024MB-2047MB                  | 64           | 32k        |
| 2048MB-4095MB                  | 128          | 64k        |





**文件操作函数：**

fseek函数可以把位置指针调整到文件任意位置

fseek函数里需要提供一个基准位置和目标位置到基准位置间的距离，计算机根据他们计算出目标位置 
SEEK_SET 0 把文件头作为基准位置 
SEEK_CUR 1 把当前位置作为基准位置

SEEK_END 2 把文件尾作为基准位置



