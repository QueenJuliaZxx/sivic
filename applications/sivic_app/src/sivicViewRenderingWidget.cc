/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */



#include <sivicViewRenderingWidget.h>
#include <vtkSivicController.h>
#include <vtkJPEGReader.h>

#define MINIMUM_RANGE_FACTOR 100

vtkStandardNewMacro( sivicViewRenderingWidget );
vtkCxxRevisionMacro( sivicViewRenderingWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicViewRenderingWidget::sivicViewRenderingWidget()
{
    this->titleWidget = NULL;
    this->infoWidget = NULL;
    this->viewerWidget = NULL;
    this->specViewerWidget = NULL;

    this->it = vtkTextActor::New();
    this->it->SetTextScaleModeToProp();
    this->it->SetPosition(0.05,0.05);
    this->it->SetPosition2(1,1);

    this->plotController = NULL;
    this->overlayController = NULL;

}


/*! 
 *  Destructor
 */
sivicViewRenderingWidget::~sivicViewRenderingWidget()
{

    if( this->titleWidget != NULL ) {
        this->titleWidget->Delete();
        this->titleWidget = NULL;
    }

    if( this->infoWidget != NULL ) {
        this->infoWidget->Delete();
        this->infoWidget = NULL;
    }

    if( this->it != NULL ) {
        this->it->Delete();
        this->it = NULL;
    }

    if( this->viewerWidget != NULL ) {
        this->viewerWidget->Delete();
        this->viewerWidget = NULL;
    }

    if( this->specViewerWidget != NULL ) {
        this->specViewerWidget->Delete();
        this->specViewerWidget = NULL;
    }

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicViewRenderingWidget::CreateWidget()
{
/*  This method will create our main window. The main window is a 
    vtkKWCompositeWidget with a vtkKWRendWidget. */

    // Check if already created
    if ( this->IsCreated() )
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the composite widget container
    this->Superclass::CreateWidget();

    this->titleWidget = vtkKWRenderWidget::New();
    this->titleWidget->SetParent(this);
    this->titleWidget->Create();

    this->infoWidget = vtkKWRenderWidget::New();
    this->infoWidget->SetParent(this);
    this->infoWidget->Create();
   
    // Create the widget that will hold the overlay view 
    this->viewerWidget = vtkKWRenderWidget::New();
    this->viewerWidget->SetParent(this);
    this->viewerWidget->Create();

    //Create the widget that will hold the spectroscopy data grid
    this->specViewerWidget = vtkKWRenderWidget::New();
    this->specViewerWidget->SetParent(this);
    this->specViewerWidget->Create();

    // ========================================================================================
    //  View/Controller Creation
    // ========================================================================================

    vtkTextActor* tt = vtkTextActor::New();
    tt->SetInput( "SIVIC: RESEARCH SOFTWARE" ) ;
    tt->GetTextProperty()->SetFontSize( 18 );
    tt->GetTextProperty()->BoldOn();
    tt->SetPosition(10,3);

    vtkTextActor* it = vtkTextActor::New();
    it->SetInput( "INFO:" ) ;
    it->GetTextProperty()->SetFontSize( 11 );
    it->SetPosition(10,3);
    
    this->titleWidget->GetRenderer()->AddActor(tt);
    tt->Delete();
    this->infoWidget->GetRenderer()->AddActor(it);
    it->Delete();

    overlayController->SetRWInteractor(this->viewerWidget->GetRenderWindowInteractor());
    plotController->SetRWInteractor(this->specViewerWidget->GetRenderWindowInteractor());
    

    int row = 0; 
    this->Script("grid %s -row %d -column 0 -sticky nsew -columnspan 2", this->viewerWidget->GetWidgetName(), row );
    this->Script("grid %s -row %d -column 2 -sticky nsew", this->specViewerWidget->GetWidgetName(), row );
    this->Script("grid %s -row %d -column 3 -sticky ewns", this->infoWidget->GetWidgetName(), row );
    row++; 

    this->Script("grid rowconfigure %s 0 -weight 100 -minsize 200", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 0 -weight 50 -uniform 1 -minsize 350", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 0",  this->GetWidgetName() );
    this->Script("grid columnconfigure %s 2 -weight 50 -uniform 1 -minsize 300", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 3 -weight 0 -uniform 1 -minsize 185", this->GetWidgetName() );



    // Here we will add callbacks 
    this->AddCallbackCommandObserver(
        this->viewerWidget->GetRenderWindowInteractor(), vtkCommand::SelectionChangedEvent );

    this->AddCallbackCommandObserver(
        this->specViewerWidget->GetRenderWindowInteractor(), vtkCommand::SelectionChangedEvent );

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicViewRenderingWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    if (  caller == this->viewerWidget->GetRenderWindowInteractor() 
        && event == vtkCommand::SelectionChangedEvent ) {
        int * tlcBrc = overlayController->GetTlcBrc();
        plotController->SetTlcBrc( tlcBrc );
        this->specViewerWidget->Render();
        this->viewerWidget->Render();

    // Respond to a selection change in the plot grid view
    } else if (  caller == this->specViewerWidget->GetRenderWindowInteractor() 
            && event == vtkCommand::SelectionChangedEvent ) {
        int * tlcBrc = plotController->GetTlcBrc();
        overlayController->SetTlcBrc( tlcBrc );
        this->specViewerWidget->Render();
        this->viewerWidget->Render();
    } 


    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);

}


/*!
 * Reset the text describing the loaded data.
 */
void sivicViewRenderingWidget::ResetInfoText()
{
    this->infoWidget->GetRenderer()->RemoveAllViewProps();
    string currentImageName = model->GetDataFileName( "AnatomicalData" );
    string currentSpectraName = model->GetDataFileName( "SpectroscopicData" );
    string currentOverlayName = model->GetDataFileName( "OverlayData" );
    string currentMetaboliteName = model->GetDataFileName( "MetaboliteData" );

    stringstream infoSS;
    size_t pos;

    // Title:
    //infoSS << "SIVIC: Research Software \n\n" << endl;
    
    if( model->DataExists( "AnatomicalData" ) ) {
        // Patient Code??
        infoSS << "Patient Code:  " << endl;

        // Study Date
        string studyDate = model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->GetStringValue("StudyDate");

        // Temporarily removing study date just in case for phi while making screenshots
        if( studyDate.size() > 0) {
            infoSS << "Scan Date:  " << studyDate.substr( studyDate.size()-4, 2) << "/" 
                                 << studyDate.substr( studyDate.size()-2, 2) << "/"
                                 << studyDate.substr( 0, 4 ) << endl;
        } else {
            infoSS << "Scan Date:  " << "?" << endl;

        }

        // Image Name
        pos = currentImageName.find_last_of("/"); 
        if( pos == currentImageName.npos ) {
            pos = 0;
        } else if( pos+1 < currentImageName.npos) {
            pos++;
        }
        infoSS << "Image File:  " << currentImageName.substr(pos) << endl;

        // Image Series
        if( model->GetDataObject( "AnatomicalData" ) ) {
            int seriesNumber = model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->GetIntValue("SeriesNumber");
            infoSS << "Image Series:  " << seriesNumber << endl;
        }

        // Image FOV
        int rows = model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->GetIntValue("Rows");
        int cols = model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->GetIntValue("Columns");
                        string pixelSpacing = model->GetDataObject( "AnatomicalData")->GetDcmHeader()->GetStringSequenceItemElement("PixelMeasuresSequence", 0, "PixelSpacing" , "SharedFunctionalGroupsSequence");

        pos = pixelSpacing.find_last_of("\\"); 
        infoSS << "Image FOV:  " << atof(pixelSpacing.substr(0,pos).c_str()) * rows << " X " 
                               << atof(pixelSpacing.substr(pos+1).c_str()) * cols << endl;

        // Image Coil
        string imageCoil = model->GetDataObject( "AnatomicalData")->GetDcmHeader()->GetStringSequenceItemElement("MRReceiveCoilSequence", 0, "ReceiveCoilName" , "SharedFunctionalGroupsSequence");


        infoSS << "Image Coil:  " << imageCoil << endl << endl;

    } else {
        infoSS << "Patient Code:  " << endl;
        infoSS << "Scan Date:  " << endl; 
        infoSS << "Image File:  " << endl;
        infoSS << "Image Series:  " << endl;
        infoSS << "Image FOV:  " << endl;

    }

    // Get Spectroscopy Parameters
    if( model->DataExists( "SpectroscopicData" ) ) {

        // Get The Spectra Name
        pos = currentSpectraName.find_last_of("/"); 
        if( pos == currentSpectraName.npos ) {
            pos = 0;
        } else if( pos+1 < currentSpectraName.npos) {
            pos++;
        }
        infoSS << "CSI File: " << currentSpectraName.substr(pos) << endl;

        // Slice
        infoSS << "CSI Slice No:  " << this->plotController->GetSlice() + 1 << endl;
        float sliceLocation[3];

        // Slice Position:
        int sliceIndex[3] = {0,0,this->plotController->GetSlice() +1};
        model->GetDataObject( "SpectroscopicData" )->GetPositionFromIndex( sliceIndex, sliceLocation );
        infoSS << "CSI Slice Pos:  " << sliceLocation[2] << endl;
        
        // Coil
        string specCoil = model->GetDataObject( "SpectroscopicData")->GetDcmHeader()->GetStringSequenceItemElement("MRReceiveCoilSequence", 0, "ReceiveCoilName" , "SharedFunctionalGroupsSequence");
        infoSS << "Coil:  " << specCoil << endl;
        string echoTime = model->GetDataObject( "SpectroscopicData")->GetDcmHeader()->GetStringSequenceItemElement("MREchoSequence", 0, "EffectiveEchoTime" , "SharedFunctionalGroupsSequence");
        infoSS << "TE:  " << echoTime << " ms" << endl;

        // Selected Region 

        // Size
        infoSS.setf(ios::fixed,ios::floatfield);            // floatfield not set
        infoSS.precision(1);
        float dims[3]; 
        
        svkMrsImageData::SafeDownCast(model->GetDataObject( "SpectroscopicData" ))->GetSelectionBoxDimensions( dims );
        infoSS << "Selected Region:  " << dims[0]*dims[1]*dims[2] / 1000.0 << "cc" << endl;
        infoSS << "Size RAS: " << dims[0] << " " << dims[1] << " " << dims[2] << "mm"<< endl;

        // Center
        float center[3]; 
        model->GetDataObject( "SpectroscopicData" )->GetImageCenter( center );
        infoSS << "Center RAS: " << center[0] << " " << center[1] << " " << center[2] << "mm"<< endl << endl;
        double* spacing = model->GetDataObject( "SpectroscopicData" )->GetSpacing();
        infoSS << "CSI Resolution:" << spacing[0] * spacing[1] * spacing[2] / 1000.0 << "cc" <<  endl;
        infoSS << "Size RAS: " << spacing[0] << " " << spacing[1] << " " << spacing[2] << "mm" <<  endl;
        infoSS << "Center RAS: " << center[0] << " " << center[1] << " " << center[2] << " mm"<< endl << endl;
    } else {
        infoSS << "CSI File: " << endl;
        infoSS << "CSI Slice No:" << endl;
        infoSS << "CSI Slice Pos:" << endl;
        infoSS << "Coil:  " << endl;
        infoSS << "TE:" << endl;
        infoSS << "Selected Region:" << endl;
        infoSS << "Size:" << endl;
        infoSS << "Center:" << endl;
        infoSS << "CSI Resolution:" << endl;
        infoSS << "Size:" << endl;
        infoSS << "Center:" << endl << endl;
    } 



    pos = currentOverlayName.find_last_of("/"); 
    if( pos == currentOverlayName.npos ) {
        pos = 0;
    } else if( pos+1 < currentOverlayName.npos) {
        pos++;
    }
    infoSS << "Overlay: " << currentOverlayName.substr(pos) << endl;

    pos = currentMetaboliteName.find_last_of("/"); 
    if( pos == currentMetaboliteName.npos ) {
        pos = 0;
    } else if( pos+1 < currentMetaboliteName.npos) {
        pos++;
    }
    infoSS << "Metabolites: " << GetMetaboliteName( currentMetaboliteName ) << endl;
    infoSS << "Metabolites File: " <<  currentMetaboliteName.substr(pos) << endl; 


    it->SetInput( infoSS.str().c_str() ) ;
    it->GetTextProperty()->SetFontSize( 12 );
    it->GetTextProperty()->SetFontFamilyToTimes();
    it->GetTextProperty()->BoldOn();
    it->SetPosition(3,3);
    if( !this->infoWidget->GetRenderer()->HasViewProp( it ) ){
        this->infoWidget->GetRenderer()->AddActor( it );
    }
    this->infoWidget->GetRenderer()->Render();
    this->infoWidget->GetRenderer()->Modified();
    this->infoWidget->Render();
    
}


const char* sivicViewRenderingWidget::GetMetaboliteName( string fileName )
{
    if( strcmp( fileName.c_str(), "" ) == 0 ) {
        return "";
    }
    size_t pos;

    pos = fileName.rfind("."); 
    if( pos != fileName.npos ) {
        fileName = fileName.substr(0,pos);
    }
    pos = fileName.rfind("cor"); 
    if( pos == fileName.npos ) {
        pos = fileName.rfind("_"); 
        if( pos + 2 < fileName.npos ) {
            pos += 2;
        } else {
            return "";
        }
    } 

    if(        strcmp( fileName.substr(pos).c_str(), "r01") == 0 ) { 
        return "CHO (area)";
    } else if( strcmp( fileName.substr(pos).c_str(), "rh01") == 0 ) { 
        return "CHO (ht)";
    } else if( strcmp( fileName.substr(pos).c_str(), "r02") == 0 ) { 
        return "CRE (area)";
    } else if( strcmp( fileName.substr(pos).c_str(), "rh02") == 0 ) { 
        return "CRE (ht)";
    } else if( strcmp( fileName.substr(pos).c_str(), "r03") == 0 ) { 
        return "NAA (area)";
    } else if( strcmp( fileName.substr(pos).c_str(), "rh03") == 0 ) { 
        return "NAA (ht)";
    } else if( strcmp( fileName.substr(pos).c_str(), "r04") == 0 ) { 
        return "LAC/LIP (area)";
    } else if( strcmp( fileName.substr(pos).c_str(), "rh04") == 0 ) { 
        return "LAC/LIP (ht)";
    } else if( strcmp( fileName.substr(pos).c_str(), "r01ares02") == 0 ) { 
        return "CCRI (area)";
    } else if( strcmp( fileName.substr(pos).c_str(), "r01hres02") == 0 ) { 
        return "CCRI (ht)";
    } else if( strcmp( fileName.substr(pos).c_str(), "r01ares03") == 0 ) { 
        return "CNI (area)";
    } else if( strcmp( fileName.substr(pos).c_str(), "r01hres03") == 0 ) { 
        return "CNI (ht)";
    } else { 
        return "UNK";
    }
    
}


