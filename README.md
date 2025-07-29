

## 编译普通内核
```bash
make SMP=2 GUEST=n
# qemu 启动
make SMP=2 run
```

## 编译guest
编译一个vcpu=2，所有输出带有[guest:0] 前缀的guest。加载地址是0x7020_0000

```bash
make SMP=2 GUEST_LABEL='[guest:0] ' LOAD_ADDR=0x70200000
```