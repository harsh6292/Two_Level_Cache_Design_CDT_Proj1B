#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>


struct cache
{
	unsigned int c_size;
	unsigned int c_assoc;
	unsigned int c_blocksize;
	//unsigned int c_replacePolicy;
	//unsigned int c_writePolicy;
	unsigned int c_numOfSets;

	//int level;

	unsigned int* c_tagArray;
	unsigned int* dirty_bit;
	unsigned int* valid_in_bit;
	int* LRUCounter;
	//int* count_set;

	unsigned int readCounter;
	unsigned int readMissCounter;
	unsigned int writeCounter;
	unsigned int writeMissCounter;
	unsigned int memoryAccessCounter;
	unsigned int noOfWritebacks;
	int noOfSwaps;
	

	struct cache* nextLevel;

};


int powerOfTwo(int  num);
void extractAddressParams(unsigned int addressInInt, struct cache* l1Cache, unsigned int* indexLocation, unsigned int* tagAddress);
int readFromAddress(struct cache* cache_ds, unsigned int addressInInt, struct cache* victimCache, unsigned int vc_size);
int writeToAddress(struct cache* cache_ds, unsigned int addressInInt, struct cache* victimCache, unsigned int vc_size);

void LRUForHit(struct cache* l1Cache, unsigned int indexLocation, unsigned int tagLocation);
void LRUForMiss(struct cache* l1Cache, unsigned int indexLocation, unsigned int* tagLocation);

int readFromVictimCache(struct cache* victimCache, unsigned int addressInInt, struct cache* cache_ds, unsigned int tagLocationCache, char rw);
void LRUForHitVC(struct cache* cache_ds, unsigned int indexLocation);
void LRUForMissVC(struct cache* cache_ds, unsigned int* tagLocation);


