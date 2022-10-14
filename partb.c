#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>

int main (int argc, char **argv)
{
	printf("The size of a bool is %ld\n", sizeof(bool));
	long n;
	printf("To which number do you want to find the primes?\n");
	fflush(stdout);
	scanf("%ld",&n);
	printf("%ld primes selected.\n",n);
	
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
	printf("The primes are \n");
	for (long i=0;i<=n; i++){
		if (pb[i])
		{
			for (long j=i*i; j<=n; j+=i){ // optimized a bit
				pb[j] = false;
				totaloperations++;
			}
			printf("%ld \n",i);
		}
	}
	printf("The total number of operations is %ld\n", totaloperations);
	exit(EXIT_SUCCESS);
}
