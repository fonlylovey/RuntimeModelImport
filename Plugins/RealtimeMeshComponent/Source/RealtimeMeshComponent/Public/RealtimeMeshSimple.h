﻿// Copyright (c) 2015-2024 TriAxis Games, L.L.C. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMesh.h"
#include "RealtimeMeshConfig.h"
#include "Data/RealtimeMeshLOD.h"
#include "Data/RealtimeMeshSection.h"
#include "Data/RealtimeMeshSectionGroup.h"
#include "Interface_CollisionDataProviderCore.h"
#include "Mesh/RealtimeMeshBuilder.h"
#include "Mesh/RealtimeMeshDataStream.h"
#include "Mesh/RealtimeMeshDistanceField.h"
#include "Mesh/RealtimeMeshCardRepresentation.h"
#include "Mesh/RealtimeMeshSimpleData.h"
#include "RealtimeMeshSimple.generated.h"


#define LOCTEXT_NAMESPACE "RealtimeMesh"

class URealtimeMeshStreamSet;
class URealtimeMeshSimple;
using namespace RealtimeMesh;


namespace RealtimeMesh
{
	/**
	 * @brief Concrete implementation of FRealtimeMeshSection for simple realtime mesh implementation
	 */
	class REALTIMEMESHCOMPONENT_API FRealtimeMeshSectionSimple : public FRealtimeMeshSection
	{
	protected:
		// Is the mesh collision enabled for this section?
		bool bShouldCreateMeshCollision;

	public:
		FRealtimeMeshSectionSimple(const FRealtimeMeshSharedResourcesRef& InSharedResources, const FRealtimeMeshSectionKey& InKey);
		virtual ~FRealtimeMeshSectionSimple() override;

		/**
		 * @brief Do we currently have complex collision enabled for this section?
		 * @return 
		 */
		bool HasCollision() const { return bShouldCreateMeshCollision; }

		/**
		 * @brief Turns on/off the complex collision support for this section
		 * @param bNewShouldCreateMeshCollision New setting for building complex collision from this section
		 */
		void SetShouldCreateCollision(bool bNewShouldCreateMeshCollision);

		/**
		 * @brief Update the stream range for this section
		 * @param Commands Running command queue that we send RT commands too. This is used for command batching.
		 * @param InRange New section stream range
		 */
		virtual void UpdateStreamRange(FRealtimeMeshProxyCommandBatch& Commands, const FRealtimeMeshStreamRange& InRange) override;

		/**
		 * @brief Generates the collision mesh data for this section, adding it to the supplied CollisionData.
		 * @param CollisionData Collision data to add new collision mesh data too.
		 * @return Whether the generation succeeded. 
		 */
		virtual bool GenerateCollisionMesh(FRealtimeMeshTriMeshData& CollisionData);
		
		/**
		 * @brief Serializes this section to the running archive.
		 * @param Ar Archive to serialize too.
		 * @return 
		 */
		virtual bool Serialize(FArchive& Ar) override;

		/**
		 * @brief Resets the section to a default state
		 * @param Commands Running command queue that we send RT commands too. This is used for command batching.
		 */
		virtual void Reset(FRealtimeMeshProxyCommandBatch& Commands) override;

	protected:
		/**
		 * @brief Handler for when streams are added/removed/updated so we can do things in response like update collision
		 * @param SectionGroupKey Parent section group key, since this event handler can receive events from any SectionGroup
		 * @param StreamKey Key used to identify the stream
		 * @param ChangeType The type of change applied to this stream, whether added/removed/updated
		 */
		virtual void HandleStreamsChanged(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshStreamKey& StreamKey, ERealtimeMeshChangeType ChangeType) const;

		/**
		 * @brief Calculates the bounds from the mesh data for this section
		 * @return The new calculated bounds.
		 */
		virtual FBoxSphereBounds3f CalculateBounds() const override;

		/**
		 * @brief Marks the collision dirty, to request an update to collision
		 */
		void MarkCollisionDirtyIfNecessary(bool bForceUpdate = false) const;
	};

