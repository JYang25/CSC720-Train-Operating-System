#include <kernel.h>

WINDOW train_wnd;
char* train_id = "20";
char* default_speed = "5";

// Support functions
void str_concat(char* str1, char* str2);
char probe(char* contact_id);
int probe_board();
void send_cmd(char* cmd, int input_len, char* input_buffer);
void set_switch(char* switch_id, char* setting);
void set_train_speed(char* speed);
int probe_contact(char* contact_id, char* status);
void clear_buffer2();
int probe_zamboni();

// Configs
void config1();
void config1z();
void config2z();
void config3();
void config3z();
void config4();
void config4z();

// Core functions
void init_train_settings();
void train_process(PROCESS self, PARAM param);
void init_train(WINDOW* wnd);

// Auxillary functions
void start_train();
void stop_train();

/**
 * Starts the train
 */
void start_train()
{
  set_train_speed(default_speed);
}

/**
 * Stops the train
 */
void stop_train()
{
  set_train_speed("0");
}

/**
 * Reverses the train direction
 */
void reverse_train()
{
  set_train_speed("D");
}

/**
 * Concatenate strings to build the command string
 * @param str1 
 * @param str2
 */
void str_concat(char* str1, char* str2) {
  int i;
  int str1_len;
  for (str1_len = 0; str1[str1_len] != '\0'; str1_len ++);
  for (i = 0; str2[i] != '\0'; i ++)
    str1 [i + str1_len] = str2[i];
  str1 [i + str1_len] = '\0';
}

/**
 * Probes a contact ID
 * @param  contact_id 
 * @return result of probe
 */
char probe(char* contact_id)
{
  char cmd[10];
  char input[3];

  clear_buffer2();

  cmd[0] = 'C';
  cmd[1] = '\0';
  str_concat(cmd, contact_id);
  send_cmd(cmd, 3, input);
  return input[1];
}

/**
 * Probes the state of the board
 * @return int that indicates state of board
 */
int probe_board()
{
  char input;
  input = probe("8");

  if (probe("8") == '1')
    return 1;
  else if (probe("16") == '1')
    return 4;
  else
    return 3;
}

/**
 * Sends a command to the com1 port
 * @param cmd          the command
 * @param input_len    input length
 * @param input_buffer the input buffer
 */
void send_cmd(char* cmd, int input_len, char* input_buffer)
{
  COM_Message msg;
  char full_cmd[20];

  full_cmd[0] = '\0';
  str_concat(full_cmd, cmd);
  str_concat(full_cmd, "\015");
  msg.output_buffer = full_cmd;
  msg.len_input_buffer = input_len;
  msg.input_buffer = input_buffer;
  sleep(10);
  send(com_port, &msg);
}

/**
 * Sets switch ID setting to 'R' or 'G'
 * @param switch_id 
 * @param setting 
 */
void set_switch(char* switch_id, char* setting)
{
   char cmd[10];
   char temp;

   cmd[0] = 'M';
   cmd[1] = '\0';
   str_concat(cmd, switch_id);
   str_concat(cmd, setting);
   send_cmd(cmd, 0, &temp);
}

/**
 * Controls train speed
 * @param speed from 0-5 or D
 */
void set_train_speed(char* speed)
{
   char cmd[10];
   char temp;

   cmd[0] = 'L';
   cmd[1] = '\0';
   str_concat(cmd, train_id);
   if (speed[0] != 'D') {
      str_concat(cmd, "S");
      str_concat(cmd, speed);
   } else 
      str_concat(cmd, "D");
   send_cmd(cmd, 0, &temp);
}

/**
 * Probes contact ID's state
 * @param  contact_id 
 * @param  status 0 or 1
 * @return
 */
int probe_contact(char* contact_id, char* status)
{
  char input;

  while (42) {
    input = probe(contact_id);
    if (input == status[0])
       return 1;
  }
  return 0;
}

/**
 * Clears s88 memory buffer
 */
void clear_buffer2()
{
  char cmd[10];
  char temp;

  cmd[0] = 'R';
  cmd[1] = '\0';
  send_cmd(cmd, 0, &temp);
}

/**
 * Probes if Zamboni is running
 * @return [description]
 */
int probe_zamboni()
{
  int i;
  int found;
  int dir;
  found = 0;
  dir = 0;
  wprintf(&train_wnd, "Checking for Zamboni\n");

  for (i = 0; i < 30; i++) {
    sleep(10);
    if (probe("7") == '1') {
      wprintf(&train_wnd, "Zamboni is found\n");
      found = 1;
      break;
    }
  }

  if (found) {
    wprintf(&train_wnd, "Checking for direction\n");
    for (i = 0; i < 30; i++) {
      sleep(10);
      if (probe("10") == '1') {
        wprintf(&train_wnd, "Zamboni is clocky\n");
        dir = 710;
        break;
      }

      if (probe("6") == '1') {
        wprintf(&train_wnd, "Zamboni is anti-clocky\n");
        dir = 76;
        break;
      }
    }
  }
  if (!found && !dir) {
    wprintf(&train_wnd, "No Zamboni\n");
    return found;
  }

  return dir;
}

// Config 4 with Zamboni
void config4z()
{
  wprintf(&train_wnd, "Running config 4 with Zamboni\n");
  probe_contact("4", "1");
  wprintf(&train_wnd, "Starting engine\n");
  set_train_speed(default_speed);

  set_switch("5", "R");
  set_switch("6", "G");

  probe_contact("14", "1");
  set_train_speed(default_speed);
  sleep(200);
  
  probe_contact("14", "1");
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");
  set_switch("9", "G");
  set_train_speed(default_speed);

  sleep(480);
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");

  wprintf(&train_wnd, "Waiting\n");
  probe_contact("14", "1");

  wprintf(&train_wnd, "Resuming\n");
  set_train_speed(default_speed);
  
  sleep(100);
  probe_contact("14", "1");
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");

  set_switch("9", "R");
  set_switch("1", "R");
  set_switch("7", "G");
  set_train_speed(default_speed);

  probe_contact("4", "1");
  set_switch("4", "R");
  set_switch("3", "R");

  probe_contact("5", "1");
  set_train_speed("0");
  set_switch("4", "G");
  wprintf(&train_wnd, "Back home safely\n");
}

