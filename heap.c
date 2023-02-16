/*
 * This is a C implementation of malloc( ) and free( ), based on the buddy
 * memory allocation algorithm. 
 */
#include <stdio.h> // printf

/*
 * The following global variables are used to simulate memory allocation
 * Cortex-M's SRAM space.
 */
// Heap
char array[0x8000];            // simulate SRAM: 0x2000.0000 - 0x2000.7FFF
int heap_top   = 0x20001000;   // the top of heap space
int heap_bot   = 0x20004FE0;   // the address of the last 32B in heap
int max_size   = 0x00004000;   // maximum allocation: 16KB = 2^14
int min_size   = 0x00000020;   // minimum allocation: 32B = 2^5

// Memory Control Block: 2^10B = 1KB space
int mcb_top    = 0x20006800;   // the top of MCB
int mcb_bot    = 0x20006BFE;   // the address of the last MCB entry
int mcb_ent_sz = 0x00000002;   // 2B per MCB entry
int mcb_total  = 512;          // # MCB entries: 2^9 = 512 entries

/*
 * Convert a Cortex SRAM address to the corresponding array index.
 * @param  sram_addr address of Cortex-M's SRAM space starting at 0x20000000.
 * @return array index.
 */
int m2a( int sram_addr ) {
	// TODO: part 1 - add comment to each following line of code
	int index = sram_addr - 0x20000000; // subtract sram address with 0x200000 and then store it as the index we want 
	return index; // index is returned 
}

/*
 * Reverse an array index back to the corresponding Cortex SRAM address.
 * @param  array index.
 * @return the corresponding Cortex-M's SRAM address in an integer.
 */ 
int a2m( int array_index ) {
	// TODO: part 1 - add comment to each following line of code
	return array_index + 0x20000000; //  add sram_address to array index to get desired address 
}

/*
 * In case if you want to print out, all array elements that correspond
 * to MCB: 0x2006800 - 0x20006C00.
 */
void printArray( ) {
	printf( "memroy ............................\n" );
	// TODO: part 1 - add comment to each following line of code
	for ( int i = 0; i < 0x8000; i+=4 ) // loop through array until index is less than 0x800. increment by 4 each time. 
		if ( a2m( i ) >= 0x20006800 ) 	// checks if the sram_address of the index i in a2m is greater than or equal to 0x20006800
			printf( "%x = %x(%d)\n", a2m( i ), *(int *)&array[i], *(int *)&array[i] ); // prints out the sram_address + index, array[i]'s hex value, array[i]'s value in Decimal. 
}

/*
 * _ralloc is _kalloc's helper function that is recursively called to
 * allocate a requested space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  size  the size of a requested memory space
 * @param  left_mcb_addr  the address of the left boundary of MCB entries to examine
 * @param  right_mcb_addr the address of the right boundary of MCB entries to examine
 * @return the address of Cortex-M's SRAM space. While the computation is
 *         made in integers, cast it to (void *). The gcc compiler gives
 *         a warning sign:
                cast to 'void *' from smaller integer type 'int'
 *         Simply ignore it.
 */
