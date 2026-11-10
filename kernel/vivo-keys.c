#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <poll.h>
#include <time.h>

static int find_dev(const char *name){
    DIR *d=opendir("/sys/class/input"); struct dirent *e;
    char p[256],n[128];
    if(!d) return -1;
    while((e=readdir(d))){
        if(strncmp(e->d_name,"event",5)) continue;
        snprintf(p,sizeof(p),"/sys/class/input/%s/device/name",e->d_name);
        FILE *f=fopen(p,"r"); if(!f) continue;
        if(fgets(n,sizeof(n),f) && !strncmp(n,name,strlen(name))){ fclose(f); closedir(d);
            snprintf(p,sizeof(p),"/dev/input/%s",e->d_name); return open(p,O_RDONLY|O_NONBLOCK); }
        fclose(f);
    }
    closedir(d); return -1;
}

int main(void){
    int f4=find_dev("vivo_virtual_key"), f0=find_dev("gpio-keys");
    int f5=find_dev("Synaptics SAM-ONCW");
    char lp[256]="/sys/class/leds/rgb:kbd_backlight/brightness";
    { DIR *d=opendir("/sys/class/leds"); struct dirent *e;
      if(d){ while((e=readdir(d))){ if(strstr(e->d_name,"kbd")||strstr(e->d_name,"white")){ snprintf(lp,sizeof(lp),"/sys/class/leds/%s/brightness",e->d_name); break; } } closedir(d);} }
    int led=open(lp,O_WRONLY);
    int lit=0; long long last=0;
    if(f4<0) return 1;
    int u=open("/dev/uinput",O_WRONLY|O_NONBLOCK);
    if(u<0) return 1;
    ioctl(u,UI_SET_EVBIT,EV_KEY); ioctl(u,UI_SET_EVBIT,EV_SYN);
    int keys[]={KEY_LEFTMETA,KEY_ESC,KEY_BACK,KEY_APPSELECT,KEY_LEFTALT,KEY_TAB};
    for(unsigned i=0;i<sizeof(keys)/sizeof(keys[0]);i++) ioctl(u,UI_SET_KEYBIT,keys[i]);
    struct uinput_setup us; memset(&us,0,sizeof(us));
    us.id.bustype=BUS_VIRTUAL; us.id.vendor=1; us.id.product=1;
    strcpy(us.name,"vivo-keys");
    ioctl(u,UI_DEV_SETUP,&us); ioctl(u,UI_DEV_CREATE);
    usleep(200000);
    struct pollfd p[3]={{f4,POLLIN,0},{f0,POLLIN,0},{f5,POLLIN,0}};
    for(;;){
        int pr=poll(p,3,200);
        struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts);
        long long now=ts.tv_sec*1000LL+ts.tv_nsec/1000000;
        if(lit && now-last>3000){ if(led>=0) write(led,"0",1); lit=0; }
        if(pr<0) break;
        for(int i=0;i<3;i++) if(p[i].revents&POLLIN){
            struct input_event ev;
            int fd = i==0?f4:(i==1?f0:f5);
            while(read(fd,&ev,sizeof(ev))==sizeof(ev)){
                if(ev.type==EV_SYN) continue;
                if(led>=0){ if(!lit){ write(led,"255",3); lit=1; } last=now; }
                if(ev.type!=EV_KEY) continue;
                int out=-1;
                if(ev.code==KEY_APPSELECT){
                    struct input_event evs[6]={
                        {.type=EV_KEY,.code=KEY_LEFTALT,.value=1},
                        {.type=EV_KEY,.code=KEY_TAB,.value=1},
                        {.type=EV_SYN,.code=SYN_REPORT,.value=0},
                        {.type=EV_KEY,.code=KEY_TAB,.value=0},
                        {.type=EV_KEY,.code=KEY_LEFTALT,.value=0},
                        {.type=EV_SYN,.code=SYN_REPORT,.value=0},
                    };
                    if(ev.value==1) write(u,evs,sizeof(evs));
                    continue;
                }
                else if(ev.code==KEY_BACK) out=KEY_ESC;
                else if(ev.code==KEY_HOME||ev.code==KEY_HOMEPAGE) out=KEY_LEFTMETA;
                else continue;
                struct input_event o={.type=EV_KEY,.code=out,.value=ev.value};
                write(u,&o,sizeof(o));
                struct input_event s={.type=EV_SYN,.code=SYN_REPORT,.value=0};
                write(u,&s,sizeof(s));
            }
        }
    }
    return 0;
}
