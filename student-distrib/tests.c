#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"
#include "RTC.h"
#include "filesystem.h"

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

 /* exception_divide0_test
 *   DESCIRPTION: test exception 0
 *   INPUT:  none
 *   OUTPUT: none
 */
void exception_divide0_test(){
	TEST_HEADER;
	int a = 1;
	int b = 0;
	a = a/b;
}
 /* IDT_test_null
 *   DESCIRPTION: test page fault
 *   INPUT:  none
 *   OUTPUT: none
 */
void IDT_test_null() {
	TEST_HEADER;
	int* ptr = NULL;
	int test;
	test = *(ptr);
}

 /* page_fault
 *   DESCIRPTION: test page fault
 *   INPUT:  none
 *   OUTPUT: none
 */
void page_fault() {
	TEST_HEADER;
	uint32_t addr = 0x800000;
	int* ptr = (int*)(addr + 8);
	int a;
	a = *(ptr);

}

 /* exception_11
 *   DESCIRPTION: segment not present
 *   INPUT:  none
 *   OUTPUT: none
 */
void exception_11() {
	asm volatile("int $11");
}
 
/* Checkpoint 2 tests */

int terminal_raed_write_test(int32_t read_nbytes, int32_t write_nbytes) {
	int32_t bytes_read;
	int32_t bytes_write;
	uint8_t buf[128];
	clear();
	while (1)
	{
		bytes_read = terminal_read(0, buf, read_nbytes);
		bytes_write = terminal_write(0, buf, write_nbytes);
		printf("Read %d bytes, wrote %d bytes\n", bytes_read, bytes_write);
	}

	return PASS;
	
}

void test_open_close_terminal(){
	clear();
	char buf[128];
	int32_t fd = NULL;
	terminal_open(0);
	while (1)
	{
		printf("please enter 'Hello': ");
		terminal_read(fd, buf,128);
		if (!strncmp("Hello\n",buf,128)){
			break;
		}
	}
	
	terminal_close(0);
	printf("/nterminal close here!");
}

int test_terminal_write_null(int32_t bytes_write){
	clear();
	char* buf = NULL;
	terminal_write(0, buf, bytes_write);
	printf("\n");
	return PASS;
}

int test_terminal_driver(){
    TEST_HEADER;
	clear();
    int nbytes;
    char buf[1024];

    while(1){
        terminal_write(0, (uint8_t*)"TESTING bytes size 100\n", 23);
        nbytes = terminal_read(0, buf, 100);
        terminal_write(0, buf, nbytes);
		terminal_write(0, (uint8_t*)"\n", 1);


        terminal_write(0, (uint8_t*)"TESTING bytes size 128\n", 23);
        nbytes = terminal_read(0, buf, 128);
        terminal_write(0, buf, nbytes);
		terminal_write(0, (uint8_t*)"\n", 1);

        terminal_write(0, (uint8_t*)"TESTING bytes size 129\n", 23);
        nbytes = terminal_read(0, buf, 129);
        terminal_write(0, buf, nbytes);
		terminal_write(0, (uint8_t*)"\n", 1);
    }
    return PASS;
}
 /* RTC_open_close_test
 *   DESCIRPTION: test functionality of open and close function
 *   INPUT:  none
 *   OUTPUT: none
 */
void RTC_open_close_test() {
	uint32_t buff;
	int i;
	clear();	// clear screen first
	RTC_open(0);	// open RTC and initializaed as 2Hz
	for (i = 0; i < 10; i++) {
		RTC_read(0, &buff, 4);	// read waits for handler process 2 interrupts
		putc('1');
	}
	RTC_close(0);
	printf("\nRTC close");
}

 /* RTC_write_test
 *   DESCIRPTION: test functionality of write and read function
 *   INPUT:  none
 *   OUTPUT: none
 */
void RTC_write_test() {
	uint32_t freq;
	int i, j, write_result; 
	freq = 2;
	clear();
	RTC_open(0);
	// go through all frequency
	for (j = 0; j < 10; j++) {
		write_result = RTC_write(0 ,&freq, 4);
		if (write_result == 0) {
			printf("Write success!\n");
			printf("Frequency: %d\n", freq);
		}
		for (i = 0; i < 10; i++) {
			RTC_read(0, &freq, 4);	// print out '1' at each rate
			putc('1');
		}
		freq = freq*2;
		printf("\n");
	}
	RTC_close(0);
}

