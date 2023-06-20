// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "StreetMapComponent.h"
#include "StreetMapRuntime.h"

#include "StreetMapSceneProxy.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "Engine/StaticMeshActor.h"
#include "PolygonTools.h"

#include "PhysicsEngine/BodySetup.h"
#include "ProceduralMeshComponent.h"
#include "Carla/OpenDrive/OpenDriveGenerator.h"
#include "Carla/BlueprintLibary/MapGenFunctionLibrary.h"

#if WITH_EDITOR
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#endif //WITH_EDITOR

DEFINE_LOG_CATEGORY(LogNoriega);

UStreetMapComponent::UStreetMapComponent(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer),
    StreetMap(nullptr),
    CachedLocalBounds(ForceInit)
{
  // We make sure our mesh collision profile name is set to NoCollisionProfileName at initialization.
  // Because we don't have collision data yet!
  SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

  // We don't currently need to be ticked.  This can be overridden in a derived class though.
  PrimaryComponentTick.bCanEverTick = false;
  this->bAutoActivate = false;	// NOTE: Components instantiated through C++ are not automatically active, so they'll only tick once and then go to sleep!

  // We don't currently need InitializeComponent() to be called on us.  This can be overridden in a
  // derived class though.
  bWantsInitializeComponent = false;

  // Turn on shadows.  It looks better.
  CastShadow = true;

  // Our mesh is too complicated to be a useful occluder.
  bUseAsOccluder = false;

  // Our mesh can influence navigation.
  bCanEverAffectNavigation = true;

  static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterialAsset(TEXT("/StreetMap/StreetMapDefaultMaterial"));
  StreetMapDefaultMaterial = DefaultMaterialAsset.Object;

}


FPrimitiveSceneProxy* UStreetMapComponent::CreateSceneProxy()
{
  FStreetMapSceneProxy* StreetMapSceneProxy = nullptr;

  if( HasValidMesh() )
  {
    StreetMapSceneProxy = new FStreetMapSceneProxy( this );
    StreetMapSceneProxy->Init( this, Vertices, Indices );
  }

  return StreetMapSceneProxy;
}


int32 UStreetMapComponent::GetNumMaterials() const
{
  // NOTE: This is a bit of a weird thing about Unreal that we need to deal with when defining a component that
  // can have materials assigned.  UPrimitiveComponent::GetNumMaterials() will return 0, so we need to override it
  // to return the number of overridden materials, which are the actual materials assigned to the component.
  return HasValidMesh() ? GetNumMeshSections() : GetNumOverrideMaterials();
}


void UStreetMapComponent::SetStreetMap(class UStreetMap* NewStreetMap, bool bClearPreviousMeshIfAny /*= false*/, bool bRebuildMesh /*= false */)
{
  if (StreetMap != NewStreetMap)
  {
    StreetMap = NewStreetMap;

    if (bClearPreviousMeshIfAny)
      InvalidateMesh();

    if (bRebuildMesh)
      BuildMesh();

  }
}


bool UStreetMapComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{

  if (!CollisionSettings.bGenerateCollision || !HasValidMesh())
  {
    return false;
  }

  // Copy vertices data
  const int32 NumVertices = Vertices.Num();
  CollisionData->Vertices.Empty();
  CollisionData->Vertices.AddUninitialized(NumVertices);

  for (int32 VertexIndex = 0; VertexIndex < NumVertices; VertexIndex++)
  {
    CollisionData->Vertices[VertexIndex] = Vertices[VertexIndex].Position;
  }

  // Copy indices data
  const int32 NumTriangles = Indices.Num() / 3;
  FTriIndices TempTriangle;
  for (int32 TriangleIndex = 0; TriangleIndex < NumTriangles * 3; TriangleIndex += 3)
  {

    TempTriangle.v0 = Indices[TriangleIndex + 0];
    TempTriangle.v1 = Indices[TriangleIndex + 1];
    TempTriangle.v2 = Indices[TriangleIndex + 2];


    CollisionData->Indices.Add(TempTriangle);
    CollisionData->MaterialIndices.Add(0);
  }

  CollisionData->bFlipNormals = true;
  CollisionData->bDeformableMesh = true;

  return HasValidMesh();
}


