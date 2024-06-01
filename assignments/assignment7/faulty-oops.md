# Faulty driver test

### Trigger the test
    root@qemuarm64:~# echo "Hello World!" >> /dev/faulty 

### The kernel panics and restarts, following messages are printed on screen and through dmesg

```
[  305.675246] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
[  305.675915] Mem abort info:
[  305.676187]   ESR = 0x0000000096000045
[  305.676311]   EC = 0x25: DABT (current EL), IL = 32 bits
[  305.676463]   SET = 0, FnV = 0
[  305.676570]   EA = 0, S1PTW = 0
[  305.676808]   FSC = 0x05: level 1 translation fault
[  305.689479] Data abort info:
[  305.693017]   ISV = 0, ISS = 0x00000045
[  305.697214]   CM = 0, WnR = 1
[  305.711999] user pgtable: 4k pages, 39-bit VAs, pgdp=0000000043653000
[  305.724706] [0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
[  305.738138] Internal error: Oops: 0000000096000045 [#2] PREEMPT SMP
[  305.746558] Modules linked in: scull(O) hello(O) faulty(O)
[  305.748725] CPU: 0 PID: 353 Comm: sh Tainted: G      D    O      5.15.157-yocto-standard #1
[  305.749297] Hardware name: linux,dummy-virt (DT)
[  305.750743] pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[  305.753632] pc : faulty_write+0x18/0x20 [faulty]
[  305.754167] lr : vfs_write+0xf8/0x29c
[  305.754527] sp : ffffffc0096f3d80
[  305.761964] x29: ffffffc0096f3d80 x28: ffffff800f892000 x27: 0000000000000000
[  305.762305] x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
[  305.762569] x23: 0000000000000000 x22: ffffffc0096f3dc0 x21: 0000005569655a50
[  305.762817] x20: ffffff80037acc00 x19: 000000000000000d x18: 0000000000000000
[  305.763041] x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
[  305.763285] x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
[  305.763505] x11: 0000000000000000 x10: 0000000000000000 x9 : ffffffc00826b8bc
[  305.763703] x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
[  305.763881] x5 : 0000000000000001 x4 : ffffffc000b80000 x3 : ffffffc0096f3dc0
[  305.764139] x2 : 000000000000000d x1 : 0000000000000000 x0 : 0000000000000000
[  305.764336] Call trace:
[  305.764447]  faulty_write+0x18/0x20 [faulty]
[  305.764675]  ksys_write+0x74/0x10c
[  305.764819]  __arm64_sys_write+0x24/0x30
[  305.764957]  invoke_syscall+0x5c/0x130
[  305.765122]  el0_svc_common.constprop.0+0x4c/0x100
[  305.765268]  do_el0_svc+0x4c/0xb4
[  305.765402]  el0_svc+0x28/0x80
[  305.765529]  el0t_64_sync_handler+0xa4/0x130
[  305.765642]  el0t_64_sync+0x1a0/0x1a4
[  305.765788] Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
[  305.765950] ---[ end trace ea1aa49d95745447 ]---
Segmentation fault

Poky (Yocto Project Reference Distro) 4.0.18 qemuarm64 /dev/ttyAMA0

```

### Analysis

- At Line 1 
  > `Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000`

- This means that a NULL pointer was dereferenced, the virtual address was 0000000000000000 which is a NULL
- When reading the Call trace, we can check the location of the program counter (PC) to find the location of the error 
    > `[  305.753632] pc : faulty_write+0x18/0x20 [faulty]`

- From this line, we find out that the program counter was at function "faulty_write", 0x18 bytes into the function, which is 0x20 long