int main(int argc, char **argv)
{

	struct cache* l1Cache = (struct cache*)malloc(sizeof(struct cache));
	struct cache* l2Cache  = (struct cache*)malloc(sizeof(struct cache));
	struct cache* victimCache = (struct cache*)malloc(sizeof(struct cache)) ;
	
	FILE* trace_file;

	unsigned int countTraceEntries = 0, noOfTagEntriesL1 = 0, noOfTagEntriesL2 = 0, addressInInt = 0;
	unsigned int i=0, j=0, noOfTagEntriesVC = 0, k=0, tagKey = 0, dirtyKey = 0;
	int key = 0;
	char readAddress[100];
	char *address, *isItReadOrWrite;
	double missRateL1 = 0, accessTimeL1 = 0, missPenaltyL1 = 0, cacheHitTimeL1 = 0;
	double missRateL2 = 0, accessTimeL2 = 0, missPenaltyL2 = 0, cacheHitTimeL2 = 0;

	if(argc!=8)
	{
		printf("\nInsufficient Arguments Supplied. Returning..\n");
		return(0);
	}

	
	if( powerOfTwo( atoi(argv[1]) ) == 0 )
	{
		printf("\nBlock size is not a power of two. Returning..\n");
		return(0);
	}

	l1Cache->c_blocksize = atoi(argv[1]);
	l1Cache->c_size = atoi(argv[2]);
	l1Cache->c_assoc = atoi(argv[3]);	

	l1Cache->c_numOfSets = (l1Cache->c_size/(l1Cache->c_blocksize*l1Cache->c_assoc));
	
	if(l1Cache->c_numOfSets !=1)
	{
		if( powerOfTwo( l1Cache->c_numOfSets ) == 0 )
		{
			printf("\nNumber of sets in L1 Cache is not a power of two. Returning..\n");
			return(0);
		}
	}


	//Initialize L1 Cache
	noOfTagEntriesL1 = l1Cache->c_numOfSets*l1Cache->c_assoc;
	l1Cache->c_tagArray = (unsigned int*)malloc( (noOfTagEntriesL1*sizeof(unsigned int)) );
	l1Cache->dirty_bit = (unsigned int*)malloc( (noOfTagEntriesL1*sizeof(unsigned int)) );
	l1Cache->valid_in_bit = (unsigned int*)malloc( (noOfTagEntriesL1*sizeof(unsigned int)) );
	l1Cache->LRUCounter = (int*)malloc( (noOfTagEntriesL1*sizeof(int)) );

	memset( l1Cache->c_tagArray, 0, (sizeof(l1Cache->c_tagArray[0])*noOfTagEntriesL1) );
	memset( l1Cache->dirty_bit, 0, (sizeof(l1Cache->dirty_bit[0])*noOfTagEntriesL1) );
	memset( l1Cache->valid_in_bit, 0, (sizeof(l1Cache->valid_in_bit[0])*noOfTagEntriesL1) );
	memset( l1Cache->LRUCounter, 0, (sizeof(l1Cache->LRUCounter[0])*noOfTagEntriesL1) );


	l1Cache->readCounter = 0;
	l1Cache->writeCounter = 0;
	l1Cache->readMissCounter = 0;
	l1Cache->writeMissCounter = 0;
	l1Cache->memoryAccessCounter = 0;
	l1Cache->noOfWritebacks = 0;
	l1Cache->nextLevel = NULL;


	//Checking for L2 Cache Presence
	if( atoi(argv[5]) == 0 )
	{
		l2Cache->c_size = atoi(argv[5]);
	}
	else
	{
		l2Cache->c_blocksize = atoi(argv[1]);
		l2Cache->c_size = atoi(argv[5]);
		l2Cache->c_assoc = atoi(argv[6]);

		l1Cache->nextLevel = l2Cache;
		l2Cache->nextLevel = NULL;

		l2Cache->c_numOfSets = (l2Cache->c_size/(l2Cache->c_blocksize*l2Cache->c_assoc));
		if( powerOfTwo( l2Cache->c_numOfSets ) == 0 )
		{
			printf("\nNumber of sets in L2 Cache is not a power of two. Returning..\n");
			return(0);
		}

		noOfTagEntriesL2 = l2Cache->c_numOfSets*l2Cache->c_assoc;
		l2Cache->c_tagArray = (unsigned int*)malloc( (noOfTagEntriesL2*sizeof(unsigned int)) );
		l2Cache->dirty_bit = (unsigned int*)malloc( (noOfTagEntriesL2*sizeof(unsigned int)) );
		l2Cache->valid_in_bit = (unsigned int*)malloc( (noOfTagEntriesL2*sizeof(unsigned int)) );
		l2Cache->LRUCounter = (int*)malloc( (noOfTagEntriesL2*sizeof(int)) );


		memset( l2Cache->c_tagArray, 0, (sizeof(l2Cache->c_tagArray[0])*noOfTagEntriesL2) );
		memset( l2Cache->dirty_bit, 0, (sizeof(l2Cache->dirty_bit[0])*noOfTagEntriesL2) );
		memset( l2Cache->valid_in_bit, 0, (sizeof(l2Cache->valid_in_bit[0])*noOfTagEntriesL2) );
		memset( l2Cache->LRUCounter, 0, (sizeof(l2Cache->LRUCounter[0])*noOfTagEntriesL2) );


		l2Cache->readCounter = 0;
		l2Cache->writeCounter = 0;
		l2Cache->readMissCounter = 0;
		l2Cache->writeMissCounter = 0;
		l2Cache->memoryAccessCounter = 0;
		l2Cache->noOfWritebacks = 0;

	}



	//Checking for Victim Cache Presence
	if( atoi(argv[4]) == 0 )
	{
		victimCache->c_size = atoi(argv[4]);
	}
	else
	{
		victimCache->c_blocksize = l1Cache->c_blocksize;
		victimCache->c_size = atoi(argv[4]);	
		victimCache->nextLevel =NULL;

		victimCache->c_numOfSets = 1;
		victimCache->c_assoc = (victimCache->c_size/victimCache->c_blocksize);

		noOfTagEntriesVC = victimCache->c_numOfSets*victimCache->c_assoc;
		victimCache->c_tagArray = (unsigned int*)malloc( (noOfTagEntriesVC*sizeof(unsigned int)) );
		victimCache->dirty_bit = (unsigned int*)malloc( (noOfTagEntriesVC*sizeof(unsigned int)) );
		victimCache->valid_in_bit = (unsigned int*)malloc( (noOfTagEntriesVC*sizeof(unsigned int)) );
		victimCache->LRUCounter = (int*)malloc( (noOfTagEntriesVC*sizeof(int)) );

		memset( victimCache->c_tagArray, 0, (sizeof(victimCache->c_tagArray[0])*noOfTagEntriesVC) );
		memset( victimCache->dirty_bit, 0, (sizeof(victimCache->dirty_bit[0])*noOfTagEntriesVC) );
		memset( victimCache->valid_in_bit, 0, (sizeof(victimCache->valid_in_bit[0])*noOfTagEntriesVC) );
		memset( victimCache->LRUCounter, 0, (sizeof(victimCache->LRUCounter[0])*noOfTagEntriesVC) );


		victimCache->readCounter = 0;
		victimCache->writeCounter = 0;
		victimCache->readMissCounter = 0;
		victimCache->writeMissCounter = 0;
		victimCache->memoryAccessCounter = 0;
		victimCache->noOfWritebacks = 0;
		victimCache->noOfSwaps = 0;


	}



	if( (trace_file = fopen(argv[7], "r")) == NULL)
	{
		printf("\nNo Trace File Provided. Returning..");
		return(0);
	}


	while( fgets(readAddress, 100, trace_file)!=NULL ) 
	{
		isItReadOrWrite = strtok(readAddress, " ");
		address = strtok(NULL, "\n");
		addressInInt = strtoll(address, NULL, 16); 

		if( isItReadOrWrite[0] == 'r' || isItReadOrWrite[0] == 'R')
		{
			//printf("\n\n%d. Address %x READ", (countTraceEntries+1), addressInInt);
			readFromAddress(l1Cache, addressInInt, victimCache, victimCache->c_size);// indexLocation, tagAddress);
		}
		else if( isItReadOrWrite[0] == 'w' || isItReadOrWrite[0] == 'W')
		{
			//printf("\n\n%d. Address %x WRITE", (countTraceEntries+1), addressInInt);
			writeToAddress(l1Cache, addressInInt, victimCache, victimCache->c_size);
		}
		else
		{
			printf("\nTrace file doesn't specify R/W at line %d", (countTraceEntries+1));
			continue;
		}

		countTraceEntries++;
		
		

	}


	printf("===== Simulator configuration =====\n");
	printf("%-30s%-d\n", "BLOCKSIZE:", l1Cache->c_blocksize);
	printf("%-30s%-d\n", "L1_SIZE:", l1Cache->c_size);
	printf("%-30s%-d\n", "L1_ASSOC:", l1Cache->c_assoc);
	printf("%-30s%-d\n", "Victim_Cache_SIZE:", victimCache->c_size);
	printf("%-30s%-d\n", "L2_SIZE:", l2Cache->c_size);
	printf("%-30s%-d\n", "L2_ASSOC:", l2Cache->c_assoc);
	printf("%-30s%-s\n", "trace_file:", argv[7]);
	printf("===================================");
	printf("\n===== L1 contents =====\n");

	for( i=0; i<l1Cache->c_numOfSets; i++)
	{
		//sort L1 cache based on LRU counter
		for( j=1; j<l1Cache->c_assoc; j++)
		{
			key = l1Cache->LRUCounter[i + (j*l1Cache->c_numOfSets)];
			tagKey = l1Cache->c_tagArray[i + (j*l1Cache->c_numOfSets)];
			dirtyKey = l1Cache->dirty_bit[i + (j*l1Cache->c_numOfSets)];
			k = j-1;

			while( ((int)k>=0) && ( key < ( l1Cache->LRUCounter[i + (k*l1Cache->c_numOfSets)] ) ) )
			{
				l1Cache->LRUCounter[i + ((k+1)*l1Cache->c_numOfSets)] = l1Cache->LRUCounter[i + (k*l1Cache->c_numOfSets)];
				l1Cache->c_tagArray[i + ((k+1)*l1Cache->c_numOfSets)] = l1Cache->c_tagArray[i + (k*l1Cache->c_numOfSets)];
				l1Cache->dirty_bit[i + ((k+1)*l1Cache->c_numOfSets)] = l1Cache->dirty_bit[i + (k*l1Cache->c_numOfSets)];;

				k = k-1;
			}

			l1Cache->LRUCounter[i + ((k+1)*l1Cache->c_numOfSets)] = key;
			l1Cache->c_tagArray[i + ((k+1)*l1Cache->c_numOfSets)] = tagKey;
			l1Cache->dirty_bit[i + ((k+1)*l1Cache->c_numOfSets)] = dirtyKey;

		}

		printf("set %d: ", i);
		for( j=0; j<l1Cache->c_assoc; j++)
		{
			if( l1Cache->dirty_bit[i + (j*l1Cache->c_numOfSets)] == 1)
				printf("%-7x%-3c",l1Cache->c_tagArray[i + (j*l1Cache->c_numOfSets)], 'D');
			else
				printf("%-10x",l1Cache->c_tagArray[i + (j*l1Cache->c_numOfSets)]);
		}
		printf("\n");
	}


	if( victimCache->c_size != 0)
	{
		printf("===== Victim Cache contents =====\n");
		printf("set 0: ");
		

		//sort victim cache based on LRU counter
		for( i=1; i<victimCache->c_assoc; i++)
		{
			key = victimCache->LRUCounter[i];
			tagKey = victimCache->c_tagArray[i];
			dirtyKey = victimCache->dirty_bit[i];
			j = i-1;

			while( ((int)j>=0) && (key<victimCache->LRUCounter[j]) )
			{
				victimCache->LRUCounter[j+1] = victimCache->LRUCounter[j];
				victimCache->c_tagArray[j+1] = victimCache->c_tagArray[j];
				victimCache->dirty_bit[j+1] = victimCache->dirty_bit[j];;

				j = j-1;
			}

			victimCache->LRUCounter[j+1] = key;
			victimCache->c_tagArray[j+1] = tagKey;
			victimCache->dirty_bit[j+1] = dirtyKey;
		}




		for( i=0; i<victimCache->c_assoc; i++)
		{
			if( victimCache->dirty_bit[i] == 1)
				printf("%-8x%-3c",victimCache->c_tagArray[i], 'D');
			else
				printf("%-10x",victimCache->c_tagArray[i]);
		}
		printf("\n");
	}



	if( l2Cache->c_size != 0)
	{

		printf("===== L2 contents =====\n");

		for( i=0; i<l2Cache->c_numOfSets; i++)
		{
	
			//sort L2 cache based on LRU counter
			for( j=1; j<l2Cache->c_assoc; j++)
			{
				key = l2Cache->LRUCounter[i + (j*l2Cache->c_numOfSets)];
				tagKey = l2Cache->c_tagArray[i + (j*l2Cache->c_numOfSets)];
				dirtyKey = l2Cache->dirty_bit[i + (j*l2Cache->c_numOfSets)];
				k = j-1;

				while( ((int)k>=0) && ( key < ( l2Cache->LRUCounter[i + (k*l2Cache->c_numOfSets)] ) ) )
				{
					l2Cache->LRUCounter[i + ((k+1)*l2Cache->c_numOfSets)] = l2Cache->LRUCounter[i + (k*l2Cache->c_numOfSets)];
					l2Cache->c_tagArray[i + ((k+1)*l2Cache->c_numOfSets)] = l2Cache->c_tagArray[i + (k*l2Cache->c_numOfSets)];
					l2Cache->dirty_bit[i + ((k+1)*l2Cache->c_numOfSets)] = l2Cache->dirty_bit[i + (k*l2Cache->c_numOfSets)];;

					k = k-1;
				}

				l2Cache->LRUCounter[i + ((k+1)*l2Cache->c_numOfSets)] = key;
				l2Cache->c_tagArray[i + ((k+1)*l2Cache->c_numOfSets)] = tagKey;
				l2Cache->dirty_bit[i + ((k+1)*l2Cache->c_numOfSets)] = dirtyKey;

			}


			printf("set %d: ", i);
			for( j=0; j<l2Cache->c_assoc; j++)
			{
				if( l2Cache->dirty_bit[i + (j*l2Cache->c_numOfSets)] == 1)
					printf("%-7x%-3c",l2Cache->c_tagArray[i + (j*l2Cache->c_numOfSets)], 'D');
				else
					printf("%-10x",l2Cache->c_tagArray[i + (j*l2Cache->c_numOfSets)]);
			}
			printf("\n");
		}
	}


	//L1 calculation
	missRateL1 = ( (double)( (int)l1Cache->readMissCounter + (int)l1Cache->writeMissCounter )/(double)( (int)l1Cache->readCounter + (int)l1Cache->writeCounter ) );
	missPenaltyL1 = (double)(20 + 0.5*( ((double)l1Cache->c_blocksize/16) ) );

	l1Cache->c_size = atoi(argv[2]);
	l1Cache->c_assoc = atoi(argv[3]);

	cacheHitTimeL1 = (double)( 0.25 + 2.5*( (double)l1Cache->c_size/(512*1024) ) + 0.025*( (double)l1Cache->c_blocksize/16 ) + 0.025*( (double)l1Cache->c_assoc ) );
	accessTimeL1 = (double)( (double)cacheHitTimeL1 + ( (double)missRateL1*(double)missPenaltyL1 ) );


	printf("====== Simulation results (raw) ======\n");
	printf("%-38s%-6d\n", "a. number of L1 reads:", l1Cache->readCounter);
	printf("%-38s%-6d\n", "b. number of L1 read misses:", l1Cache->readMissCounter);
	printf("%-38s%-6d\n", "c. number of L1 writes:", l1Cache->writeCounter);
	printf("%-38s%-6d\n", "d. number of L1 write misses:", l1Cache->writeMissCounter);
	printf("%-38s%-2.4f\n", "e. L1 miss rate:", missRateL1);
	

	//Victim cache results
	printf("%-38s%-6d\n", "f. number of swaps:", victimCache->noOfSwaps);
	printf("%-38s%-6d\n", "g. number of victim cache writeback:", victimCache->noOfWritebacks);

	//L2 cache calculation
	missRateL2 = ( (double)( (int)l2Cache->readMissCounter )/(double)( (int)l2Cache->readCounter ) );
	missPenaltyL2 = (double)(20 + 0.5*( ((double)l2Cache->c_blocksize/16) ) );

	l2Cache->c_size = atoi(argv[5]);
	l2Cache->c_assoc = atoi(argv[6]);

	cacheHitTimeL2 = (double)( 2.5 + 2.5*( (double)l2Cache->c_size/(512*1024) ) + 0.025*( (double)l2Cache->c_blocksize/16 ) + 0.025*( (double)l2Cache->c_assoc ) );
	
	printf("%-38s%-6d\n", "h. number of L2 reads:", l2Cache->readCounter);
	printf("%-38s%-6d\n", "i. number of L2 read misses:", l2Cache->readMissCounter);
	printf("%-38s%-6d\n", "j. number of L2 writes:", l2Cache->writeCounter);
	printf("%-38s%-6d\n", "k. number of L2 write misses:", l2Cache->writeMissCounter);
	if( l2Cache->c_size == 0 )
		printf("%-38s%-6c\n", "l. L2 miss rate:", '0');
	else
		printf("%-38s%-2.4f\n", "l. L2 miss rate:", missRateL2);
	printf("%-38s%-6d\n", "m. number of L2 writebacks:", l2Cache->noOfWritebacks);

	if(l2Cache->c_size == 0)
		printf("%-38s%-6d\n", "n. total memory traffic:", l1Cache->memoryAccessCounter);
	else
		printf("%-38s%-6d\n", "n. total memory traffic:", l2Cache->memoryAccessCounter);


	if(l2Cache->c_size != 0)
	{
		accessTimeL2 = ( cacheHitTimeL1 + ( (double)missRateL1*( cacheHitTimeL2 + ( (double)missRateL2*(double)missPenaltyL2 ) ) ) );
	}

	printf("==== Simulation results (performance) ====\n");
	
	if(l2Cache->c_size != 0)
	{
		printf("1. average access time:%15.4f ns\n", accessTimeL2);  //========================================AAt calculate
	}
	else
		printf("1. average access time:%15.4f ns\n", accessTimeL1);

	return(0);
}



