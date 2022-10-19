#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<bsp.h>

long P; // number of processors requested
long N; // maximum prime to be found

bool** primesalloc(long alpha){
    long amountofsteps = 0;
    long test = alpha;
    for (;test<N; test++){
        test = test*test;
        amountofsteps++;
    }
}

long sequential_sieve(long n, bool* pb){
	pb[0]=false;
	pb[1]=false;

	long totalprimes = 0;
	// Now the sieve
	for (long i=0;i<n; i++){
		if (pb[i])
		{
			for (long j=i*i; j<n; j+=i){ // optimized a bit
				pb[j] = false;
			}
            totalprimes++;
		}
	}
    return totalprimes;
}

void sieve() {
	bsp_begin( P );
    N = 300;
    long p = P;
	long s = bsp_pid();
    long alpha = p*100; // We assume p devides alpha

    bsp_size_t tagSize = sizeof(long);
    bsp_set_tagsize(&tagSize);
    bsp_sync();

    // We calculate the primes until (not including alpha) at processor 0
    if (s==0)
    {
        bool* pb = malloc(alpha*sizeof(bool));
        for (long i=0; i<alpha; i++){
            pb[i] = true;
        }

        long noOfPrimes = sequential_sieve(alpha, pb);
        long* primesending = malloc(noOfPrimes*sizeof(long));
        long j=0;
        for (long i=0;i<alpha; i++){
		    if (pb[i])
		    {
			    primesending[j] = i;
                j++;
                printf("%ld\n", i);
		    }
	    }
        ////////////////////
        // sending
        ////////////////////
        for (long t=0; t<p; t++){
            bsp_send(t,&noOfPrimes,primesending, noOfPrimes*sizeof(long));
        }
    }
    bsp_sync();

    /////////////////////////////////////////
    /// Next superstep
    /////////////////////////////////////////

    /*
    // First, we allocate all the numbers.
    bool** pb2 = primesalloc(alpha);


    long lowprimetest = alpha; // in every step, this number is squared
    long highprimetest = min(N,alpha*alpha);
    while(lowprimetest < N)
    {
        // How to make sure that we already assign all the numbers? Should we?



        lowprimetest = highprimetest;
        highprimetest = min(N, lowprimetest*lowprimetest);
    }
    */
    bsp_nprocs_t nparts_recvd=0;
    bsp_size_t nbytes_recvd=0;
    bsp_qsize(&nparts_recvd, &nbytes_recvd);
    long* arrivedprimes = malloc(nbytes_recvd);
    long totalcount = 0;
    for (long j=0; j<nparts_recvd; j++){
        bsp_size_t payload_size=0;
        long count=-1;
        bsp_get_tag(&payload_size, &count);
        bsp_move(&arrivedprimes[totalcount], payload_size);
        totalcount += count;
    }
    // Now, we allocate new memory for the new primes
    long size = (alpha*alpha-alpha)/p;
    bool* pb2 = malloc(size*sizeof(bool));
    for (long i=0; i<size; i++){
        pb2[i] = true;
    }
    // Now we loop over the provided primes:
    long a = (p-s)*alpha/p + s*alpha*alpha/p;
    long b = (p-s-1)*alpha/p + (s+1)*alpha*alpha/p;
    for (long j=0; j<totalcount; j++){
        long q = arrivedprimes[j];
        long d = (a+q-1)/q; //  $\lceil a/q \rceil$
        for (; d*q<b;d++){
            pb2[d*q-a] = false;
        }
    }
    for (long i=0; i<size; i++){
        if (pb2[i]){
            printf("%ld\n", i+a);
        }
    }


	bsp_end();
}

int main( int argc, char **argv ) {
    bsp_init(sieve, argc, argv);
    printf( "How many processors do you want to use?\n");
    fflush(stdout);
    scanf("%ld",&P);
    if (P > bsp_nprocs()){
        printf("Sorry, only %u processors available.\n",
               bsp_nprocs());
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    //printf("To which number do you want to find the primes?\n");
	//fflush(stdout);
	//scanf("%ld",&N);
	sieve();
	exit(EXIT_SUCCESS);
}
