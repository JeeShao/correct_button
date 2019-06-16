/*借助条件变量模拟 生产者-消费者 问题*/
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <iostream>

using namespace std;
/*链表作为公享数据,需被互斥量保护*/
int A=0;
int FLAG=0;
int RESF = 0;
int SHARE = 0;

/* 静态初始化 一个条件变量和一个互斥量*/
pthread_cond_t has_product = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *consumer1(void *p)
{
    for (int i=0;i<10000;i++)
    {
        cout<<"进入T2"<<endl;
        pthread_mutex_lock(&lock);
        if(i%3==1)
        {
            FLAG=1;
            cout<<"i="<<i<<endl;
            pthread_cond_wait(&has_product, &lock);
            cout<<"被唤醒"<<endl;
            cout<<"-Consume1 --- "<< i<<' ' <<SHARE<<endl;
        }
        cout<<"T2运行中"<<endl;

        pthread_mutex_unlock(&lock);
        sleep(rand() % 5);
    }
}

void *producer(void *p)
{
    for (;;)
    {
        printf("-Produce ---%d\n", ++A);
        pthread_mutex_lock(&lock);
        if(FLAG==1){
            SHARE = A;
            FLAG = 0;
            pthread_cond_signal(&has_product);  // 将等待在该条件变量上的一个线程唤醒
            cout<<"唤醒"<<endl;
        }
        pthread_mutex_unlock(&lock);


//        pthread_cond_signal(&has_product);  // 将等待在该条件变量上的一个线程唤醒
        sleep(rand() % 5);
    }
}

int main0(int argc, char *argv[])
{
    pthread_t pid, cid;
    srand(time(NULL));

    pthread_create(&pid, NULL, producer, NULL);
    pthread_create(&cid, NULL, consumer1, NULL);

    pthread_join(pid, NULL);
    pthread_join(cid, NULL);

    return 0;
}