// Config 4 without Zamboni
void config4() {
  wprintf(&train_wnd, "Running config 4 without Zamboni\n");
  set_switch("9", "G");
  wprintf(&train_wnd, "Starting engine\n");
  set_train_speed(default_speed);

  probe_contact("14", "1");
  sleep(350);
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");

  set_train_speed(default_speed);

  set_switch("4", "R");
  set_switch("3", "R");
  
  probe_contact("5", "1");
  set_train_speed("0");
  wprintf(&train_wnd, "Back home safely\n");

}

// Config 3 with Zamboni
void config3z()
{
  wprintf(&train_wnd, "Running config 3 with Zamboni\n");
  probe_contact("10", "1");

  wprintf(&train_wnd, "Starting engine\n");
  set_train_speed(default_speed);

  probe_contact("6", "1");
  set_switch("4", "R");
  set_switch("3", "G");
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");
  set_train_speed(default_speed);

  set_switch("8", "R");

  probe_contact("12", "1");
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");

  set_switch("7", "G");
  set_train_speed(default_speed);
  set_switch("3", "R");

  probe_contact("5", "1");
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");
  wprintf(&train_wnd, "Back home safely\n");  

}

// Config 3 without Zamboni
void config3()
{
  wprintf(&train_wnd, "Running config 3 without Zamboni\n");
  set_switch("5", "R");
  set_switch("6", "G");
  wprintf(&train_wnd, "Starting engine\n");
  set_train_speed(default_speed);
  
  probe_contact("12", "1");
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");

  set_train_speed(default_speed);
  
  probe_contact("13", "1");
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");

  set_train_speed(default_speed);

  set_switch("4", "R");
  set_switch("3", "R");

  probe_contact("5", "1");
  wprintf(&train_wnd, "Back home safely\n");
  set_train_speed("0");

}

// Config 2 with Zamboni
void config2z()
{
  wprintf(&train_wnd, "Running config 2 with Zamboni\n");
  probe_contact("4", "1");
  set_switch("4", "R");
  wprintf(&train_wnd, "Starting engine\n");
  set_train_speed(default_speed);
  
  probe_contact("1", "1");
  set_switch("4", "G");
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");

  wprintf(&train_wnd, "Waiting\n");
  probe_contact("4", "1");
  wprintf(&train_wnd, "Resuming\n");
  set_train_speed(default_speed);
  set_switch("5", "R");

  probe_contact("8", "1");
  wprintf(&train_wnd, "Back home safely\n");
  set_train_speed("0");
}

// Config 1 with Zamboni
void config1z()
{
  wprintf(&train_wnd, "Running config 1 with Zamboni\n");
  set_switch("4", "R");
  set_switch("3", "G");
  probe_contact("10", "1");
  set_train_speed(default_speed);
  wprintf(&train_wnd, "Starting engine\n");

  clear_buffer2();
  probe_contact("1", "1");

  wprintf(&train_wnd, "Waiting\n");
  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");
  probe_contact("7", "1");
  set_switch("5", "G");

  probe_contact("10", "1");
  set_switch("5", "R");

  wprintf(&train_wnd, "Resuming\n");
  set_train_speed(default_speed);
  probe_contact("8", "1");
  set_switch("5", "G");
  set_train_speed("0");
  wprintf(&train_wnd, "Back home safely\n");
}


// Config 1 without Zamboni
void config1()
{
  wprintf(&train_wnd, "Running config 1/2 without Zamboni\n");
  set_switch("5", "R");
  set_switch("4", "R");
  set_switch("3", "G");

  set_train_speed(default_speed);
  wprintf(&train_wnd, "Starting engine\n");

  probe_contact("1", "1");

  set_train_speed("0");
  wprintf(&train_wnd, "Reversing\n");
  set_train_speed("D");
  set_train_speed(default_speed);

  probe_contact("8", "1");
  set_train_speed("0");
  wprintf(&train_wnd, "Back home safely\n");
}

/**
 * Initialize the train switches to sane settings
 */
void init_train_settings()
{
  wprintf(&train_wnd, "Initializing settings\n");  
  set_switch("8", "G");
  set_switch("6", "R");
  set_switch("9", "R");
  set_switch("1", "G");
  set_switch("2", "R");
  set_switch("7", "R");
  set_switch("5", "G");
  set_switch("9", "R");
  set_switch("4", "G");
  set_switch("3", "G");
}

/**
 * Main train process
 * @param self  
 * @param param 
 */
void train_process(PROCESS self, PARAM param)
{
  int board_state;
  int is_zam;
  is_zam = 0;

  init_train_settings();

  board_state = probe_board();

  is_zam = probe_zamboni();

  switch (board_state) {
    case 1:
      if (!is_zam)
        config1();
      else if (is_zam == 710)
        config1z();
      else if (is_zam == 76)
        config2z();
      break;
    case 3:
      if (!is_zam)
        config3();
      else
        config3z();
      break;
    case 4:
      if (!is_zam)
        config4();
      else
        config4z();
      break;
    default:
      break;
  }
  remove_ready_queue(active_proc);
  resign();
}

/**
 * Initialize the train process
 * @param wnd Window to run the process
 */
void init_train(WINDOW* wnd)
{
  create_process(train_process, 5, (PARAM) wnd, "Train process");
}
