/*==============================================================================

  Copyright (c) Radiation Medicine Program, University Health Network,
  Princess Margaret Hospital, Toronto, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kevin Wang, Princess Margaret Cancer Centre 
  and was supported by Cancer Care Ontario (CCO)'s ACRU program 
  with funds provided by the Ontario Ministry of Health and Long-Term Care
  and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).

==============================================================================*/

// SlicerRt includes
#include "PlmCommon.h"
#include "vtkMRMLRTBeamNode.h"
#include "vtkMRMLRTPlanNode.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkSlicerSegmentationsModuleLogic.h"

// Plastimatch includes
#include "image_center.h"

// MRML includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLSubjectHierarchyConstants.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkIntArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>

//------------------------------------------------------------------------------
const char* vtkMRMLRTBeamNode::NEW_BEAM_NODE_NAME_PREFIX = "NewBeam_";

//------------------------------------------------------------------------------
static const char* ISOCENTER_FIDUCIAL_REFERENCE_ROLE = "isocenterFiducialRef";
static const char* TARGET_SEGMENTATION_REFERENCE_ROLE = "targetContourRef";
static const char* MLCPOSITION_REFERENCE_ROLE = "MLCPositionRef";
static const char* DRR_REFERENCE_ROLE = "DRRRef";
static const char* CONTOUR_BEV_REFERENCE_ROLE = "contourBEVRef";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLRTBeamNode);

//----------------------------------------------------------------------------
vtkMRMLRTBeamNode::vtkMRMLRTBeamNode()
{
  this->BeamNumber = -1;
  this->BeamDescription = NULL;

  this->TargetSegmentID = NULL;

  this->BeamType = vtkMRMLRTBeamNode::Static;
  this->RadiationType = vtkMRMLRTBeamNode::Proton;
  this->CollimatorType = vtkMRMLRTBeamNode::SquareHalfMM;

  this->NominalEnergy = 80.0;
  this->NominalmA = 1.0;
  this->BeamOnTime = 0.0;

  this->IsocenterSpecification = CenterOfTarget;
  this->Isocenter[0] = 0.0;
  this->Isocenter[1] = 0.0;
  this->Isocenter[2] = 0.0;  
  this->ReferenceDosePoint[0] = 0.0;
  this->ReferenceDosePoint[1] = 0.0;
  this->ReferenceDosePoint[2] = 0.0;  

  this->X1Jaw = -100.0;
  this->X2Jaw = 100.0;
  this->Y1Jaw = -100.0;
  this->Y2Jaw = 100.0;

  this->GantryAngle = 0.0;
  this->CollimatorAngle = 0.0;
  this->CouchAngle = 0.0;
  this->Smearing = 0.0;

  this->SAD = 2000.0;
  this->BeamWeight = 1.0;

  // Register parent transform modified event
  //TODO: Needed?
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  vtkObserveMRMLObjectEventsMacro(this, events);
}