int powerOfTwo(int num)
{
	if(num!=1)
	{
		while( (num%2 == 0) && num>1)
			num = num/2;

		if( num == 1)
			return(1);
	}
	
	return(0);
}



void extractAddressParams(unsigned int addressInInt, struct cache* cache_ds, unsigned int* indexLocation, unsigned int* tagAddress)
{
	int noOfBlockBits = 0, noOfIndexBits = 0, tempIndexNo = 0, i=0;
	
	noOfBlockBits = log2(cache_ds->c_blocksize);
	noOfIndexBits = log2(cache_ds->c_numOfSets);

	*indexLocation = addressInInt>>noOfBlockBits;

	for( i=0; i<noOfIndexBits; i++)
	{
		tempIndexNo = ( 1 | tempIndexNo<<1 );
	}

	*indexLocation = ( *indexLocation & tempIndexNo );
	*tagAddress = addressInInt>>(noOfBlockBits + noOfIndexBits);
}





//Recursive solution

int readFromAddress(struct cache* cache_ds, unsigned int addressInInt, struct cache* victimCache, unsigned int vc_size)
{
	int i=0, foundInvalidEntry = 0, noOfBlockBits = 0, noOfIndexBits = 0;
	unsigned int tagLocation = 0, indexLocation = 0, tagAddress = 0, tempAdd=0, temptagLocation = 0;

	noOfBlockBits = log2(cache_ds->c_blocksize);
	noOfIndexBits = log2(cache_ds->c_numOfSets);


	cache_ds->readCounter +=1;
	extractAddressParams(addressInInt, cache_ds, &indexLocation, &tagAddress);


	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		if( cache_ds->c_tagArray[indexLocation + (i*cache_ds->c_numOfSets)] == tagAddress )	//Checking Tag Entries
		{
		
			if( cache_ds->valid_in_bit[indexLocation + (i*cache_ds->c_numOfSets)] != 0 )
			{
				LRUForHit(cache_ds, indexLocation, ( indexLocation + (i*cache_ds->c_numOfSets) ) );
								
				return(0);
			}
		}
	}


	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		if( cache_ds->valid_in_bit[indexLocation + (i*cache_ds->c_numOfSets)] == 0 )
		{
			cache_ds->valid_in_bit[indexLocation + (i*cache_ds->c_numOfSets)] = 1;
			tagLocation =  indexLocation + (i*cache_ds->c_numOfSets);
			foundInvalidEntry = 1;
			break;
		}
	}

	if( foundInvalidEntry == 1 )
	{
		cache_ds->readMissCounter += 1;

		if( cache_ds->nextLevel != NULL)
		{
			readFromAddress(cache_ds->nextLevel, addressInInt, victimCache, 0);
		}
		
		cache_ds->memoryAccessCounter += 1;		//increase the memory traffic counter
		cache_ds->c_tagArray[tagLocation] = tagAddress;
		cache_ds->dirty_bit[tagLocation] = 0;
		LRUForMiss(cache_ds, indexLocation, &temptagLocation);
		cache_ds->LRUCounter[tagLocation] = 0;
		return(0);
	}



	if( vc_size != 0 )
	{

		LRUForMiss(cache_ds, indexLocation, &tagLocation);
		cache_ds->LRUCounter[tagLocation] = 0;

		readFromVictimCache(victimCache, addressInInt, cache_ds, tagLocation, 'r');

		return(0);
	}

	cache_ds->readMissCounter += 1;

	cache_ds->memoryAccessCounter += 1;		//increase the memory traffic counter
	LRUForMiss(cache_ds, indexLocation, &tagLocation);
	cache_ds->LRUCounter[tagLocation] = 0;
	
	
	if( (int)cache_ds->dirty_bit[tagLocation] == 1 )
	{
		
		cache_ds->memoryAccessCounter += 1;
		if( cache_ds->nextLevel != NULL)
		{
			tempAdd = cache_ds->c_tagArray[tagLocation];
			tempAdd = ( ( (tempAdd<<noOfIndexBits)|(tagLocation%cache_ds->c_numOfSets) )<<noOfBlockBits );
			writeToAddress(cache_ds->nextLevel, tempAdd, victimCache, 0);
		}
		cache_ds->noOfWritebacks += 1;
		cache_ds->dirty_bit[tagLocation] = 0;
	}


	if( cache_ds->nextLevel != NULL)
	{
		readFromAddress(cache_ds->nextLevel, addressInInt, victimCache, 0);
	}

	cache_ds->c_tagArray[tagLocation] = tagAddress;

	return(0);
}



 
int readFromVictimCache(struct cache* victimCache, unsigned int addressInInt, struct cache* cache_ds, unsigned int tagLocationCache, char rw)
{
	int noOfBlockBits = 0, noOfIndexBitsCache = 0, i=0;
	unsigned int tagAddress = 0, indexLocOfCache = 0, vcTagLocation = 0;
	unsigned int tagAddressOfCache, tempTagAddress, constructedAddress = 0;
	
	noOfBlockBits = log2(victimCache->c_blocksize);
	tagAddress = addressInInt>>noOfBlockBits;

	noOfIndexBitsCache = log2(cache_ds->c_numOfSets);

	extractAddressParams(addressInInt, cache_ds, &indexLocOfCache, &tagAddressOfCache);

	for( i=0; i< (int)victimCache->c_assoc; i++)
	{
		if( victimCache->c_tagArray[i] == tagAddress )	//Checking Tag Entries
		{
			if( victimCache->valid_in_bit[i] != 0 )
			{
				
				tempTagAddress = cache_ds->c_tagArray[tagLocationCache];
				cache_ds->c_tagArray[tagLocationCache] = tagAddressOfCache;
				victimCache->c_tagArray[i] = ( (tempTagAddress<<noOfIndexBitsCache)|(indexLocOfCache%cache_ds->c_numOfSets) );

				tempTagAddress =  cache_ds->dirty_bit[tagLocationCache];
				cache_ds->dirty_bit[tagLocationCache] = victimCache->dirty_bit[i];
				victimCache->dirty_bit[i] = tempTagAddress;
				LRUForHitVC(victimCache, i);
				victimCache->noOfSwaps += 1;
				if( rw == 'w' )
					cache_ds->dirty_bit[tagLocationCache] = 1;
				return(0);
			}
		}
	}

	//It's a Cache Miss
	if( rw == 'r' )
		cache_ds->readMissCounter += 1;
	else
		cache_ds->writeMissCounter += 1;

	//printf("\nMISS VICTIM CACHE");
	
	cache_ds->memoryAccessCounter += 1;
	

	LRUForMissVC(victimCache, &vcTagLocation);
	if( (int)victimCache->dirty_bit[vcTagLocation] == 1)
	{
		//printf("\nVICTIM CACHE WRITE BACK");
		
		if( cache_ds->nextLevel != NULL )
		{
			constructedAddress = victimCache->c_tagArray[vcTagLocation]<<noOfBlockBits;
			writeToAddress(cache_ds->nextLevel, constructedAddress, victimCache, 0);

		}	
		victimCache->noOfWritebacks += 1;
		cache_ds->memoryAccessCounter += 1;	
	}


	if( cache_ds->nextLevel != NULL )
	{
		readFromAddress(cache_ds->nextLevel, addressInInt, victimCache, 0);
	}

	tempTagAddress = cache_ds->c_tagArray[tagLocationCache];
	victimCache->c_tagArray[vcTagLocation] = ( (tempTagAddress<<noOfIndexBitsCache)|(indexLocOfCache%cache_ds->c_numOfSets) );

	tempTagAddress =  cache_ds->dirty_bit[tagLocationCache];
	victimCache->dirty_bit[vcTagLocation] = tempTagAddress;

	victimCache->valid_in_bit[vcTagLocation] = 1;
	cache_ds->c_tagArray[tagLocationCache] = tagAddressOfCache;

	if( rw == 'r' )
		cache_ds->dirty_bit[tagLocationCache] = 0;
	else
		cache_ds->dirty_bit[tagLocationCache] = 1;

	return(0);
}





