

 #define _GNU_SOURCE
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>
 #include <dirent.h>
#include <sys/types.h>
#include <ctype.h>




 
 #define MAX_BUFFER_LINE 2048
 #define MAX_HISTORY 1000
 
 extern void tty_raw_mode(void);
 
 // Buffer where line is stored
 int line_length;
 char line_buffer[MAX_BUFFER_LINE];
 
 // Simple history array
 // This history does not change. 
 // Yours have to be updated.
 int history_index = 0;
 char *history[MAX_HISTORY] = {0};
 int history_length = 0;
 
 void read_line_print_usage()
 {
   char * usage = "\n"
     " ctrl-?       Print usage\n"
     " Backspace    Deletes last character\n"
     " up arrow     See last command in the history\n";
 
   write(1, usage, strlen(usage));
 }


 int common_prefix_len(char **matches, int count) {
  if (count == 0) return 0;
  int i = 0;
  while (1) {
    char c = matches[0][i];
    for (int j = 1; j < count; j++) {
      if (matches[j][i] != c || c == '\0') {
        return i;
      }
    }
    i++;
  }
}

int complete_path(char *line_buffer, int *line_length, int *cursor_pos, int list_all) {
  int start = *cursor_pos - 1;
  while (start >= 0 && !isspace(line_buffer[start])) {
    start--;
  }
  start++;

  int word_len = *cursor_pos - start;
  if (word_len <= 0) return 0;

  char word[1024];
  strncpy(word, &line_buffer[start], word_len);
  word[word_len] = '\0';

  DIR *dir = opendir(".");
  if (!dir) return 0;

  struct dirent *entry;
  char *matches[1024];
  int match_count = 0;

  while ((entry = readdir(dir)) != NULL) {
    if (strncmp(entry->d_name, word, word_len) == 0) {
      matches[match_count++] = strdup(entry->d_name);
    }
  }
  closedir(dir);

  if (match_count == 0) return 0;

  if (match_count == 1) {
    // One match: autocomplete full
    char *match = matches[0];
    int match_len = strlen(match);
    int insert_len = match_len - word_len;

    for (int i = *line_length - 1; i >= *cursor_pos; i--) {
      line_buffer[i + insert_len] = line_buffer[i];
    }

    for (int i = 0; i < insert_len; i++) {
      line_buffer[start + word_len + i] = match[word_len + i];
    }

    *line_length += insert_len;
    *cursor_pos += insert_len;
    line_buffer[*line_length] = '\0';

    write(1, &line_buffer[*cursor_pos - insert_len], insert_len);
    for (int i = 0; i < (*line_length - *cursor_pos); i++) {
      char back = 8;
      write(1, &back, 1);
    }

    free(match);
    return 1;
  } else {
    // Multiple matches
    int common_len = common_prefix_len(matches, match_count);

    if (common_len > word_len) {
      // Insert common prefix only
      int insert_len = common_len - word_len;

      for (int i = *line_length - 1; i >= *cursor_pos; i--) {
        line_buffer[i + insert_len] = line_buffer[i];
      }

      for (int i = 0; i < insert_len; i++) {
        line_buffer[start + word_len + i] = matches[0][word_len + i];
      }

      *line_length += insert_len;
      *cursor_pos += insert_len;
      line_buffer[*line_length] = '\0';

      write(1, &line_buffer[*cursor_pos - insert_len], insert_len);
      for (int i = 0; i < (*line_length - *cursor_pos); i++) {
        char back = 8;
        write(1, &back, 1);
      }

      for (int i = 0; i < match_count; i++) free(matches[i]);
      return 1;
    }

    if (list_all) {
      // Double-tab: list all matches
      write(1, "\n", 1);
      for (int i = 0; i < match_count; i++) {
        write(1, matches[i], strlen(matches[i]));
        write(1, "\t", 1);
      }
      write(1, "\n", 1);

      // Reprint current line
      write(1, line_buffer, *line_length);
      for (int i = 0; i < *line_length - *cursor_pos; i++) {
        char back = 8;
        write(1, &back, 1);
      }
    }

    for (int i = 0; i < match_count; i++) {
      free(matches[i]);
    }
  }

  return 1;
}



 
 
 /* 
  * Input a line with some basic editing.
  */
 char * read_line() {
 
   // Set terminal in raw mode
   tty_raw_mode();
 
   line_length = 0;
   int cursor_pos = 0;
   int browsing_history = -1;
 
   // Read one line until enter is typed
   int last_char_tab = 0;
   while (1) {
 
     // Read one character in raw mode.
     char ch;
     read(0, &ch, 1);
 
     if (ch >= 32) {
       // It is a printable character.
 
       // If max number of character reached ignore input
       if (line_length == MAX_BUFFER_LINE - 2) continue;
 
       // Shift characters right to make room
       for (int i = line_length; i > cursor_pos; i--) {
         line_buffer[i] = line_buffer[i - 1];
       }
 
       // Insert character at cursor position
       line_buffer[cursor_pos] = ch;
       line_length++;
       cursor_pos++;
 
       // Rewrite line from cursor
       write(1, &line_buffer[cursor_pos - 1], line_length - cursor_pos + 1);
 
       // Move cursor back to correct position
       for (int i = 0; i < line_length - cursor_pos; i++) {
         ch = 8; // backspace
         write(1, &ch, 1);
       }
 
     }
     else if (ch == 10) {
       // <Enter> was typed. Return line
 
       // Print newline
       write(1, &ch, 1);
 
       // Save to history
       if (line_length > 0) {
         line_buffer[line_length] = 0;
         if (history_length < MAX_HISTORY) {
           history[history_length++] = strdup(line_buffer);
           //printf("[DEBUG] Added to history: %s (index %d)\n", line_buffer, history_length - 1);
         }
       }
 
       browsing_history = -1; // reset history pointer
       break;
     }
     else if (ch == 31) {
       // ctrl-?
       read_line_print_usage();
       line_buffer[0] = 0;
       break;
     }
     else if (ch == 8 || ch == 127) {
       // <backspace> was typed. Remove previous character read.
       if (cursor_pos > 0) {
         cursor_pos--;
         for (int i = cursor_pos; i < line_length - 1; i++) {
           line_buffer[i] = line_buffer[i + 1];
         }
         line_length--;
 
         // Move cursor back
         ch = 8;
         write(1, &ch, 1);
 
         // Rewrite rest of line and clear tail
         write(1, &line_buffer[cursor_pos], line_length - cursor_pos);
         write(1, " ", 1);
 
         // Move cursor back to original pos
         for (int i = 0; i <= line_length - cursor_pos; i++) {
           ch = 8;
           write(1, &ch, 1);
         }
       }
     }
     else if (ch == 27) {
       // Escape sequence. Read two chars more
     
       //
       char ch1;
       char ch2;
       read(0, &ch1, 1);
       read(0, &ch2, 1);
 
       if (ch1 == 91 && ch2 == 65) {
         // Up arrow. Print previous line in history.
 
         if (history_length == 0) continue;
 
         if (browsing_history == -1) {
           browsing_history = history_length - 1;
         } else if (browsing_history > 0) {
           browsing_history--;
         }
 
         if (browsing_history < 0 || browsing_history >= history_length || history[browsing_history] == NULL)
           continue;
 
         // Erase current line
         while (cursor_pos > 0) {
           ch = 8;
           write(1, &ch, 1);
           cursor_pos--;
         }
         for (int i = 0; i < line_length; i++) {
           write(1, " ", 1);
         }
         for (int i = 0; i < line_length; i++) {
           ch = 8;
           write(1, &ch, 1);
         }
 
         // Load from history
         strcpy(line_buffer, history[browsing_history]);
         line_length = strlen(line_buffer);
         cursor_pos = line_length;
         write(1, line_buffer, line_length);
       }
       else if (ch1 == 91 && ch2 == 66) {
         // Down arrow. Go forward in history.
 
         if (browsing_history >= 0 && browsing_history < history_length - 1) {
           browsing_history++;
         } else {
           browsing_history = -1;
         }
 
         // Erase current line
         while (cursor_pos > 0) {
           ch = 8;
           write(1, &ch, 1);
           cursor_pos--;
         }
         for (int i = 0; i < line_length; i++) {
           write(1, " ", 1);
         }
         for (int i = 0; i < line_length; i++) {
           ch = 8;
           write(1, &ch, 1);
         }
 
         if (browsing_history >= 0 && browsing_history < history_length && history[browsing_history]!= NULL) {
           strcpy(line_buffer, history[browsing_history]);
           line_length = strlen(line_buffer);
           cursor_pos = line_length;
           write(1, line_buffer, line_length);
         } else {
           line_buffer[0] = '\0';
           line_length = 0;
           cursor_pos = 0;
         }
       }
       else if (ch1 == 91 && ch2 == 68) {
         // Left arrow
         if (cursor_pos > 0) {
           ch = 8;
           write(1, &ch, 1);
           cursor_pos--;
         }
       }
       else if (ch1 == 91 && ch2 == 67) {
         // Right arrow
         if (cursor_pos < line_length) {
           ch = line_buffer[cursor_pos];
           write(1, &ch, 1);
           cursor_pos++;
         }
       }
     }
     else if (ch == 4) {
       // Ctrl-D: Delete at cursor
       if (cursor_pos < line_length) {
         for (int i = cursor_pos; i < line_length - 1; i++) {
           line_buffer[i] = line_buffer[i + 1];
         }
         line_length--;
 
         // Rewrite rest of line
         write(1, &line_buffer[cursor_pos], line_length - cursor_pos);
         write(1, " ", 1);
 
         // Move cursor back
         for (int i = 0; i <= line_length - cursor_pos; i++) {
           ch = 8;
           write(1, &ch, 1);
         }
       }
     }
     else if (ch == 1) {
       // Ctrl-A: Move to beginning of line
       while (cursor_pos > 0) {
         ch = 8;
         write(1, &ch, 1);
         cursor_pos--;
       }
     }
     else if (ch == 5) {
       // Ctrl-E: Move to end of line
       while (cursor_pos < line_length) {
         ch = line_buffer[cursor_pos];
         write(1, &ch, 1);
         cursor_pos++;
       }
     }else if(ch == 9) {
       // Ctrl-I: Tab completion
       if (last_char_tab) {
        complete_path(line_buffer, &line_length, &cursor_pos, 1); // list all
        last_char_tab = 0;
      } else {
        complete_path(line_buffer, &line_length, &cursor_pos, 0); // complete shared prefix
        last_char_tab = 1;
      }
    } else {
      last_char_tab = 0;
    }
   }
 
   // Add eol and null char at the end of string
   line_buffer[line_length] = 10;
   line_length++;
   line_buffer[line_length] = 0;
 
   return line_buffer;
 }



 
