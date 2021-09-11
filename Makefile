install:
	gcc lamport.c -o lamport $(DEFINES)

help:
	@echo "Defines:"
	@echo "-D DMB_ST_ENABLED"
	@echo "-D DMB_SY_ENABLED"
	@echo "-D GET_ADDRESS"
	@echo "-D PADDING"
	@echo "-D NB_THREADS=N"
