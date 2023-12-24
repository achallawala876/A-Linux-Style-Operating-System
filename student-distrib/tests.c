#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "file_system.h"
#include "RTC.h"
#include "keyboard.h"
// #include <stdint.h>

// #include "../syscalls/ece391support.h"
// #include "MP3/syscalls/ece391syscall.h"


// #define BUFSIZE 1024

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here
/*
* Tests for exceptions
*/
int idt_division_tests(){
	TEST_HEADER;

	int i = 1;
	int j = 2/(i-1);

	return j;
}

int idt_overflow_test() {
	TEST_HEADER;

	asm volatile (
		"movl $0x7FFFFFFF, %eax;"
		"addl $1, %eax;"
		"into;"
	);

	return 0;
}

int idt_bound_range_exceeded_test() {
    int lower_bound = -5;
    int upper_bound = 5;
    int value_to_check = -10;

    asm volatile (
        "movl %2, %%eax;"
        "movl %0, %%ebx;"
        "bound %%eax, (%%ebx);"
        :
        : "r" (&lower_bound), "r" (&upper_bound), "r" (value_to_check)
        : "eax", "ebx"
    );

	return 0;
}

int idt_invalid_opcode_test() {
    asm volatile (
        ".byte 0x0F, 0xFF;"  // Invalid opcode
    );
	return 0;
}

int page_fault_test() {
    int* ptr = NULL;  // Null pointer
    *ptr = 42;        // Dereference null pointer, causing a page fault

	return 0;
}

int page_invalid_addr(){
	int i;
	char* ptr;
	int val;
	for(i = 0x400000; i < 0x800000; i++){
		ptr = (char *)i;
		val = *ptr;
	}
	return PASS;
}

int page_invalid_access_greater_than_max(){
	int i = 0x810000;
	char* ptr;
	int val;
	ptr = (char *)i;
	val = *ptr;
	return FAIL;
}

int page_invalid_access_less_than_min(){
	int i = 0x390000;
	char* ptr;
	int val;
	ptr = (char *)i;
	val = *ptr;
	return FAIL;
}

int video_mem_addr(){
	int i;
	char* ptr;
	int val;
	for(i = 0xb8000; i < 0xb9000; i++){
		ptr = (char *)i;
		val = *ptr;
	}
	return PASS;
}


/* Checkpoint 2 tests */

//////////////////////////////////////////////////////////////////////////////TEST CASES FOR FILES BEGIN////////////////////////////////////////////////////////////////

// FUNCTIONS TESTS: file_open, read_dentry_by_name, read_dentry_by_index
// PARAMETERS: a file name of your choice
// EDGE CASES TESTED: concatenating filename greater than 32 chars, checking for uppercase, blank strings
// int test_open_file(const uint8_t *file_to_test){
// 	dentry_t file_we_want;
// 	int i, checker;

// 	checker = file_open(file_to_test, &file_we_want);

// 	if (checker == 0){
// 		for (i = 0; i < strlen((int8_t*)file_we_want.filename); i++){
// 			printf("%c", file_we_want.filename[i]);
// 		}
// 		printf("\n");
// 		file_close();
// 		return PASS;
// 	}

// 	printf("File not found!\n");
// 	return FAIL;
// }

// // TESTS: file_open, file_read, read_data
// // PARAMETERS: want to be able to set file name, byte count and possibly buffer size?
// // EDGE CASES TESTED: reading past the end of the file data
// int test_read_file_once(){
// 	dentry_t dentry_1;
// 	int i;
// 	// uint32_t inode_number;
// 	int file_valid;

// 	uint8_t buf[5000];
// 	uint32_t byte_count = 5000; 
// 	uint8_t* file_to_test = (uint8_t*)"frame0.txt";

// 	file_valid = file_open(file_to_test, &dentry_1);

// 	if (file_valid == -1)
// 		return FAIL;

// 	// using opened file dentry henceforth

// 	file_valid = file_read(dentry_1.filename, byte_count, buf);
// 	printf("bytes read: %d\n", file_valid);

// 	if (file_valid == -1){
// 		return FAIL;
// 	}

// 	for(i = 0; i < file_valid; i++) {
// 		printf("%c", buf[i]);
// 	}
// 	printf("\n");
// 	file_close();
// 	return PASS;
// }

// int test_read_file_beyond(){
// 	dentry_t dentry_1;
// 	int i;
// 	// uint32_t inode_number;
// 	int file_valid;

// 	uint8_t buf[200];
// 	uint8_t buf1[100];

// 	uint32_t byte_count = 187; 
// 	uint8_t* file_to_test = (uint8_t*)"frame0.txt";

// 	file_valid = file_open(file_to_test, &dentry_1);

// 	if (file_valid == -1)
// 		return FAIL;

