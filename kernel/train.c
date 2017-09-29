// student name: Yang, Jizhou
// student ID  : 917270853

#include <kernel.h>

// a function to concatenate two string, the new string is str1
void concat_str(char* str1, char* str2, int window_tid);

// to send a command to the train simulator, "015" will be attached
void send_cmd(char* cmd, int input_len, char* input_buffer, int window_tid);

// to clear the command buffer
void clear_train_buffer();

// probe if the train is on contact #
char probe(char* contact_id, int window_tid);

// change the direction of the train, and set speed to 4
void change_dir(int window_tid);

// initiate the switches to avoid zamboni ran out of track
void init_switch(int window_tid);

// determine whether zamboni is present
int find_zam(int window_tid);

// check the initial configuration of the panel
int check_configure(int window_tid);

// run the train according to the configuration
void run_configure(int window_tid);

void run_30(int window_tid);
void run_30z(int window_tid);
void run_40(int window_tid);
void run_40z(int window_tid);
void run_10(int window_tid);
void run_10z(int window_tid);
void run_20(int window_tid);
void run_20z(int window_tid);


void concat_str(char* str1, char* str2, int window_tid){
	int str1_end;
	int j;
	str1_end = 0;
	while(str1[str1_end] != '\0')
		str1_end++;
	for(j = 0; str2[j] != '\0'; j++)
		str1[str1_end+j] = str2[j];
	str1[str1_end + j] = '\0';
}


void send_cmd(char* cmd, int input_len, char* input_buffer, int window_tid){
	COM_Message msg;
	char full_cmd[20];
	full_cmd[0] = '\0';
	concat_str(full_cmd, cmd,window_tid);
	concat_str(full_cmd, "\015",window_tid);
	msg.output_buffer = full_cmd;
	msg.len_input_buffer = input_len;
	msg.input_buffer = input_buffer;
	send(com_port, &msg);
}



void clear_train_buffer(int window_tid){
	char temp;
	send_cmd("R", 0, &temp,window_tid);
}


char probe(char* contact_id,int window_tid){
	wm_print(window_tid,"probe contact: %s\n",contact_id);
	char input[3];
	clear_train_buffer(window_tid);
	char cmd[10];
	cmd[0]='C';cmd[1]='\0';
	concat_str(cmd, contact_id,window_tid);
	send_cmd(cmd, 3, input,window_tid);
	return input[1];
}

void keep_probe(char* contact_id,int window_tid){
	while(1){
		if(probe(contact_id,window_tid)=='1'){
			return;
		}
		sleep(5);
	}
}


void change_speed(char* speed, int window_tid){
	char input;
	char cmd[10];
	cmd[0]='L';cmd[1]='2';cmd[2]='0';cmd[3]='S';cmd[4]='\0';
	concat_str(cmd,speed,window_tid);
	clear_train_buffer(window_tid);
	send_cmd(cmd,0,&input,window_tid);
	wm_print(window_tid,"change speed to: %s\n",speed);
}

void change_dir(int window_tid){
	change_speed("0",window_tid);
	char input;
	char cmd[10];
	cmd[0]='L';cmd[1]='2';cmd[2]='0';cmd[3]='D';cmd[4]='\0';
	sleep(5);
	clear_train_buffer(window_tid);
	send_cmd(cmd,0,&input,window_tid);
	clear_train_buffer(window_tid);
	sleep(5);
	change_speed("5",window_tid);
	wm_print(window_tid,"direction changed\n");
}
	
void set_switch(char* switch_id,char* dir, int window_tid){
	clear_train_buffer(window_tid);
	char input;
	char cmd[10];
	cmd[0] = 'M';
	cmd[1] = '\0';
	concat_str(cmd,switch_id,window_tid);
	concat_str(cmd,dir,window_tid);
	sleep(5);
	send_cmd(cmd,0,&input,window_tid);
	wm_print(window_tid,"change switch %s to %s\n",switch_id,dir);
}



void init_switch(int window_tid){
	set_switch("4","G",window_tid);
	set_switch("1","G",window_tid);
	set_switch("9","R",window_tid);
	set_switch("8","G",window_tid);
	set_switch("5","G",window_tid);
}



int find_zam(int window_tid){
// use a for loop to check if zamboni is present, keep probing contact 3
	char ck;	
	int found = 0;
	for(int i=0; i<25; i++){
		ck = probe("3",window_tid);
		if(ck=='1'){
			found = 1;
			wm_print(window_tid,"zamboni is running on the track\n");
			break;
		}
		sleep(30);
	}
	if(found){
		return 1;
	}
	else{
		wm_print(window_tid,"zamboni is NOT running on the track\n");
		return 0;
	}
}