	DECLARE_DELEGATE_RetVal_OneParam(FRealtimeMeshSectionConfig, FRealtimeMeshPolyGroupConfigHandler, int32);

	/**
	 * @brief Concrete implementation of FRealtimeMeshSectionGroup for simple realtime mesh implementation
	 */
	class REALTIMEMESHCOMPONENT_API FRealtimeMeshSectionGroupSimple : public FRealtimeMeshSectionGroup
	{
		// Store the actual mesh data on CPU side, this is so we can support fire-and-forget
		FRealtimeMeshStreamSet Streams;

		// Handler for setting up section config based on found poly groups
		FRealtimeMeshPolyGroupConfigHandler ConfigHandler;

		// Should we auto create sections for the poly groups
		uint8 bAutoCreateSectionsForPolygonGroups : 1;

	public:
		FRealtimeMeshSectionGroupSimple(const FRealtimeMeshSharedResourcesRef& InSharedResources, const FRealtimeMeshSectionGroupKey& InKey)
			: FRealtimeMeshSectionGroup(InSharedResources, InKey)
			, bAutoCreateSectionsForPolygonGroups(true)
		{
		}

		/**
		 * @brief Get the valid range of the contained streams. This is the maximal renderable region of the streams
		 * @return 
		 */
		FRealtimeMeshStreamRange GetValidStreamRange() const;


		void SetShouldAutoCreateSectionsForPolyGroups(bool bNewValue) { bAutoCreateSectionsForPolygonGroups = bNewValue; }
		bool ShouldAutoCreateSectionsForPolygonGroups() const { return bAutoCreateSectionsForPolygonGroups; }

		/*
		 * @brief Get the stream by key if it exists, nullptr otherwise
		 * @param StreamKey Key to identify the stream
		 */
		const FRealtimeMeshStream* GetStream(FRealtimeMeshStreamKey StreamKey) const;

		void SetPolyGroupSectionHandler(const FRealtimeMeshPolyGroupConfigHandler& NewHandler);
		void ClearPolyGroupSectionHandler();

		/*
		 * @brief Get readonly access to the stream data, so you can read from it without copying it.
		 * @param ProcessFunc Function to process the mesh data, you have threadsafe access while in this function.
		 */
		void ProcessMeshData(TFunctionRef<void(const FRealtimeMeshStreamSet&)> ProcessFunc) const;

		/*
		 * @brief Edit the mesh data, you can modify the mesh data in this function in a threadsafe manner.
		 * @param EditFunc Function to edit the mesh data, you have threadsafe access while in this function.
		 */
		TFuture<ERealtimeMeshProxyUpdateStatus> EditMeshData(TFunctionRef<TSet<FRealtimeMeshStreamKey>(FRealtimeMeshStreamSet&)> EditFunc);

		/*
		 * @brief Create or update a stream in the mesh data
		 * @param Commands Running command queue that we send RT commands too. This is used for command batching.
		 * @param Stream The stream to create or update
		 */
		virtual void CreateOrUpdateStream(FRealtimeMeshProxyCommandBatch& Commands, FRealtimeMeshStream&& Stream) override;

		/*
		 * @brief Remove a stream from the mesh data
		 * @param Commands Running command queue that we send RT commands too. This is used for command batching.
		 * @param StreamKey Key to identify the stream
		 */
		virtual void RemoveStream(FRealtimeMeshProxyCommandBatch& Commands, const FRealtimeMeshStreamKey& StreamKey) override;


		using FRealtimeMeshSectionGroup::SetAllStreams;
		/*
		 * @brief Set all the streams in the mesh data
		 * @param Commands Running command queue that we send RT commands too. This is used for command batching.
		 * @param InStreams The new streams to set
		 */
		virtual void SetAllStreams(FRealtimeMeshProxyCommandBatch& Commands, FRealtimeMeshStreamSet&& InStreams) override;

		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		UE_DEPRECATED(all, "FRealtimeMeshSimpleMeshData is deprecated, use FRealtimeMeshStreamSet and TRealtimeMeshBuilderLocal instead and its helper functions which are more efficient and flexible")
		TFuture<ERealtimeMeshProxyUpdateStatus> UpdateFromSimpleMesh(const FRealtimeMeshSimpleMeshData& MeshData);
		
