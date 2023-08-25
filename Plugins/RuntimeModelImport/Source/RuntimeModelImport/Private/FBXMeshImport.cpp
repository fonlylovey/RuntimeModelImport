#include "FBXMeshImport.h"
#include "../Public/RMIDelegates.h"
#include "../Public/RuntimeActor.h"
#include <ModelOperator.h>

FBXMeshImport::FBXMeshImport()
	: m_pRootMesh(nullptr)
{
}

FBXMeshImport::~FBXMeshImport()
{
	SectionList.Empty();
	MeshList.Empty();
}

TSharedPtr<FModelMesh> FBXMeshImport::LoadMesh(FbxScene* scene)
{
	FbxNode* rootNode = scene->GetRootNode();
	if (rootNode)
	{
		//获取全部子节点类型为eMesh的数量
		meshCount = GetRootNodeTypeNum(rootNode, FbxNodeAttribute::EType::eMesh, true);
		readIndex = 0;
		
		//转中心轴 
		//rootNode->ConvertPivotAnimationRecursive(NULL, FbxNode::EPivotSet::eDestinationPivot, 30);

		m_pRootMesh = MakeShared<FModelMesh>(rootNode->GetUniqueID(), UTF8_TO_TCHAR(rootNode->GetName()));

		m_pRootMesh->IsRoot = true;
		MeshList.Add(m_pRootMesh);
		traverseNode(rootNode, m_pRootMesh);
		return m_pRootMesh;
	}
	return m_pRootMesh;
}

void FBXMeshImport::traverseNode(FbxNode* pNode, TSharedPtr<FModelMesh> pMesh)
{
	if (pMesh == nullptr)
		return;

	//拿到FbxNode的名称
	FString sNodeName = FString(UTF8_TO_TCHAR(pNode->GetName()));
	//矩阵计算
	pMesh->MeshMatrix = readTransform(pNode);

	//根据类型转, 目前只处理了eMesh
	FbxNodeAttribute::EType attributeType = FbxNodeAttribute::EType::eUnknown;
	if (pNode->GetNodeAttribute())
	{
		attributeType = pNode->GetNodeAttribute()->GetAttributeType();
		switch (attributeType)
		{
		case FbxNodeAttribute::eMesh:
		{
			//ReadMeshInformation
			readMesh(pNode, pMesh);

			//呼叫主线程去SpawnActor
			Async(EAsyncExecution::TaskGraphMainThread, [=]()
				{
					FString strInfo = TEXT("读取模型几何数据-：") + sNodeName;
					++readIndex;
					FRMIDelegates::OnImportProgressDelegate.Broadcast(ERMIImportProgressStage::Stage1_2_ReadMesh,
						readIndex, MeshNodeCount, strInfo);
					UE_LOG(LogTemp, Log, TEXT("%s"), *strInfo);
				});
		}
			break;
		case FbxNodeAttribute::eSkeleton:
			//loadSkeleton(pNode);
			break;
		case FbxNodeAttribute::eLight:
			//loadLight(pNode);
			break;
		case FbxNodeAttribute::eCamera:
			//loadCamera(pNode);
			break;
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); ++i)
	{
		FbxNode* child = pNode->GetChild(i);
		FString strChildName = FString(UTF8_TO_TCHAR(child->GetName()));
		TSharedPtr<FModelMesh, ESPMode::ThreadSafe> pUMeshObj = MakeShareable(new FModelMesh(child->GetUniqueID(), strChildName));
		pMesh->Children.Add(pUMeshObj);
		pUMeshObj->Parent = pMesh;
		pUMeshObj->ParentID = pMesh->MeshID;
		traverseNode(child, pUMeshObj);
	}
}

