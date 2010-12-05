#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <mysac.h>

#define BUFS (1024*1024)
char buf[BUFS];
char res[BUFS];
char _q[BUFS];

void usage(char *cmd) {
	fprintf(stderr,
		"usage: %s -h host -l login -p pass -d db {-|query}\n"
		"  host  : \"path\" or \"<ip>:<port>\"\n"
		"  login : database login\n"
		"  pass  : database password\n"
		"  db    : database\n"
		"  -     : read request on stdin\n"
		"	query : SQL request\n",
		cmd
	);
	exit(1);
}

int run_io(MYSAC *my) {
	fd_set fds;
	int ret_code;
	int fd;
	const char *func;

	while (1) {
		ret_code = mysac_io(my);

		if (ret_code == MYERR_WANT_WRITE) {
			fd = mysac_get_fd(my);
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			select(fd+1, NULL, &fds, NULL, NULL);
		}

		else if (ret_code == MYERR_WANT_READ) {
			fd = mysac_get_fd(my);
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			select(fd+1, &fds, NULL, NULL, NULL);
		}

		else if (ret_code != 0) {

			/* resolve io function */
			if (my != NULL) {
				if (my->call_it == NULL)
					func = "no function";
				else if (my->call_it == mysac_send_database)
					func = "mysac_send_database";
				else if (my->call_it == mysac_send_query)
					func = "mysac_send_query";
				else if (my->call_it == mysac_connect)
					func = "mysac_connect";
				else
					func = "unknown function";
			}

			fprintf(stderr, "mysac_io error into \"%s\" %d: %s\n",
			        func, ret_code, mysac_advance_error(my));
			exit(1);
		}

		else 
			return ret_code;
	}
}

int main(int argc, char *argv[]) {
	int len;
	int i;
	char *q;
	struct timeval start, stop, diff;
	MYSAC my;
	MYSAC_RES *r;
	const char *host;
	const char *login;
	const char *pass;
	const char *db;

	/* check */
	if (argc != 10)
		usage(argv[0]);

	/* read cmd line */
	for (i=1; i<argc; i++) {
		if (strcmp(argv[i], "-h")==0) {
			i++;
			host = argv[i];
		}
		else if (strcmp(argv[i], "-l")==0) {
			i++;
			login = argv[i];
		}
		else if (strcmp(argv[i], "-p")==0) {
			i++;
			pass = argv[i];
		}
		else if (strcmp(argv[i], "-d")==0) {
			i++;
			db = argv[i];
		}
	}

	/* init memory */
	mysac_init(&my, buf, BUFS);

	/* init connection */
	mysac_setup(&my, host, login, pass, db, 0);
	run_io(&my);

	/* choose database */
	mysac_set_database(&my, db);
	run_io(&my);

	/***************************************
	 * send query cache reset
	 ***************************************/

	/* init resource */
	r = mysac_init_res(res, BUFS);

	/* send query */
	mysac_set_query(&my, r, "RESET QUERY CACHE;");
	run_io(&my);

	/* read request from stdin */
	if (strcmp(argv[9], "-")==0) {
		len = fread(_q, 1, BUFS, stdin);
		q = _q;
	}
	else {
		q = argv[9];
		len = strlen(q);
	}

	/***************************************
	 * declare request
	 ***************************************/

	/* init resource */
	r = mysac_init_res(res, BUFS);

	/* prepare query */
	mysac_b_set_query(&my, r, q, len);

	/* get time */
	gettimeofday(&start, NULL);

	/* send query to database */
	run_io(&my);

	/* get time */
	gettimeofday(&stop, NULL);

	/* display */
	diff.tv_sec  = stop.tv_sec  - start.tv_sec;
	diff.tv_usec = stop.tv_usec - start.tv_usec;
	if (diff.tv_usec < 0) {
		diff.tv_sec--;
		diff.tv_usec += 1000000;
	}
	printf("%lu rows in set (%d.%06d s)\n", mysac_num_rows(r),
	       (int)diff.tv_sec, (int)diff.tv_usec);

	exit(0);
}
