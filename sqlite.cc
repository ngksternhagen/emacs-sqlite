#include "sqlite.hpp"
#include <string>
#include <exception>


sqlite3* SQLiteConnection::_connection = nullptr;

SQLiteConnection::SQLiteConnection(){
}

SQLiteConnection::~SQLiteConnection(){
}

SQLiteConnection::SQLiteConnection(const std::string& file){
  if(_connection != nullptr){
    throw SQLException("Connection already open");
  }
  open(file);
}

void SQLiteConnection::open(const std::string& fp){
  if(_connection != nullptr){
    /* close existing connection */
    close();
  }
  int ret = sqlite3_open_v2(fp.c_str(), &_connection,
			    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if(ret != SQLITE_OK){
    throw SQLException("Connection error: "+ std::string(sqlite3_errmsg(_connection)));
  }
}

void SQLiteConnection::close(){
  if(_connection != nullptr){
    sqlite3_close(_connection);
  }
  _connection = nullptr;
}

SQLiteQuery::SQLiteQuery(){
  if(SQLiteConnection::get_instance() == nullptr){
    throw SQLException("No connection available");
  }
}


SQLiteQuery::SQLiteQuery(std::string query){
  if((_connection = SQLiteConnection::get_instance()) == nullptr){
    throw SQLException("No connection available");
  }

  sqlite3_stmt* statement;
  int code = sqlite3_prepare_v2(_connection, query.c_str(), query.size(), &statement, 0);
  if(code != SQLITE_OK){
    throw SQLException("Query error: "+ std::string(sqlite3_errmsg(_connection)));
  }

  while(1){
    code = sqlite3_step(statement);
    if(code == SQLITE_ROW){
      int columns = sqlite3_column_count(statement);
      row_t row;
      row.reserve(columns);
      for(int i = 0; i < columns; i++){
	std::string value = reinterpret_cast<const char*>(sqlite3_column_text(statement, i));
	row.push_back(value);
      }
      if(row.size() > 0) query_data.push_back(row);
    }
    else break;
  }

  if(code != SQLITE_DONE){
    throw SQLException(std::string(sqlite3_errmsg(_connection)));
  }

  if(sqlite3_finalize(statement) != SQLITE_OK)
    throw SQLException(std::string(sqlite3_errmsg(_connection)));
}

SQLiteQuery::~SQLiteQuery(){

}