bool UStreetMapComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
  return HasValidMesh() && CollisionSettings.bGenerateCollision;
}


bool UStreetMapComponent::WantsNegXTriMesh()
{
  return false;
}


void UStreetMapComponent::CreateBodySetupIfNeeded(bool bForceCreation /*= false*/)
{
  if (StreetMapBodySetup == nullptr || bForceCreation == true)
  {
    // Creating new BodySetup Object.
    StreetMapBodySetup = NewObject<UBodySetup>(this);
    StreetMapBodySetup->BodySetupGuid = FGuid::NewGuid();
    StreetMapBodySetup->bDoubleSidedGeometry = CollisionSettings.bAllowDoubleSidedGeometry;

    // shapes per poly shape for collision (Not working in simulation mode).
    StreetMapBodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
  }
}


void UStreetMapComponent::GenerateCollision()
{
  if (!CollisionSettings.bGenerateCollision || !HasValidMesh())
  {
    return;
  }

  // create a new body setup
  CreateBodySetupIfNeeded(true);


  if (GetCollisionProfileName() == UCollisionProfile::NoCollision_ProfileName)
  {
    SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
  }

  // Rebuild the body setup
  StreetMapBodySetup->InvalidatePhysicsData();
  StreetMapBodySetup->CreatePhysicsMeshes();
  UpdateNavigationIfNeeded();
}


void UStreetMapComponent::ClearCollision()
{

  if (StreetMapBodySetup != nullptr)
  {
    StreetMapBodySetup->InvalidatePhysicsData();
    StreetMapBodySetup = nullptr;
  }

  if (GetCollisionProfileName() != UCollisionProfile::NoCollision_ProfileName)
  {
    SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
  }

  UpdateNavigationIfNeeded();
}

class UBodySetup* UStreetMapComponent::GetBodySetup()
{
  if (CollisionSettings.bGenerateCollision == true)
  {
    // checking if we have a valid body setup.
    // A new one is created only if a valid body setup is not found.
    CreateBodySetupIfNeeded();
    return StreetMapBodySetup;
  }

  if (StreetMapBodySetup != nullptr) StreetMapBodySetup = nullptr;

  return nullptr;
}