int check_configure(int window_tid){
	int found = 0;	
	wm_print(window_tid,"probe the train and wagon\n");
	if(probe("2",window_tid)=='1' && probe("12",window_tid)=='1'){
		change_speed("5",window_tid);
		sleep(20);
		if(probe("1",window_tid)=='1'){  // it is configure 1
			change_speed("0",window_tid);
			found = find_zam(window_tid);
			if(found)
				return 11;
			else 
				return 10;
		} else{     // it is configure 2
			change_speed("0",window_tid);
			found = find_zam(window_tid);
			if(found)
				return 21;
			else 
				return 20;
		}
	} else if(probe("5",window_tid)=='1' && probe("11",window_tid)=='1'){
		found = find_zam(window_tid);		
		if(found)
			return 31;
		else 
			return 30;
	} else if(probe("2",window_tid)=='1' && probe("16",window_tid)=='1'){
		found = find_zam(window_tid);		
		if(found)
			return 41;
		else 
			return 40;
	} else{
		wm_print(window_tid,"unkonwn configuration!!!!\n");
		return -1;
	}
	return -2;
}


void run_configure(int window_tid){

	int conf = check_configure(window_tid);
//	wm_print(window_tid,"return is %d\n",conf);
	switch (conf){
		case 30:
			wm_print(window_tid,"it is configure 3 without zamboni\n");
			run_30(window_tid);
			break;
		case 31:
			wm_print(window_tid,"it is configure 3 with zamboni\n");
			run_30z(window_tid);
			break;
		case 40:
			wm_print(window_tid,"it is configure 4 without zamboni\n");
			run_40(window_tid);
			break;
		case 41:
			wm_print(window_tid,"it is configure 4 with zamboni\n");
			run_40z(window_tid);
			break;
		case 10:
			wm_print(window_tid,"it is configure 1 without zamboni\n");
			run_10(window_tid);
			break;
		case 11:
			wm_print(window_tid,"it is configure 1 with zamboni\n");
			run_10z(window_tid);
			break;
		case 20:
			wm_print(window_tid,"it is configure 2 without zamboni\n");
			run_20(window_tid);
			break;
		case 21:
			wm_print(window_tid,"it is configure 2 with zamboni\n");
			run_20z(window_tid);
			break;
	}
	while(1);
}

// run configure 1 without zamboni, return to contact 2
void run_10(int window_tid){
	change_speed("5",window_tid);
	set_switch("8","R",window_tid);
	keep_probe("11",window_tid);
	sleep(57);
	change_speed("0",window_tid);
	wm_print(window_tid,"connected to wagon\n");
	set_switch("7","G",window_tid);
	set_switch("4","R",window_tid);
	set_switch("3","G",window_tid);
	change_dir(window_tid);
	keep_probe("2",window_tid);
	change_speed("0",window_tid);  
	wm_print(window_tid,"train and wagon have returned!!!\n");
}

// run configure 1 with zamboni, return to contact 2 
void run_10z(int window_tid){
	keep_probe("6",window_tid);
	change_dir(window_tid);
	keep_probe("10",window_tid);
	set_switch("5","R",window_tid);
	set_switch("6","G",window_tid);
	keep_probe("9",window_tid);
	sleep(50);
	change_speed("0",window_tid);
	wm_print(window_tid,"connected to wagon\n");
	set_switch("5","G",window_tid);
	keep_probe("14",window_tid);
	set_switch("7","R",window_tid);
	change_dir(window_tid);
	keep_probe("3",window_tid);
	set_switch("1","R",window_tid);
	set_switch("2","G",window_tid);
	keep_probe("2",window_tid);
	change_speed("0",window_tid);  
	wm_print(window_tid,"train and wagon have returned!!!\n");
	set_switch("1","G",window_tid);	
}

// run configure 2 without zamboni, return to contact 12
void run_20(int window_tid){
	set_switch("2","G",window_tid);
	set_switch("1","R",window_tid);
	change_speed("5",window_tid);
	keep_probe("14",window_tid);
	set_switch("9","R",window_tid);
	change_dir(window_tid);
	keep_probe("1",window_tid);
	sleep(50);
	change_speed("0",window_tid);
	wm_print(window_tid,"connected to wagon\n");
	change_dir(window_tid);
	set_switch("8","R",window_tid);
	keep_probe("12",window_tid);
	change_speed("0",window_tid);  
	wm_print(window_tid,"train and wagon have returned!!!\n");
}