#define NAME_SIZE 32
#define S_BUFF_SIZE 1000
#define L_BUFF_SIZE 5277

/* read_short_file
 * Description: read frame1.txt
 * Input: None
 * Output: PASS or FAIL
 * Side Effects: print content of frame1.txt on the screen
 */
int read_short_file() {
	/* change ptr value to test*/
	clear();
	uint8_t fname[NAME_SIZE] = "frame1.txt";
	int32_t open;
	int8_t buf[S_BUFF_SIZE];
	int32_t i, count;
	open = file_open(fname);
	if(open==0){
		printf("file successfully opened!\n");
		count = file_read(1, buf, S_BUFF_SIZE);
		for (i = 0; i < count; i++) {
			putc(buf[i]);
		}
		// printf("Bytes read: %d\n", count);
		file_close(0);
		printf("file successfully closed!");
		return PASS;
	}
	else{
		printf("file can't be opened");
		return FAIL;
	}
}

/* read_long_file
 * Description: read verylargetextwithverylongname.txt
 * Input: None
 * Output: PASS or FAIL
 * Side Effects: should not be able to open file because name is too lang
 */
int read_long_file() {
	/* change ptr value to test*/
	clear();
	uint8_t fname[33] = "verylargetextwithverylongname.txt";
	int32_t open;
	int8_t buf[L_BUFF_SIZE];
	int32_t i, count;
	open = file_open(fname);
	if(open==0){
		printf("file successfully opened!\n");
		count = file_read(1, buf, S_BUFF_SIZE);
		for (i = 0; i < count; i++) {
			putc(buf[i]);
		}
		// printf("Bytes read: %d\n", count);
		file_close(0);
		printf("file successfully closed!");
		return PASS;
	}
	else{
		printf("file can't be opened");
		return FAIL;
	}
}

/* read_executable
 * Description: read fish
 * Inputs: None
 * Outputs: PASS or FAIL
 * Side Effects: print content of fish on the screen
 */
int read_executable() {
	clear();
	uint8_t fname[NAME_SIZE] = "fish";
	int32_t open;
	int8_t buf[L_BUFF_SIZE];
	int32_t i, count;
	open = file_open(fname);
	if(open==0){
		printf("file successfully opened!\n");
		count = file_read(1, buf, S_BUFF_SIZE);
		for (i = 0; i < count; i++) {
			putc(buf[i]);
		}
		// printf("Bytes read: %d\n", count);
		file_close(0);
		printf("file successfully closed!");
		return PASS;
	}
	else{
		printf("file can't be opened");
		return FAIL;
	}
}

/* test_dir_read
 * Description: list all files in the directory
 * Inputs: None
 * Outputs: PASS
 * Side Effects: will print a list of file name, type, and size in the directory
 */
int test_dir_read() {
	clear();
	int8_t buf[S_BUFF_SIZE];	
	printf("Files in this directory: \n");
	dir_read (0, buf, S_BUFF_SIZE);
	return PASS;	
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// launch your tests here
	/* Checkpoint 1 tests */
	// TEST_OUTPUT("idt_test", idt_test());
	// exception_divide0_test();
	// IDT_test_null();
	// page_fault();
	//exception_11();
	
	/* Checkpoint 2 tests */
	// TEST_OUTPUT("test terminal read and write", terminal_raed_write_test(128, 128));
	// test_open_close_terminal();
	//TEST_OUTPUT("test terminal driver", test_terminal_driver());
	// RTC_open_close_test();
	//RTC_write_test();
	//TEST_OUTPUT("read_short_file", read_short_file());
	TEST_OUTPUT("read_long_file", read_long_file());
	//TEST_OUTPUT("read_executable", read_executable());
	//TEST_OUTPUT("test_dir_read", test_dir_read());
	//TEST_OUTPUT("test terminal write NULL", test_terminal_write_null(128));
}


