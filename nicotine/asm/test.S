.global _start

_start:
    // syscall write(int fd, const void *buf, size_t count)
    mov     X0, #1      // fd := stdout
    adr     X1, msg    // buf := msg
    mov     X2, #14    // count := length of string
    mov     X16, #4     // write is syscall #4
    svc     #0          // invoke syscall
    
    mov     X0, #0      //status 0
    mov     X16, #1     //exit is syscall 93
    svc     #0          //invoke syscall

msg:    .ascii      "Hello, World!\n"