		UE_DEPRECATED(all, "FRealtimeMeshSimpleMeshData is deprecated, use FRealtimeMeshStreamSet and TRealtimeMeshBuilderLocal instead and its helper functions which are more efficient and flexible")
		void UpdateFromSimpleMesh(FRealtimeMeshProxyCommandBatch& Commands, const FRealtimeMeshSimpleMeshData& MeshData);
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		/*
		 * @brief Setup the proxy for this section group
		 * @param Commands Running command queue that we send RT commands too. This is used for command batching.
		 */
		virtual void InitializeProxy(FRealtimeMeshProxyCommandBatch& Commands) override;

		/*
		 * @brief Reset this section group, removing all streams, and config
		 * @param Commands Running command queue that we send RT commands too. This is used for command batching.
		 */
		virtual void Reset(FRealtimeMeshProxyCommandBatch& Commands) override;

		/*
		 * @brief Serialize this section group
		 */
		virtual bool Serialize(FArchive& Ar) override;

		/*
		 * @brief Generate the collision mesh data for this section group, used to setup PhysX/Chaos collision
		 */
		virtual bool GenerateCollisionMesh(FRealtimeMeshTriMeshData& CollisionData);
		
	protected:

		virtual void UpdatePolyGroupSections(FRealtimeMeshProxyCommandBatch& Commands, bool bUpdateDepthOnly);
		virtual FRealtimeMeshSectionConfig DefaultPolyGroupSectionHandler(int32 PolyGroupIndex) const;
		
		bool ShouldCreateSingularSection() const;
	};

	/*
	 * @brief Concrete implementation of FRealtimeMeshLODData for the simple realtime mesh implementation
	 */
	class REALTIMEMESHCOMPONENT_API FRealtimeMeshLODSimple : public FRealtimeMeshLODData
	{
	public:
		FRealtimeMeshLODSimple(const FRealtimeMeshSharedResourcesRef& InSharedResources, const FRealtimeMeshLODKey& InKey)
			: FRealtimeMeshLODData(InSharedResources, InKey)
		{
		}

		/*
		 * @brief Generate the collision mesh data for this LOD, used to setup PhysX/Chaos collision
		 */
		virtual bool GenerateCollisionMesh(FRealtimeMeshTriMeshData& CollisionData);
	};

	DECLARE_MULTICAST_DELEGATE(FRealtimeMeshSimpleCollisionDataChangedEvent);

	/**
	 * @brief Shared resources for the simple realtime mesh implementation
	 */
	class REALTIMEMESHCOMPONENT_API FRealtimeMeshSharedResourcesSimple : public FRealtimeMeshSharedResources
	{
		FRealtimeMeshSimpleCollisionDataChangedEvent CollisionDataChangedEvent;

	public:
		FRealtimeMeshSimpleCollisionDataChangedEvent& OnCollisionDataChanged() { return CollisionDataChangedEvent; }
		void BroadcastCollisionDataChanged() const { CollisionDataChangedEvent.Broadcast(); }


		virtual FRealtimeMeshSectionRef CreateSection(const FRealtimeMeshSectionKey& InKey) const override
		{
			return MakeShared<FRealtimeMeshSectionSimple>(ConstCastSharedRef<FRealtimeMeshSharedResources>(this->AsShared()), InKey);
		}

		virtual FRealtimeMeshSectionGroupRef CreateSectionGroup(const FRealtimeMeshSectionGroupKey& InKey) const override
		{
			return MakeShared<FRealtimeMeshSectionGroupSimple>(ConstCastSharedRef<FRealtimeMeshSharedResources>(this->AsShared()), InKey);
		}

