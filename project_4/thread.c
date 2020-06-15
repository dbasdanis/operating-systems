#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void thread_func(void *args);

int main(int argc,char *argv[]){
	int i;
	int value;
	pthread_t *thr;

	thr = (pthread_t*)malloc(sizeof(pthread_t)*10);
	if(thr == NULL){
		printf("error in allocation memory\n");
		return 1;
	}

	for(i=0;i<10;i++){
		value = pthread_create(&thr[i], NULL, (void*)thread_func ,NULL);
		if(value){
			fprintf(stderr,"Error creating thread\n");
			return (1);
		}	
		printf("(%d) thread created.\n",i);
	}

	for(i=0;i<10;i++){
		pthread_join(thr[i],NULL);
	}

	return 0;
}

void thread_func(void *args){
	
	FILE *stream;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
 
	stream = fopen("test1.txt", "r");
	if (stream == NULL)
		exit(EXIT_FAILURE);
 
	while ((read = getline(&line, &len, stream)) != -1) {
		printf("%s", line);
	}
 
	free(line);
	fclose(stream);
	exit(EXIT_SUCCESS);
}
