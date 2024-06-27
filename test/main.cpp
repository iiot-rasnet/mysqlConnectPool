#include <iostream>
#include <memory>
#include "MysqlConn.h"
#include "ConnectionPool.h"
using namespace std;
//单线程：使用连接池/不使用
//2.多线程：使用连接池/不使用
void op1(int begin, int end) {
	for (int i = begin; i < end; ++i) {
		MysqlConn conn;
		conn.connect("root", "2000221Rhs-", "testdb", "59.67.152.133", 3306);
		char sql[1024] = { 0 };
		sprintf(sql, "insert into person values(%d,25,'man','tom')", i);
		conn.update(sql);
	}
}
void op2(ConnectionPool * pool, int begin, int end) {
	for (int i = begin; i < end; ++i) {
		shared_ptr<MysqlConn>conn=pool->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into person values(%d,25,'man','tom')", i);
		conn->update(sql);
	}
}
void test1() {
#if 0
	//非连接池，单线程，用时：3596214000纳秒3596毫秒
	steady_clock::time_point begin = steady_clock::now();

	op1(0, 500);
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "非连接池，单线程，用时：" << length.count() << "纳秒"<< length.count() / 1000000 << endl;
#else
	//连接池，单线程，用时：296180500纳秒296毫秒
	ConnectionPool* pool=ConnectionPool::getConnetPool();
	steady_clock::time_point begin = steady_clock::now();
	op2( pool,0, 500);
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "连接池，单线程，用时：" << length.count() << "纳秒" << length.count() / 1000000 <<"毫秒"<< endl;
#endif
}
void test2() {
#if 1//非连接池，多线程，用时：14248916100纳秒14248秒
	MysqlConn conn;
	conn.connect("root", "2000221Rhs-", "testdb", "59.67.152.133", 3306);
	steady_clock::time_point begin = steady_clock::now();
	thread t1(op1, 0, 1000);
	thread t2(op1, 1000, 2000);
	thread t3(op1, 2000, 3000);
	thread t4(op1, 3000, 4000);
	thread t5(op1, 4000, 5000);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "非连接池，多线程，用时：" << length.count() << "纳秒" <<
		length.count()/1000000<<endl;
#else //连接池，多线程，用时：1797915200纳秒1797毫秒
	ConnectionPool* pool = ConnectionPool::getConnetPool();
	steady_clock::time_point begin = steady_clock::now();
	thread t1(op1, pool, 0, 1000);
	thread t2(op1, pool,1000, 2000);
	thread t3(op1, pool, 2000, 3000);
	thread t4(op1, pool, 3000, 4000);
	thread t5(op1, pool, 4000, 5000);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "连接池，多线程，用时：" << length.count() << "纳秒" <<
		length.count() / 1000000 << endl;
#endif
}
int query() {
	MysqlConn conn;
	conn.connect("root","2000221Rhs-","testdb","59.67.152.133",3306 );
	/*cout << open << endl;*/
	string sql = "insert into person values(6,25,'man','tom')";
	bool flag=conn.update(sql);
	cout << "flag value:" << flag << endl;
	sql = "select * from person";
	conn.query(sql);
	while (conn.next()) {
		cout << conn.value(0) << ","
			<< conn.value(1) << ","
			<< conn.value(2) << ","
			<< conn.value(3) << endl;
	}
	return 0;
}

int main() {

	query();
	//test1();
	//test2();
	return 0;
}