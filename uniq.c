#include "types.h"
#include "stat.h"
#include "user.h"

char buf[10000];
int n;
char lines[1000][1000];
int lineIdx = 0;
int charIdx = 0;
char curr[10000];
char prev[10000];
int count=1;
int c_flag=0,d_flag=0,i_flag =0,noext=0;

void case_check(char* text){
    int a=0;
    while(text[a]!='\0'){
        if(text[a]>='A' && text[a]<='Z'){
            text[a]+='a'-'A';
        }
        ++a;
    }
}

void uniq(int fd){
    while((n=read(fd,buf,sizeof(buf)))>0){
        for(int i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                lines[lineIdx][charIdx] ='\0';
                lineIdx++;
                charIdx = 0;
            } else {
                lines[lineIdx][charIdx] = buf[i];
                charIdx++;
            }
        }

        for(int i=0;i<lineIdx;i++) {
            strcpy(prev, lines[i]);
            strcpy(curr, lines[i+1]);

            if(c_flag==1 && i_flag==1){
                char a[1000],b[1000];
                strcpy(a, prev);
                strcpy(b, curr);
                case_check(a);
                case_check(b);
                // printf(1,"prev %s,curr %s\n",prev,curr);
                //if(strcmp(prev,curr)!=0){
                    if(strcmp(a,b)==0){
                        strcpy(lines[i+1],lines[i]);
                        strcpy(curr,lines[i+1]);
                        count++;
                    }else{
                        printf(1,"%d %s\n",count, prev);
                        count =1;
                    }
                // }else{
                //     count++;
                // }
            }
            else if(d_flag==1 && i_flag==1){
                char a[1000],b[1000];
                strcpy(a, prev);
                strcpy(b, curr);
                case_check(a);
                case_check(b);
                if(strcmp(a,b)==0 && count==1){
                    printf(1,"%s\n",prev);
                    count++;
                }
                else{
                    count=1;
                }
            }
            else if(c_flag==1){
                if(strcmp(prev,curr)!=0){
                    printf(1,"%d %s\n",count,prev);
                    count =1;
                }else{
                    count++;
                }
            }
            
            else if(d_flag==1){
                if(strcmp(prev,curr)==0 && count==1){
                    printf(1,"%s\n",prev);
                    count++;
                }
                else{
                    count=1;
                }
            }
            
            else if(i_flag==1){
                char x[1000],y[1000];
                strcpy(x,prev);
                strcpy(y,curr);
                case_check(x);
                case_check(y);
                if(strcmp(prev,curr)!=0){
                    if(strcmp(x,y)==0){
                        strcpy(lines[i+1],lines[i]); // Replace current value using previous value in the char arr which we are looping currently.
                        // strcpy(prev,curr);  // Replace this instance of previous with current.
                        strcpy(curr,lines[i+1]);
                    }else{
                        printf(1,"%s\n",prev);
                    }
                }
            }       
            else{
                if(strcmp(prev,curr)!=0){
                    printf(1,"%s\n",prev);
                }
            }
        }
        if(c_flag==1 && i_flag==1){
                 printf(1,"%d %s\n",count,curr);
             }
        else if(c_flag==1){
                 printf(1,"%d %s\n",count,curr);
             }
        else if(d_flag!=1 && i_flag==1){
                 printf(1,"%s\n",curr);
             }
        else if(i_flag==0 && c_flag ==0 && d_flag ==0){
                printf(1,"%s\n",curr);
             }
 
    }

    
}
int main(int argc,char* argv[]){
    int fd,i;

    
    for(i=1;i<argc;i++){
        
        if(strcmp(argv[i],"-c")==0){
            c_flag=1;
        }
        else if(strcmp(argv[i],"-d")==0){
            d_flag=1;
        }
        else if(strcmp(argv[i],"-i")==0){
            i_flag=1;
        }
        else{
            if((fd=open(argv[i],0))<0){
            printf(1,"uniq cannot open the file %s\n",argv[i]);
            exit();
            }
        }
    }
    uniq(fd);
    close(fd);
    exit();
}