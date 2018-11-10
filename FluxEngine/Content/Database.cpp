#include "FluxEngine.h"
#include "Database.h"

#include <sqlite3.h>
#pragma comment(lib, "sqlite3.lib")

struct SQLiteFile : public sqlite3_file
{
	MemoryStream* pFile = nullptr;
};

struct SQLiteVFS : public sqlite3_vfs
{
	MemoryStream* pMemStream;
};
SQLiteVFS* g_pVfs = nullptr;

bool CheckError(int code)
{
	if (code > SQLITE_OK && code < SQLITE_ROW)
	{
		FLUX_LOG(Warning, "[SQLite Error] %s", sqlite3_errstr(code));
		return false;
	}
	return true;
}

namespace SQLiteFunctions
{
	static int Close(sqlite3_file* /*pFile*/)
	{
		return SQLITE_OK;
	}

	static int Read(sqlite3_file* pFile, void* zBuf, int iAmt, sqlite_int64 iOfst)
	{
		SQLiteFile *p = (SQLiteFile*)pFile;
		p->pFile->SetPointer((size_t)iOfst);
		p->pFile->Read(zBuf, iAmt);
		return SQLITE_OK;
	}

	static int Write(sqlite3_file* /*pFile*/, const void* /*zBuf*/, int /*iAmt*/, sqlite_int64 /*iOfst*/)
	{
		checkf(false, "Write is not supported for this VFS");
		return SQLITE_OK;
	}

	static int Truncate(sqlite3_file* /*pFile*/, sqlite_int64 /*size*/)
	{
		checkf(false, "Truncate is not supported for this VFS");
		return SQLITE_OK;
	}

	static int Sync(sqlite3_file* /*pFile*/, int /*flags*/)
	{
		return SQLITE_OK;
	}

	static int FileSize(sqlite3_file *pFile, sqlite_int64 *pSize)
	{
		SQLiteFile *p = (SQLiteFile*)pFile;
		*pSize = p->pFile->GetSize();
		return SQLITE_OK;
	}

	static int Lock(sqlite3_file* /*pFile*/, int /*eLock*/)
	{
		return SQLITE_OK;
	}

	static int Unlock(sqlite3_file* /*pFile*/, int /*eLock*/)
	{
		return SQLITE_OK;
	}

	static int CheckReservedLock(sqlite3_file* /*pFile*/, int* pResOut)
	{
		*pResOut = 0;
		return SQLITE_OK;
	}

	static int FileControl(sqlite3_file* /*pFile*/, int /*op*/, void* /*pArg*/)
	{
		return SQLITE_OK;
	}

	static int SectorSize(sqlite3_file* /*pFile*/)
	{
		return 0;
	}

	static int DeviceCharacteristics(sqlite3_file* /*pFile*/)
	{
		return 0;
	}

	static int DBOpen(sqlite3_vfs* pVfs, const char* /*zName*/, sqlite3_file* pFile, int /*flags*/, int* /*pOutFlags*/)
	{
		SQLiteVFS* v = (SQLiteVFS*)pVfs;

		static const sqlite3_io_methods io =
		{
			1,                        /* iVersion */
			Close,                    /* xClose */
			Read,                     /* xRead */
			Write,                    /* xWrite */
			Truncate,                 /* xTruncate */
			Sync,                     /* xSync */
			FileSize,                 /* xFileSize */
			Lock,                     /* xLock */
			Unlock,                   /* xUnlock */
			CheckReservedLock,        /* xCheckReservedLock */
			FileControl,              /* xFileControl */
			SectorSize,               /* xSectorSize */
			DeviceCharacteristics     /* xDeviceCharacteristics */
		};

		SQLiteFile *p = (SQLiteFile*)pFile;
		p->pFile = v->pMemStream;
		p->pMethods = &io;
		return SQLITE_OK;
	}

	static int FullPathname(sqlite3_vfs* /*pVfs*/, const char* /*zPath*/, int /*nPathOut*/, char* /*zPathOut*/)
	{
		return SQLITE_OK;
	}
}

void Database::RegisterVFS()
{
	sqlite3_vfs * pOriginal = sqlite3_vfs_find(nullptr);
	g_pVfs = (SQLiteVFS*)pOriginal;
	g_pVfs->szOsFile = sizeof(SQLiteFile);
	g_pVfs->zName = "memvfs";
	g_pVfs->xOpen = &SQLiteFunctions::DBOpen;
	g_pVfs->xFullPathname = &SQLiteFunctions::FullPathname;
	sqlite3_vfs_register(g_pVfs, 1);
}

PreparedStatement::PreparedStatement(Database* pDb, const std::string& query)
	: m_pDatabase(pDb), m_ParametersSet(0), m_Query(query)
{
	size_t idx = query.find('?');
	while (idx != std::string::npos)
	{
		++m_ParameterCount;
		idx = query.find('?', idx + 1);
	}
}

PreparedStatement::~PreparedStatement()
{
	if (pStatement)
	{
		sqlite3_finalize(pStatement);
	}
}

Database::Database(Context* pContext)
	: Resource(pContext)
{
}

