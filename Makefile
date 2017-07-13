CC=g++
OPTIONS	= -std=gnu99 -lm -fno-rtti 
INC	= 
SELF_INC= ./
CFLAGS 	= -lm -g
DEPS   	= 
OBJ     =   sequence_player.o main.o drive_five.o

CPPOBJ    = 

LDFLAGS = -lpthread

%.o: %.cpp $(DEPS)
	$(CC) $(addprefix -I,$(INC)) -c -o $@ $< $(CFLAGS)
%.o: %.c $(DEPS)
	$(CC) $(addprefix -I,$(INC)) $(addprefix -I,$(SELF_INC)) -c -o $@ $< $(OPTIONS) $(CFLAGS)
	
seqplay: $(OBJ)
	$(CC) -o  $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o $(PRG).elf *.bak 
	rm -rf ../core/can/*.o
	rm -rf ../core/msg/*.o
	rm -rf ../core/misc/*.o
	rm -rf *.lst *.map

