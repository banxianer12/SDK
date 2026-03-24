需要先执行 pushptr 脚本，将所有必须文件推到板端

进入 avm_test 目录，直接执行
 ./avm_test 0 1 2 3 100
0   	/dev/video0
1   	/dev/video1
2   	/dev/video2
3   	/dev/video3
100     定时器时间，100s后自动进入休眠，然后按键唤醒系统继续测试
这个 demo 主要是简单演示 avm 算法，此套算法为第三方算法，仅提供预览参考作用。



