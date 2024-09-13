#include "STLReader.h"
#include <algorithm>
#include <exception>
#include <fstream>
#include <sstream>

//二进制文件格式
/*
 * uint8[80]  Header
 * uint32     number of triangles
 * float[3]   normal
 * float[3]   vertex 1
 * float[3]   vertex 2
 * float[3]   vertex 3
 * float[3]   attribute
 */

//ASCII 文件格式
/*
* solid object_name
*	facet normal x y z
*		outer loop
*			vertex x y z
*			vertex x y z
*			vertex x y z
*		endloop
*	endfacet
* endsolid object_name
 */

FSTLReader::FSTLReader(const FString& FilePath)
	: FReaderBase(FilePath)
{
	
}

FSTLReader::FSTLReader(const FString& FilePath, const FImportOptions& Options)
	: FReaderBase(FilePath, Options)
{
}

FSTLReader::~FSTLReader()
{
	
}

FModelMesh* FSTLReader::ReadFile(const FString& strPath, const FImportOptions& options)
{
	FModelMesh* modelMesh = new FModelMesh(0, "");
	modelMesh->IsRoot = true;
	std::ifstream* file = new std::ifstream;
	
	//前80个字节是文件头，后4字节是文件面数
	file->open(*strPath, std::ios::in | std::ios::binary);
	bool isOpen = file->is_open();
	if (isOpen)
	{
		if (isBinaryFile(file))
		{
			ReadBinarySTL(file, modelMesh);
		}
		else
		{
			ReadAsciiSTL(file, modelMesh);
		}
		file->close();
	}
	return modelMesh;
}

void FSTLReader::ReadAsciiSTL(std::ifstream* File, FModelMesh* Model)
{
	
}

void FSTLReader::ReadBinarySTL(std::ifstream* File, FModelMesh* &Model)
{
	File->ignore(80);
	int faceCount;
	File->read(reinterpret_cast<char*>(&faceCount), 4);
	
	Model->Sections.Empty(0);
	TSharedPtr<FModelSection> section = MakeShared<FModelSection>();
	Model->Sections.Add(section);
	
	float normalX = 0, normalY = 0, normalZ = 0;
	float vertexX = 0, vertexY = 0, vertexZ = 0;
	for (int i = 0; i < faceCount; i++)
	{
		//normal   12 byte
		File->read(reinterpret_cast<char*>(&normalX), 4);
		File->read(reinterpret_cast<char*>(&normalY), 4);
		File->read(reinterpret_cast<char*>(&normalZ), 4);
		section->Normals.Add(FVector3f(normalX, normalY, normalZ));
		
		//vertex 1 12 byte
		File->read(reinterpret_cast<char*>(&vertexX), 4);
		File->read(reinterpret_cast<char*>(&vertexY), 4);
		File->read(reinterpret_cast<char*>(&vertexZ), 4);
		section->Vertexes.Add(FVector(vertexX, vertexY, vertexZ));
		section->BoundBox += FVector(normalX, normalY, normalZ);
		
		//vertex 2 12 byte
		File->read(reinterpret_cast<char*>(&vertexX), 4);
		File->read(reinterpret_cast<char*>(&vertexY), 4);
		File->read(reinterpret_cast<char*>(&vertexZ), 4);
		section->Vertexes.Add(FVector(vertexX, vertexY, vertexZ));
		section->BoundBox += FVector(normalX, normalY, normalZ);
		
		//vertex 3 12 byte
		File->read(reinterpret_cast<char*>(&vertexX), 4);
		File->read(reinterpret_cast<char*>(&vertexY), 4);
		File->read(reinterpret_cast<char*>(&vertexZ), 4);
		section->Vertexes.Add(FVector(vertexX, vertexY, vertexZ));
		section->BoundBox += FVector(normalX, normalY, normalZ);

		section->TexCoord0.Add(FVector2f(0, 0));
		//attribute 2 byte
		File->ignore(2);

		section->Indexes.Add(i * 3 + 0);
		section->Indexes.Add(i * 3 + 1);
		section->Indexes.Add(i * 3 + 2);
	}
}

bool FSTLReader::isBinaryFile(std::ifstream* file)
{
	std::string header, tail;
	file->seekg(0, std::ios::end);
	size_t fileSize = file->tellg();
	int flag = (fileSize - 84) % 50;
	if ( flag == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

