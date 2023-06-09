// Copyright 2016-2020 TriAxis Games L.L.C. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshModifier.h"
#include "RuntimeMeshModifierAdjacency.generated.h"

/**
 * 
 */
UCLASS()
class RUNTIMEMESHCOMPONENT_API URuntimeMeshModifierAdjacency : public URuntimeMeshModifier
{
	GENERATED_BODY()
	

public:
	URuntimeMeshModifierAdjacency();

	virtual void ApplyToMesh_Implementation(FRuntimeMeshRenderableMeshData& MeshData) override;


	/*
	*	Calculates the tessellation indices for the supplied mesh data, setting the result back to the tessellation triangles in the mesh data.
	*/
	UFUNCTION(BlueprintCallable, Category="RuntimeMesh|Modifiers|Adjacency")
	static void CalculateTessellationIndices(FRuntimeMeshRenderableMeshData& MeshData);

private:
	struct Vertex;
	struct Edge;
	struct Corner;
	struct Triangle;

	static FORCEINLINE uint32 HashValue(const FVector3f& Vec)
	{
		return 31337 * GetTypeHash(Vec.X) + 13 * GetTypeHash(Vec.Y) + 3 * GetTypeHash(Vec.Z);
	}

	static FORCEINLINE uint32 HashValue(const Vertex& Vert)
	{
		return HashValue(Vert.Position);
	}

	using EdgeDictionary = TMap<Edge, Edge>;
	using PositionDictionary = TMap<FVector, Corner>;

	struct Vertex
	{
		FVector3f Position;
		FVector2f TexCoord;

		Vertex() { }
		Vertex(const FVector3f& InPosition, const FVector2f& InTexCoord)
			: Position(InPosition), TexCoord(InTexCoord)
		{ }

		FORCEINLINE bool operator==(const Vertex& Other) const
		{
			return Position == Other.Position;
		}
		FORCEINLINE bool operator<(const Vertex& Other) const
		{
			return Position.X < Other.Position.X
				|| Position.Y < Other.Position.Y
				|| Position.Z < Other.Position.Z;
		}
	};

	struct Edge
	{
	private:
		uint32 IndexFrom;
		uint32 IndexTo;

		Vertex VertexFrom;
		Vertex VertexTo;

		uint32 CachedHash;

	public:
		Edge() : CachedHash(0) { }
		Edge(uint32 InIndexFrom, uint32 InIndexTo, const Vertex& InVertexFrom, const Vertex& InVertexTo)
			: IndexFrom(InIndexFrom), IndexTo(InIndexTo), VertexFrom(InVertexFrom), VertexTo(InVertexTo)
		{
			// Hash should only consider position, not index. 
			// We want values with different indices to compare true.
			CachedHash = 7 * HashValue(VertexFrom) + 2 * HashValue(VertexTo);
		}

		Vertex GetVertex(uint32 I) const
		{
			switch (I)
			{
			case 0:
				return VertexFrom;
			case 1:
				return VertexTo;
			default:
				checkNoEntry();
				return Vertex();
			}
		}

		uint32 GetIndex(uint32 I) const
		{
			switch (I)
			{
			case 0:
				return IndexFrom;
			case 1:
				return IndexTo;
			default:
				checkNoEntry();
				return 0;
			}
		}

		Edge GetReverse() const
		{
			return Edge(IndexTo, IndexFrom, VertexTo, VertexFrom);
		}

		FORCEINLINE bool operator<(const Edge& Other) const
		{
			// Quick out, otherwise we have to compare vertices
			if (IndexFrom == Other.IndexFrom && IndexTo == Other.IndexTo)
			{
				return false;
			}

			return VertexFrom < Other.VertexFrom || VertexTo < Other.VertexTo;
		}

		FORCEINLINE bool operator==(const Edge& Other) const
		{
			return (IndexFrom == Other.IndexFrom && IndexTo == Other.IndexTo) ||
				(VertexFrom == Other.VertexFrom && VertexTo == Other.VertexTo);
		}

		friend FORCEINLINE uint32 GetTypeHash(const Edge& E)
		{
			return E.CachedHash;
		}
	};

	struct Corner
	{
		uint32 Index;
		FVector2f TexCoord;

		Corner() : Index(0) { }
		Corner(uint32 InIndex, FVector2f InTexCoord)
			: Index(InIndex), TexCoord(InTexCoord)
		{ }
	};

	struct Triangle
	{
		Edge Edge0;
		Edge Edge1;
		Edge Edge2;

	public:
		Triangle(uint32 Index0, uint32 Index1, uint32 Index2, const Vertex& Vertex0, const Vertex& Vertex1, const Vertex& Vertex2)
			: Edge0(Index0, Index1, Vertex0, Vertex1)
			, Edge1(Index1, Index2, Vertex1, Vertex2)
			, Edge2(Index2, Index0, Vertex2, Vertex0)
		{ }

		FORCEINLINE bool operator<(const Triangle& Other) const
		{
			return Edge0 < Other.Edge0 || Edge1 < Other.Edge1 || Edge2 < Other.Edge2;
		}

		FORCEINLINE const Edge& GetEdge(uint32 I)
		{
			return ((Edge*)&Edge0)[I];
		}
		FORCEINLINE uint32 GetIndex(uint32 I)
		{
			return GetEdge(I).GetIndex(0);
		}

	};

	static void AddIfLeastUV(PositionDictionary& PosDict, const Vertex& Vert, uint32 Index);

	static void ReplacePlaceholderIndices(FRuntimeMeshRenderableMeshData& MeshData, EdgeDictionary& EdgeDict, PositionDictionary& PosDict);

	static void ExpandIB(FRuntimeMeshRenderableMeshData& MeshData, EdgeDictionary& OutEdgeDict, PositionDictionary& OutPosDict);
};
