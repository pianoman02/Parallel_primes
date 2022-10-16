#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<bsp.h>

long P; // number of processors requested
long N; // maximum prime to be found


long sequential_sieve(long n, bool* pb){
	pb[0]=false;
	pb[1]=false;
	// Initialise the rest of the values
	for (long i=2;i<n; i++){
		pb[i] = true;
	}

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

    long p = P;
	long s = bsp_pid();
    long alpha = N+1;

    bsp_size_t tagSize = sizeof(long);
    bsp_set_tagsize(&tagSize);
    bsp_sync();

    // We calculate the primes until (not including alpha) at processor 0
    if (s==0)
    {
        bool* pb = malloc(alpha*sizeof(bool));
        long noOfPrimes = sequential_sieve(alpha, pb);
        long* primesending = malloc(noOfPrimes*sizeof(long));
        long j=0;
        for (long i=0;i<alpha; i++){
		    if (pb[i])
		    {
			    primesending[j] = i;
                j++;
		    }
	    }
        ////////////////////
        // sending
        ////////////////////
        for (long t=0; t<p; t++){
            bsp_send(t,&noOfPrimes,primesending, noOfPrimes*sizeof(long));
            printf("The provided tag to processor %ld is %ld\n", t,noOfPrimes);
        }
    }
    bsp_sync();
    /////////////////////////////////////////
    bsp_nprocs_t nparts_recvd=0;
    bsp_size_t nbytes_recvd=0;
    bsp_qsize(&nparts_recvd, &nbytes_recvd);
    long* arrivedprimes = malloc(nbytes_recvd);
    long totalcount = 0;
    for (long j=0; j<nparts_recvd; j++){
        bsp_size_t payload_size=0;
        long count=-1;
        printf("count = %ld in processor %ld before get_tag\n", count, s);
        bsp_get_tag(&payload_size, &count);
        printf("count = %ld in processor %ld after get_tag\n", count, s);
        bsp_move(&arrivedprimes[totalcount], payload_size);
        totalcount += payload_size/sizeof(long); // apparently count is not working
    }
    for (long i=0; i<totalcount; i++){
        printf("%ld, %ld\n",arrivedprimes[i], s); // The transmission does work well, on the contrary.
    }


    // Now assume we already fixed the stuff until a certain point.
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
    printf("To which number do you want to find the primes?\n");
	fflush(stdout);
	scanf("%ld",&N);
	sieve();
	exit(EXIT_SUCCESS);
}