//Recursive solution

int writeToAddress(struct cache* cache_ds, unsigned int addressInInt, struct cache* victimCache, unsigned int vc_size)
{

	int i=0, foundInvalidEntry = 0;
	unsigned int tagLocation = 0, indexLocation = 0, tagAddress = 0, tempAdd = 0, temptagLocation = 0;

	int noOfBlockBits = 0, noOfIndexBits = 0;
	
	noOfBlockBits = log2(cache_ds->c_blocksize);
	noOfIndexBits = log2(cache_ds->c_numOfSets);

	cache_ds->writeCounter += 1;
	extractAddressParams(addressInInt, cache_ds, &indexLocation, &tagAddress);


	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		if( cache_ds->c_tagArray[indexLocation + (i*cache_ds->c_numOfSets)] == tagAddress )	//Checking Tag Entries
		{
			if( cache_ds->valid_in_bit[indexLocation + (i*cache_ds->c_numOfSets)] != 0 )
			{
				
				cache_ds->dirty_bit[indexLocation + (i*cache_ds->c_numOfSets)] = 1;			
				LRUForHit(cache_ds, indexLocation, ( indexLocation + (i*cache_ds->c_numOfSets) ) );
			
				return(0);
			}
		}
	}


	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		if( cache_ds->valid_in_bit[indexLocation + (i*cache_ds->c_numOfSets)] == 0 )
		{
			cache_ds->valid_in_bit[indexLocation + (i*cache_ds->c_numOfSets)] = 1;
			tagLocation =  indexLocation + (i*cache_ds->c_numOfSets);
			foundInvalidEntry = 1;
			break;
		}
	}


	if( foundInvalidEntry == 1 )
	{
		cache_ds->writeMissCounter += 1;

		if( cache_ds->nextLevel != NULL)
		{
			readFromAddress(cache_ds->nextLevel, addressInInt, victimCache, 0);
		}
		cache_ds->memoryAccessCounter += 1;		//increase the memory traffic counter
		cache_ds->c_tagArray[tagLocation] = tagAddress;
		cache_ds->dirty_bit[tagLocation] = 1;

		LRUForMiss(cache_ds, indexLocation, &temptagLocation);
		cache_ds->LRUCounter[tagLocation] = 0;
		return(0);
	}


	if( vc_size != 0 )
	{
		LRUForMiss(cache_ds, indexLocation, &tagLocation);
		cache_ds->LRUCounter[tagLocation] = 0;

		readFromVictimCache(victimCache, addressInInt, cache_ds, tagLocation, 'w');

		return(0);
	}

	cache_ds->writeMissCounter += 1;

	cache_ds->memoryAccessCounter += 1;		//increase the memory traffic counter
	LRUForMiss(cache_ds, indexLocation, &tagLocation);
	cache_ds->LRUCounter[tagLocation] = 0;

	if( (int)cache_ds->dirty_bit[tagLocation] == 1 )
	{
		cache_ds->memoryAccessCounter += 1;
		if( cache_ds->nextLevel != NULL)
		{
			tempAdd = cache_ds->c_tagArray[tagLocation];
			tempAdd = ( ( (tempAdd<<noOfIndexBits)|(tagLocation%cache_ds->c_numOfSets) )<<noOfBlockBits );
			writeToAddress(cache_ds->nextLevel, tempAdd, victimCache, 0);
		}
		cache_ds->noOfWritebacks += 1;
		
	}


	if( cache_ds->nextLevel != NULL)
	{
		readFromAddress(cache_ds->nextLevel, addressInInt, victimCache, 0);
	}

	cache_ds->dirty_bit[tagLocation] = 1;
	cache_ds->c_tagArray[tagLocation] = tagAddress;


	return(0);
}