		virtual FRealtimeMeshLODDataRef CreateLOD(const FRealtimeMeshLODKey& InKey) const override
		{
			return MakeShared<FRealtimeMeshLODSimple>(ConstCastSharedRef<FRealtimeMeshSharedResources>(this->AsShared()), InKey);
		}

		virtual FRealtimeMeshRef CreateRealtimeMesh() const override;
		virtual FRealtimeMeshSharedResourcesRef CreateSharedResources() const override { return MakeShared<FRealtimeMeshSharedResourcesSimple>(); }
	};

	class REALTIMEMESHCOMPONENT_API FRealtimeMeshSimple : public FRealtimeMesh
	{
	protected:
		// Configuration for the collision on this mesh
		FRealtimeMeshCollisionConfiguration CollisionConfig;

		// Collection of simple geometry objects used for the simple collision representation
		FRealtimeMeshSimpleGeometry SimpleGeometry;

		// Mesh geometry used for the complex collision representation
		FRealtimeMeshStreamSet ComplexMeshGeometry;

		// Pending collision update promise. Used to alert when the collision finishes updating
		mutable TSharedPtr<TPromise<ERealtimeMeshCollisionUpdateResult>> PendingCollisionPromise;

		// Distance field representation for this mesh
		FRealtimeMeshDistanceField DistanceField;

		// Lumen card representation for this mesh
		TUniquePtr<FRealtimeMeshCardRepresentation> CardRepresentation;
		
	public:
		FRealtimeMeshSimple(const FRealtimeMeshSharedResourcesRef& InSharedResources)
			: FRealtimeMesh(InSharedResources)
		{
			SharedResources->As<FRealtimeMeshSharedResourcesSimple>().OnCollisionDataChanged().AddRaw(this, &FRealtimeMeshSimple::MarkCollisionDirtyNoCallback);
		}

		virtual ~FRealtimeMeshSimple() override
		{
			if (PendingCollisionPromise)
			{
				PendingCollisionPromise->SetValue(ERealtimeMeshCollisionUpdateResult::Ignored);
			}
			
			SharedResources->As<FRealtimeMeshSharedResourcesSimple>().OnCollisionDataChanged().RemoveAll(this);
		}

		FRealtimeMeshCollisionConfiguration GetCollisionConfig() const;
		TFuture<ERealtimeMeshCollisionUpdateResult> SetCollisionConfig(const FRealtimeMeshCollisionConfiguration& InCollisionConfig);
		FRealtimeMeshSimpleGeometry GetSimpleGeometry() const;
		TFuture<ERealtimeMeshCollisionUpdateResult> SetSimpleGeometry(const FRealtimeMeshSimpleGeometry& InSimpleGeometry);

		bool HasCustomComplexMeshGeometry() const { return ComplexMeshGeometry.Num() > 0; }
		TFuture<ERealtimeMeshCollisionUpdateResult> ClearCustomComplexMeshGeometry();
		TFuture<ERealtimeMeshCollisionUpdateResult> SetCustomComplexMeshGeometry(FRealtimeMeshStreamSet&& InComplexMeshGeometry);
		TFuture<ERealtimeMeshCollisionUpdateResult> SetCustomComplexMeshGeometry(const FRealtimeMeshStreamSet& InComplexMeshGeometry);
		void ProcessCustomComplexMeshGeometry(TFunctionRef<void(const FRealtimeMeshStreamSet&)> ProcessFunc) const;
		TFuture<ERealtimeMeshCollisionUpdateResult> EditCustomComplexMeshGeometry(TFunctionRef<void(FRealtimeMeshStreamSet&)> EditFunc);

		const FRealtimeMeshDistanceField& GetDistanceField() const;
		virtual void SetDistanceField(FRealtimeMeshProxyCommandBatch& Commands, FRealtimeMeshDistanceField&& InDistanceField) override;
		using FRealtimeMesh::SetDistanceField;
		virtual void ClearDistanceField(FRealtimeMeshProxyCommandBatch& Commands) override;
		using FRealtimeMesh::ClearDistanceField;

