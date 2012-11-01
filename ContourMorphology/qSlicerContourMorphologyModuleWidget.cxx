/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kevin Wang, Radiation Medicine Program, 
  University Health Network and was supported by Cancer Care Ontario (CCO)'s ACRU program 
  with funds provided by the Ontario Ministry of Health and Long-Term Care
  and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).

==============================================================================*/

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerContourMorphologyModuleWidget.h"
#include "ui_qSlicerContourMorphologyModule.h"

// Isodose includes
#include "vtkSlicerContourMorphologyModuleLogic.h"
#include "vtkMRMLContourMorphologyNode.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLContourNode.h>

// VTK includes
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

// STD includes
#include <sstream>
#include <string>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerContourMorphologyModuleWidgetPrivate: public Ui_qSlicerContourMorphologyModule
{
  Q_DECLARE_PUBLIC(qSlicerContourMorphologyModuleWidget);
protected:
  qSlicerContourMorphologyModuleWidget* const q_ptr;
public:
  qSlicerContourMorphologyModuleWidgetPrivate(qSlicerContourMorphologyModuleWidget &object);
  ~qSlicerContourMorphologyModuleWidgetPrivate();
  vtkSlicerContourMorphologyModuleLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerContourMorphologyModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerContourMorphologyModuleWidgetPrivate::qSlicerContourMorphologyModuleWidgetPrivate(qSlicerContourMorphologyModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerContourMorphologyModuleWidgetPrivate::~qSlicerContourMorphologyModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerContourMorphologyModuleLogic*
qSlicerContourMorphologyModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerContourMorphologyModuleWidget);
  return vtkSlicerContourMorphologyModuleLogic::SafeDownCast(q->logic());
} 

