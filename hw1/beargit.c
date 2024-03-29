#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void)
{
  fs_mkdir(".beargit");

  FILE *findex = fopen(".beargit/.index", "w");
  fclose(findex);

  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");

  return 0;
}

/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char *filename)
{
  FILE *findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while (fgets(line, sizeof(line), findex))
  {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0)
    {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char *filename)
{
  /* COMPLETE THE REST */

  FILE *findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");
  int doesNameExist = 0;
  char line[FILENAME_SIZE];
  while (fgets(line, sizeof(line), findex))
  {
    strtok(line, "\n");
    if (strcmp(line, filename) != 0)
    {
      fprintf(fnewindex, "%s\n", line);
    }
    else
    {
      doesNameExist = 1;
    }
  }
  fclose(findex);
  fclose(fnewindex);
  fs_mv(".beargit/.newindex", ".beargit/.index");

  if (doesNameExist)
  {
    return 0;
  }
  fprintf(stderr, "ERROR: File %s not tracked\n", filename);
  return 1;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char *go_bears = "GO BEARS!";

int is_commit_msg_ok(const char *msg)
{
  char *ret;
  ret = strstr(msg, go_bears);
  if (ret)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
void next_commit_id(char *commit_id)
{
  /* COMPLETE THE REST */
  char lastChar;
  int lastCharIndex;
  int i;
  for (i = 0; i < strlen(commit_id); i = i + 1)
  {

    if (commit_id[i] == '0' && !lastChar)
    {
      if (i == 0)
      {
        lastChar = commit_id[i];
        lastCharIndex = i;
      }
      else
      {
        lastChar = commit_id[i - 1];
        lastCharIndex = i;
      }
    }
  }
  if (lastChar == '0')
    commit_id[0] = '6';
  else
  {
    char new_commit_id[COMMIT_ID_SIZE];
    strcpy(new_commit_id, commit_id);
    // probably should iterate but its 3 vals
    if (lastChar == '6')
      new_commit_id[(lastCharIndex - 1)] = '1';
    if (lastChar == '1')
      new_commit_id[(lastCharIndex - 1)] = 'c';
    if (lastChar == 'c')
      new_commit_id[lastCharIndex] = '6';
    strcpy(commit_id, new_commit_id);
  }
}

int beargit_commit(const char *msg)
{
  if (!is_commit_msg_ok(msg))
  {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);

  char commit_id_path[COMMIT_ID_SIZE + 10];
  strcpy(commit_id_path, ".beargit/");
  strcat(commit_id_path, commit_id);
  printf("%s", commit_id_path);
  fs_mkdir(commit_id_path);
  // copy index and prev into commit path
  char copy_path[COMMIT_ID_SIZE + FILENAME_SIZE];
  strcpy(copy_path, commit_id_path);
  strcat(copy_path, "/.index");
  fs_cp(".beargit/.index", copy_path);
  strcpy(copy_path, commit_id_path);
  strcat(copy_path, "/.prev");
  fs_cp(".beargit/.prev", copy_path);

  FILE *fp;
  char buff[FILENAME_SIZE];
  fp = fopen("./.beargit/.index", "r");

  while (fscanf(fp, "%s", buff) == 1)
  {
    strcpy(copy_path, commit_id_path);
    strcat(copy_path, "/");
    strcat(copy_path, buff);
    char from_path[FILENAME_SIZE + 3];
    strcpy(from_path, "./");
    strcat(from_path, buff);

    fs_cp(from_path, copy_path);
  }
  char msg_path[COMMIT_ID_SIZE + FILENAME_SIZE];
  strcpy(msg_path, commit_id_path);
  strcat(msg_path, "/.msg");
  FILE *findex = fopen(msg_path, "w");
  fclose(findex);
  write_string_to_file(msg_path, msg);

  write_string_to_file("./.beargit/.prev", commit_id);
  return 0;
}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int beargit_status()
{
  /* COMPLETE THE REST */
  printf("Tracked Files: \n\n");
  FILE *fp;
  char buff[FILENAME_SIZE];

  fp = fopen("./.beargit/.index", "r");

  while (fscanf(fp, "%s", buff) == 1)
  {
    printf("%s\n", buff);
  }
  fclose(fp);
  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 *
 */

int beargit_log()
{
  /* COMPLETE THE REST */
  char msg[MSG_SIZE];
  char commit_id[COMMIT_ID_SIZE];
  char commit_id_path[COMMIT_ID_SIZE + 20];
  char commit_id_file_path[COMMIT_ID_SIZE + 30];
  read_string_from_file("./.beargit/.prev", commit_id, COMMIT_ID_SIZE);
  if (commit_id[0] == '0')
  {
    fprintf(stderr, "ERROR: There are no commits!\n");
    return 1;
  }
  strcpy(commit_id_path, "./.beargit/");
  strcat(commit_id_path, commit_id);

  strcpy(commit_id_file_path, commit_id_path);
  strcat(commit_id_file_path, "/");
  strcat(commit_id_file_path, ".msg");

  read_string_from_file(commit_id_file_path, msg, MSG_SIZE);

  while (commit_id[0] != '0')
  {
    printf("\n");
    printf("commit %s\n", commit_id);
    printf("%s\n", msg);
    strcpy(commit_id_file_path, commit_id_path);
    strcat(commit_id_file_path, "/");
    strcat(commit_id_file_path, ".prev");
    read_string_from_file(commit_id_file_path, commit_id, COMMIT_ID_SIZE);
    if (commit_id[0] != '0')
    {
      strcpy(commit_id_path, "./.beargit/");
      strcat(commit_id_path, commit_id);

      strcpy(commit_id_file_path, commit_id_path);
      strcat(commit_id_file_path, "/");
      strcat(commit_id_file_path, ".msg");
      read_string_from_file(commit_id_file_path, msg, MSG_SIZE);
    }
  }
  return 0;
}
