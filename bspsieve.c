#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<bsp.h>

long P; // number of processors requested
long N; // maximum prime to be found

long Min(long a, long b){
    return a<b?a:b;
}

// This function a double array to the places where the primes are stored.
// To call this function, one must have N>alpha
bool** primesalloc(long alpha, long* size, long** noOfPrimes){
    // We neglect for simplicity the storage of the first alpha-1 numbers which are calculated by the first processor.
    bool* storage = malloc(sizeof(bool)*(N-alpha)/P);
    for (long i=0; i<(N-alpha)/P; i++)
        storage[i] = true;
    long amountOfRounds = 0; // The amount of rounds that have to happen while calculating the other prime numbers.
    long endOfMemory = alpha;
    while (endOfMemory < N) // there still has to be added memory
    {
        endOfMemory = endOfMemory*endOfMemory;
        amountOfRounds++;
    }
    bool** primearray = malloc(sizeof(bool*)*amountOfRounds);
    long* amountOfPrimes = malloc(sizeof(long)*amountOfRounds);

    long beginOfMemory = alpha;
    endOfMemory = Min(alpha*alpha,N);
    long i=0;
    while (beginOfMemory < N)
    {
        primearray[i] = storage + ((beginOfMemory - alpha)/P);
        amountOfPrimes[i] = (endOfMemory-beginOfMemory)/P;
        beginOfMemory = endOfMemory;
        endOfMemory = Min(endOfMemory*endOfMemory,N);
        i++;
    }
    // Returning
    if (size != NULL)
        *size = amountOfRounds;
    *noOfPrimes = amountOfPrimes;
    return primearray;
}

void primesfree(bool** primearray){
    free(primearray[0]);
    free(primearray);
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

void sequential_part_sieve(long alpha,long p){
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
        free(primesending);
        free(pb);
}

void checkPrimeOnSection(bool* section, long s, long q, long a, long b, long*primesamount){
    long d = (a+q-1)/q; //  $\lceil a/q \rceil$
    for (; d*q<b;d++){
        if(section[d*q-a]){
            section[d*q-a] = false;
            (*primesamount)--;
        }
    }
}

void checkPrimesOnSection(bool* section, long s, long* arrivedprimes, long totalcount, long a, long b, long*primesamount){
    for (long j=0; j<totalcount; j++){
        checkPrimeOnSection(section,s,arrivedprimes[j],a,b,primesamount);
    }
}

void parallel_part_sieve(long s,long alpha, long p,long n){
    // First, we allocate all the numbers. We assume that N > alpha
    long size;
    long* amountOfPrimes;
    bool** pb2 = primesalloc(alpha,&size, &amountOfPrimes);
    
    long lowprimetest = alpha; // in every step, this number is squared
    long highprimetest = Min(N,alpha*alpha);
    long roundno = 0;
    while(lowprimetest < n)
    {
        if (s==0)
            printf("Starting new phase -------------------------\n");
        
        // Reading new primes from the input
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

        // Sieving the primes
        long sectionUnderBound = lowprimetest;
        long sectionUpperBound = highprimetest;
        for (long sectionno = roundno; sectionno<size; sectionno++){
            long a = ((p-s)*sectionUnderBound + s*sectionUpperBound)/p;
            long b = ((p-s-1)*sectionUnderBound + (s+1)*sectionUpperBound)/p;
            checkPrimesOnSection(pb2[sectionno],s,arrivedprimes,totalcount, a, b,&amountOfPrimes[sectionno]);
            sectionUnderBound = sectionUpperBound;
            sectionUpperBound = Min(N, sectionUpperBound*sectionUpperBound);
        }

        // Broadcasting the numbers
        long a_this_section = ((p-s)*lowprimetest + s*highprimetest)/p;
        long size_this_section = (highprimetest-lowprimetest)/p;
        bool* this_section = pb2[roundno];
        long* primesending = malloc(amountOfPrimes[roundno]*sizeof(long));
        long j=0;
        printf("There are %ld primes\n",amountOfPrimes[roundno]);
        for (long i=0;i<size_this_section; i++){
            if(this_section[i]){
                printf("%ld\n", i+a_this_section);
                primesending[j] = i+a_this_section;
                j++;
            }
        }
        for (long t=0; t<p; t++){
            bsp_send(t,amountOfPrimes+roundno,primesending, amountOfPrimes[roundno]*sizeof(long));
        }

        // Prepairing for next round
        lowprimetest = highprimetest;
        highprimetest = Min(N, highprimetest*highprimetest);
        roundno++;
        free(arrivedprimes);
        free(primesending);
        bsp_sync();
    }
    
    // Free the resources again
    primesfree(pb2);
    free(amountOfPrimes);
}

void sieve() {
	bsp_begin( P );
    long p = P;
	long s = bsp_pid();
    long alpha = p*5; // We assume p devides alpha

    bsp_size_t tagSize = sizeof(long);
    bsp_set_tagsize(&tagSize);
    bsp_sync();

    // We calculate the primes until (not including alpha) at processor 0
    if (s==0)
    {
        sequential_part_sieve(alpha, p);
    }
    bsp_sync();

    parallel_part_sieve(s,alpha,p,N);
    
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
    N = ((N+P-1)/P)*P; // We assume that p devides N (adjust in the future)
    printf("We take N = %ld \n",N);
	sieve();
	exit(EXIT_SUCCESS);
}
