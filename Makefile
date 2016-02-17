kbpm : ./src/kbpm.c ./src/client.c ./src/server.c ./src/process.c \
	./src/utils.c ./lib/cJSON.c ./src/box_drawing.c
	cc -o kbpm ./src/kbpm.c ./src/client.c ./src/server.c \
		./src/process.c ./src/utils.c ./lib/cJSON.c  \
		./src/box_drawing.c -lm -lpthread

install :
	cp kbpm /usr/local/bin/

uninstall :
	rm -f /usr/local/bin/kbpm

clean :
	rm -f kbpm
