#pragma once

class Resource : public Object
{
	FLUX_OBJECT(Resource, Object)

public:
	Resource();
	virtual ~Resource();

	DELETE_COPY(Resource)

	virtual bool Load(const std::string& filePath) = 0;
	virtual bool Save(const std::string& filePath);

	void SetName(const std::string& name) { m_Name = name; }
	const std::string& GetName() const { return m_Name; }

protected:
	string m_Name;
};