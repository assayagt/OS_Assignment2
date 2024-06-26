#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NCHANNEL 2
#define MAX_PRIMES 100

int is_prime(int n) {
    if (n <= 1) return 0;

    //check if n is prime
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

void generator(int channel1) {
    int num = 2;

    //generate numbers starting from 2 and put them into channel1
    while (1) {
        if (channel_put(channel1, num) == -1) break;
        num++;
    }

    //if channel1 destroyed then exiting
    printf("Generator (PID: %d) exiting\n", getpid());
}


void checker(int checker_id, int channel1, int channel2) {
    int num;
    while (1) {

        //take and check numbers from channel1
        if (channel_take(channel1, &num) == -1) break;
        if (is_prime(num)) {

                //put only prime numbers into channel2
                if (channel_put(channel2, num) == -1)
                {
                    //if channel2 is destroyed then destroy channel1
                    channel_destroy(channel1);
                    break;
                }
            }
    }

    //checker exiting
    printf("Checker %d (PID: %d) exiting\n", checker_id, getpid());
    exit(0);
}

void printer(int channel2) {
    int num;
    int prime_count = 0;

    //print 100 primes which are taken from channel2
    while (prime_count < MAX_PRIMES) {
        if (channel_take(channel2, &num) == -1) break;
        printf("%d\n", num);
        prime_count++;
    }

    //exiting after printing 100 primes
    printf("Printer (PID: %d) exiting after finding %d prime numbers\n", getpid(), prime_count);

    //destroying channel2 before exit
    channel_destroy(channel2);
    exit(0);
}

int main(int argc, char *argv[]) {

    //set checkers default value
    int num_checkers = 3;
    if (argc > 1) {

        //get user input
        num_checkers = atoi(argv[1]);
    }

    while(1)
    {
        //create channel1: generator-checker channel
        int channel1 = channel_create();

        //create channel2: checker-printer channel
        int channel2 = channel_create();


        if (channel1 == -1 || channel2 == -1) {
            printf("Failed to create channels\n");
            exit(1);
        }

        //create printer process
        if(fork() == 0)
        {
            printer(channel2);
            exit(0);
        }
        else
        {
            //create checkers processes
            int pid;
            for (int i = 0; i < num_checkers; i++) {
                pid = fork();
                if (pid == 0) {
                    checker(i, channel1, channel2);
                    exit(0);
                }
            }

            //initialize generator
            generator(channel1);
            
            //prompt for user to shutdown or cointinue
            printf("Do you want to restart the system? (y/n): ");
            char buf[10];
            gets(buf, 10);
            if (buf[0] != 'y' && buf[0] != 'Y') {
                break;
            }
        }
    }
    
    exit(0);
}