// 	// using opened file dentry henceforth

// 	// FIRST READ

// 	file_valid = file_read(dentry_1.filename, byte_count, buf);

// 	if (file_valid == -1){
// 		return FAIL;
// 	}

// 	printf("Num of bytes read: %d\n", file_valid);	

// 	for(i = 0; i < file_valid; i++) {
// 		printf("%c", buf[i]);
// 	}

// 	// SECOND READ

// 	file_valid = file_read(dentry_1.filename, 100, buf1);

// 	if (file_valid == -1){
// 		return FAIL;
// 	}

// 	printf("Num of bytes read: %d\n", file_valid);

// 	for(i = 0; i < file_valid; i++) {
// 		printf("%c", buf1[i]);
// 	}

// 	return PASS;

// }


// // TESTS: file_open, file_close, file_read, read_data
// // PARAMETERS: want to be able to set file name, byte count and possibly buffer size?
// // EDGE CASES TESTED: reading past the end of the file data (checking if written bytes are 0)
// int test_read_file_multiple_times_with_close_and_reopen(){
// 	dentry_t dentry_1;
// 	int i;
// 	// uint32_t inode_number;
// 	int file_valid;

// 	uint8_t bada_buf[200];
// 	uint8_t buf[100];
// 	uint8_t buf1[100];
// 	uint8_t buf2[100];

// 	uint32_t byte_count = 50; 
// 	uint8_t* file_to_test = (uint8_t*)"frame0.txt";

// 	file_valid = file_open(file_to_test, &dentry_1);
// 	printf("TYPE: %d\n", dentry_1.filetype);

// 	if (file_valid == -1)
// 		return FAIL;

// 	// using opened file dentry henceforth

// 	// FIRST READ

// 	file_valid = file_read(dentry_1.filename, byte_count, buf);

// 	if (file_valid == -1){
// 		return FAIL;
// 	}

// 	for(i = 0; i < file_valid; i++) {
// 		printf("%c", buf[i]);
// 	}
// 	// printf("num_bytes written to buf: %d\n", file_valid);
	
// 	// SECOND READ

// 	file_valid = file_read(dentry_1.filename, byte_count, buf1);

// 	if (file_valid == -1){
// 		return FAIL;
// 	}

// 	for(i = 0; i < file_valid; i++) {
// 		printf("%c", buf1[i]);
// 	}
// 	// printf("num_bytes written to buf1: %d\n", file_valid);

// 	// THIRD READ

// 	file_valid = file_read(dentry_1.filename, 87, buf2);

// 	if (file_valid == -1){
// 		return FAIL;
// 	}

// 	for(i = 0; i < file_valid; i++) {
// 		printf("%c", buf2[i]);
// 	}

// 	// printf("num_bytes written to buf2: %d\n", file_valid);

// 	file_close(); // close the file and reset the offset

// 	// ////////////////// OPEN FILE AND READ ALL 187 BYTES NOW

// 	file_valid = file_open(file_to_test, &dentry_1);

// 	if (file_valid == -1)
// 		return FAIL;

// 	file_valid = file_read(dentry_1.filename, 187, bada_buf);

// 	if (file_valid == -1){
// 		return FAIL;
// 	}

// 	// printf("num_bytes written to bada buf: %d\n", file_valid);

// 	for(i = 0; i < file_valid; i++) {
// 		printf("%c", bada_buf[i]);
// 	}
// 	printf("\n");

// 	return PASS;
// }

// // TESTS: dir_read 
// // PARAMETERS:
// // EDGE CASES TESTED: multiple directory reads and internal offset logic
// int test_read_directory_as_ls(){
// 	int i;
// 	char str_buf [2048]; //change to fix warning: uint_8 str_buf

// 	for(i = 0; i < 17; i++){
// 		dir_read(0, (uint8_t*)str_buf, 0); 
// 	}
	
// 	for (i = 0; i < strlen(str_buf); i++){
// 		printf("%c", str_buf[i]);
// 	}

// 	return PASS;
// }

// // TESTS: dir_read 
// // PARAMETERS:
// // EDGE CASES TESTED:
// int test_read_single_directory(int end){
// 	int i;
// 	char str_buf [2048]; //change to fix warning: uint_8 str_buf
	
// 	for(i = 0; i < end; i++){
// 		dir_read(0, (uint8_t*)str_buf, 0); 
// 	} 
	
// 	for (i = 0; i < strlen(str_buf); i++){
// 		printf("%c", str_buf[i]);
// 	}

// 	return PASS;
// }

// ////////////////////////////////////////////////////////////////TEST CASES FOR FILES END////////////////////////////////////////////////////////////////////////

