// student name: Yang, Jizhou
// student ID  : 917270853

#include <kernel.h>

void ps(int window_id);
void echo(char * cmd, int window_id);
void ppd(int wnd, PCB* p);
void shell_process (PROCESS self, PARAM param);
void about(int window_id);
void top(int window_id);


// Struct datetype
typedef struct _input_buffer
{
	int length;
	char buffer[20];
} input_buffer;



// Read input from the keystrokes
void read_input(int window_id, input_buffer *input)
{
 	char ch;
	input->length = 0;
  	do
	{
		ch = keyb_get_keystroke(window_id,TRUE);  // why it doesn't work for FALSE?		
		switch (ch) {
		case '\b':
			if (input->length == 0)
				continue;
			input->length--;
			wm_print(window_id, "%c", ch);
			break;
		case 13:
			wm_print(window_id, "%c", '\n');
			break;
		default:
			input->buffer[input->length++] = ch;
			wm_print(window_id, "%c", ch);		
			break;
		}
	} while (ch != '\n' && ch != '\r');
	input->buffer[input->length] = '\0';
}




// Removes newline from the input buffer
char * remove_newline(char *str)
{
	char *out = str, *idx = str;
	for(; *str != '\0'; ++str){
		if(*str != '\n')
			*idx++ = *str;
	}
	*idx = '\0';
	return out;
}


// Removes the front spaces from the input buffer

char * remove_space(char *str)
{
	char *out = str, *idx = str;
	for(; *str != '\0'; ++str){
	if(*str != ' ')
		*idx++ = *str;
	}
	*idx = '\0';
	return out;
}

// compare whether two strings are equal
int cmp_cmd(char* cmd1, char* cmd2){
	while(*cmd1!='\0' && *cmd2!='\0'){
		if(*cmd1 != *cmd2)
			return 0;
		cmd1++;
		cmd2++;
	}
	if(*cmd1=='\0' && *cmd2=='\0')
		return 1;
	else 
		return 0;
}

void help(int window_id){
	wm_print(window_id, "*********************************************\n");
	wm_print(window_id, "List of commands: \n\n");
	wm_print(window_id, "help    print the function list \n");
	wm_print(window_id, "cls     clears the screen/window\n");
	wm_print(window_id, "shell   launches another shell\n");
	wm_print(window_id, "pong    launches the pong game\n");
	wm_print(window_id, "echo    echoes a string to the console\n");
	wm_print(window_id, "ps      prints out the process table\n");
	wm_print(window_id, "top     prints out the process every second\n");
	wm_print(window_id, "about   show developer's name and a message\n");
	wm_print(window_id, "*********************************************\n");
}

void run_command(char * cmd, int window_id )
{
/*
	wm_print(window_id,"the command is: ");
	wm_print(window_id,cmd);
	wm_print(window_id,"###\n");
*/
	if (cmp_cmd(cmd, "cls") ) {
		wm_clear(window_id);
	} else if (cmp_cmd(cmd, "help")){
		help(window_id);
	} else if (cmp_cmd(cmd, "shell")){
		create_process(shell_process, 5, 0, "Shell Process");
	} else if (cmp_cmd(cmd, "pong")){
		start_pong();
	} else if (*cmd=='e' && *(cmd+1)=='c' && *(cmd+2)=='h' && *(cmd+3)=='o'){
		echo(cmd, window_id);
	} else if (cmp_cmd(cmd, "ps")){
		ps(window_id);
	} else if (cmp_cmd(cmd, "top")){
		top(window_id);
	} else if (cmp_cmd(cmd, "train")){
		init_train();
	} else if (cmp_cmd(cmd, "about")){
		about(window_id);
	} else {
		wm_print(window_id, "command not found. try help\n");
	}  

}

void about(int window_id){
	wm_print(window_id,"Welcome to TOS system\n");
	wm_print(window_id,"Developed by Jizhou Yang\n");

}



void echo(char * cmd, int window_id){
	char *idx = cmd+4;
 	wm_print(window_id,  idx);
	wm_print(window_id,  "\n");
}


// function top to print all the processes every second
void top(int window_id){	
	int i;
	char ch;
    	PCB* p = pcb;
	while(1){
		wm_clear(window_id);
		sleep(5);		
		wm_print(window_id,"State           Active Prio Name\n");
		wm_print(window_id,"------------------------------------------------\n");
		i = 0;
		p = pcb;
		for (i = 0; i < MAX_PROCS; i++, p++) {
			if (!p->used)
		    		continue;
			ppd(window_id, p);
		}
		sleep(35);
// to exit top by keyboard input
		if(ch = keyb_get_keystroke(window_id,FALSE))
			break;
	}
}


// function ps to print all the processes
void ps(int window_id){
	int i;
    	PCB* p = pcb;
	wm_print(window_id,"State           Active Prio Name\n");
	wm_print(window_id,"------------------------------------------------\n");
	
	for (i = 0; i < MAX_PROCS; i++, p++) {
		if (!p->used)
	    		continue;
		ppd(window_id, p);
	}


}

// print the details of process
void ppd(int wnd, PCB* p){
	static const char *state[] = 
	{ "READY          ",
	  "SEND_BLOCKED   ",
	  "REPLY_BLOCKED  ",
	  "RECEIVE_BLOCKED",
	  "MESSAGE_BLOCKED",
	  "INTR_BLOCKED   "
	};
        if (!p->used) {
		wm_print(wnd, "PCB slot unused!\n");
		return;
    	}
/* State */
    	wm_print(wnd, state[p->state]);
/* Check for active_proc */
    	if (p == active_proc)
		wm_print(wnd, " *      ");
    	else
		wm_print(wnd, "        ");
/* Priority */
    	wm_print(wnd, "  %2d", p->priority);

/* Name */
    	wm_print(wnd, " %s\n", p->name);

}

void shell_process (PROCESS self, PARAM param)
{
		
	int window_id = wm_create(10,3,50,17);
	wm_print(window_id,"Welcome to TOS shell!\n");
	wm_print(window_id,"Input help for list of commands\n");

	input_buffer input;
	input_buffer* input_p = &input;

	while(1){
/*
		char ch = keyb_get_keystroke(window_id,TRUE);
		wm_print(window_id,"Got key: %c\n",ch);
*/	
		wm_print(window_id,">");		
		read_input(window_id, input_p);
// check whether the command is too long, max length 20	
		if(input_p->length>20){
			wm_print(window_id, "The command is too long! Max length is 20!\n");
			continue;
		}
		char *cmd = remove_newline(&input_p->buffer[0]);
		char *cmd1 = remove_space(cmd);
		run_command(cmd1,window_id);

		
	}
}

/**
 * Initialize the shell process
 */

void start_shell()
{
	create_process(shell_process, 5, 0, "Shell Process");
  	resign();
	
}