void UStreetMapComponent::GenerateMesh()
{
  /////////////////////////////////////////////////////////
  // Visual tweakables for generated Street Map mesh
  //
  const float RoadZ = MeshBuildSettings.RoadOffesetZ;
  const bool bWant3DBuildings = MeshBuildSettings.bWant3DBuildings;
  const float BuildingLevelFloorFactor = MeshBuildSettings.BuildingLevelFloorFactor;
  const bool bWantLitBuildings = MeshBuildSettings.bWantLitBuildings;
  const bool bWantBuildingBorderOnGround = !bWant3DBuildings;
  const float StreetThickness = MeshBuildSettings.StreetThickness;
  const FColor StreetColor = MeshBuildSettings.StreetColor.ToFColor( false );
  const float MajorRoadThickness = MeshBuildSettings.MajorRoadThickness;
  const FColor MajorRoadColor = MeshBuildSettings.MajorRoadColor.ToFColor( false );
  const float HighwayThickness = MeshBuildSettings.HighwayThickness;
  const FColor HighwayColor = MeshBuildSettings.HighwayColor.ToFColor( false );
  const float BuildingBorderThickness = MeshBuildSettings.BuildingBorderThickness;
  FLinearColor BuildingBorderLinearColor = MeshBuildSettings.BuildingBorderLinearColor;
  const float BuildingBorderZ = MeshBuildSettings.BuildingBorderZ;
  const FColor BuildingBorderColor( BuildingBorderLinearColor.ToFColor( false ) );
  const FColor BuildingFillColor( FLinearColor( BuildingBorderLinearColor * 0.33f ).CopyWithNewOpacity( 1.0f ).ToFColor( false ) );
  /////////////////////////////////////////////////////////


  CachedLocalBounds = FBox( ForceInit );
  Vertices.Reset();
  Indices.Reset();

  if( StreetMap != nullptr )
  {
    FBox MeshBoundingBox;
    MeshBoundingBox.Init();

    const auto& Roads = StreetMap->GetRoads();
    const auto& Nodes = StreetMap->GetNodes();
    auto& Buildings = StreetMap->GetBuildings();

    /*for (const auto& Road : Roads)
    {
      float RoadThickness = StreetThickness;
      FColor RoadColor = StreetColor;
      switch( Road.RoadType )
      {
        case EStreetMapRoadType::Highway:
          RoadThickness = HighwayThickness;
          RoadColor = HighwayColor;
          break;

        case EStreetMapRoadType::MajorRoad:
          RoadThickness = MajorRoadThickness;
          RoadColor = MajorRoadColor;
          break;

        case EStreetMapRoadType::Street:
        case EStreetMapRoadType::Other:
          break;

        default:
          check( 0 );
          break;
      }

      for( int32 PointIndex = 0; PointIndex < Road.RoadPoints.Num() - 1; ++PointIndex )
      {
        AddThick2DLine(
          Road.RoadPoints[ PointIndex ],
          Road.RoadPoints[ PointIndex + 1 ],
          RoadZ,
          RoadThickness,
          RoadColor,
          RoadColor,
          MeshBoundingBox );
      }
    }
    */
    TArray< int32 > TempIndices;
    TArray< int32 > TriangulatedVertexIndices;
    TArray< FVector > TempPoints;
    for( int32 BuildingIndex = 0; BuildingIndex < Buildings.Num(); ++BuildingIndex )
    {
      auto& Building = Buildings[ BuildingIndex ];

      // Building mesh (or filled area, if the building has no height)

      // Triangulate this building
      // @todo: Performance: Triangulating lots of building polygons is quite slow.  We could easily do this
      //        as part of the import process and store tessellated geometry instead of doing this at load time.
      bool WindsClockwise;
      if( FPolygonTools::TriangulatePolygon( Building.BuildingPoints, TempIndices, /* Out */ TriangulatedVertexIndices, /* Out */ WindsClockwise ) )
      {
        // @todo: Performance: We could preprocess the building shapes so that the points always wind
        //        in a consistent direction, so we can skip determining the winding above.

        const int32 FirstTopVertexIndex = this->Vertices.Num();

        // calculate fill Z for buildings
        // either use the defined height or extrapolate from building level count
        float BuildingFillZ = 0.0f;
        if (bWant3DBuildings) {
          if (Building.Height > 0) {
            BuildingFillZ = Building.Height;
          }
          else if (Building.BuildingLevels > 0) {
            BuildingFillZ = (float)Building.BuildingLevels * BuildingLevelFloorFactor;
          }
          else {
            Building.Height = FMath::RandRange(2, 7) * BuildingLevelFloorFactor;
            BuildingFillZ = Building.Height;
          }
        }

        // Top of building
        {
          TempPoints.SetNum( Building.BuildingPoints.Num(), false );
          for( int32 PointIndex = 0; PointIndex < Building.BuildingPoints.Num(); ++PointIndex )
          {
            TempPoints[ PointIndex ] = FVector( Building.BuildingPoints[ ( Building.BuildingPoints.Num() - PointIndex ) - 1 ], BuildingFillZ );
          }
          AddTriangles( TempPoints, TriangulatedVertexIndices, FVector::ForwardVector, FVector::UpVector, BuildingFillColor, MeshBoundingBox );
        }

        if( bWant3DBuildings && (Building.Height > KINDA_SMALL_NUMBER || Building.BuildingLevels > 0) )
        {
          // NOTE: Lit buildings can't share vertices beyond quads (all quads have their own face normals), so this uses a lot more geometry!
          if( bWantLitBuildings )
          {
            // Create edges for the walls of the 3D buildings
            for( int32 LeftPointIndex = 0; LeftPointIndex < Building.BuildingPoints.Num(); ++LeftPointIndex )
            {
              const int32 RightPointIndex = ( LeftPointIndex + 1 ) % Building.BuildingPoints.Num();

              TempPoints.SetNum( 4, false );

              const int32 TopLeftVertexIndex = 0;
              TempPoints[ TopLeftVertexIndex ] = FVector( Building.BuildingPoints[ WindsClockwise ? RightPointIndex : LeftPointIndex ], BuildingFillZ );

              const int32 TopRightVertexIndex = 1;
              TempPoints[ TopRightVertexIndex ] = FVector( Building.BuildingPoints[ WindsClockwise ? LeftPointIndex : RightPointIndex ], BuildingFillZ );

              const int32 BottomRightVertexIndex = 2;
              TempPoints[ BottomRightVertexIndex ] = FVector( Building.BuildingPoints[ WindsClockwise ? LeftPointIndex : RightPointIndex ], 0.0f );

              const int32 BottomLeftVertexIndex = 3;
              TempPoints[ BottomLeftVertexIndex ] = FVector( Building.BuildingPoints[ WindsClockwise ? RightPointIndex : LeftPointIndex ], 0.0f );


              TempIndices.SetNum( 6, false );

              TempIndices[ 0 ] = BottomLeftVertexIndex;
              TempIndices[ 1 ] = TopLeftVertexIndex;
              TempIndices[ 2 ] = BottomRightVertexIndex;

              TempIndices[ 3 ] = BottomRightVertexIndex;
              TempIndices[ 4 ] = TopLeftVertexIndex;
              TempIndices[ 5 ] = TopRightVertexIndex;

              const FVector FaceNormal = FVector::CrossProduct( ( TempPoints[ 0 ] - TempPoints[ 2 ] ).GetSafeNormal(), ( TempPoints[ 0 ] - TempPoints[ 1 ] ).GetSafeNormal() );
              const FVector ForwardVector = FVector::UpVector;
              const FVector UpVector = FaceNormal;
              AddTriangles( TempPoints, TempIndices, ForwardVector, UpVector, BuildingFillColor, MeshBoundingBox );
            }
          }
          else
          {
            // Create vertices for the bottom
            const int32 FirstBottomVertexIndex = this->Vertices.Num();
            for( int32 PointIndex = 0; PointIndex < Building.BuildingPoints.Num(); ++PointIndex )
            {
              const FVector2D Point = Building.BuildingPoints[ PointIndex ];

              FStreetMapVertex& NewVertex = *new( this->Vertices )FStreetMapVertex();
              NewVertex.Position = FVector( Point, 0.0f );
              NewVertex.TextureCoordinate = FVector2D( 0.0f, 0.0f );	// NOTE: We're not using texture coordinates for anything yet
              NewVertex.TangentX = FVector::ForwardVector;	 // NOTE: Tangents aren't important for these unlit buildings
              NewVertex.TangentZ = FVector::UpVector;
              NewVertex.Color = BuildingFillColor;

              MeshBoundingBox += NewVertex.Position;
            }

            // Create edges for the walls of the 3D buildings
            for( int32 LeftPointIndex = 0; LeftPointIndex < Building.BuildingPoints.Num(); ++LeftPointIndex )
            {
              const int32 RightPointIndex = ( LeftPointIndex + 1 ) % Building.BuildingPoints.Num();

              const int32 BottomLeftVertexIndex = FirstBottomVertexIndex + LeftPointIndex;
              const int32 BottomRightVertexIndex = FirstBottomVertexIndex + RightPointIndex;
              const int32 TopRightVertexIndex = FirstTopVertexIndex + RightPointIndex;
              const int32 TopLeftVertexIndex = FirstTopVertexIndex + LeftPointIndex;

              this->Indices.Add( BottomLeftVertexIndex );
              this->Indices.Add( TopLeftVertexIndex );
              this->Indices.Add( BottomRightVertexIndex );

              this->Indices.Add( BottomRightVertexIndex );
              this->Indices.Add( TopLeftVertexIndex );
              this->Indices.Add( TopRightVertexIndex );
            }
          }
        }
      }
      else
      {
        // @todo: Triangulation failed for some reason, possibly due to degenerate polygons.  We can
        //        probably improve the algorithm to avoid this happening.
      }

      // Building border
      if( bWantBuildingBorderOnGround )
      {
        for( int32 PointIndex = 0; PointIndex < Building.BuildingPoints.Num(); ++PointIndex )
        {
          AddThick2DLine(
            Building.BuildingPoints[ PointIndex ],
            Building.BuildingPoints[ ( PointIndex + 1 ) % Building.BuildingPoints.Num() ],
            BuildingBorderZ,
            BuildingBorderThickness,		// Thickness
            BuildingBorderColor,
            BuildingBorderColor,
            MeshBoundingBox );
        }
      }
    }

    CachedLocalBounds = MeshBoundingBox;
  }
}


