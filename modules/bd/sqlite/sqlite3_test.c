#include "sqlite3.h"
#include <stdio.h>
#include <string.h>

#define DEMO_DB     "./demo.db"
#define DEMO_TABLE  "emp"

int sqlite3_db_select()
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char sql[1024];
    int ret, ncols, i, rows = 0;

    ret = sqlite3_open(DEMO_DB, &db);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db open fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select empno, ename, job, mgr, hiredate, sal, comm, deptno from %s where 1 = 1", DEMO_TABLE);
    printf("sql select: \"%s\"\n", sql);

    /*也可以用sqlite3_get_table*/

    ret = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db prepare fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW && ret != SQLITE_DONE) {
        fprintf(stderr, "db step fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    ncols = sqlite3_column_count(stmt);
    //获取某列的数据
    for(i=0; i < ncols; i++)  {
        printf("column name=%s, storage class=%i, declared=%s\n",
                sqlite3_column_name(stmt, i),
                sqlite3_column_type(stmt, i),
                sqlite3_column_decltype(stmt, i));
    }
    printf("\n");

    for ( ; ; ) {
        if (ret == SQLITE_ROW) {
            int empno, mgr, sal, comm, deptno;
            char *ename, *job, *hiredate;

            /* 索引从0开始 */ //获取数据
            empno = sqlite3_column_int(stmt, 0);
            ename = (char *)sqlite3_column_text(stmt, 1);
            job = (char *)sqlite3_column_text(stmt, 2);
            mgr = sqlite3_column_int(stmt, 3);
            hiredate = (char *)sqlite3_column_text(stmt, 4);
            sal = sqlite3_column_int(stmt, 5);
            comm = sqlite3_column_int(stmt, 6);
            deptno = sqlite3_column_int(stmt, 7);

            printf("empno=%d, ename=%s, job=%s, mgr=%d, hiredate=%s, sal=%d, comm=%d, deptno=%d\n",
                    empno, ename, job, mgr, hiredate, sal, comm, deptno);
            rows++;
        } else if (ret == SQLITE_DONE) {
            printf("select done!\n");
            break;
        } else {
            fprintf(stderr, "db step fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
            break;
        }

        ret = sqlite3_step(stmt);
    }

    printf("rows=%d\n", rows);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int sqlite3_db_get_rows()
{
    sqlite3 *db = NULL;
    char sql[1024];
    sqlite3_stmt *stmt = NULL;
    int ret, rows = 0;

    ret = sqlite3_open(DEMO_DB, &db);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db open fail, errcode[%d]\n", ret);
        return -1;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select count(1) from %s", DEMO_TABLE);
    ret = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db prepare fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    ret = sqlite3_step(stmt);
    if (ret == SQLITE_ROW)  {
        rows = sqlite3_column_int(stmt, 0);
    } else if (ret == SQLITE_DONE) {
        printf("no data!\n");
        rows = 0;
    } else {
        fprintf(stderr, "db step fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        rows = -1;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    printf("rows=%d\n", rows);
    return rows;
}

int sqlite3_db_begin_transaction(sqlite3 *db)
{
    char sql[1024];
    char *errmsg = NULL;
    int ret = 0;

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "BEGIN TRANSACTION");
    ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "begin transaction fail, errcode[%d], errmsg[%s]\n", ret, errmsg);
        sqlite3_free(errmsg);
        errmsg = NULL;
        return -1;
    }

    return 0;
}

int sqlite3_db_commit_transaction(sqlite3 *db)
{
    char sql[1024];
    char *errmsg = NULL;
    int ret = 0;

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "COMMIT TRANSACTION");

    ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "commit transaction fail, errcode[%d], errmsg[%s]\n", ret, errmsg);
        sqlite3_free(errmsg);
        errmsg = NULL;
        return -1;
    }

    return 0;
}

int sqlite3_db_rollback_transaction(sqlite3 *db)
{
    char sql[1024];
    char *errmsg = NULL;
    int ret = 0;

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "ROLLBACK TRANSACTION");

    ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "rollback transaction fail, errcode[%d], errmsg[%s]\n", ret, errmsg);
        sqlite3_free(errmsg);
        errmsg = NULL;
        return -1;
    }

    sqlite3_close(db);
    return 0;
}

int sqlite3_db_delete()
{
    sqlite3 *db = NULL;
    char sql[1024];
    char *errmsg = NULL;
    int ret = 0;

    ret = sqlite3_open(DEMO_DB, &db);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db open fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "delete from %s where empno = 7566", DEMO_TABLE);
    printf("sql delete: \"%s\"\n", sql);
    ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "delete fail, errcode[%d], errmsg[%s]\n", ret, errmsg);
        sqlite3_free(errmsg);
        errmsg = NULL;
        sqlite3_close(db);
        return -1;
    }

    sqlite3_close(db);
    return 0;
}