// run configure 2 with zamboni, return to  contact 12
void run_20z(int window_tid){
	keep_probe("10",window_tid);
	set_switch("2","G",window_tid);
	set_switch("1","R",window_tid);
	change_speed("5",window_tid);
	keep_probe("14",window_tid);
	change_dir(window_tid);
	keep_probe("1",window_tid);
	sleep(45);
	change_speed("0",window_tid);
	wm_print(window_tid,"connected to wagon\n");
	keep_probe("10",window_tid);
	change_dir(window_tid);
	set_switch("8","R",window_tid);
	keep_probe("11",window_tid);
	set_switch("8","G",window_tid);
	keep_probe("12",window_tid);
	change_speed("0",window_tid);  
	wm_print(window_tid,"train and wagon have returned!!!\n");
}


// run configure 3 without zamboni, return to contact 5
void run_30(int window_tid){
	set_switch("9","R",window_tid);
	set_switch("1","R",window_tid);
	set_switch("2","R",window_tid);
	set_switch("7","R",window_tid);
	change_speed("5",window_tid);
	keep_probe("12",window_tid);
	sleep(60);   // train has reached wagon
	wm_print(window_tid,"connected to wagon\n");
	change_dir(window_tid);
	change_speed("5",window_tid);
	set_switch("8","G",window_tid);
	set_switch("4","R",window_tid);
	set_switch("3","R",window_tid);
	keep_probe("5",window_tid);  // train has returned
	change_speed("0",window_tid);  
	wm_print(window_tid,"train and wagon have returned!!!\n");
}

// run configure 3 with zamboni. return to contact 5
void run_30z(int window_tid){
	wm_print(window_tid,"wait for zamboni reaches contact 14\n");	
	keep_probe("14",window_tid);
	change_speed("5",window_tid);
	sleep(300);
	set_switch("9","R",window_tid);
	set_switch("1","R",window_tid);
	set_switch("2","R",window_tid);
	set_switch("7","R",window_tid);
	keep_probe("12",window_tid);
	set_switch("1","G",window_tid);
	sleep(35);
	wm_print(window_tid,"connected to wagon\n");
	change_speed("0",window_tid);
	set_switch("5","R",window_tid);
	set_switch("6","G",window_tid);
	set_switch("8","G",window_tid);
	sleep(200);
	keep_probe("14",window_tid);
	change_dir(window_tid);
	keep_probe("4",window_tid);
	set_switch("4","R",window_tid);
	set_switch("3","R",window_tid);
	keep_probe("5",window_tid);
	change_speed("0",window_tid);
	wm_print(window_tid,"train and wagon have returned!!!\n");
	set_switch("4","G",window_tid);
}


// run configure 4 without zamboni, return to contact 16
void run_40(int window_tid){
	change_speed("5",window_tid);
	set_switch("4","R",window_tid);
	set_switch("3","G",window_tid);
	sleep(430);
	change_dir(window_tid);
	change_speed("5",window_tid);
	set_switch("5","G",window_tid);
	set_switch("9","G",window_tid);
	keep_probe("16",window_tid);
	change_speed("0",window_tid);
	wm_print(window_tid,"train and wagon have returned!!!\n");
}

// run configure 4 with zamboni, return to contact 16
void run_40z(int window_tid){
	keep_probe("10",window_tid);
	change_speed("5",window_tid);
	keep_probe("4",window_tid);
	set_switch("4","R",window_tid);
	set_switch("3","G",window_tid);
	keep_probe("6",window_tid);
	sleep(75);
	change_speed("0",window_tid);
	wm_print(window_tid,"connected to wagon\n");
	set_switch("4","G",window_tid);
	set_switch("8","R",window_tid);
	change_dir(window_tid);
	keep_probe("10",window_tid);
	sleep(30);
	change_speed("0",window_tid);
	keep_probe("13",window_tid);
	sleep(10);
	change_speed("5",window_tid);
	set_switch("9","G",window_tid);
	keep_probe("16",window_tid);
	change_speed("0",window_tid);
	wm_print(window_tid,"train and wagon have returned!!!\n");	
}



void train_process(PROCESS self, PARAM param)
{
	int window_tid = wm_create(0,0,80,8);
	wm_print(window_tid,"welcome to the train simulator\n");	
	wm_print(window_tid,"initiate the switches %d\n",window_tid);
	init_switch(window_tid);
	wm_print(window_tid,"check the configuration\n");
	run_configure(window_tid);
	while(1);

}


void init_train()
{
	create_process(train_process, 5, 0, "Train process");
}
