#include "FluxEngine.h"
#include "Mesh.h"

#include "Rendering/Core/IndexBuffer.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Geometry.h"

#include "Async/AsyncTaskQueue.h"
#include "Core/Graphics.h"
#include "IO/InputStream.h"

Mesh::Mesh(Context* pContext):
	Resource(pContext)
{
}

Mesh::~Mesh()
{
}

bool Mesh::Load(InputStream& inputStream)
{
	std::string fileName = inputStream.GetSource();
	AUTOPROFILE_DESC(Mesh_Load, Paths::GetFileName(fileName));

	m_Geometries.clear();

	std::string extension = Paths::GetFileExtenstion(fileName);
	if (extension != "flux")
	{
		std::stringstream stream;
		stream << "MeshLoader::LoadContent() -> '" << fileName << "' has a wrong file extension";
		FLUX_LOG(Error, stream.str());
		return false;
	}

	m_MeshName = Paths::GetFileName(fileName);

	const std::string magic = inputStream.ReadSizedString();
	const char minVersion = inputStream.ReadByte();
	const char maxVersion = inputStream.ReadByte();

	UNREFERENCED_PARAMETER(maxVersion);
	if (minVersion != MESH_VERSION)
	{
		std::stringstream stream;
		stream << "MeshLoader::LoadContent() File '" << fileName << "' version mismatch: Expects v" << MESH_VERSION << ".0 but is v" << (int)minVersion << ".0";
		FLUX_LOG(Error, stream.str());
	}

	inputStream.Read((char*)&m_BoundingBox, sizeof(BoundingBox));

	m_GeometryCount = inputStream.ReadInt();

	for (int i = 0; i < m_GeometryCount; ++i)
	{
		std::unique_ptr<Geometry> pGeometry = std::make_unique<Geometry>();
		for (;;)
		{
			std::string block = inputStream.ReadSizedString();
			for (char& c : block)
				c = (char)toupper(c);
			if (block == "ENDMESH")
				break;

			const unsigned int length = inputStream.ReadUInt();
			const unsigned int stride = inputStream.ReadUInt();

			pGeometry->GetVertexDataUnsafe(block).pData = new char[length * stride];
			pGeometry->GetVertexDataUnsafe(block).Count = length;
			pGeometry->GetVertexDataUnsafe(block).Stride = stride;
			inputStream.Read((char*)pGeometry->GetVertexDataUnsafe(block).pData, length * stride);
		}

		pGeometry->SetDrawRange(PrimitiveType::TRIANGLELIST, pGeometry->GetDataCount("INDEX"), pGeometry->GetDataCount("POSITION"));
		m_Geometries.push_back(std::move(pGeometry));
	}
	return true;
}

void Mesh::CreateBuffers(std::vector<VertexElement>& elementDesc)
{
	AUTOPROFILE_DESC(Mesh_CreateBuffers, m_MeshName);
	for (std::unique_ptr<Geometry>& pGeometry : m_Geometries)
	{
		CreateBuffersForGeometry(elementDesc, pGeometry.get());
	}
}

void Mesh::CreateBuffersForGeometry(std::vector<VertexElement>& elementDesc, Geometry* pGeometry)
{
	std::unique_ptr<VertexBuffer> pVertexBuffer = std::make_unique<VertexBuffer>(GetSubsystem<Graphics>());
	pVertexBuffer->Create(pGeometry->GetVertexCount(), elementDesc, false);

	int vertexStride = pVertexBuffer->GetVertexStride();
	if (vertexStride == 0)
	{
		FLUX_LOG(Error, "MeshFilter::CreateBuffers() > VertexStride of the InputLayout is 0");
		return;
	}

	char* pDataLocation = new char[vertexStride * pGeometry->GetVertexCount()];
	char* pVertexDataStart = pDataLocation;

	//Instead of getting the info every vertex, cache it in a local struct
	struct ElementInfo
	{
		ElementInfo(const VertexElement& element) :
			semanticName(VertexElement::GetSemanticOfType(element.Semantic)),
			elementSize(VertexElement::GetSizeOfType(element.Type))
		{ }
		std::string semanticName;
		unsigned int elementSize;
	};
	std::vector<ElementInfo> elementInfo;
	for (VertexElement& element : elementDesc)
		elementInfo.push_back(element);

	AsyncTaskQueue* pQueue = GetSubsystem<AsyncTaskQueue>();
	const int taskCount = 4;
	int vertexCountPerThread = pGeometry->GetVertexCount() / taskCount;
	int remaining = pGeometry->GetVertexCount() % taskCount;

	const std::map <std::string, Geometry::VertexData>& rawData = pGeometry->GetRawData();
	for (int i = 0; i < taskCount; ++i)
	{
		int vertexCount = vertexCountPerThread;
		if (i >= taskCount - 1)
			vertexCount += remaining;
		int startVertex = i * vertexCountPerThread;

		AsyncTask* pTask = pQueue->GetFreeTask();
		pTask->Action.BindLambda([pVertexDataStart, vertexCount, startVertex, rawData, elementInfo, vertexStride](AsyncTask* pTask, unsigned index)
		{
			UNREFERENCED_PARAMETER(pTask);
			UNREFERENCED_PARAMETER(index);
			char* pDataStart = (char*)pVertexDataStart + startVertex * vertexStride;
			for (int i = 0; i < vertexCount; i++)
			{
				int currentVertex = startVertex + i;
				for (size_t e = 0; e < elementInfo.size(); e++)
				{
					const ElementInfo& element = elementInfo[e];

					const void* pData = (const char*)rawData.at(element.semanticName).pData + element.elementSize * currentVertex;
					memcpy(pDataStart, pData, element.elementSize);
					pDataStart = (char*)pDataStart + element.elementSize;
				}
			}
		});
		pQueue->AddWorkItem(pTask);
	}
	if (pGeometry->HasData("INDEX"))
	{
		AsyncTask* pTask = pQueue->GetFreeTask();
		pTask->Action.BindLambda([&, this, pGeometry](AsyncTask* pTask, unsigned index)
		{
			UNREFERENCED_PARAMETER(pTask);
			UNREFERENCED_PARAMETER(index);

			std::unique_ptr<IndexBuffer> pIndexBuffer = std::make_unique<IndexBuffer>(GetSubsystem<Graphics>());
			pIndexBuffer->Create(pGeometry->GetIndexCount(), false, false);
			pIndexBuffer->SetData(pGeometry->GetVertexData("INDEX").pData);
			pGeometry->SetIndexBuffer(pIndexBuffer.get());
			m_IndexBuffers.push_back(std::move(pIndexBuffer));
		});
		pQueue->AddWorkItem(pTask);
	}

	pQueue->JoinAll();

	pVertexBuffer->SetData(pVertexDataStart);
	pGeometry->SetVertexBuffer(pVertexBuffer.get());
	m_VertexBuffers.push_back(std::move(pVertexBuffer));

	m_BuffersInitialized = true;
	delete[] pVertexDataStart;
}

