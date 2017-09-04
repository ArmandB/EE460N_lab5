.orig x3000
lea r1, addr
ldw r1, r1, #0; ldw test
;
stw r0, r1, #1; stw test, check x3002
;
ldb r0, r1, #1; ldb test
;
stb r0, r1, #0; stb test, check x3000 lsb and msb same
;
halt; trap test
addr .fill x8000
.end
