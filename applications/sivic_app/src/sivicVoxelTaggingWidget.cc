/*
 *  Copyright © 2009-2011 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice, 
 *      this list of conditions and the following disclaimer in the documentation 
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name 
 *      "The Regents of the University of California," or the names of any of its 
 *      contributors may be used to endorse or promote products derived from this 
 *      software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 *  OF SUCH DAMAGE.
 */
/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */


#include <sivicVoxelTaggingWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicVoxelTaggingWidget );
vtkCxxRevisionMacro( sivicVoxelTaggingWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicVoxelTaggingWidget::sivicVoxelTaggingWidget()
{
    this->tagNames.push_back("ROI");
    this->tagValues.push_back(1);
	this->tagsTable = NULL;
	this->createTagVolumeButton = NULL;

}


/*! 
 *  Destructor
 */
sivicVoxelTaggingWidget::~sivicVoxelTaggingWidget()
{

	if( this->tagsTable != NULL ) {
		this->tagsTable->Delete();
		this->tagsTable = NULL;
	}

	if( this->createTagVolumeButton != NULL ) {
		this->createTagVolumeButton->Delete();
		this->createTagVolumeButton = NULL;
	}

	if( this->addTagButton != NULL ) {
		this->addTagButton->Delete();
		this->addTagButton = NULL;
	}

	if( this->removeTagButton != NULL ) {
		this->removeTagButton->Delete();
		this->removeTagButton = NULL;
	}
}


/*!
 * Initializes a tagging volume for the currently active 4DImageData.
 */
void sivicVoxelTaggingWidget::CreateTagVolume()
{
	svk4DImageData* voxelData = this->sivicController->GetActive4DImageData();
	if( voxelData != NULL ) {
		svkMriImageData* voxelTagData = svkVoxelTaggingUtils::CreateVoxelTagData( voxelData, this->tagNames, this->tagValues );
		this->model->AddDataObject("VoxelTagData", voxelTagData );
		this->sivicController->OpenOverlayFromModel("VoxelTagData");
		voxelTagData->Delete();
		this->removeTagButton->EnabledOn();
		this->addTagButton->EnabledOn();
		this->tagsTable->EnabledOn();
	}
}


/*!
 *  Adds a tag to the tagging volume.
 */
void sivicVoxelTaggingWidget::AddTag()
{
	svkImageData* voxelTagData = this->model->GetDataObject("VoxelTagData");
	if( voxelTagData!= NULL ) {
		string newTagName = "New Tag";
		int newTagValue = 0;
		for( int i = 0; i < this->tagValues.size(); i++ ) {
			if( this->tagValues[i] > newTagValue ) {
				newTagValue = this->tagValues[i];
			}
		}
		newTagValue++;
		svkVoxelTaggingUtils::AddTagToVoxelData( voxelTagData, newTagName, newTagValue);
		this->tagNames.push_back(newTagName);
		this->tagValues.push_back(newTagValue);
		this->UpdateTagsList();
	}
}


/*!
 *  Removes a tag from the tagging volume.
 */
void sivicVoxelTaggingWidget::RemoveTag(int tagVolumeNumber)
{
	svkImageData* voxelTagData = this->model->GetDataObject("VoxelTagData");
	if( voxelTagData!= NULL ) {
		int selectedRow = -1;
		this->tagsTable->GetWidget()->GetSelectedRows(&selectedRow);
		int volumeNumber = this->tagsTable->GetWidget()->GetCellTextAsInt(selectedRow,0);
		if( volumeNumber < this->tagNames.size() && volumeNumber >= 0 ) {
			svkVoxelTaggingUtils::RemoveTagFromVoxelData( voxelTagData, volumeNumber);
			this->tagNames.erase(this->tagNames.begin() + volumeNumber);
			this->tagValues.erase(this->tagValues.begin() + volumeNumber);
			this->UpdateTagsList();
			this->overlayController->GetView()->GetRenderer(svkOverlayView::PRIMARY)->Modified();
			this->overlayController->GetView()->Refresh();
			this->plotController->GetView()->Refresh();
		}
	}
}


/*!
 *  Sets the name for a given tag.
 */
void sivicVoxelTaggingWidget::SetTagName( string tagName, int tagVolume )
{
	svkImageData* voxelTagData = this->model->GetDataObject("VoxelTagData");
	if( voxelTagData != NULL ) {
		svkVoxelTaggingUtils::SetTagName(voxelTagData, tagName, tagVolume);
	}

}


/*!
 *  Sets the value for a given tag.
 */
void sivicVoxelTaggingWidget::SetTagValue(int tagValue, int tagVolume)
{
	svkImageData* voxelTagData = this->model->GetDataObject("VoxelTagData");
	if( voxelTagData != NULL ) {
		svkVoxelTaggingUtils::SetTagValue(voxelTagData, tagValue, tagVolume);
	}

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicVoxelTaggingWidget::CreateWidget()
{

    // Check if already created
    if ( this->IsCreated() )
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the composite widget container
    this->Superclass::CreateWidget();

    this->tagsTable = vtkKWMultiColumnListWithScrollbars::New();
    this->tagsTable->SetParent(this);
    this->tagsTable->Create();
    int col_index;
    col_index = this->tagsTable->GetWidget()->AddColumn("Tag Index");
    col_index = this->tagsTable->GetWidget()->AddColumn("Tag Name");
    this->tagsTable->GetWidget()->ColumnEditableOn(col_index);
    col_index = this->tagsTable->GetWidget()->AddColumn("Tag Value");
    this->tagsTable->GetWidget()->ColumnEditableOn(col_index);
    this->tagsTable->EnabledOff();

    this->createTagVolumeButton = vtkKWPushButton::New();
    this->createTagVolumeButton->SetParent( this );
    this->createTagVolumeButton->Create( );
    this->createTagVolumeButton->SetText( "Create Tag Data");
    this->createTagVolumeButton->SetBalloonHelpString("Creates a voxel tag volume and sets it as the current overlay.");
    this->createTagVolumeButton->EnabledOn();

    this->addTagButton = vtkKWPushButton::New();
    this->addTagButton->SetParent( this );
    this->addTagButton->Create( );
    this->addTagButton->SetText( "Add Tag");
    this->addTagButton->SetBalloonHelpString("Adds a new tag to the volume.");
    this->addTagButton->EnabledOff();

    this->removeTagButton = vtkKWPushButton::New();
    this->removeTagButton->SetParent( this );
    this->removeTagButton->Create( );
    this->removeTagButton->SetText( "Remove Tag");
    this->removeTagButton->SetBalloonHelpString("Removes the tag entirely from the volume.");
    this->removeTagButton->EnabledOff();

    this->Script("grid %s -row 0 -column 0 -columnspan 3 -sticky wnse", this->tagsTable->GetWidgetName());
    this->Script("grid %s -row 1 -column 0 -sticky wnse", this->createTagVolumeButton->GetWidgetName());
    this->Script("grid %s -row 1 -column 1 -sticky wnse", this->addTagButton->GetWidgetName());
    this->Script("grid %s -row 1 -column 2 -sticky wnse", this->removeTagButton->GetWidgetName());

    //  Callbacks
    this->AddCallbackCommandObserver( this->tagsTable->GetWidget(), vtkKWMultiColumnList::CellUpdatedEvent );
    this->AddCallbackCommandObserver( this->tagsTable->GetWidget(), vtkKWMultiColumnList::SelectionChangedEvent );
    this->AddCallbackCommandObserver( this->createTagVolumeButton, vtkKWPushButton::InvokedEvent );
    this->AddCallbackCommandObserver( this->removeTagButton, vtkKWPushButton::InvokedEvent );
    this->AddCallbackCommandObserver( this->addTagButton, vtkKWPushButton::InvokedEvent );

}


/*!
 *
 *  Updates the list of tags.
 */
void sivicVoxelTaggingWidget::UpdateTagsList( )
{
    // We start at one since the first plot index is the active spectra above...
	this->tagsTable->GetWidget()->DeleteAllRows();

    for( int i = 0; i < this->tagNames.size(); i ++ ) {
		this->tagsTable->GetWidget()->InsertCellText(i, 0, svkUtils::IntToString(i).c_str());
		this->tagsTable->GetWidget()->InsertCellText(i, 1, this->tagNames[i].c_str());
		this->tagsTable->GetWidget()->InsertCellText(i, 2, svkUtils::IntToString(this->tagValues[i]).c_str());
    }

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicVoxelTaggingWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    if( caller == this->createTagVolumeButton && event == vtkKWPushButton::InvokedEvent ) {
    	this->CreateTagVolume();
    } else if( caller == this->addTagButton && event == vtkKWPushButton::InvokedEvent ) {
    	this->AddTag();
    } else if( caller == this->removeTagButton && event == vtkKWPushButton::InvokedEvent ) {
    	this->RemoveTag(0);
    } else if( caller == this->tagsTable->GetWidget() ) {
    	svkImageData* voxelTagData = this->model->GetDataObject("VoxelTagData");
		int selectedRow = -1;
		this->tagsTable->GetWidget()->GetSelectedRows(&selectedRow);
		int selectedVolume = this->tagsTable->GetWidget()->GetCellTextAsInt(selectedRow, 0);
		string selectedTagName = this->tagsTable->GetWidget()->GetCellText(selectedRow, 1);
		int selectedTagValue = this->tagsTable->GetWidget()->GetCellTextAsInt(selectedRow, 2);
		if( selectedRow >= 0 && selectedRow < this->tagsTable->GetWidget()->GetNumberOfRows() && voxelTagData != NULL ) {
			int volumeNumber = this->tagsTable->GetWidget()->GetCellTextAsInt(selectedRow,0);
			svkOverlayView::SafeDownCast(this->overlayController->GetView())->SetActiveOverlayVolume( volumeNumber );
			//voxelTagData->GetPointData()->SetActiveScalars( voxelTagData->GetPointData()->GetArray( volumeNumber)->GetName() );
			voxelTagData->Modified();
			this->overlayController->GetView()->Refresh();
			if( selectedTagName != this->tagNames[ selectedVolume ] ) {
				this->tagNames[ selectedVolume ] = selectedTagName;
				this->SetTagName( selectedTagName, selectedVolume);
			}
			if( selectedTagValue != this->tagValues[ selectedVolume ] ) {
				this->tagValues[ selectedVolume ] = selectedTagValue;
				this->SetTagValue( selectedTagValue, selectedVolume);
			}
		}
	}
    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}