void LRUForHit(struct cache* l1Cache, unsigned int indexLocation, unsigned int tagLocation)
{
	int i = 0;

	for( i=0; i< (int)l1Cache->c_assoc; i++)
	{
		if( (int)l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)] < (int)l1Cache->LRUCounter[tagLocation] )	
		{
			l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)] = ((int)l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)]) + 1;
		}
	}
	
	l1Cache->LRUCounter[tagLocation] = 0;
}



void LRUForMiss(struct cache* l1Cache, unsigned int indexLocation, unsigned int* tagLocation)
{
	unsigned int i = 0;
	int max = -1;
	*tagLocation = 0;
	//printf("\nL1 UPDATE LRU");
	for( i=0; i<l1Cache->c_assoc; i++)
	{
		if( (int)l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)] > (int)max )
		{
			max = l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)];
			*tagLocation = ( indexLocation + (i*l1Cache->c_numOfSets) );
		}
	}


	for( i=0; i<l1Cache->c_assoc; i++)
	{
		l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)] = ((int)l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)]) + 1;
	}
}




void LRUForHitVC(struct cache* cache_ds, unsigned int indexLocation)
{
	int i = 0;

	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		if( (int)cache_ds->LRUCounter[i] < (int)cache_ds->LRUCounter[indexLocation] )	
		{
			cache_ds->LRUCounter[i] = ((int)cache_ds->LRUCounter[i] + 1);
		}
	}
	
	cache_ds->LRUCounter[indexLocation] = 0;
}



void LRUForMissVC(struct cache* cache_ds, unsigned int* tagLocation)
{
	unsigned int i = 0;
	int max = -1;
	*tagLocation = 0;

	for( i=0; i<cache_ds->c_assoc; i++)
	{
		if( (int)cache_ds->LRUCounter[i] > (int)max )
		{
			max = cache_ds->LRUCounter[i];
			*tagLocation = i;
		}
	}


	for( i=0; i<cache_ds->c_assoc; i++)
	{
		cache_ds->LRUCounter[i] = ((int)cache_ds->LRUCounter[i] + 1);
	}

	cache_ds->LRUCounter[*tagLocation] = 0;

}