int sqlite3_db_bind_insert()
{
    sqlite3 *db = NULL;
    char sql[1024];
    sqlite3_stmt *stmt = NULL;
    int ret, i, rv = 0;

    ret = sqlite3_open(DEMO_DB, &db);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db open fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    /*在对 sqlite3 insert into 等操作时速度比较慢。

原因：它以文件的形式存在磁盘中，每次访问时都要打开一次文件，如果对数据库进行大量的操作，
就很慢。

解决办法：用事物的形式提交，因为开始事务后，进行的大量操作语句都保存在内存中，
当提交时才全部写入数据库，此时，数据库文件也只用打开一次。如果操作错误，还可以回滚事务。
*/
    /* 开启事务 */
    ret = sqlite3_db_begin_transaction(db);
    if (ret < 0) {
        fprintf(stderr, "begin transaction fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "insert into %s (empno, ename, job, mgr, hiredate, sal, comm, deptno) values(?, ?, ?, ?, ?, ?, ?, ?)", DEMO_TABLE);
    printf("sql bind insert: \"%s\"\n", sql);

    ret = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db prepare fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    /*
        不确定数据。开始先以??插入。后面绑定数据。
    */
    for (i=0; i<100; i++) {
        /* 索引从1开始 */
        sqlite3_bind_int(stmt, 1, 7521);
        sqlite3_bind_text(stmt, 2, "WARD", strlen("WARD"), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, "SALESMAN", strlen("SALESMAN"), SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, 7698);
        sqlite3_bind_text(stmt, 5, "22-02-1981", strlen("22-02-1981"), SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, 1250);
        sqlite3_bind_null(stmt, 7);
        sqlite3_bind_int(stmt, 8, 30);

        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            fprintf(stderr, "db step fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
            break;
        }
        //在调用 sqlite3_step之后执行 充值sql语句。便于后面重新赋值
        ret = sqlite3_reset(stmt);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "db reset fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
            break;
        }
    }

    /* 提交事务 */
    ret = sqlite3_db_commit_transaction(db);
    if (ret < 0) {
        fprintf(stderr, "commit transaction fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    ret = sqlite3_finalize(stmt);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db finalize fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    sqlite3_close(db);
    return 0;
}

int sqlite3_db_insert()
{
    sqlite3 *db = NULL;
    char sql[1024];
    char *errmsg = NULL;
    int ret = 0;

    ret = sqlite3_open(DEMO_DB, &db);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db open fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "insert into %s (empno, ename, job, mgr, hiredate, sal, comm, deptno) values(%d, '%s', '%s', %d, '%s', %d, %d, %d)",
                  DEMO_TABLE, 7566, "JONES", "MANAGER", 7839, "02-04-1981", 2975, 500 , 20);
    printf("sql insert: \"%s\"\n", sql);
    ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db insert fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_free(errmsg);
        errmsg = NULL;
        sqlite3_close(db);
        return -1;
    }

    sqlite3_close(db);
    return 0;
}

int sqlite3_db_table_exist(char *dbfile, const char *tablename)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char sql[1024];
    int ret = 0, rows = 0;

    ret = sqlite3_open(dbfile, &db);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db open fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select count(1) from sqlite_master where type='table' and name='%s'", tablename);
    //将sql文本转换成一个准备语句。并不执行
    ret = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db prepare fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    //执行前面sqlite3_prepare创建的准备语句
    ret = sqlite3_step(stmt);
    if (ret == SQLITE_ROW)  {
            //没执行一次sqlite3_step 执行一个准备语句得到的结果集的当前行中返回一个列
        rows = sqlite3_column_int(stmt, 0);
    } else if (ret == SQLITE_DONE) {
        printf("no data!\n");
        rows = 0;
    } else {
        fprintf(stderr, "db step fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        rows = 0;
    }
    //每个准备语句。必须执行一次这个。用于销毁
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (rows > 0) {
        return 1;
    } else {
        return 0;
    }
}

int sqlite3_db_create()
{
    sqlite3 *db = NULL;
    char sql[1024];
    char *errmsg = NULL;
    int ret = 0;

    if (sqlite3_db_table_exist(DEMO_DB, DEMO_TABLE)) {
        printf("table exist\n");
        return 0;
    }

    ret = sqlite3_open(DEMO_DB, &db);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "db open fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "create table %s ("\
                 "empno     int(4) not null, "\
                 "ename     varchar(10), "\
                 "job       varchar(9), "\
                 "mgr       int(4), "\
                 "hiredate  date, "\
                 "sal       int(7), "\
                 "comm      int(7), "\
                 "deptno    int(2))", DEMO_TABLE);

    printf("sql create: \"%s\"\n", sql);
    //以;分割。执行sql语句
    ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "create table fail, errcode[%d], errmsg[%s]\n", ret, sqlite3_errmsg(db));
        sqlite3_free(errmsg);
        errmsg = NULL;
        sqlite3_close(db);
        return -1;
    }

    sqlite3_close(db);
    return 0;
}

int main()
{
    sqlite3_db_create();
    sqlite3_db_insert();
    sqlite3_db_bind_insert();
    sqlite3_db_get_rows();
    sqlite3_db_select();
    sqlite3_db_delete();
    return 0;
}