//----------------------------------------------------------------------------
vtkMRMLRTBeamNode::~vtkMRMLRTBeamNode()
{
  this->SetBeamDescription(NULL);
  this->SetTargetSegmentID(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  if (this->TargetSegmentID != NULL) 
  {
    of << indent << " TargetSegmentID=\"" << this->TargetSegmentID << "\"";
  }
  //TODO: Beam parameters
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName = NULL;
  const char* attValue = NULL;

  while (*atts != NULL) 
  {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "TargetSegmentID")) 
    {
      std::stringstream ss;
      ss << attValue;
      this->SetTargetSegmentID(ss.str().c_str());
    }
    //TODO: Beam parameters
  }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLRTBeamNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  this->DisableModifiedEventOn();

  vtkMRMLRTBeamNode *node = (vtkMRMLRTBeamNode *) anode;

  //TODO: Beam parameters
  this->SetSmearing( node->GetSmearing() );
  this->SetSAD(node->GetSAD());
  double iso[3];
  node->GetIsocenterPosition (iso);
  this->Isocenter[0] = iso[0];
  this->Isocenter[1] = iso[1];
  this->Isocenter[2] = iso[2];

  this->SetTargetSegmentID(node->TargetSegmentID);

  this->SetIsocenterSpecification(node->GetIsocenterSpecification());

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  //TODO: Beam parameters
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, eventID, callData);

  if (!this->Scene)
  {
    vtkErrorMacro("ProcessMRMLEvents: Invalid MRML scene!");
    return;
  }
  if (this->Scene->IsBatchProcessing())
  {
    return;
  }

  if (eventID == vtkMRMLMarkupsNode::PointModifiedEvent)
  {
    // Update the model
    this->Modified();
    this->InvokeCustomModifiedEvent(vtkMRMLRTBeamNode::IsocenterModifiedEvent);
  }
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkMRMLRTBeamNode::GetIsocenterFiducialNode()
{
  return vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference(ISOCENTER_FIDUCIAL_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetAndObserveIsocenterFiducialNode(vtkMRMLMarkupsFiducialNode* node)
{
  this->SetNodeReferenceID(ISOCENTER_FIDUCIAL_REFERENCE_ROLE, (node ? node->GetID() : NULL));

  if (node)
  {
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkMRMLMarkupsNode::PointModifiedEvent);
    vtkObserveMRMLObjectEventsMacro(node, events);
  }
}

//----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkMRMLRTBeamNode::GetTargetSegmentationNode()
{
  return vtkMRMLSegmentationNode::SafeDownCast( this->GetNodeReference(TARGET_SEGMENTATION_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetAndObserveTargetSegmentationNode(vtkMRMLSegmentationNode* node)
{
  this->SetNodeReferenceID(TARGET_SEGMENTATION_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkOrientedImageData> vtkMRMLRTBeamNode::GetTargetLabelmap()
{
  vtkSmartPointer<vtkOrientedImageData> targetLabelmap;
  vtkMRMLSegmentationNode* targetSegmentationNode = this->GetTargetSegmentationNode();
  if (!targetSegmentationNode)
  {
    vtkErrorMacro("GetTargetLabelmap: Failed to get target segmentation node");
    return targetLabelmap;
  }

  vtkSegmentation *segmentation = targetSegmentationNode->GetSegmentation();
  if (!segmentation)
  {
    vtkErrorMacro("GetTargetLabelmap: Failed to get segmentation");
    return targetLabelmap;
  }

  vtkSegment *segment = segmentation->GetSegment(this->GetTargetSegmentID());
  if (!segment) 
  {
    vtkErrorMacro("GetTargetLabelmap: Failed to get segment");
    return targetLabelmap;
  }

  //segmentationNode->GetImageData ();
  if (segmentation->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
  {
    targetLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
    targetLabelmap->DeepCopy( vtkOrientedImageData::SafeDownCast(
        segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) ) );
  }
  else
  {
    // Need to convert
    targetLabelmap = vtkSmartPointer<vtkOrientedImageData>::Take(
      vtkOrientedImageData::SafeDownCast(
        vtkSlicerSegmentationsModuleLogic::CreateRepresentationForOneSegment(
          segmentation,
          this->GetTargetSegmentID(),
          vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())));
    if (!targetLabelmap.GetPointer())
    {
      std::string errorMessage("Failed to convert target segment into binary labelmap");
      vtkErrorMacro("GetTargetLabelmap: " << errorMessage);
      return targetLabelmap;
    }
  }

  // Apply parent transformation nodes if necessary
  if (!vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToOrientedImageData(targetSegmentationNode, targetLabelmap))
  {
    std::string errorMessage("Failed to apply parent transformation to target segment!");
    vtkErrorMacro("GetTargetLabelmap: " << errorMessage);
    return targetLabelmap;
  }
  return targetLabelmap;
}

//----------------------------------------------------------------------------
bool vtkMRMLRTBeamNode::ComputeTargetVolumeCenter(double* center)
{
  if (!this->Scene)
  {
    vtkErrorMacro("ComputeTargetVolumeCenter: Invalid MRML scene");
    return false;
  }

  // Get a labelmap for the target
  vtkSmartPointer<vtkOrientedImageData> targetLabelmap = this->GetTargetLabelmap();
  if (targetLabelmap == NULL)
  {
    return false;
  }

  // Convert inputs to plm image
  Plm_image::Pointer plmTgt 
    = PlmCommon::ConvertVtkOrientedImageDataToPlmImage(targetLabelmap);
  if (!plmTgt)
  {
    std::string errorMessage("Failed to convert reference segment labelmap into Plm_image");
    vtkErrorMacro("ComputeTargetVolumeCenter: " << errorMessage);
    return false;
  }

  // Compute image center
  Image_center ic;
  ic.set_image(plmTgt);
  ic.run();
  itk::Vector<double,3> com = ic.get_image_center_of_mass();

  // Copy to output argument, and convert LPS -> RAS
  center[0] = -com[0];
  center[1] = -com[1];
  center[2] = com[2];
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLDoubleArrayNode* vtkMRMLRTBeamNode::GetMLCPositionDoubleArrayNode()
{
  return vtkMRMLDoubleArrayNode::SafeDownCast( this->GetNodeReference(MLCPOSITION_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetAndObserveMLCPositionDoubleArrayNode(vtkMRMLDoubleArrayNode* node)
{
  this->SetNodeReferenceID(MLCPOSITION_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLRTBeamNode::GetDRRVolumeNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast( this->GetNodeReference(DRR_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetAndObserveDRRVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  this->SetNodeReferenceID(DRR_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLRTBeamNode::GetContourBEVVolumeNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast( this->GetNodeReference(CONTOUR_BEV_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetAndObserveContourBEVVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  this->SetNodeReferenceID(CONTOUR_BEV_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
vtkMRMLRTPlanNode* vtkMRMLRTBeamNode::GetPlanNode()
{
  vtkMRMLSubjectHierarchyNode* beamShNode = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(this);
  if (!beamShNode)
  {
    vtkErrorMacro("GetRTPlanNode: Subject hierarchy must be enabled for this operation!");
    return NULL;
  }

  if (!beamShNode->GetParentNode())
  {
    vtkErrorMacro("GetRTPlanNode: Beam node " << this->Name << " does not have a subject hierarchy parent, so cannot access RT plan!");
    return NULL;
  }

  vtkMRMLSubjectHierarchyNode* planShNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(beamShNode->GetParentNode());
  return vtkMRMLRTPlanNode::SafeDownCast(planShNode->GetAssociatedNode());
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetIsocenterSpecification(vtkMRMLRTBeamNode::IsocenterSpecificationType isoSpec)
{
  if (isoSpec == this->GetIsocenterSpecification())
  {
    return;
  }
  if (isoSpec == CenterOfTarget)
  {
    this->SetIsocenterToTargetCenter();
  }

  this->IsocenterSpecification = isoSpec;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetIsocenterToTargetCenter()
{
  double center[3] = {0.0,0.0,0.0};
  if (this->ComputeTargetVolumeCenter(center))
  {
    this->SetIsocenterPosition(center);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::GetIsocenterPosition(double* iso)
{
  vtkMRMLMarkupsFiducialNode* fiducialNode = this->GetIsocenterFiducialNode();
  fiducialNode->GetNthFiducialPosition(0,iso);
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetIsocenterPosition(double* iso)
{
  vtkMRMLMarkupsFiducialNode* fiducialNode = this->GetIsocenterFiducialNode();
  fiducialNode->SetNthFiducialPositionFromArray(0,iso);
}

//----------------------------------------------------------------------------
const double* vtkMRMLRTBeamNode::GetReferenceDosePointPosition()
{
  return this->ReferenceDosePoint;
}

//----------------------------------------------------------------------------
double vtkMRMLRTBeamNode::GetReferenceDosePointPosition(int dim)
{
  return this->ReferenceDosePoint[dim];
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetReferenceDosePointPosition(const float* position)
{
  for (int d = 0; d < 3; d++) 
  {
    this->ReferenceDosePoint[d] = position[d];
  }
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::SetReferenceDosePointPosition(const double* position)
{
  for (int d = 0; d < 3; d++) 
  {
    this->ReferenceDosePoint[d] = position[d];
  }
}

//----------------------------------------------------------------------------
void vtkMRMLRTBeamNode::CreateDefaultDisplayNodes()
{
  //TODO: Create display node here when splitting CreateDefaultBeamModel
  Superclass::CreateDefaultDisplayNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLRTBeamNode::CreateDefaultBeamModel()
{
  if (!this->GetScene())
  {
    vtkErrorMacro ("CreateDefaultBeamModel: Invalid MRML scene");
    return;
  }

  //TODO: Split this function into CreateDefaultDisplayNodes, CreateBeamTransformNodes, etc.
  // and call them when MRML scene is set (make sure transforms are not created twice)
  // Create beam model
  vtkSmartPointer<vtkPolyData> beamModelPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->CreateBeamPolyData(beamModelPolyData);

  // Transform for visualization
  //TODO: Awful names for transforms. They should be barToFooTransform
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->Identity();
  transform->RotateZ(0);
  transform->RotateY(0);
  transform->RotateX(-90);

  vtkSmartPointer<vtkTransform> transform2 = vtkSmartPointer<vtkTransform>::New();
  transform2->Identity();
  transform2->Translate(0.0, 0.0, 0.0);

  transform->PostMultiply();
  transform->Concatenate(transform2->GetMatrix());

  // Create transform node for beam
  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
  transformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->AddNode(transformNode));
  transformNode->SetMatrixTransformToParent(transform->GetMatrix());
  transformNode->SetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyExcludeFromTreeAttributeName().c_str(), "1");

  // Create display node for beam
  vtkSmartPointer<vtkMRMLModelDisplayNode> beamModelDisplayNode = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
  beamModelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(this->GetScene()->AddNode(beamModelDisplayNode));
  beamModelDisplayNode->SetColor(0.0, 1.0, 0.2);
  beamModelDisplayNode->SetOpacity(0.3);
  beamModelDisplayNode->SetBackfaceCulling(0); // Disable backface culling to make the back side of the contour visible as well
  beamModelDisplayNode->VisibilityOn(); 
  beamModelDisplayNode->SliceIntersectionVisibilityOn();

  // Setup beam model node
  this->SetAndObservePolyData(beamModelPolyData);
  this->SetAndObserveTransformNodeID(transformNode->GetID());
  this->SetAndObserveDisplayNodeID(beamModelDisplayNode->GetID());
}

//---------------------------------------------------------------------------
void vtkMRMLRTBeamNode::CreateBeamPolyData(vtkPolyData* beamModelPolyData)
{
  if (!beamModelPolyData)
  {
    vtkErrorMacro("CreateBeamPolyData: Invalid beam node");
    return;
  }

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();

  vtkMRMLDoubleArrayNode* mlcArrayNode = this->GetMLCPositionDoubleArrayNode();
  if (mlcArrayNode)
  {
    // First we extract the shape of the MLC
    int x1count = this->X1Jaw/10.0;
    int x2count = this->X2Jaw/10.0;
    int numLeavesVisible = x2count - x1count; // Calculate the number of leaves visible
    int numPointsEachSide = numLeavesVisible *2;

    double y2LeafPosition[40];
    double y1LeafPosition[40];

    // Calculate Y2 first
    for (int i = x2count; i >= x1count; i--)
    {
      double leafPosition = mlcArrayNode->GetArray()->GetComponent(-(i-20), 1);
      if (leafPosition < this->Y2Jaw)
      {
        y2LeafPosition[-(i-20)] = leafPosition;
      }
      else
      {
        y2LeafPosition[-(i-20)] = this->Y2Jaw;
      }
    }
    // Calculate Y1 next
    for (int i = x2count; i >= x1count; i--)
    {
      double leafPosition = mlcArrayNode->GetArray()->GetComponent(-(i-20), 0);
      if (leafPosition < this->Y1Jaw)
      {
        y1LeafPosition[-(i-20)] = leafPosition;
      }
      else
      {
        y1LeafPosition[-(i-20)] = this->Y1Jaw;
      }
    }

    // Create beam model
    points->InsertPoint(0,0,0,this->SAD);

    int count = 1;
    for (int i = x2count; i > x1count; i--)
    {
      points->InsertPoint(count,-y2LeafPosition[-(i-20)]*2, i*10*2, -this->SAD );
      count ++;
      points->InsertPoint(count,-y2LeafPosition[-(i-20)]*2, (i-1)*10*2, -this->SAD );
      count ++;
    }

    for (int i = x1count; i < x2count; i++)
    {
      points->InsertPoint(count,y1LeafPosition[-(i-20)]*2, i*10*2, -this->SAD );
      count ++;
      points->InsertPoint(count,y1LeafPosition[-(i-20)]*2, (i+1)*10*2, -this->SAD );
      count ++;
    }

    for (int i = 1; i <numPointsEachSide; i++)
    {
      cellArray->InsertNextCell(3);
      cellArray->InsertCellPoint(0);
      cellArray->InsertCellPoint(i);
      cellArray->InsertCellPoint(i+1);
    }
    // Add connection between Y2 and Y1
    cellArray->InsertNextCell(3);
    cellArray->InsertCellPoint(0);
    cellArray->InsertCellPoint(numPointsEachSide);
    cellArray->InsertCellPoint(numPointsEachSide+1);
    for (int i = numPointsEachSide+1; i <2*numPointsEachSide; i++)
    {
      cellArray->InsertNextCell(3);
      cellArray->InsertCellPoint(0);
      cellArray->InsertCellPoint(i);
      cellArray->InsertCellPoint(i+1);
    }

    // Add connection between Y2 and Y1
    cellArray->InsertNextCell(3);
    cellArray->InsertCellPoint(0);
    cellArray->InsertCellPoint(2*numPointsEachSide);
    cellArray->InsertCellPoint(1);

    // Add the cap to the bottom
    cellArray->InsertNextCell(2*numPointsEachSide);
    for (int i = 1; i <= 2*numPointsEachSide; i++)
    {
      cellArray->InsertCellPoint(i);
    }
  }
  else
  {
    points->InsertPoint(0,0,0,this->SAD);
    points->InsertPoint(1, -2*this->Y2Jaw, -2*this->X2Jaw, -this->SAD );
    points->InsertPoint(2, -2*this->Y2Jaw, -2*this->X1Jaw, -this->SAD );
    points->InsertPoint(3, -2*this->Y1Jaw, -2*this->X1Jaw, -this->SAD );
    points->InsertPoint(4, -2*this->Y1Jaw, -2*this->X2Jaw, -this->SAD );

    cellArray->InsertNextCell(3);
    cellArray->InsertCellPoint(0);
    cellArray->InsertCellPoint(1);
    cellArray->InsertCellPoint(2);

    cellArray->InsertNextCell(3);
    cellArray->InsertCellPoint(0);
    cellArray->InsertCellPoint(2);
    cellArray->InsertCellPoint(3);

    cellArray->InsertNextCell(3);
    cellArray->InsertCellPoint(0);
    cellArray->InsertCellPoint(3);
    cellArray->InsertCellPoint(4);

    cellArray->InsertNextCell(3);
    cellArray->InsertCellPoint(0);
    cellArray->InsertCellPoint(4);
    cellArray->InsertCellPoint(1);

    // Add the cap to the bottom
    cellArray->InsertNextCell(4);
    cellArray->InsertCellPoint(1);
    cellArray->InsertCellPoint(2);
    cellArray->InsertCellPoint(3);
    cellArray->InsertCellPoint(4);
  }

  beamModelPolyData->SetPoints(points);
  beamModelPolyData->SetPolys(cellArray);
}
