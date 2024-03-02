
.global enable_interrupt

.text

enable_interrupt:
	ei
	jr $ra
	nop