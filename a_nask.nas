[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

		GLOBAL	_api_putchar
		GLOBAL	_api_end
		GLOBAL  _api_getpid
		GLOBAL  _api_int_str
		GLOBAL  _api_putstr0
		GLOBAL  _api_sleep
		GLOBAL  _api_exit
		GLOBAL  _api_open
		GLOBAL  _api_write
		GLOBAL  _api_read
		GLOBAL  _api_close
		GLOBAL  _api_crate


[SECTION .text]

_api_putchar:	; void api_putchar(int c);
		MOV		EDX,1
		MOV		AL,[ESP+4]		; c
		INT		0x40
		RET

_api_putstr0:	; void api_putstr0(char *s);
		PUSH	EBX
		MOV		EDX,2
	  MOV		EBX,[ESP+8]		; s
		INT		0x40
		POP		EBX
		RET

_api_end:	; void api_end(void);
		MOV		EDX,4
		INT		0x40

_api_getpid: ; int api_getpid(void);
		MOV   EDX,5
		INT 	0x40
		ret

_api_int_str:
    MOV   EDX,6
		MOV   AL,[ESP+4]
		INT   0x40
		RET

_api_sleep:
		MOV   EDX,7
		MOV   EAX,[ESP+4]
		INT   0x40
		RET

_api_exit:
    MOV   EDX,8
		INT   0x40

_api_open:
		PUSH	EBX
		MOV   EDX,9
		MOV		EBX,[ESP+8]		; s
		MOV   EAX,[ESP+12]
		INT		0x40
		POP		EBX
		RET

_api_write:
		PUSH  EBX
		PUSH  ECX
		MOV   EAX,[ESP+12]
		MOV   EBX,[ESP+16]
		MOV   ECX,[ESP+20]
		MOV   EDX,10
		INT   0x40
		POP   ECX
		POP   EBX
		RET

_api_read:
		PUSH  EBX
		PUSH  ECX
		MOV   EAX,[ESP+12]
		MOV   EBX,[ESP+16]
		MOV   ECX,[ESP+20]
		MOV   EDX,11
		INT   0x40
		POP   ECX
		POP   EBX
		RET

_api_close:
		MOV   EAX,[ESP+4]
		MOV   EDX,12
		INT   0x40
		RET

_api_crate:
		PUSH  EBX
		MOV   EBX,[ESP+8]
		MOV   EDX,13
		INT   0x40
		POP   EBX
		RET
