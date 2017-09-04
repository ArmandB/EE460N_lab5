.orig x1200
	 stw r2, r6, #-1
	 stw r1, r6, #-2
	 stw r0, r6, #-3 ; r0 on top of stack
	 add r6, r6, #-6
;
	 lea r1, addr
	 ldw r1, r1, #0 ; r1 addr
	 lea r2, numPages
	 ldw r2, r2, #0 ; r2 ctr

clearRef ldw r0, r1, #0
	 and r0, r0, #-2 ; zero out reference bit
	 stw r0, r1, #0
	 add r1, r1, #2 ; next word
	 add r2, r2, #-1
	 brp clearRef
;
	 ldw r0, r6, #0 ;at location 0x121c
	 ldw r1, r6, #1
	 ldw r2, r6, #2
	 add r6, r6, #6
	 rti ;at location 0x1224
addr     .fill x1000 ; start of the page table
numPages .fill x80 ;128
.end
