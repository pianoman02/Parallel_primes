#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<math.h>
#include <unistd.h>
#include <sys/time.h>

int main (int argc, char **argv)
{
	long n;
	printf("To which number do you want to find the primes?\n");
	fflush(stdout);
	scanf("%ld",&n);
	bool printPrimes;
	char printChar;
	printf("Do you want to print the primes? (y/n)\n");
	scanf(" %c",&printChar);
    switch(printChar){
        case 'y': printPrimes=true; break;
        case 'n': printPrimes=false; break;
        default: printf("Sorry, that is not a 'y' or 'n' \n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
	bool* pb;
	pb = malloc((n+1)*sizeof(bool));
	pb[0]=false;
	pb[1]=false;
	// Initialise the rest of the values
	for (long i=2;i<=n; i++){
		pb[i] = true;
	}

	long totaloperations = 0;
	// Now the sieve
	long stopat = -1;
	if (printPrimes){
		printf("The primes are \n");
		stopat = n;
	}
	else
	{
		stopat = ceil(sqrt(n));
	}
	double start, end;
	struct timeval timecheck;
	gettimeofday(&timecheck, NULL);
	start = timecheck.tv_sec + (double)timecheck.tv_usec / 1000000;
	for (long i=0;i<=n; i++){
		if (pb[i])
		{
			for (long j=i*i; j<=n; j+=i){ // optimized a bit
				pb[j] = false;
				totaloperations++;
			}
			if (printPrimes)
				printf("%ld \n",i);
		}
	}
	gettimeofday(&timecheck, NULL);
	end = timecheck.tv_sec + (double)timecheck.tv_usec / 1000000;
	double difference = end-start;
	printf("It took in total %f seconds\n", difference);
	printf("The total number of operations is %ld\n", totaloperations);
	exit(EXIT_SUCCESS);
}