// int rtc_test(){
//     printf("rtc_test");
//     int i;
//     int j;
//     int val=0;
//     val += rtc_open(NULL);
//     printf("%d",val);
//     printf("rtc_open");
//     for(i = 2; i <= 1024; i*=2){
//         // sleep(1);
//         val += rtc_write(NULL, &i, sizeof(uint32_t));
//         printf("Frequency= %d Hz\n", i);
//         for (j=0;j<i;j++){
//             val+=rtc_read(NULL,NULL,NULL);
//             printf("1");
//         }
//         printf("\n");
//     }
//     if (val ==0)
//     return PASS;
//     else
//     printf("%d",val);
//     return FAIL;

// }

// int rtc_open_test(){
//     int val=0;
//     int j;
//     val += rtc_open(NULL);
//     for (j=0;j<100;j++){
//         val+=rtc_read(NULL,NULL,NULL);
//         printf("1");
//     }
//     if (val ==0)
//     return PASS;
//     else
//     printf("%d",val);
//     return FAIL;
// }

// //////////////////////////////////////////////////////////////////////////////TEST CASES FOR Terminal////////////////////////////////////////////////////////////////

// int terminal_read_write() {
//     int32_t cnt;
//     uint8_t buf[1024];


// 	write_terminal(1, (uint8_t*)"Hi, what's your name? ", strlen((int8_t*)"Hi, what's your name? "));
// 	cnt = read_terminal(buf, 1023);

// 	if (cnt == -1)
// 		return FAIL;

// 	buf[cnt] = '\0';
//     write_terminal (1, (uint8_t*)"Hello, ", strlen((int8_t*)"Hello, "));
//     write_terminal (1, buf, strlen((int8_t*)buf));

// 	write_terminal(1, (uint8_t*)"Ok, how old are you? ", strlen((int8_t*)"Ok, how old are you? "));
// 	cnt = read_terminal(buf, 1023);

// 	if (cnt == -1)
// 		return FAIL;

// 	buf[cnt] = '\0';
//     write_terminal (1, (uint8_t*)"Damn bro! You're ", strlen((int8_t*)"Damn bro! You're "));
//     write_terminal (1, buf, strlen((int8_t*)buf));

// 	return PASS;
// }

// int test_continous_terminal() {
// 	int32_t cnt;
//     uint8_t buf[1024];

// 	while(1) {
// 		cnt = read_terminal(buf, 1023);
// 		write_terminal (1, buf, cnt);
// 	}
// }


/* Checkpoint 3 tests */

// int check_sys_call_linkage() {

//     // int32_t cnt, rval;
//     // uint8_t buf[BUFSIZE];
//     // ece391_fdputs (1, (uint8_t*)"Starting 391 Shell\n");

// 	asm volatile (
// 		"PUSHL	%EBX;"
// 		"movl $3, %eax;"
// 		"movl $3, %ebx;"
// 		"movl $3, %ecx;"
// 		"movl $3, %edx;"
// 		"INT $0x80;"
// 		"POPL	%EBX"
// 	);

// 	return PASS;
// }









/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// clear_screen();
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("idt_division_tests", idt_division_tests()); // divide by zero
	// TEST_OUTPUT("idt_overflow_test", idt_overflow_test()); // overflow test
	// TEST_OUTPUT("idt_bound_range_exceeded_test", idt_bound_range_exceeded_test()); // bound_range_exceeded
	// TEST_OUTPUT("idt_invalid_opcode_test", idt_invalid_opcode_test()); // invalid opcode
	// TEST_OUTPUT("page_fault_test", page_fault_test()); // page fault test

	// TEST_OUTPUT("page_invalid_addr", page_invalid_addr());
	// TEST_OUTPUT("page_invalid_access_greater_than_max", page_invalid_access_greater_than_max());
	// TEST_OUTPUT("page_invalid_access_less_than_min", page_invalid_access_less_than_min());
	// TEST_OUTPUT("video_mem_addr", video_mem_addr());

	// TEST_OUTPUT("terminal_read_write", terminal_read_write());
	// TEST_OUTPUT("test_continous_terminal", test_continous_terminal());

	// TEST_OUTPUT("test_open_file", test_open_file("FISH"));
	// TEST_OUTPUT("test_read_file_once", test_read_file_once());
	// TEST_OUTPUT("test_read_file_multiple_times_with_close_and_reopen()", test_read_file_multiple_times_with_close_and_reopen());
	// TEST_OUTPUT("test_read_single_directory", test_read_single_directory(7));
	// TEST_OUTPUT("test_read_directory_as_ls", test_read_directory_as_ls());
	// TEST_OUTPUT("test_read_file_beyond", test_read_file_beyond());

	// TEST_OUTPUT("RTC write", rtc_test());
	// TEST_OUTPUT("RTC open", rtc_open_test());

	// TEST_OUTPUT("check_sys_call_linkage", check_sys_call_linkage());
}
