#define _GNU_SOURCE
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#define min(a, b) a < b ? a : b

const int max_password_value = 500000;
const char cmd_format[300] = "unzip -P%d -t %s 2>&1";

FILE *popen(const char *command, const char *type);

int main(int argc, char **argv) {
  FILE *fp;
  FILE *input;
  char filename[100];
  char ret[200];
  char cmd[400];
  double t;
  int nt, i, blocks, tasks, j;

  if (argc < 2) {
    fprintf(stderr, "Error: missing path to input file\n");
    return 1;
  }

  if ((input = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "Error: could not open file\n");
    return 1;
  }

  // Read inputs
  fscanf(input, "%d", &nt);
  fscanf(input, "%s", filename);

  // Do not touch this line
  omp_set_num_threads(nt);

//  tasks = max_password_value/nt;
  tasks = 5000;
  blocks = (tasks - 1 + max_password_value)/tasks;
  t = omp_get_wtime(); 
  // Parallelize this loop using tasks!
#pragma omp parallel 
#pragma omp single 
  {
    for (i = 0; i < blocks; ++i) {
#pragma omp task firstprivate(i) private(cmd, fp, ret, j) shared(filename, tasks)
      {
//        printf("From %d to %d\n", i * tasks, min(max_password_value, (i + 1) * tasks));
      for (j = i * tasks; j < min(max_password_value, (i + 1) * tasks); ++j) {
        sprintf((char *)&cmd, cmd_format, j, filename);

        fp = popen(cmd, "r");
        while (!feof(fp)) {
          fgets((char *)&ret, 200, fp);
          if (strcasestr(ret, "ok") != NULL) {
            printf("Password: %d\n", j);
            t = omp_get_wtime() - t;
            fprintf(stderr, "%lf\n", t);
            exit(0);
          }
        }
        pclose(fp);
      }
      }
    }
  }


  t = omp_get_wtime() - t;
  fprintf(stderr, "%lf\n", t);
}