void *_ralloc( int size, int left_mcb_addr, int right_mcb_addr ) {
	// initial parameter computation
	// TODO: part 1 - add comment to each following line of code
	int entire_mcb_addr_space = right_mcb_addr - left_mcb_addr  + mcb_ent_sz; // entire_mcb_addr_space is initialized by subtracting right_mcb_addr and left_mcb_addr and fianlly adds the entry size. 
	int half_mcb_addr_space = entire_mcb_addr_space / 2;	// half_mcb_addr_space is initialized by dividing entire_mcb_addr_space by the value of 2. 
	int midpoint_mcb_addr = left_mcb_addr + half_mcb_addr_space; // midpoint_mcb_addr is initalized by adding up left_mcb_addr + half_mcb_addr_space. 
	int heap_addr = 0;	// heap address is initialized to 0. 
	int act_entire_heap_size = entire_mcb_addr_space * 16; // act_entire_heap_size is initialized by multiplying by 16 entire_mcb_addr_space. 
	int act_half_heap_size = half_mcb_addr_space * 16; // act_half_heap_size is initialized by multiply half_mcb_addr_space by 16. 

	// base case
	// TODO: part 1 - add comment to each following line of code
	if ( size <= act_half_heap_size ) { // compares the size of requested memory to "half of heap size" and checks if size is less than or greater than act_half_heap_size
		void* heap_addr = _ralloc( size, left_mcb_addr, midpoint_mcb_addr - mcb_ent_sz );	// "ralloc" is called recursivly and head_address is initialized to outcome of the call. "right" is set to midpoint and subtracted by mcb_entry_size. 
		if ( heap_addr == 0 ) { // checks if the heap_addr(heap adress) is equal to 0. 
			return _ralloc( size, midpoint_mcb_addr, right_mcb_addr );	// "ralloc" is recursivly called. left is set to midpoint in process, and the result is then returned after call. 
		}
		if ( ( array[ m2a( midpoint_mcb_addr ) ] & 0x01 ) == 0 )	// first retrieve the index of midpoint_mcb_addr and then check if its LSB is equal to '0'. 
			*(short *)&array[ m2a( midpoint_mcb_addr ) ] = act_half_heap_size;	//  the value of act_half_heap_size is then stored into the index of midpoint_mcb_addr 
		return heap_addr; // heap_addr(heap address) is returned 
	}
	else {	
		if ( ( array[ m2a( left_mcb_addr ) ] & 0x01 ) != 0 ) {	// retrieve the value of left_mcb_addr and check if its value's LSB IS NOT EQUAL TO 0. 
			return 0;	// if above statement is true, this returns 0
		}
		else {
			if ( *(short *)&array[ m2a( left_mcb_addr ) ] < act_entire_heap_size )	//	fetch the index of left_mcb_addr and then CHECK if it's value is less than act_entire_heap_size
				return 0;	//	IF SO, returns 0. 
			*(short *)&array[ m2a( left_mcb_addr ) ] = act_entire_heap_size | 0x01;	//	OR bitwise operation. retrieve index of the array of left_mcb_addr, then update its value to outcome of act_entire_heap_size.  **ADD COMMENTS**
			return (void *)( heap_top + ( left_mcb_addr - mcb_top ) * 16 );	// calculate value of subtracting left_mcb_addr with mcb_top and then adding heap_top, then finally multiply by 16...and then returned that value. 
		}
	}
	return 0;	// returns a 0
} 

/*
 * _rfree is _kfree's helper function that is recursively called to
 * deallocate a space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  mcb_addr that corresponds to a SRAM space to deallocate
 * @return the same as the mcb_addr argument in success, otherwise 0.
 */
int _rfree( int mcb_addr ) {
	// TODO: part 1 - add comment to each following line of code
	short mcb_contents = *(short *)&array[ m2a( mcb_addr ) ];	// mbc_contents is initialized as a short and value from the array is stored there. 
	int mcb_index = mcb_addr - mcb_top;	//	mcb_index is initialized and result of mcb_addr minus mcb_top is stored there. 
	int mcb_disp = ( mcb_contents /= 16 ); // mcb_disp and initialized by the result of mcb_content divided by 16 is stored there. results of mcb_contents is ALSO divided by 16 and stored in the process. 
	int my_size = ( mcb_contents *= 16 );	// 	my_size is initialized by the result of mcb_content is multiplied by 16. results of mcb_contents is ALSO multiplied by 16. 

	// mcb_addr's used bit was cleared
	*(short *)&array[ m2a( mcb_addr ) ] = mcb_contents; // mcb_contents updates the mcb_addr's index value. 
	
	// TODO: part 1 - add comment to each following line of code
	if ( ( mcb_index / mcb_disp ) % 2 == 0 ) {  //	check if the result of mcb_index divided by mcb_disp, modulo by 2, is equal to 0.
		if ( mcb_addr + mcb_disp >= mcb_bot ) // checks if sum of mcb_addr and mcb_disp is greater-than or equal to mcb_bot.
			return 0; // my buddy is beyond mcb_bot!
		else {
			short mcb_buddy = *(short *)&array[ m2a( mcb_addr + mcb_disp ) ];   // mcb_buddy is initialized as a short and set to value of array's index after it gets the index of the sum of mcb_addr and mcb_disp(mcb_addr + mcb_disp)
			if ( ( mcb_buddy & 0x0001 ) == 0 ) {	// checks if LSB of mcb_buddy is equal to the value of 0. 
				mcb_buddy = ( mcb_buddy / 32 ) * 32;	// mcb_buddy divided by 32 and then multiplied by 32 in order for it to round
				if ( mcb_buddy == my_size ) {	// checks if mcb_buddy is equal to my_size 
					*(short *)&array[ m2a( mcb_addr + mcb_disp ) ] = 0;	// retrive the array index by adding mcb_addr and mcb_disp, then update the value to 0. 
					my_size *= 2;	// multiply my_size by 2 and update my_size 
					*(short *)&array[ m2a( mcb_addr ) ] = my_size;	// track the index of mcb_addr, then update the value at the index of the array to my_size
					return _rfree( mcb_addr );	// returns the result of the recursive call "_rfree", passing in parameter of mcb_addr
				}
			}
		}
	}
	else {  
		if ( mcb_addr - mcb_disp < mcb_top ) //	checks if the difference of mcb_addr and mcb_dsip is LESS THAN mcb_top 
			return 0; // my buddy is below mcb_top!
		else {
			short mcb_buddy = *(short *)&array[ m2a( mcb_addr - mcb_disp ) ];	//	retrive the index of the difference of mcb_addr and mcb_disp, then retrive the number at that specific index, and update the value of mcb_buddy. 
			if ( ( mcb_buddy & 0x0001 ) == 0 ) {	//	checks if the LSB of mcb_buddy is equal to 0. 
				mcb_buddy = ( mcb_buddy / 32 ) * 32; // dividing mcb_buddy by 32, and then multiply the result by 32 in order to round. *CHECK THIS OVER*
				if ( mcb_buddy == my_size ) { //	checks if the mcb_buddy is EQUAL to my_size.
					*(short *)&array[ m2a( mcb_addr ) ] = 0;	//	retrieve the array index of mcb_addr and set its value equal to 0. 
					my_size *= 2;	//	multiply my_size by 2 and update my_size. 
					*(short *)&array[ m2a( mcb_addr - mcb_disp ) ] = my_size;	//	retreive the index of the difference of mcb_addr and mcb_disp, then set the array index's value to my_size. 
					return _rfree( mcb_addr - mcb_disp );	//	return the recursive call of _rfree as a result, passing in the parameter of the DIFFERENCE of mcb_addr and mcb_disp(mcb_addr - mcb_disp)
				}
			}
		}
	}
  
  return mcb_addr; // finally return mcb_addr
}

