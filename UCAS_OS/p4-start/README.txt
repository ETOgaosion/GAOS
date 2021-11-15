用法：
1. 将Project3的目录拷贝为Project4
2. 执行命令（尖括号部分替换成相应路径）./apply-p4-start-code.sh <Project4路径>

该脚本会：
1. 通过patch工具将有修改的文件自动打补丁打进去
2. 将tiny_libc/syscall.S改名为tiny_libc/invoke_syscall.S
3. 将arch/riscv/kernel/head.S拷贝到arch/riscv/kernel/start.S
4. 试图将head.S中的call main改成call boot_kernel，因为p4需要先进入boot_kernel函数建立内核页表
5. 将新增文件拷贝到相应位置

Makefile改动太大，请大家自己参考着改吧。可以考虑用新的Makefile替换掉旧的，然后再把旧Makefile中的内核入口地址之类的修改自己手工弄过来。

常见问题是：
由于大家的文件修改各不一样，可能有些补丁打不进去。如果出现补丁打不进去的报错，按照报错提示打开相应的.rej文件。.rej文件里写了没打进去的补丁。大家手工把修改写进去就行。
