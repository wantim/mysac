#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <mysac.h>

#include "dump_response.h"

#define BUFS (1024*1024)
char buf[BUFS];
char res[BUFS];
char _q[BUFS];

static inline int wait(fd, code) {
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	if (code == MYERR_WANT_WRITE)
		select(fd+1, NULL, &fds, NULL, NULL);
	else if (code == MYERR_WANT_READ)
		select(fd+1, &fds, NULL, NULL, NULL);
	else
		return 0;
	return 1;
}

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

int main(int argc, char *argv[]) {
	int ret_code;
	int len;
	int i;
	char *q;
	struct timeval start, stop, diff;
	MYSAC my;
	MYSAC_RES *r;
	const char *host = NULL;
	const char *login = NULL;
	const char *pass = NULL;
	const char *db = NULL;

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

	/* check */
	if (host == NULL) {
		fprintf(stderr, "host expected\n");
		usage(argv[0]);
	}
	if (login == NULL) {
		fprintf(stderr, "login expected\n");
		usage(argv[0]);
	}
	if (pass == NULL) {
		fprintf(stderr, "password expected\n");
		usage(argv[0]);
	}
	if (db == NULL) {
		fprintf(stderr, "database expected\n");
		usage(argv[0]);
	}

	/* check */
	if (argc != 10) {
		fprintf(stderr, "query expected\n");
		usage(argv[0]);
	}

	/* init memory */
	mysac_init(&my, buf, BUFS);

	/* init connection */
	mysac_setup(&my, host, login, pass, db, 0);
	while (1) {
		ret_code = mysac_connect(&my);
		if (!wait(mysac_get_fd(&my), ret_code))
			break;
	}
	if (ret_code != 0) {
		fprintf(stderr, "mysac_connect error %d: %s\n", ret_code, mysac_advance_error(&my));
		exit(1);
	}

	/* choose database */
	mysac_set_database(&my, db);
	while (1) {
		ret_code = mysac_send_database(&my);
		if (!wait(mysac_get_fd(&my), ret_code))
			break;
	}
	if (ret_code != 0) {
		fprintf(stderr, "mysac_send_database error %d: %s\n", ret_code, mysac_advance_error(&my));
		exit(1);
	}

	/* send flush */
	r = mysac_init_res(res, BUFS);
	mysac_set_query(&my, r, "RESET QUERY CACHE;");

	/* send query to database */
	while (1) {
		ret_code = mysac_send_query(&my);
		if (!wait(mysac_get_fd(&my), ret_code))
			break;
	}

	/* la reponse est arrivée */
	if (ret_code != 0) {
		fprintf(stderr, "mysac_send_query(RESET QUERY CACHE) error %d: %s\n",
		        ret_code, mysac_advance_error(&my));
		exit(1);
	}

	/* read request from stdin */
	if (strcmp(argv[9], "-")==0) {
		len = fread(_q, 1, BUFS, stdin);
		q = _q;
	}
	else {
		q = argv[9];
		len = strlen(q);
	}

	/* declare request */
	r = mysac_new_res(1024, 1);
	mysac_b_set_query(&my, r, q, len);

	/* get time */
	gettimeofday(&start, NULL);

	/* send query to database */
	while (1) {
		ret_code = mysac_send_query(&my);
		if (!wait(mysac_get_fd(&my), ret_code))
			break;
	}

	/* la reponse est arrivée */
	if (ret_code != 0) {
		fprintf(stderr, "mysac_send_query error %d: %s\n", ret_code, mysac_advance_error(&my));
		exit(1);
	}

	/* get time */
	gettimeofday(&stop, NULL);

	/* display */
	diff.tv_sec  = stop.tv_sec  - start.tv_sec;
	diff.tv_usec = stop.tv_usec - start.tv_usec;
	if (diff.tv_usec < 0) {
		diff.tv_sec--;
		diff.tv_usec += 1000000;
	}

	/* update resource */
	r = mysac_get_res(&my);

	/* display data */
	dump_response(r);

	printf("%lu rows in set (%d.%06d s)\n", mysac_num_rows(r),
	       (int)diff.tv_sec, (int)diff.tv_usec);
	
	exit(0);
}