/*
 * Initializes MCB entries. In step 2's assembly coding, this routine must
 * be called from Reset_Handler in startup_TM4C129.s before you invoke
 * driver.c's main( ).
 */
void _kinit( ) {
	// TODO: part 1 - add comment to each following line of code
	for ( int i = 0x20001000; i < 0x20005000; i++ )	//	start index at 0x20001000 and loop until 0x20005000, increment normally(by 1)
		array[ m2a( i ) ] = 0;	//	sets array[m2a(i)] to the value of 0. 

		*(short *)&array[ m2a( mcb_top ) ] = max_size;	//	use max_size to update the value in the array

	for ( int i = 0x20006804; i < 0x20006C00; i += 2 ) {	//	start index at 0x20006804, loop until 0x20006C00. increment by 2 each time. 
		array[ m2a( i ) ] = 0;	//	use 0 to update array[m2a(i)] to zero. 
		array[ m2a( i + 1) ] = 0;	//	use 0 to update array[m2a(i)+1] to 0. 
	}
}

/*
 * Step 2 should call _kalloc from SVC_Handler.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_kalloc( int size ) {	// method _kalloc takes in the parameter of int size. 
  return _ralloc( size, mcb_top, mcb_bot );	// recursive call of _ralloc is returned, with the values of size, mcb_top, and mcb_bot, passed in as paramters. 
}

/*
 * Step 2 should call _kfree from SVC_Handler.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_kfree( void *ptr ) {
	// TODO: part 1 - add comment to each following line of code
	int addr = (int )ptr;	//	pointer of an int is casted, then updates the value at int addr(set to addr).

	if ( addr < heap_top || addr > heap_bot )	//	checks if the address is less than heap_top OR IF the address is greater than heap_bot. 
		return NULL;	// is so, returns null. 
	int mcb_addr =  mcb_top + ( addr - heap_top ) / 16;	//	mcb_addr is intialized to mcb_top, which is added by the difference of addr and heap_top(addr-heap_top), then finally divided by 16. 

	if ( _rfree( mcb_addr ) == 0 )	//	check if _rfree(mcb_addr) is equal to 0. 
		return NULL;	// if so, return NULL.
	else
		return ptr; // otherwise, return the pointer. 
}

/*
 * _malloc should be implemented in stdlib.s in step 2.
 * _kalloc must be invoked through SVC in step 2.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_malloc( int size ) {	// method _malloc takes in the size as a parameter
	static int init = 0;	//	0 is initalized to a static variable of int init
	if ( init == 0 ) {	//	checks if init is equal to 0. 
		init = 1;	//	init is set to 1. 1 updates the value of init.
		_kinit( );	//	call helper method _kinit. 
	}
	return _kalloc( size );	// method recursive call of _kalloc is returned, taking in as paramter the variable int size. 
}

/*
 * _free should be implemented in stdlib.s in step 2.
 * _kfree must be invoked through SVC in step 2.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_free( void *ptr ) {	//	method of _free takes in a void pointer
	return _kfree( ptr );	//	helper method call of _kree is called and Returned, taking in the value of a pointer as a paramter. 
}
