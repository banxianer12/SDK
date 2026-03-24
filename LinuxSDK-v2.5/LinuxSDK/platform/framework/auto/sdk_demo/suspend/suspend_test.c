
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>
 
#define DEV_NAME   "/dev/input/event1"  //power key

#define POWER_KEY 116

#define POWER_FILE /sys/power/state

enum {
	KEY_RELEASE=0,
	KEY_PRESS=1
};


int main(int argc, char **argv)
{
	int fd,retval;
	fd_set readfds;
	
	struct input_event input;   
	fd = open(DEV_NAME, O_RDONLY);
	if (fd < 0)
	{
		printf("can't open %s,errno:%s\n",DEV_NAME,strerror(errno));
		return -1;
	}


	while(1)
	{
		FD_ZERO( &readfds );  
		FD_SET( fd, &readfds ); 
		printf("wait power key\n");		
		retval = select(fd+1, &readfds, NULL, NULL, NULL);  
		if(retval == 0) 
		{  
			printf( "Time out!\n" );  
		}  
		if(FD_ISSET(fd,&readfds)) 
		{  
			
			retval=read(fd, &input,sizeof(struct input_event));
			if (retval <= 0)
			{
				printf("read power key fail %s\n",strerror(errno));
			}
			if(input.type == EV_KEY)
			{
				printf("get input keycode is %d,value%d\n",input.code,input.value);
			    if( input.code == POWER_KEY && input.value == KEY_RELEASE)
				{
					printf("system will enter suspend\n");
					system("echo mem > /sys/power/state");
					break;
				}
			}
			
		}	
	}
	
	close(fd);
	
	return 0;
}

