#pragma once
#include "Resource.h"
#include "IO/MemoryStream.h"

struct sqlite3;
struct sqlite3_stmt;

DECLARE_DELEGATE_RET(DatabaseExecuteDelegate, int, void*, int, char**, char**);

class Database;

struct PreparedStatement
{
public:
	PreparedStatement(Database* pDb, const std::string& query);
	~PreparedStatement();

	bool Step();
	bool Reset(bool resetParameters);
	bool ResetParameters();
	void GetColumn(int index, const char** pText) const;
	void GetColumn(int index, int* pValue) const;
	void GetColumn(int index, float* pValue) const;
	const int Columns() const;
	const char* GetColumnName(int index) const;

	bool BindParameter(const int index, const std::string& value);
	bool BindParameter(const int index, const int value);
	bool BindParameter(const int index, const float value);
	bool CompleteBinding();

private:
	enum class State
	{
		BindState,
		BindComplete,
		Stepping,
		Done,
	};

	friend class Database;

	int m_ParameterCount = 0;
	int m_ParametersSet = 0;

	int m_CurrentIndex = 0;
	sqlite3_stmt* pStatement = nullptr;
	Database* m_pDatabase = nullptr;
	State m_State = State::BindState;
	std::string m_Query;
};

class Database : public Resource
{
	FLUX_OBJECT(Database, Resource);

public:
	Database(Context* pContext);
	~Database();

	virtual bool Load(InputStream& inputStream) override;

	PreparedStatement Prepare(const std::string query);
	bool Execute(const std::string& query, const DatabaseExecuteDelegate& callback);

	std::string GetLastError();
	sqlite3* GetResource() const { return m_pDatabase; }

	void Close();

private:
	static void RegisterVFS();

	sqlite3* m_pDatabase = nullptr;
	std::vector<unsigned char> m_Data;
	MemoryStream m_Stream;
};