		const FRealtimeMeshCardRepresentation* GetCardRepresentation() const;
		virtual void SetCardRepresentation(FRealtimeMeshProxyCommandBatch& Commands, FRealtimeMeshCardRepresentation&& InCardRepresentation) override;
		using FRealtimeMesh::SetCardRepresentation;
		virtual void ClearCardRepresentation(FRealtimeMeshProxyCommandBatch& Commands) override;
		using FRealtimeMesh::ClearCardRepresentation;

		
		
		virtual bool GenerateCollisionMesh(FRealtimeMeshTriMeshData& CollisionData);

		virtual void InitializeProxy(FRealtimeMeshProxyCommandBatch& Commands) const override;
		virtual void Reset(FRealtimeMeshProxyCommandBatch& Commands, bool bRemoveRenderProxy) override;

		virtual bool Serialize(FArchive& Ar, URealtimeMesh* Owner) override;

		void MarkCollisionDirtyNoCallback() const;
	protected:
		void MarkForEndOfFrameUpdate() const;
		TFuture<ERealtimeMeshCollisionUpdateResult> MarkCollisionDirty() const;

		virtual void ProcessEndOfFrameUpdates() override;

		friend class URealtimeMeshSimple;
	};
}


DECLARE_DYNAMIC_DELEGATE_OneParam(FRealtimeMeshSimpleCompletionCallback, ERealtimeMeshProxyUpdateStatus, ProxyUpdateResult);

DECLARE_DYNAMIC_DELEGATE_OneParam(FRealtimeMeshSimpleCollisionCompletionCallback, ERealtimeMeshCollisionUpdateResult, CollisionResult);


