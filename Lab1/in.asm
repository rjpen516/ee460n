	.ORIG x4000
	ADD R1, R1, R1
	ADD R1, R1, #10
	AND R1, R1, R1
testLabel	AND R1, R1, #10
	BR testLabel
	BRn testLabel
	BRz testLabel
	BRp testLabel
	BRzp testLabel
	BRnp testLabel
	BRnz testLabel
	BRnzp testLabel
	JMP R5
	RET
	JSR testLabel
	JSRR r7
	LDB r6, r4, #10
	LDW r6, r4, #10
	LEA r1, testLabel
	NOT r1,r4
	RTI
	.END
