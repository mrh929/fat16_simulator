https://blog.csdn.net/sikuon/article/details/75939434

https://blog.csdn.net/cassie_huang/article/details/62227023

https://blog.csdn.net/yeruby/article/details/41978199

最详细：https://blog.csdn.net/mj5742356/article/details/20125511

长短文件名 https://wenku.baidu.com/view/d729d62a680203d8cf2f2474.html



对于FAT16的一个分区：

每簇占多少个扇区：64

DBR（保留扇区数）0h

FAT1（FAT扇区数）：标记某个簇的状态（已使用 未使用 坏簇），以两个字节为一个单位。  1000h

FAT2（FAT扇区数）：与FAT1完全一样，作为FAT1的备份  1D800h

根目录（32个扇区）    3C000h

数据区（第二个簇）  40000h



第五个簇58000h

第六个簇60000h



1.文件系统中的数据以小端序保存（数字的低位 存在 地址的低位上）-

2.DBR(DOS boot record) 占一个扇区，里面储存了整个文件系统的各种配置信息（如每个扇区的字节数，每个簇的扇区数，FAT区的个数，每个FAT所占扇区数等等）

3.FAT表中每个记录的位数成为FAT大小（fat16）



![img](https://img-blog.csdn.net/20170318145547933?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvY2Fzc2llX2h1YW5n/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)





![img](https://img-blog.csdn.net/20170318155723701?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvY2Fzc2llX2h1YW5n/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

![img](https://img-blog.csdn.net/20170318174247672?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvY2Fzc2llX2h1YW5n/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)





8、fseek函数可以把位置指针调整到文件任意位置

fseek函数里需要提供一个基准位置和目标位置到基准位置间的距离，计算机根据他们计算出目标位置 
SEEK_SET 0 把文件头作为基准位置 
SEEK_CUR 1 把当前位置作为基准位置

SEEK_END 2 把文件尾作为基准位置





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









# 自己的理解

一、关于需要掌握的基础知识

​	十六进制计算

​	链表

​	小端序



二、FAT16文件系统的结构

​	FAT16第一个扇区是DBR，是对整个分区的格式及参数进行说明的部分，从中可以读取到FAT所占扇区数、每簇扇区数、每个扇区的字节数、保留扇区数…… 

​	系统通过这些参数可以计算出FAT1、FAT2、根目录、数据区的位置，其中，保留扇区之后的即为FAT区，FAT区由两个完全相同的FAT1、FAT2构成，FAT2作为FAT1的一个备份，保障数据安全。

​	FAT1中存放了每个簇之间的对应关系以及自身的属性。

​	通过查表我们可以了解到这个簇到底是未被使用还是已被使用，以及这个簇后面还有没有链接到下一个簇。从而将连续的数据离散储存。

​	FAT2后的是根目录，根目录占用32个扇区，当中储存了根节点下面所拥有的所有文件和文件夹（文件夹可以当作一种特殊的文件），每个文件占用32个字节（当文件名过长的时候可能会占用更多空间），这32个字节分别对应的是这个文件对应的各种属性以及簇号。

​	我们可以通过寻找该簇来读取该文件的具体内容，如果是文件夹，这个簇中储存的是文件夹内的所有子目录元素（与根目录的作用类似）。

​	文件夹创建的时候会生成两个32字节的项

​		一个是  "." ，其中储存的是文件夹自身的信息

​		另一个是  ".."， 储存了这个文件夹的父目录的信息（方便返回上一级）





三、删除文件的过程

​	找到描述该文件的那个32字节的项，将偏移值为0h的数据改为 E5，以说明该数据被删除。如果是文件夹的话，还应递归删除其目录下的所有文件。

​	