void FBXMeshImport::readMesh(FbxNode* pNode, TSharedPtr<FModelMesh> pMesh)
{
	if (pNode == nullptr || pMesh == nullptr)
		return;

	FbxMesh* fbxMesh = pNode->GetMesh();
	if (fbxMesh != nullptr)
	{
		//如果模型没有三角化，先转换成三角形
		
		if (!fbxMesh->IsTriangleMesh())
		{
			FbxGeometryConverter GeometryConverter(pNode->GetFbxManager());
			FbxNodeAttribute* ConvertedNode = GeometryConverter.Triangulate(pNode->GetNodeAttribute(), true);
			if (ConvertedNode != nullptr && ConvertedNode->GetAttributeType() == FbxNodeAttribute::eMesh)
			{
				fbxMesh = (FbxMesh*)ConvertedNode;
			}
			else
			{
				FString strMsg = "Mesh 三角化失败:" + FString(UTF8_TO_TCHAR(pNode->GetName()));
				UE_LOG(LogTemp, Error, TEXT("%s"), *strMsg);
			}
		}

		//控制点
		FbxVector4* pVertexArray = fbxMesh->GetControlPoints();
		//顶点颜色
		FbxLayerElementVertexColor* pVertexColorArray = fbxMesh->GetElementVertexColor();
		//法线
		FbxLayerElementNormal* pVertexNormalArray = fbxMesh->GetElementNormal(0);
		//UV
		FbxLayerElementUV* pUV0Array = fbxMesh->GetElementUV(0);
		FbxLayerElementUV* pUV1Array = fbxMesh->GetElementUV(1);
		//顶点切线
		FbxLayerElementTangent* pTangentArray = fbxMesh->GetElementTangent(0);
		//材质索引, 每个三角面上对应了一个材质ID, 可以根据这个拆分section
		FbxLayerElementMaterial* fbxLayerElementMat = fbxMesh->GetElementMaterial();

		FString strName = UTF8_TO_TCHAR(pNode->GetName());

		//计算pNode局部到世界的变换矩阵
		globalMatrix = localToWorldMatrix(pNode);

		//如果Mesh没有材质, fbxLayerElementMat就为空, 默认给一个为0材质索引. 避免崩溃
		FbxLayerElementArrayTemplate<int> indexArray = FbxLayerElementArrayTemplate<int>(EFbxType::eFbxInt);
		if (fbxLayerElementMat != nullptr)
		{
			indexArray = fbxLayerElementMat->GetIndexArray();
			//读取当前Node引用的材质IDs
			if (pNode->GetMaterialCount() > 0)
			{
				MatIndexArray = readNodeMaterialIDs(pNode);
			}
			
			//有可能遇到Revit模型有材质，但是导出Fbx材质丢失
			if (MatIndexArray.Num() == 0)
			{
				MatIndexArray.Add(-1);
			}
		}
		else
		{
			MatIndexArray = { -1 };
		}
		
		MeshList.Add(pMesh);
		MeshNodeCount++;

		//三角面的
		int triangleCount = fbxMesh->GetPolygonCount();
		int count = fbxMesh->GetPolygonVertexCount();
		int vertexCount = fbxMesh->GetControlPointsCount();
		int matCount = pNode->GetMaterialCount();

		int meshVexIndex = 0;
		//将Section的大小设置为材质数量的大小
		pMesh->SectionList.Reset(matCount);

		TMap<int, TSharedPtr<FRuntimeMeshSectionData>> MatIndexSectionMap;
		//挨个读取多边形
		for (int i = 0; i < triangleCount; i++)
		{
			//获取当前多边形所使用的材质索引[0, matCount]
			const int matIndex = indexArray[i];
			
			//按照材质索引获取或者创建Section   pMesh->SectionList.Num() > matIndex
			TSharedPtr<FRuntimeMeshSectionData> pSection = nullptr;
			if (MatIndexSectionMap.Contains(matIndex))
			{
				pSection = MatIndexSectionMap[matIndex];
			}
			else
			{
				
				pSection = MakeShared<FRuntimeMeshSectionData>();
				pSection->MeshData.Triangles = FRuntimeMeshTriangleStream(true);
				pSection->MeshData.TexCoords = FRuntimeMeshVertexTexCoordStream(true);
				pSection->MeshData.Tangents = FRuntimeMeshVertexTangentStream(true);
				pMesh->SectionList.Add(pSection);
				MatIndexSectionMap.Add(matIndex, pSection);
				int32 maiID = -1;
				if (matIndex > 0 && matIndex < MatIndexArray.Num())
				{
					maiID = MatIndexArray[matIndex];
				}
				
				pSection->Properties.MaterialSlot = maiID;
				SectionList.Add(pSection);
				MeshNodeCount++;
			}

			//循环多边形的每个顶点
			int ployVexterSize = fbxMesh->GetPolygonSize(i);
			for (int j = 0; j < ployVexterSize; j++)
			{
				//获取该顶点在mesh顶点数组中的顶点索引, i第几个三角形, j是三角形的第几个顶点
				int vertexIndex = fbxMesh->GetPolygonVertex(i, j);

				//读取顶点
				FVector3f Position = readVertex(pVertexArray, vertexIndex);

				//顶点颜色
				FColor Color = FColor(1, 1, 1, 1);
				if (pVertexColorArray != nullptr)
				{
					Color = readColor(pVertexColorArray, vertexIndex, meshVexIndex);
				}
				pSection->MeshData.Colors.Add(Color);

				//读取UV
				int textureIndex = fbxMesh->GetTextureUVIndex(i, j);
				if (pUV0Array != nullptr)
				{
					FVector2f UV0 = readUV(pUV0Array, vertexIndex, textureIndex, 0);
					pSection->MeshData.TexCoords.Add(UV0, 0);
				}
				else if (pUV1Array != nullptr)
				{
					FVector2f UV1 = readUV(pUV1Array, vertexIndex, textureIndex, 1);
					pSection->MeshData.TexCoords.Add(UV1, 0);
				}
				else
				{
					pSection->MeshData.TexCoords.Add(FVector2f(0, 0), 0);
				}

				//读取法线
				FVector3f Normal;
				if (pVertexNormalArray != nullptr)
				{
					Normal = readNormal(pVertexNormalArray, vertexIndex, meshVexIndex);
				}

				//读取切线
				FVector3f Tangent;
				if (pTangentArray != nullptr)
				{
					Tangent = readTangent(pTangentArray, vertexIndex, meshVexIndex);
				}

				pSection->MeshData.Tangents.Add(Normal, Tangent);
				//添加的时候获取顶点索引, 待优化项 目的节省内存
				int index = pSection->MeshData.Positions.Add(Position);
				pSection->MeshData.Triangles.Add(index);
				meshVexIndex++;
			}

		}
	}
}