#if WITH_EDITOR
void UStreetMapComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  bool bNeedRefreshCustomizationModule = false;

  // Check to see if the "StreetMap" property changed.
  if (PropertyChangedEvent.Property != nullptr)
  {
    const FName PropertyName(PropertyChangedEvent.Property->GetFName());
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UStreetMapComponent, StreetMap))
    {
      bNeedRefreshCustomizationModule = true;
    }
    else if (IsCollisionProperty(PropertyName)) // For some unknown reason , GET_MEMBER_NAME_CHECKED(UStreetMapComponent, CollisionSettings) is not working ??? "TO CHECK LATER"
    {
      if (CollisionSettings.bGenerateCollision == true)
      {
        GenerateCollision();
      }
      else
      {
        ClearCollision();
      }
      bNeedRefreshCustomizationModule = true;
    }
  }

  if (bNeedRefreshCustomizationModule)
  {
    FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.NotifyCustomizationModuleChanged();
  }

  // Call the parent implementation of this function
  Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif	// WITH_EDITOR


void UStreetMapComponent::BuildMesh()
{
  // Wipes out our cached mesh data. Maybe unnecessary in case GenerateMesh is clearing cached mesh data and creating a new SceneProxy  !
  InvalidateMesh();

  GenerateMesh();

  if (HasValidMesh())
  {
    // We have a new bounding box
    UpdateBounds();
  }
  else
  {
    // No mesh was generated
  }

  GenerateCollision();

  // Mark our render state dirty so that CreateSceneProxy can refresh it on demand
  MarkRenderStateDirty();

  AssignDefaultMaterialIfNeeded();

  Modify();
}