UCLASS(Blueprintable)
class REALTIMEMESHCOMPONENT_API URealtimeMeshSimple : public URealtimeMesh
{
	GENERATED_UCLASS_BODY()
protected:

public:
	TSharedRef<RealtimeMesh::FRealtimeMeshSimple> GetMeshData() const { return StaticCastSharedRef<RealtimeMesh::FRealtimeMeshSimple>(GetMesh()); }

	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, bool bShouldAutoCreateSectionsForPolyGroups = true);

	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, FRealtimeMeshStreamSet&& MeshData, bool bShouldAutoCreateSectionsForPolyGroups = true);
	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshStreamSet& MeshData, bool bShouldAutoCreateSectionsForPolyGroups = true);
	
	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, FRealtimeMeshStreamSet&& MeshData);
	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshStreamSet& MeshData);
	
	// DEPRECATE
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	UE_DEPRECATED(all, "FRealtimeMeshSimpleMeshData is deprecated, use FRealtimeMeshStreamSet and TRealtimeMeshBuilderLocal instead and its helper functions which are more efficient and flexible")
	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleMeshData& MeshData);
	UE_DEPRECATED(all, "FRealtimeMeshSimpleMeshData is deprecated, use FRealtimeMeshStreamSet and TRealtimeMeshBuilderLocal instead and its helper functions which are more efficient and flexible")
	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleMeshData& MeshData);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	
	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSection(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSectionConfig& Config,
															const FRealtimeMeshStreamRange& StreamRange, bool bShouldCreateCollision = false);

	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionConfig(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSectionConfig& Config, bool bShouldCreateCollision = false);
	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionRange(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshStreamRange& StreamRange);


	TArray<FRealtimeMeshSectionGroupKey> GetSectionGroups(const FRealtimeMeshLODKey& LODKey) const;
	TSharedPtr<FRealtimeMeshSectionGroupSimple> GetSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey) const;
	
	bool ProcessMesh(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const TFunctionRef<void(const FRealtimeMeshStreamSet&)>& ProcessFunc) const;
	TFuture<ERealtimeMeshProxyUpdateStatus> EditMeshInPlace(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const TFunctionRef<TSet<FRealtimeMeshStreamKey>(FRealtimeMeshStreamSet&)>& EditFunc);



	bool HasCustomComplexMeshGeometry() const;
	TFuture<ERealtimeMeshCollisionUpdateResult> ClearCustomComplexMeshGeometry();
	TFuture<ERealtimeMeshCollisionUpdateResult> SetCustomComplexMeshGeometry(FRealtimeMeshStreamSet&& InComplexMeshGeometry);
	TFuture<ERealtimeMeshCollisionUpdateResult> SetCustomComplexMeshGeometry(const FRealtimeMeshStreamSet& InComplexMeshGeometry);
	void ProcessCustomComplexMeshGeometry(TFunctionRef<void(const FRealtimeMeshStreamSet&)> ProcessFunc) const;
	TFuture<ERealtimeMeshCollisionUpdateResult> EditCustomComplexMeshGeometry(TFunctionRef<void(FRealtimeMeshStreamSet&)> EditFunc);


	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionGroup", meta= (AutoCreateRefTerm = "CompletionCallback"))
	void CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, URealtimeMeshStreamSet* MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	UE_DEPRECATED(all, "FRealtimeMeshSimpleMeshData is deprecated, use FRealtimeMeshStreamSet and TRealtimeMeshBuilderLocal instead and its helper functions which are more efficient and flexible")
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionGroupFromSimpleMesh", meta= (AutoCreateRefTerm = "CompletionCallback"))
	void CreateSectionGroupFromSimple(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleMeshData& MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionGroupUnique", meta= (AutoCreateRefTerm = "CompletionCallback"))
	FRealtimeMeshSectionGroupKey CreateSectionGroupUnique(const FRealtimeMeshLODKey& LODKey, URealtimeMeshStreamSet* MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	UE_DEPRECATED(all, "FRealtimeMeshSimpleMeshData is deprecated, use FRealtimeMeshStreamSet and TRealtimeMeshBuilderLocal instead and its helper functions which are more efficient and flexible")
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionGroupUniqueFromSimpleMesh", meta= (AutoCreateRefTerm = "CompletionCallback"))
	FRealtimeMeshSectionGroupKey CreateSectionGroupUniqueFromSimple(const FRealtimeMeshLODKey& LODKey,const FRealtimeMeshSimpleMeshData& MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="UpdateSectionGroup", meta= (AutoCreateRefTerm = "CompletionCallback"))
	void UpdateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, URealtimeMeshStreamSet* MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	UE_DEPRECATED(all, "FRealtimeMeshSimpleMeshData is deprecated, use FRealtimeMeshStreamSet and TRealtimeMeshBuilderLocal instead and its helper functions which are more efficient and flexible")
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="UpdateSectionGroupFromSimpleMesh", meta= (AutoCreateRefTerm = "CompletionCallback"))
	void UpdateSectionGroupFromSimple(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleMeshData& MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS


	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSection", meta = (AutoCreateRefTerm = "Config, StreamRange, CompletionCallback"))
	void CreateSection(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSectionConfig& Config,
								 const FRealtimeMeshStreamRange& StreamRange, bool bShouldCreateCollision, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	UE_DEPRECATED(all, "Use CreateSection instead")
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionUnique", meta = (AutoCreateRefTerm = "Config, StreamRange, CompletionCallback"))
	FRealtimeMeshSectionKey CreateSectionUnique(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSectionConfig& Config,
														  const FRealtimeMeshStreamRange& StreamRange, bool bShouldCreateCollision,
														  const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	



	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="UpdateSectionConfig", meta = (AutoCreateRefTerm = "SectionKey, bShouldCreateCollision, CompletionCallback"))
	void UpdateSectionConfig(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSectionConfig& Config, bool bShouldCreateCollision,
									   const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);


	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh")
	TArray<FRealtimeMeshSectionKey> GetSectionsInGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey);


	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="RemoveSection", meta = (AutoCreateRefTerm = "SectionKey, CompletionCallback"))
	void RemoveSection(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="RemoveSectionGroup", meta = (AutoCreateRefTerm = "SectionGroupKey, CompletionCallback"))
	void RemoveSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);


	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh")
	void SetShouldAutoCreateSectionsForPolyGroups(const FRealtimeMeshSectionGroupKey& SectionGroupKey, bool bNewValue);
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh")
	bool ShouldAutoCreateSectionsForPolygonGroups(const FRealtimeMeshSectionGroupKey& SectionGroupKey) const;

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", meta = (AutoCreateRefTerm = "SectionKey"))
	FRealtimeMeshSectionConfig GetSectionConfig(const FRealtimeMeshSectionKey& SectionKey) const;

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", meta = (AutoCreateRefTerm = "SectionKey"))
	bool IsSectionVisible(const FRealtimeMeshSectionKey& SectionKey) const;

	TFuture<ERealtimeMeshProxyUpdateStatus> SetSectionVisibility(const FRealtimeMeshSectionKey& SectionKey, bool bIsVisible);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="SetSectionVisibility", meta = (AutoCreateRefTerm = "SectionKey, CompletionCallback"))
	void SetSectionVisibility(const FRealtimeMeshSectionKey& SectionKey, bool bIsVisible, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", meta = (AutoCreateRefTerm = "SectionKey"))
	bool IsSectionCastingShadow(const FRealtimeMeshSectionKey& SectionKey) const;

	TFuture<ERealtimeMeshProxyUpdateStatus> SetSectionCastShadow(const FRealtimeMeshSectionKey& SectionKey, bool bCastShadow);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="SetSectionCastShadow", meta = (AutoCreateRefTerm = "SectionKey, CompletionCallback"))
	void SetSectionCastShadow(const FRealtimeMeshSectionKey& SectionKey, bool bCastShadow, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	
	TFuture<ERealtimeMeshProxyUpdateStatus> RemoveSection(const FRealtimeMeshSectionKey& SectionKey);

	TFuture<ERealtimeMeshProxyUpdateStatus> RemoveSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey);


	const FRealtimeMeshDistanceField& GetDistanceField() const;
	
	TFuture<ERealtimeMeshProxyUpdateStatus> SetDistanceField(FRealtimeMeshDistanceField&& InDistanceField);
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", meta = (AutoCreateRefTerm = "CompletionCallback"))
	void SetDistanceField(const FRealtimeMeshDistanceField& DistanceField, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	
	TFuture<ERealtimeMeshProxyUpdateStatus> ClearDistanceField();

	const FRealtimeMeshCardRepresentation* GetCardRepresentation() const;
	
	TFuture<ERealtimeMeshProxyUpdateStatus> SetCardRepresentation(FRealtimeMeshCardRepresentation&& InCardRepresentation);
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", meta = (AutoCreateRefTerm = "CompletionCallback"))
	void SetCardRepresentation(const FRealtimeMeshCardRepresentation& CardRepresentation, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	
	TFuture<ERealtimeMeshProxyUpdateStatus> ClearCardRepresentation();
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh")
	FRealtimeMeshCollisionConfiguration GetCollisionConfig() const;

	TFuture<ERealtimeMeshCollisionUpdateResult> SetCollisionConfig(const FRealtimeMeshCollisionConfiguration& InCollisionConfig);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="SetCollisionConfig")
	void SetCollisionConfig(const FRealtimeMeshCollisionConfiguration& InCollisionConfig, const FRealtimeMeshSimpleCollisionCompletionCallback& CompletionCallback);
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh")
	FRealtimeMeshSimpleGeometry GetSimpleGeometry() const;

	TFuture<ERealtimeMeshCollisionUpdateResult> SetSimpleGeometry(const FRealtimeMeshSimpleGeometry& InSimpleGeometry);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="SetSimpleGeometry")
	void SetSimpleGeometry(const FRealtimeMeshSimpleGeometry& InSimpleGeometry, const FRealtimeMeshSimpleCollisionCompletionCallback& CompletionCallback);
	
	virtual void Reset(bool bCreateNewMeshData) override;
	
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
};

#undef LOCTEXT_NAMESPACE
