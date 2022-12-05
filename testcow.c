#include "types.h"
#include "stat.h"
#include "user.h"

int a = 1;

void test()
{
    printf(1,"%d free pages before forking\n",getNumFreePages());
    printf(1,"Parent and Child share the global variable a \n");
    printf(1,"Parent: a = %d\n",a);
    int pid = fork();
    if(pid==0)
    {
        printf(1,"Child: a = %d\n",a);
        printf(1,"%d free pages before any changes\n",getNumFreePages());
        a = 2;
        printf(1,"Child: a = %d\n",a);
        printf(1,"%d free pages after changing a\n",getNumFreePages());
        exit();
    }
    
    wait();
    printf(1,"Parent: a = %d\n",a);
    printf(1,"%d free pages after wait\n",getNumFreePages());
    return ;
}


int main(void)
{
    printf(1,"Test1 running....\n");
    test();
    printf(1,"Test1 finished\n");

    exit();
}