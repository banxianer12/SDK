版本说明：
1、支持T507系列整包固件升级；
2、支持升级备份rootfs区的根文件系统；
3、增加了nand环境下烧写boot0及uboot；

当前版本局限性：
1、当前仅支持整包固件升级中的boot0,uboot,env,boot-resource, boot. rootfs分区，后面会增加所有downloadfile升级；
2、因为除了boot0和uboot分区按照整个mmc或者nand某个起始扇区升级，其他的都按照分区升级，所以除了boot0，uboot，其他的分区
  要与原分区大小相等，并且如果增加新分区，增加的分区有可能导致升级失败；

使用步骤：
1、修改device/config/chips/t507/configs/demo2.0/longan/sys_partition.fex，例如修改如下：
[partition]
    name         = rootfs
    size         = 4194304
    downloadfile = "rootfs.fex"
	user_type    = 0x8000
增加：
[partition]
    name         = rootfsbak
    size         = 4194304
	user_type    = 0x8000
	
2、将ota文件夹拷贝到platform/framework目录,并在该目录Make生成upgrade可执行文件；
3、将upgrade放在u盘或者SD卡或者直接推送到T507目标板根目录；
4、将out/t507_linux_demo2.0_uart0.img放在与upgrade同级目录；
5、./upgrade t507_linux_demo2.0_uart0.img mmcblk0p5
6、当更新完毕，看到system OTA successfull!就表示更新成功；

说明：
目前的T507的rootfs在/dev/mmcblk0p4，分区，我们将新的固件更新到/dev/mmcblk0p5,在OTA成功后，在系统启动时，可以看到
RootDevice is "/dev/mmcblk0p5"

更新说明：
1、使用了开源的simg2img代码，解决了原来代码解析并烧录sparse格式后导致rootfs一些错误导致其只读；
2、该版本将读出的分区文件直接写入，而不再先写入存储器在写入分区；
3、修改了Makefile文件，新的.c和.h会自动处理
