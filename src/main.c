#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <examples.h>

#define MAX_LENGTH 10
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

void read_command(char* buffer);
void print_help();
int run_example(int choice);

static int (*examples[2])(void) = {
  examples_core_simple_window,
  examples_core_simple_platformer,
};

int main(void) {
  bool should_exit = false;

  // Clear terminal
  printf("\e[1;1H\e[2J");

  printf("+-----------------------------+\n");
  printf("|                             |\n");
  printf("|    Raylib example Runner    |\n");
  printf("|                         v1.0|\n");
  printf("+-----------------------------+\n\n");

  print_help();

  char* buffer = (char*)malloc(sizeof(char) * MAX_LENGTH);

  while (!should_exit) {
    bool handled = false;

    printf("> ");
    read_command(buffer);

    if (strcmp(buffer, "exit") == 0) {
      should_exit = true;
      handled = true;
    }
    else if (strcmp(buffer, "help") == 0) {
      print_help();
      handled = true;
    }
    else {
      int choice = atoi(buffer);
      if (choice > 0 && run_example(choice) == 0){
        handled = true;
      }
    }

    if (!handled) {
      printf("Unknown command: '%s'.\n", buffer);
    }
  }


  //examples_core_simple_window();

  free(buffer);
  return 0;
}

void read_command(char* buffer) {
  fgets(buffer, MAX_LENGTH, stdin);

  // Clears the new line character from the command
  size_t cch = strlen(buffer);
  if (cch > 1 && buffer[cch - 1] == '\n')
  {
    buffer[cch - 1] = '\0';
  }
}

void print_help() {
  printf("\n\n");
  printf("Available examples:\n");
  printf("\t1 - [core] Simple Window\n");
  printf("\t2 - [core] Simple Platformer\n");
  printf("\n\n");
}

int run_example(int choice) {
  size_t length = NELEM(examples);
  if (choice > length)
  {
    return 1;
  }

  return examples[choice-1]();
}