void UStreetMapComponent::AssignDefaultMaterialIfNeeded()
{
  if (this->GetNumMaterials() == 0 || this->GetMaterial(0) == nullptr)
  {
    if (!HasValidMesh() || GetDefaultMaterial() == nullptr)
      return;

    this->SetMaterial(0, GetDefaultMaterial());
  }
}


void UStreetMapComponent::UpdateNavigationIfNeeded()
{
  if (bCanEverAffectNavigation || bNavigationRelevant)
  {
    FNavigationSystem::UpdateComponentData(*this);
  }
}

void UStreetMapComponent::InvalidateMesh()
{
  Vertices.Reset();
  Indices.Reset();
  CachedLocalBounds = FBoxSphereBounds(FBox(ForceInit));
  ClearCollision();
  // Mark our render state dirty so that CreateSceneProxy can refresh it on demand
  MarkRenderStateDirty();
  Modify();
}

FBoxSphereBounds UStreetMapComponent::CalcBounds( const FTransform& LocalToWorld ) const
{
  if( HasValidMesh() )
  {
    FBoxSphereBounds WorldSpaceBounds = CachedLocalBounds.TransformBy( LocalToWorld );
    WorldSpaceBounds.BoxExtent *= BoundsScale;
    WorldSpaceBounds.SphereRadius *= BoundsScale;
    return WorldSpaceBounds;
  }
  else
  {
    return FBoxSphereBounds( LocalToWorld.GetLocation(), FVector::ZeroVector, 0.0f );
  }
}