Database::~Database()
{
	Close();
}

bool Database::Load(InputStream& inputStream)
{
	if (g_pVfs == nullptr)
	{
		RegisterVFS();
	}
	inputStream.ReadAllBytes(m_Data);
	m_Stream = MemoryStream(m_Data.data(), m_Data.size());

	g_pVfs->pMemStream = &m_Stream;
	sqlite3_open(inputStream.GetSource().c_str(), (sqlite3**)&m_pDatabase);

	return true;
}

PreparedStatement Database::Prepare(const std::string query)
{
	PreparedStatement s(this, query);
	sqlite3_prepare_v2(m_pDatabase, query.c_str(), -1, &s.pStatement, nullptr);
	return std::move(s);
}

bool Database::Execute(const std::string& query, const DatabaseExecuteDelegate& callback)
{
	char* pErrorMsg;
	int rc = sqlite3_exec(m_pDatabase, query.c_str(), [](void* pData, int argc, char** argv, char** colName)
	{
		return ((DatabaseExecuteDelegate*)pData)->Execute(pData, argc, argv, colName);
	}, const_cast<DatabaseExecuteDelegate*>(&callback), &pErrorMsg);

	if (rc != SQLITE_OK)
	{
		FLUX_LOG(Warning, "[Database::Execute] > '%s'", pErrorMsg);
		sqlite3_free(pErrorMsg);
		return false;
	}
	return true;
}

std::string Database::GetLastError()
{
	int code = sqlite3_errcode(m_pDatabase);
	return sqlite3_errstr(code);
}

void Database::Close()
{
	if (m_pDatabase)
	{
		sqlite3_close(m_pDatabase);
	}
}

bool PreparedStatement::Step()
{
	if (m_State < State::BindComplete)
	{
		FLUX_LOG(Warning, "[PreparedStatement::Step()] Statement is not ready for stepping. Make sure to call CompleteBinding() first");
		return false;
	}

	int result = sqlite3_step(pStatement);
	if (CheckError(result) == false)
	{
		return false;
	}

	switch (result)
	{
	case SQLITE_ROW:
		++m_CurrentIndex;
		m_State = State::Stepping;
		return true;
	case SQLITE_DONE:
		++m_CurrentIndex;
		m_State = State::Done;
		return false;
	default:
		return false;
	}
}

bool PreparedStatement::Reset(bool resetParameters)
{
	check(pStatement);
	switch (m_State)
	{
	case State::BindState:
	case State::BindComplete:
		return true;
	case State::Stepping:
	case State::Done:

		if (CheckError(sqlite3_reset(pStatement)) == false)
		{
			return false;
		}

		if (resetParameters)
		{
			return ResetParameters();
		}
		return true;
	default:
		return false;
	}
}

bool PreparedStatement::ResetParameters()
{
	check(pStatement);
	m_ParametersSet = 0;
	m_State = State::BindState;
	return CheckError(sqlite3_clear_bindings(pStatement));
}

void PreparedStatement::GetColumn(int index, const char** pText) const
{
	check(pStatement);
	*pText = reinterpret_cast<const char*>(sqlite3_column_text(pStatement, index));
}

void PreparedStatement::GetColumn(int index, int* pValue) const
{
	check(pStatement);
	*pValue = sqlite3_column_int(pStatement, index);
}

void PreparedStatement::GetColumn(int index, float* pValue) const
{
	check(pStatement);
	*pValue = static_cast<float>(sqlite3_column_double(pStatement, index));
}

const int PreparedStatement::Columns() const
{
	return sqlite3_column_count(pStatement);
}

const char* PreparedStatement::GetColumnName(int index) const
{
	return sqlite3_column_name(pStatement, index);
}

bool PreparedStatement::BindParameter(const int index, const std::string& value)
{
	check(pStatement);
	bool success = CheckError(sqlite3_bind_text(pStatement, index + 1, value.c_str(), -1, nullptr));
	if (success)
	{
		m_ParametersSet |= (1 << index);
	}
	return success;
}

bool PreparedStatement::BindParameter(const int index, const int value)
{
	check(pStatement);
	bool success = CheckError(sqlite3_bind_int(pStatement, index + 1, value));
	if (success)
	{
		m_ParametersSet |= (1 << index);
	}
	return success;
}

bool PreparedStatement::BindParameter(const int index, const float value)
{
	check(pStatement);
	bool success = CheckError(sqlite3_bind_double(pStatement, index + 1, (double)value));
	if (success)
	{
		m_ParametersSet |= (1 << index);
	}
	return success;
}

bool PreparedStatement::CompleteBinding()
{
	if (m_State != State::BindState)
	{
		FLUX_LOG(Warning, "[PreparedStatement::CompleteBinding()] This function is called at the wrong time.");
		return false;
	}

	const int requiredBitmask = (1 << m_ParameterCount) - 1;
	if ((requiredBitmask & m_ParametersSet) != requiredBitmask)
	{
		FLUX_LOG(Warning, "[PreparedStatement::CompleteBinding()] Not all parameters were set.");
		return false;
	}
	m_State = State::BindComplete;
	return true;
}
