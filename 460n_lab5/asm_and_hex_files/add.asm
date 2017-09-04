;calculate sum of first 20 bytes at xc000
;store sum in xc014
;jump to address pointed to by sum in xc014
.orig x3000
;
	lea r1, addends
	ldw r1, r1, #0 ; r1 addr
	and r2, r2, #0
	add r2, r2, #10 ; r2 ctr
	add r2, r2, #10
	and r3, r3, #0 ; sum
loop	ldb r0, r1, #0 ; get addend[i]
	add r3, r3, r0 ; add to sum
	add r1, r1, #1 ; inc addr
	add r2, r2, #-1 ; dec ctr
	brp loop
	stw r3, r1, #0
	jmp r3
	halt ; shouldn't reach halt
addends .fill xc000
.end