void UStreetMapComponent::AddThick2DLine( const FVector2D Start, const FVector2D End, const float Z, const float Thickness, const FColor& StartColor, const FColor& EndColor, FBox& MeshBoundingBox )
{
  const float HalfThickness = Thickness * 0.5f;

  const FVector2D LineDirection = ( End - Start ).GetSafeNormal();
  const FVector2D RightVector( -LineDirection.Y, LineDirection.X );

  const int32 BottomLeftVertexIndex = Vertices.Num();
  FStreetMapVertex& BottomLeftVertex = *new( Vertices )FStreetMapVertex();
  BottomLeftVertex.Position = FVector( Start - RightVector * HalfThickness, Z );
  BottomLeftVertex.TextureCoordinate = FVector2D( 0.0f, 0.0f );
  BottomLeftVertex.TangentX = FVector( LineDirection, 0.0f );
  BottomLeftVertex.TangentZ = FVector::UpVector;
  BottomLeftVertex.Color = StartColor;
  MeshBoundingBox += BottomLeftVertex.Position;

  const int32 BottomRightVertexIndex = Vertices.Num();
  FStreetMapVertex& BottomRightVertex = *new( Vertices )FStreetMapVertex();
  BottomRightVertex.Position = FVector( Start + RightVector * HalfThickness, Z );
  BottomRightVertex.TextureCoordinate = FVector2D( 1.0f, 0.0f );
  BottomRightVertex.TangentX = FVector( LineDirection, 0.0f );
  BottomRightVertex.TangentZ = FVector::UpVector;
  BottomRightVertex.Color = StartColor;
  MeshBoundingBox += BottomRightVertex.Position;

  const int32 TopRightVertexIndex = Vertices.Num();
  FStreetMapVertex& TopRightVertex = *new( Vertices )FStreetMapVertex();
  TopRightVertex.Position = FVector( End + RightVector * HalfThickness, Z );
  TopRightVertex.TextureCoordinate = FVector2D( 1.0f, 1.0f );
  TopRightVertex.TangentX = FVector( LineDirection, 0.0f );
  TopRightVertex.TangentZ = FVector::UpVector;
  TopRightVertex.Color = EndColor;
  MeshBoundingBox += TopRightVertex.Position;

  const int32 TopLeftVertexIndex = Vertices.Num();
  FStreetMapVertex& TopLeftVertex = *new( Vertices )FStreetMapVertex();
  TopLeftVertex.Position = FVector( End - RightVector * HalfThickness, Z );
  TopLeftVertex.TextureCoordinate = FVector2D( 0.0f, 1.0f );
  TopLeftVertex.TangentX = FVector( LineDirection, 0.0f );
  TopLeftVertex.TangentZ = FVector::UpVector;
  TopLeftVertex.Color = EndColor;
  MeshBoundingBox += TopLeftVertex.Position;

  Indices.Add( BottomLeftVertexIndex );
  Indices.Add( BottomRightVertexIndex );
  Indices.Add( TopRightVertexIndex );

  Indices.Add( BottomLeftVertexIndex );
  Indices.Add( TopRightVertexIndex );
  Indices.Add( TopLeftVertexIndex );
};


void UStreetMapComponent::AddTriangles( const TArray<FVector>& Points, const TArray<int32>& PointIndices, const FVector& ForwardVector, const FVector& UpVector, const FColor& Color, FBox& MeshBoundingBox )
{
  const int32 FirstVertexIndex = Vertices.Num();
  const int32 FirstProcVertexIndex = VerticesPositions.Num();

  for( FVector Point : Points )
  {
    FStreetMapVertex& NewVertex = *new( Vertices )FStreetMapVertex();
    NewVertex.Position = Point;
    NewVertex.TextureCoordinate = FVector2D( 0.0f, 0.0f );	// NOTE: We're not using texture coordinates for anything yet
    NewVertex.TangentX = ForwardVector;
    NewVertex.TangentZ = UpVector;
    NewVertex.Color = Color;
    VerticesPositions.Add(Point);
    VerticesNormals.Add(UpVector);
    MeshBoundingBox += NewVertex.Position;
  }

  for( int32 PointIndex : PointIndices )
  {
    Indices.Add( FirstVertexIndex + PointIndex );
    ProcIndices.Add(FirstProcVertexIndex + PointIndex);
  }
};


FString UStreetMapComponent::GetStreetMapAssetName() const
{
  return StreetMap != nullptr ? StreetMap->GetName() : FString(TEXT("NONE"));
}

TArray<AActor*> UStreetMapComponent::GenerateTopsOfBuildings(FString MapName, UMaterialInstance* MaterialInstance)
{
  TArray<AActor*> Returning;
  const float BuildingLevelFloorFactor = MeshBuildSettings.BuildingLevelFloorFactor;
  auto& Buildings = StreetMap->GetBuildings();
  for( int32 BuildingIndex = 0; BuildingIndex < Buildings.Num(); ++BuildingIndex )
  {
    Returning.Add(GenerateTopOfBuilding(BuildingIndex, MapName, MaterialInstance));
  }

  return Returning;
}

