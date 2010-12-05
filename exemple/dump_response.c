#include <stdio.h>

#include <mysac.h>

void dump_response(MYSAC_RES *r) 
{
	int i;
	MYSAC_ROW *rr;
	char buf[1024];
	struct tm tm;

	/* print headers */
	printf("|");
	for (i=0; i<r->nb_cols; i++)
		printf("%s|", r->cols[i].name);
	printf("\n");

	/* print data */
	mysac_first_row(r);

	while ((rr = mysac_fetch_row(r)) != NULL) {

		printf("|");

		for (i=0; i<r->nb_cols; i++) {

			switch(r->cols[i].type) {

			/* apply offset on data pointer */
			case MYSQL_TYPE_TIME:
				localtime_r(&rr[i].tv.tv_sec, &tm);
				strftime(buf, 1024, "%Y-%m-%d %H:%M:%S", &tm);
				printf("%s.%06d|", buf, (int)rr[i].tv.tv_usec);
				break;

			case MYSQL_TYPE_YEAR:
			case MYSQL_TYPE_TIMESTAMP:
			case MYSQL_TYPE_DATETIME:
			case MYSQL_TYPE_DATE:
				strftime(buf, 1024, "%Y-%m-%d %H:%M:%S", rr[i].tm);
				printf("%s|", buf);
				break;

			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VARCHAR:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_TINY_BLOB:
			case MYSQL_TYPE_MEDIUM_BLOB:
			case MYSQL_TYPE_LONG_BLOB:
			case MYSQL_TYPE_BLOB:
				printf("%s|", rr[i].string);
				break;

			/* do nothing for other types */
			case MYSQL_TYPE_TINY:
				printf("%d|", rr[i].stiny);
				break;

			case MYSQL_TYPE_SHORT:
				printf("%d|", rr[i].ssmall);
				break;

			case MYSQL_TYPE_LONG:
				printf("%d|", rr[i].uint);
				break;

			case MYSQL_TYPE_LONGLONG:
				printf("%lld|", rr[i].sbigint);
				break;

			case MYSQL_TYPE_FLOAT:
				printf("%f|", rr[i].mfloat);
				break;

			case MYSQL_TYPE_DOUBLE:
				printf("%f|", rr[i].mdouble);
				break;


			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_NULL:
			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_NEWDATE:
			case MYSQL_TYPE_BIT:
			case MYSQL_TYPE_NEWDECIMAL:
			case MYSQL_TYPE_ENUM:
			case MYSQL_TYPE_SET:
			case MYSQL_TYPE_GEOMETRY:
				printf("unsuported|");
				break;
			}
		}

		printf("\n");

	}

}