//-----------------------------------------------------------------------------
// qSlicerContourMorphologyModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerContourMorphologyModuleWidget::qSlicerContourMorphologyModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerContourMorphologyModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerContourMorphologyModuleWidget::~qSlicerContourMorphologyModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerContourMorphologyModuleWidget);

  this->Superclass::setMRMLScene(scene);

  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()) );

  // Find parameters node or create it if there is no one in the scene
  if (scene &&  d->logic()->GetContourMorphologyNode() == 0)
    {
    vtkMRMLNode* node = scene->GetNthNodeByClass(0, "vtkMRMLContourMorphologyNode");
    if (node)
      {
      this->setContourMorphologyNode( vtkMRMLContourMorphologyNode::SafeDownCast(node) );
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::enter()
{
  this->onEnter();
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::onEnter()
{
  if (!this->mrmlScene())
    {
    return;
    }

  Q_D(qSlicerContourMorphologyModuleWidget);

  // First check the logic if it has a parameter node
  if (d->logic() == NULL)
    {
    return;
    }
  vtkMRMLContourMorphologyNode* paramNode = d->logic()->GetContourMorphologyNode();

  // If we have a parameter node select it
  if (paramNode == NULL)
    {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLContourMorphologyNode");
    if (node)
      {
      paramNode = vtkMRMLContourMorphologyNode::SafeDownCast(node);
      d->logic()->SetAndObserveContourMorphologyNode(paramNode);
      return;
      }
    else 
      {
      vtkSmartPointer<vtkMRMLContourMorphologyNode> newNode = vtkSmartPointer<vtkMRMLContourMorphologyNode>::New();
      this->mrmlScene()->AddNode(newNode);
      d->logic()->SetAndObserveContourMorphologyNode(newNode);
      }
    }

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerContourMorphologyModuleWidget);

  vtkMRMLContourMorphologyNode* paramNode = d->logic()->GetContourMorphologyNode();
  if (paramNode && this->mrmlScene())
  {
    d->MRMLNodeComboBox_ParameterSet->setCurrentNode(paramNode);
    if (paramNode->GetContourNodeID() && strcmp(paramNode->GetContourNodeID(),""))
    {
      d->MRMLNodeComboBox_CurrentContour->setCurrentNode(paramNode->GetContourNodeID());
    }
    else
    {
      this->currentContourNodeChanged(d->MRMLNodeComboBox_CurrentContour->currentNode());
    }
    if (paramNode->GetExpansion())
    {
      d->radioButton_Expand->setChecked(true);
      d->radioButton_Shrink->setChecked(false);
    }
    else
    {
      d->radioButton_Expand->setChecked(false);
      d->radioButton_Shrink->setChecked(true);
    }

    std::ostringstream sstream_xsize;
    sstream_xsize << paramNode->GetXSize();
    d->lineEdit_XSize->setText(QString(sstream_xsize.str().c_str()));

    std::ostringstream sstream_ysize;
    sstream_ysize << paramNode->GetYSize();
    d->lineEdit_YSize->setText(QString(sstream_ysize.str().c_str()));

    std::ostringstream sstream_zsize;
    sstream_zsize << paramNode->GetZSize();
    d->lineEdit_ZSize->setText(QString(sstream_zsize.str().c_str()));
  }
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::onLogicModified()
{
  Q_D(qSlicerContourMorphologyModuleWidget);

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::setup()
{
  Q_D(qSlicerContourMorphologyModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Make connections
  this->connect( d->MRMLNodeComboBox_ParameterSet, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT( setContourMorphologyNode(vtkMRMLNode*) ) );
  this->connect( d->MRMLNodeComboBox_CurrentContour, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT( currentContourNodeChanged(vtkMRMLNode*) ) );
  this->connect( d->MRMLNodeComboBox_OutputContour, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT( outputContourNodeChanged(vtkMRMLNode*) ) );

  this->connect( d->radioButton_Expand, SIGNAL(clicked()), this, SLOT(radioButtonExpandClicked()));
  this->connect( d->radioButton_Shrink, SIGNAL(clicked()), this, SLOT(radioButtonShrinkClicked()));

  this->connect( d->lineEdit_XSize, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditXSizeChanged(const QString &)));
  this->connect( d->lineEdit_YSize, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditYSizeChanged(const QString &)));
  this->connect( d->lineEdit_ZSize, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditZSizeChanged(const QString &)));

  this->connect( d->pushButton_Apply, SIGNAL(clicked()), this, SLOT(applyClicked()) );

  this->updateButtonsState();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::setContourMorphologyNode(vtkMRMLNode *node)
{
  Q_D(qSlicerContourMorphologyModuleWidget);

  vtkMRMLContourMorphologyNode* paramNode = vtkMRMLContourMorphologyNode::SafeDownCast(node);

  // Each time the node is modified, the qt widgets are updated
  qvtkReconnect( d->logic()->GetContourMorphologyNode(), paramNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()) );

  d->logic()->SetAndObserveContourMorphologyNode(paramNode);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::currentContourNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerContourMorphologyModuleWidget);

  vtkMRMLContourMorphologyNode* paramNode = d->logic()->GetContourMorphologyNode();
  if (!paramNode || !this->mrmlScene() || !node)
  {
    return;
  }

  paramNode->DisableModifiedEventOn();
  paramNode->SetAndObserveContourNodeID(node->GetID());
  paramNode->DisableModifiedEventOff();

  this->updateButtonsState();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::outputContourNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerContourMorphologyModuleWidget);

  vtkMRMLContourMorphologyNode* paramNode = d->logic()->GetContourMorphologyNode();
  if (!paramNode || !this->mrmlScene() || !node)
  {
    return;
  }

  paramNode->DisableModifiedEventOn();
  paramNode->SetAndObserveOutputContourNodeID(node->GetID());
  paramNode->DisableModifiedEventOff();

  this->updateButtonsState();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::radioButtonExpandClicked()
{
  Q_D(qSlicerContourMorphologyModuleWidget);

  vtkMRMLContourMorphologyNode* paramNode = d->logic()->GetContourMorphologyNode();
  if (!paramNode || !this->mrmlScene())
  {
    return;
  }
  paramNode->DisableModifiedEventOn();
  paramNode->SetExpansion(true);
  paramNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::radioButtonShrinkClicked()
{
  Q_D(qSlicerContourMorphologyModuleWidget);

  vtkMRMLContourMorphologyNode* paramNode = d->logic()->GetContourMorphologyNode();
  if (!paramNode || !this->mrmlScene())
  {
    return;
  }
  paramNode->DisableModifiedEventOn();
  paramNode->SetExpansion(false);
  paramNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::lineEditXSizeChanged(const QString & text)
{
  Q_D(qSlicerContourMorphologyModuleWidget);
  
  vtkMRMLContourMorphologyNode* paramNode = d->logic()->GetContourMorphologyNode();
  if (!paramNode || !this->mrmlScene())
  {
    return;
  }
  paramNode->DisableModifiedEventOn();
  paramNode->SetXSize(text.toDouble());
  paramNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::lineEditYSizeChanged(const QString & text)
{
  Q_D(qSlicerContourMorphologyModuleWidget);
  
  vtkMRMLContourMorphologyNode* paramNode = d->logic()->GetContourMorphologyNode();
  if (!paramNode || !this->mrmlScene())
  {
    return;
  }
  paramNode->DisableModifiedEventOn();
  paramNode->SetYSize(text.toDouble());
  paramNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::lineEditZSizeChanged(const QString & text)
{
  Q_D(qSlicerContourMorphologyModuleWidget);
  
  vtkMRMLContourMorphologyNode* paramNode = d->logic()->GetContourMorphologyNode();
  if (!paramNode || !this->mrmlScene())
  {
    return;
  }
  paramNode->DisableModifiedEventOn();
  paramNode->SetZSize(text.toDouble());
  paramNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::updateButtonsState()
{
  Q_D(qSlicerContourMorphologyModuleWidget);
  if (!this->mrmlScene())
  {
    return;
  }

  bool applyEnabled = d->logic()->GetContourMorphologyNode()
                   && d->logic()->GetContourMorphologyNode()->GetContourNodeID()
                   && strcmp(d->logic()->GetContourMorphologyNode()->GetContourNodeID(), "");
  d->pushButton_Apply->setEnabled(applyEnabled);
}

//-----------------------------------------------------------------------------
void qSlicerContourMorphologyModuleWidget::applyClicked()
{
  Q_D(qSlicerContourMorphologyModuleWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  // 
  d->logic()->MorphContour();

  QApplication::restoreOverrideCursor();
}