AActor* UStreetMapComponent::GenerateTopOfBuilding(int Index, FString MapName, UMaterialInstance* MaterialInstance)
{
  const float BuildingLevelFloorFactor = MeshBuildSettings.BuildingLevelFloorFactor;
  auto& Buildings = StreetMap->GetBuildings();

  auto& Building = Buildings[ Index ];
  AStaticMeshActor* TempActor = GetWorld()->SpawnActor<AStaticMeshActor>();
  UStaticMeshComponent* StaticMeshComponent = TempActor->GetStaticMeshComponent();

  TempActor->SetActorLabel(FString("SM_Roof_") + FString::FromInt(Index));
  StaticMeshComponent->CastShadow = false;

  TArray<FVector> BPositions;
  TArray<FVector> BNormals;
  TArray<int32> BIndices;
  TArray<int32> TempIndices;
  TArray<int32> TriangulatedVertexIndices;
  TArray<FVector> TempPoints;
  // Building mesh (or filled area, if the building has no height)

  // Triangulate this building
  // @todo: Performance: Triangulating lots of building polygons is quite slow.  We could easily do this
  //        as part of the import process and store tessellated geometry instead of doing this at load time.
  bool WindsClockwise;
  if( FPolygonTools::TriangulatePolygon( Building.BuildingPoints, TempIndices, /* Out */ TriangulatedVertexIndices, /* Out */ WindsClockwise ) )
  {
    const int32 FirstTopVertexIndex = this->Vertices.Num();

    // calculate fill Z for buildings
    // either use the defined height or extrapolate from building level count
    float BuildingFillZ = 0.0f;
    if (Building.Height > 0) {
      BuildingFillZ = Building.Height;
    }
    else if (Building.BuildingLevels > 0) {
      BuildingFillZ = (float)Building.BuildingLevels * BuildingLevelFloorFactor;
    }
    else {
      Building.Height = FMath::RandRange(2, 7) * BuildingLevelFloorFactor;
      BuildingFillZ = Building.Height;
    }

    // Top of building
    {
      TempPoints.SetNum( Building.BuildingPoints.Num(), false );
      for( int32 PointIndex = 0; PointIndex < Building.BuildingPoints.Num(); ++PointIndex )
      {
        TempPoints[ PointIndex ] = FVector( Building.BuildingPoints[ ( Building.BuildingPoints.Num() - PointIndex ) - 1 ], BuildingFillZ );
      }

      if(WindsClockwise){
        for( int32 PointIndex = 0; PointIndex < Building.BuildingPoints.Num(); PointIndex++ )
        {
          BPositions.Add( FVector(Building.BuildingPoints[PointIndex], BuildingFillZ) );
          BNormals.Add(FVector::UpVector);
        }
      }
      else
      {
        for( int32 PointIndex = 0; PointIndex < Building.BuildingPoints.Num(); PointIndex++ )
        {
          int RealIndex = ( Building.BuildingPoints.Num() - 1 ) - PointIndex;
          BPositions.Add( FVector(Building.BuildingPoints[RealIndex], BuildingFillZ) );
          BNormals.Add(FVector::UpVector);
        }
      }
      for( int32 PointIndex : TriangulatedVertexIndices )
      {
        BIndices.Add( PointIndex );
      }
    }

    FVector MeshCentroid = FVector(0,0,0);
    for( const auto& Vertex : BPositions )
    {
      MeshCentroid += Vertex;
    }

    MeshCentroid /= BPositions.Num();

    for( auto& Vertex : BPositions )
    {
      Vertex.X -= MeshCentroid.X;
      Vertex.Y -= MeshCentroid.Y;
      Vertex.Z -= MeshCentroid.Z;
    }
    FProceduralCustomMesh MeshData;
    MeshData.Vertices = BPositions;
    MeshData.Triangles = BIndices;
    MeshData.Normals = BNormals;
    TArray<FProcMeshTangent> Tangents;
    UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData,  Tangents, MaterialInstance, MapName, "Roofs", FName(TEXT("SM_RoofMesh" + FString::FromInt(Index) )));

    StaticMeshComponent->SetStaticMesh(MeshToSet);
    TempActor->SetActorLocation( MeshCentroid );
  }
  return TempActor;
}