FVector3f FBXMeshImport::readVertex(FbxVector4* meshVertexArray, int vertexIndex)
{
	FbxVector4 vec4 = meshVertexArray[vertexIndex];
	FVector3f vertex = ConvertToRightUEPos(globalMatrix.MultT(vec4));
	return vertex;
}

FVector3f FBXMeshImport::readNormal(FbxLayerElementNormal* pVertexNormalArray, int vertexIndex, int meshVertexIndex)
{
	FbxVector4 normal;
	switch (pVertexNormalArray->GetMappingMode())
	{
	case FbxLayerElement::eByControlPoint://绑定到控制点上面的
	{
		if (pVertexNormalArray->GetReferenceMode() == FbxLayerElement::eDirect)
		{
			normal = pVertexNormalArray->GetDirectArray().GetAt(vertexIndex);
		}
		else  if (pVertexNormalArray->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
		{
			int index = pVertexNormalArray->GetIndexArray().GetAt(vertexIndex);
			normal = pVertexNormalArray->GetDirectArray().GetAt(index);
		}
	}
	break;
	case FbxLayerElement::eByPolygonVertex://绑定到面上面的顶点
	{
		if (pVertexNormalArray->GetReferenceMode() == FbxLayerElement::eDirect)
		{
			normal = pVertexNormalArray->GetDirectArray().GetAt(meshVertexIndex);
		}
		else if (pVertexNormalArray->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
		{
			int index = pVertexNormalArray->GetIndexArray().GetAt(meshVertexIndex);
			normal = pVertexNormalArray->GetDirectArray().GetAt(index);
		}
	}
	break;
	}
	FbxAMatrix normalMax = globalMatrix.Inverse();
	normalMax = normalMax.Transpose();
	normal = normalMax.MultT(normal);
	FVector3f ueNormal = ConvertToRightUEPos(normal);
	ueNormal = ueNormal.GetSafeNormal();
	return ueNormal;
}

FColor FBXMeshImport::readColor(FbxLayerElementVertexColor* pColorArray, int vertexIndex, int meshVertexIndex)
{
	FColor color;
	switch (pColorArray->GetMappingMode())
	{
	case FbxLayerElement::eByControlPoint:
	{
		if (pColorArray->GetReferenceMode() == FbxLayerElement::eDirect)
		{
			color.R = pColorArray->GetDirectArray().GetAt(vertexIndex)[0];
			color.G = pColorArray->GetDirectArray().GetAt(vertexIndex)[1];
			color.B = pColorArray->GetDirectArray().GetAt(vertexIndex)[2];
			color.A = pColorArray->GetDirectArray().GetAt(vertexIndex)[3];
		}
		else  if (pColorArray->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
		{
			int index = pColorArray->GetIndexArray().GetAt(vertexIndex);
			color.R = pColorArray->GetDirectArray().GetAt(index)[0];
			color.G = pColorArray->GetDirectArray().GetAt(index)[1];
			color.B = pColorArray->GetDirectArray().GetAt(index)[2];
			color.A = pColorArray->GetDirectArray().GetAt(index)[3];
		}
	}
	break;
	case FbxLayerElement::eByPolygonVertex:
	{
		if (pColorArray->GetReferenceMode() == FbxLayerElement::eDirect)
		{
			color.R = pColorArray->GetDirectArray().GetAt(meshVertexIndex)[0];
			color.G = pColorArray->GetDirectArray().GetAt(meshVertexIndex)[1];
			color.B = pColorArray->GetDirectArray().GetAt(meshVertexIndex)[2];
			color.A = pColorArray->GetDirectArray().GetAt(meshVertexIndex)[3];
		}
		else if (pColorArray->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
		{
			int index = pColorArray->GetIndexArray().GetAt(meshVertexIndex);
			color.R = pColorArray->GetDirectArray().GetAt(index)[0];
			color.G = pColorArray->GetDirectArray().GetAt(index)[1];
			color.B = pColorArray->GetDirectArray().GetAt(index)[2];
			color.A = pColorArray->GetDirectArray().GetAt(index)[3];
		}
	}
	break;
	}
	return color;
}

FVector2f FBXMeshImport::readUV(FbxLayerElementUV* pUVArray, int vertexIndex, int meshTextureIndex, int layer)
{
	FVector2f uv;
	if (pUVArray->GetMappingMode() == FbxLayerElement::eByControlPoint)
	{
		if (pUVArray->GetReferenceMode() == FbxLayerElement::eDirect)
		{
			uv.X = pUVArray->GetDirectArray().GetAt(vertexIndex)[0];
			uv.Y = pUVArray->GetDirectArray().GetAt(vertexIndex)[1];
		}
		else  if (pUVArray->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
		{
			int index = pUVArray->GetIndexArray().GetAt(vertexIndex);
			uv.X = pUVArray->GetDirectArray().GetAt(index)[0];
			uv.Y = pUVArray->GetDirectArray().GetAt(index)[1];
		}
	}
	else
	{
		uv.X = pUVArray->GetDirectArray().GetAt(meshTextureIndex)[0];
		uv.Y = pUVArray->GetDirectArray().GetAt(meshTextureIndex)[1];
	}
	return uv;
}

FVector3f FBXMeshImport::readTangent(FbxLayerElementTangent* pTangentArray, int vertexIndex, int meshVertexIndex)
{
	FVector3f tangent;
	switch (pTangentArray->GetMappingMode())
	{
	case FbxLayerElement::eByControlPoint:
	{
		if (pTangentArray->GetReferenceMode() == FbxLayerElement::eDirect)
		{
			tangent.X = pTangentArray->GetDirectArray().GetAt(vertexIndex)[0];
			tangent.Y = pTangentArray->GetDirectArray().GetAt(vertexIndex)[1];
			tangent.Z = pTangentArray->GetDirectArray().GetAt(vertexIndex)[2];
		}
		else  if (pTangentArray->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
		{
			int id = pTangentArray->GetIndexArray().GetAt(vertexIndex);
			tangent.X = pTangentArray->GetDirectArray().GetAt(id)[0];
			tangent.Y = pTangentArray->GetDirectArray().GetAt(id)[1];
			tangent.Z = pTangentArray->GetDirectArray().GetAt(id)[2];
		}
	}
	break;
	case FbxLayerElement::eByPolygonVertex:
	{
		if (pTangentArray->GetReferenceMode() == FbxLayerElement::eDirect)
		{
			tangent.X = pTangentArray->GetDirectArray().GetAt(meshVertexIndex)[0];
			tangent.Y = pTangentArray->GetDirectArray().GetAt(meshVertexIndex)[1];
			tangent.Z = pTangentArray->GetDirectArray().GetAt(meshVertexIndex)[2];
		}
		else if (pTangentArray->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
		{
			int index = pTangentArray->GetIndexArray().GetAt(meshVertexIndex);
			tangent.X = pTangentArray->GetDirectArray().GetAt(index)[0];
			tangent.Y = pTangentArray->GetDirectArray().GetAt(index)[1];
			tangent.Z = pTangentArray->GetDirectArray().GetAt(index)[2];
		}
	}
	break;
	}
	return tangent;
}

FTransform FBXMeshImport::readTransform(FbxNode* pNode)
{

	FVector3f RotOffset = ConvertToRightUEPos(pNode->GetRotationOffset(FbxNode::EPivotSet::eSourcePivot));
	FVector3f ScaleOffset = ConvertToRightUEPos(pNode->GetScalingOffset(FbxNode::EPivotSet::eSourcePivot));

	FbxVector4& LocalTrans = pNode->GetScene()->GetAnimationEvaluator()->GetNodeLocalTranslation(pNode);
	FbxVector4& LocalScale = pNode->GetScene()->GetAnimationEvaluator()->GetNodeLocalScaling(pNode);
	FbxVector4& LocalRot = pNode->GetScene()->GetAnimationEvaluator()->GetNodeLocalRotation(pNode);

	FbxAMatrix Temp = pNode->EvaluateLocalTransform();
	FVector4f Trans = ConvertToRightUEPos(Temp.GetT());
	FVector4f Scale = ConvertScale(Temp.GetS());
	FQuat Rotation = ConvertRotToQuat(Temp.GetR());

	FVector RotEuler = Rotation.Euler();

	// Avoid singularity around 90 degree pitch, as UE4 doesn't seem to support it very well
	// See UE-75467 and UE-83049
	if (FMath::IsNearlyEqual(abs(RotEuler.Y), 90.0f))
	{
		Rotation.W += 1e-3;
		Rotation.Normalize();
	}

	// Converting exactly 180.0 degree quaternions into Euler is unreliable, so add some
	// small noise so that it produces the correct actor transform
	if (abs(RotEuler.X) == 180.0f ||
		abs(RotEuler.Y) == 180.0f ||
		abs(RotEuler.Z) == 180.0f)
	{
		Rotation.W += 1.e-7;
		Rotation.Normalize();
	}

	FbxVector4 geoTrans = pNode->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot);
	FbxVector4 geoScale = pNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot);
	FbxVector4 geoRot = pNode->GetGeometricRotation(FbxNode::EPivotSet::eSourcePivot);

	FTransform FbxGeo;
	FbxGeo.SetTranslation(FVector(ConvertToRightUEPos(geoTrans)));
	FbxGeo.SetScale3D(FVector(ConvertScale(geoScale)));
	FbxGeo.SetRotation(FQuat(ConvertRotToQuat(geoRot)));

	FTransform NewTransform;
	NewTransform.SetTranslation(FVector(Trans + RotOffset + ScaleOffset));
	NewTransform.SetScale3D(FVector(Scale)); 
	NewTransform.SetRotation(FQuat(Rotation));
	return FbxGeo * NewTransform;
}

FbxAMatrix FBXMeshImport::localToWorldMatrix(FbxNode* pNode)
{
	FbxAMatrix globalTransform = pNode->EvaluateGlobalTransform();

	FbxVector4 geoT = pNode->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot);
	FbxVector4 geoR = pNode->GetGeometricRotation(FbxNode::EPivotSet::eSourcePivot);
	FbxVector4 geoS = pNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot);

	FbxAMatrix geoMatrix = FbxAMatrix(geoT, geoR, geoS);

	return globalTransform * geoMatrix;
}

TArray<int32> FBXMeshImport::readNodeMaterialIDs(FbxNode* pNode)
{
	TArray<int32> matSet;
	FbxMesh* fbxMesh = pNode->GetMesh();
	if (fbxMesh == nullptr) return matSet;

	FbxLayerElementMaterial* fbxLayerElementMat = fbxMesh->GetElementMaterial();
	
	int matCount = pNode->GetMaterialCount();
	for (int i = 0; i < matCount; i++)
	{
		FbxSurfaceMaterial* pSurfaceMaterial = nullptr;
		if (fbxLayerElementMat->GetMappingMode() == FbxLayerElement::eByPolygon)
		{
			pSurfaceMaterial = pNode->GetMaterial(i);
		}
		else if (fbxLayerElementMat->GetMappingMode() == FbxLayerElement::eAllSame)
		{
			int index = fbxLayerElementMat->GetIndexArray()[0];
			pSurfaceMaterial = pNode->GetMaterial(index);
		}
		if (pSurfaceMaterial == nullptr)
		{
			continue;
		}
		int32 uniqueID = pSurfaceMaterial->GetUniqueID();
		matSet.Add(uniqueID);
	}
	return matSet;
}

FVector3f FBXMeshImport::ConvertToRightUEPos(FbxVector4 Vector)
{
	FVector3f Out;
	Out[0] = Vector[0];
	Out[1] = -Vector[1];
	Out[2] = Vector[2];
	return Out;
}

FVector3f FBXMeshImport::ConvertScale(FbxVector4 Vector)
{
	FVector3f Out;
	Out[0] = Vector[0];
	Out[1] = Vector[1];
	Out[2] = Vector[2];
	return Out;
}

FQuat FBXMeshImport::ConvertRotToQuat(FbxVector4 Vector)
{
	FbxAMatrix temp;
	temp.SetIdentity();
	temp.SetR(Vector);
	FbxQuaternion quat = temp.GetQ();
	FQuat UnrealQuat;
	UnrealQuat.X = quat[0];
	UnrealQuat.Y = -quat[1];
	UnrealQuat.Z = quat[2];
	UnrealQuat.W = -quat[3];
	return UnrealQuat;
}

int32 FBXMeshImport::GetRootNodeTypeNum(FbxNode* pNode, FbxNodeAttribute::EType emType, bool bRecursion)
{
	if (!pNode) return 0;

	int32 nRet = 0;
	if (pNode->GetNodeAttribute())
	{
		FbxNodeAttribute::EType attributeType = pNode->GetNodeAttribute()->GetAttributeType();
		if(attributeType == emType)
		{
			++nRet;
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); ++i)
	{
		FbxNode* pChildNode = pNode->GetChild(i);
		nRet += GetRootNodeTypeNum(pChildNode, emType, bRecursion);
	}

	return nRet;
}

