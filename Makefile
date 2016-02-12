kbpm : ./src/kbpm.c ./src/client.c ./src/server.c ./src/process.c ./lib/cJSON.c
	cc -o ./bin/kbpm ./src/kbpm.c ./src/client.c ./src/server.c ./src/process.c ./lib/cJSON.c -lm -lpthread
