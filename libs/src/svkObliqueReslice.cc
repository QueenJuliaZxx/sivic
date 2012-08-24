/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkObliqueReslice.h>

#include <vtkImageChangeInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>


using namespace svk;


vtkCxxRevisionMacro(svkObliqueReslice, "$Rev$");
vtkStandardNewMacro(svkObliqueReslice);


/*!
 *
 */
svkObliqueReslice::svkObliqueReslice()
{
#if VTK_DEBUG_ON
    //this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->reslicer      = vtkImageReslice::New(); 
    this->reslicer->SetInterpolationModeToCubic();
    this->reslicedImage = NULL;  
}


/*!
 *
 */
svkObliqueReslice::~svkObliqueReslice()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkObliqueReslice()");
    
    if (this->reslicer != NULL) {
        this->reslicer->Delete();
        this->reslicer = NULL; 
    }

}


/*!
 * 
 */
void svkObliqueReslice::SetTargetDcosFromImage(svkImageData* image)
{
    image->GetDcmHeader()->GetDataDcos(this->targetDcos); 
}


/*!
 *  
 * 
 */
void svkObliqueReslice::SetTargetDcos(double dcos[3][3])
{
    this->targetDcos[0][0] = dcos[0][0];
    this->targetDcos[0][1] = dcos[0][1];
    this->targetDcos[0][2] = dcos[0][2];
    this->targetDcos[1][0] = dcos[1][0];
    this->targetDcos[1][1] = dcos[1][1];
    this->targetDcos[1][2] = dcos[1][2];
    this->targetDcos[2][0] = dcos[2][0];
    this->targetDcos[2][1] = dcos[2][1];
    this->targetDcos[2][2] = dcos[2][2];
}


/*!
 *
 */
int svkObliqueReslice::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    this->reslicer->SetInput( this->GetImageDataInput(0) ); 

    double rotation[3][3];   
    this->SetRotationMatrix(); 

    this->reslicer->SetResliceAxesDirectionCosines(
        this->rotation[0][0],
        this->rotation[0][1],
        this->rotation[0][2],
        this->rotation[1][0],
        this->rotation[1][1],
        this->rotation[1][2],
        this->rotation[2][0],
        this->rotation[2][1],
        this->rotation[2][2]
    );
    
    reslicer->Update();

    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    // These values are not quite right... we only know the correct origin after request data is run
    outInfo->Set(vtkDataObject::SPACING(), this->reslicer->GetOutput()->GetSpacing(), 3);

    return 1;

}


/*!
 *
 *  Copy the Dcm Header, svkOrientedImageData and Provenance from the 
 *  input to the output image. 
 */
int svkObliqueReslice::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) 
{

    //  Get the allocated svk output image data (resliced image doesn't have a dcos yet): 
    this->GetOutput()->SetDcos( this->targetDcos ); 
    string activeScalarName = this->GetImageDataInput(0)->GetPointData()->GetScalars()->GetName();

    this->reslicer->SetInput( this->GetImageDataInput(0) );

    int numberOfVolumes = this->GetImageDataInput(0)->GetPointData()->GetNumberOfArrays();

    for( int i = 0; i < numberOfVolumes; i++ ) {
    	string currentScalarName = this->GetImageDataInput(0)->GetPointData()->GetArray(i)->GetName();
    	this->GetImageDataInput(0)->GetPointData()->SetActiveScalars( currentScalarName.c_str() );

    	/*
    	 *  We are going to switch the input data to NULL and then back to the image data object.
    	 *  This is because the vtk algorithm will not re-execute just because the active scalars
    	 *  have changed (although it really should). To get around this we switch the input to
    	 *  NULL and then back to force an update.
    	 */
		this->reslicer->SetInput( NULL );
		this->reslicer->SetInput( this->GetImageDataInput(0) );
    	this->reslicer->Update();

    	if( i == 0 ) {
			//  Copy the vtkImageAlgo output to the allocated svkImageData output image
			this->GetOutput()->DeepCopy( this->reslicer->GetOutput() );
    	} else {
    		// We need to copy the data array after each update because it will be reused
    		vtkDataArray* newArray = vtkDataArray::CreateDataArray( this->reslicer->GetOutput()->GetPointData()->GetScalars()->GetDataType());
    		newArray->DeepCopy(this->reslicer->GetOutput()->GetPointData()->GetScalars() );
    		this->GetOutput()->GetPointData()->AddArray(newArray);
    		newArray->Delete();
    	}
		this->GetOutput()->GetPointData()->GetArray(i)->SetName(currentScalarName.c_str());
    }
	this->GetImageDataInput(0)->GetPointData()->SetActiveScalars( activeScalarName.c_str() );



    this->UpdateHeader(); 
    outputVector->GetInformationObject(0)->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->GetOutput()->GetExtent(), 6);
    
    // When we updated the header we changed the origin and spacing so we have to update here...
    outputVector->GetInformationObject(0)->Set(vtkDataObject::ORIGIN(), this->GetOutput()->GetOrigin(), 3);
    if ( this->GetDebug() ) {
        //this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    return 1; 
}


/*!
 *  Get the rotation matrix that will transform the input orientation (dcos) to the
 *  target orientation (targetDcos):  
 *      Rot * dcosIn = targetDcos
 *      Rot * dcosIn* (dcosIn)-1 = targetDcos* (dcosIn)-1
 *      Rot  = targetDcos* (dcosIn)-1
 */
void svkObliqueReslice::SetRotationMatrix( )
{

    double dcosIn[3][3];   
    this->GetImageDataInput(0)->GetDcmHeader()->GetDataDcos(dcosIn); 

    double dcosInInverse[3][3];   
    vtkMath::Invert3x3(dcosIn, dcosInInverse);

    vtkMath::Multiply3x3(targetDcos, dcosInInverse, this->rotation);

}


/*!
 *
 */
void svkObliqueReslice::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  Set the header info to match the resliced data set: 
 */
void svkObliqueReslice::UpdateHeader()
{
    string seriesDescription("RESLICED ");
	seriesDescription.append(this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue("SeriesDescription"));

    this->reslicedImage = this->GetOutput(); 

    //  Copy the DICOM header:     
    this->GetImageDataInput(0)->GetDcmHeader()->MakeDerivedDcmHeader( 
        this->reslicedImage->GetDcmHeader(), 
        seriesDescription 
    );

    if ( this->GetDebug() ) {
        //cout << "INPUT IMAGE: " << *( this->GetImageDataInput(0) ) << endl;
        //cout << "OUTPUT IMAGE: " << *( this->reslicedImage ) << endl;
    }

    int* extent = this->reslicedImage->GetExtent(); 
    this->newNumVoxels[0] =  extent[1] + 1; 
    this->newNumVoxels[1] =  extent[3] + 1; 
    this->newNumVoxels[2] =  extent[5] + 1; 

    this->reslicedImage->GetDcmHeader()->SetValue( 
        "Columns", 
        this->newNumVoxels[0]
    );
    this->reslicedImage->GetDcmHeader()->SetValue( 
        "Rows", 
        this->newNumVoxels[1]
    );
    int numberOfVolumes = this->reslicedImage->GetPointData()->GetNumberOfArrays();
    this->reslicedImage->GetDcmHeader()->SetValue( 
        "NumberOfFrames", 
        this->newNumVoxels[2]*numberOfVolumes
    );

    //  Set Origin and Orientation and spacing
    this->SetReslicedHeaderSpacing();
    this->SetReslicedHeaderPerFrameFunctionalGroups();
    this->SetReslicedHeaderOrientation();

    if ( this->GetDebug() ) {
        //this->reslicedImage->GetDcmHeader()->PrintDcmHeader();
        //cout << "OUTPUT IMAGE (updated header): " << *( this->reslicedImage ) << endl;
    }

    
}



/*!
 *  Sets the new spacing into DCM header.
 */
void svkObliqueReslice::SetReslicedHeaderSpacing()
{

    this->reslicedImage->GetSpacing(this->newSpacing);

    cout << endl;
    ostringstream* oss = new ostringstream();
    *oss << this->newSpacing[0];
    string pixelSpacingString( oss->str() + "\\" );
    delete oss;

    oss = new ostringstream();
    *oss << this->newSpacing[1];
    pixelSpacingString.append( oss->str() );
    
    delete oss;
    oss = new ostringstream();
    *oss << this->newSpacing[2];
    string sliceThicknessString( oss->str() );

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        pixelSpacingString,
        sliceThicknessString
    );

    delete oss;
}


/*!
 *  Calculate the new slice positins. 
 */
void svkObliqueReslice::SetReslicedHeaderPerFrameFunctionalGroups()
{
    
    //  Get Center of original image volume: 
    //  first get the tlc of the stack:
    double* tlc0 = new double[3]; 
    this->GetImageDataInput(0)->GetDcmHeader()->GetOrigin(tlc0, 0);

    double* inputSpacing = new double[3]; 
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSpacing(inputSpacing); 

    int numVoxels[3]; 
    this->GetImageDataInput(0)->GetNumberOfVoxels(numVoxels);

    double dcosIn[3][3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetDataDcos(dcosIn); 

    //  Now calculate the volumetric center by displacing by 1/2 fov - 1/2 voxel from tlc position: 
    double origin[3]; 
    for (int i = 0; i < 3; i++) {
        origin[i] = tlc0[i];  
        for (int j = 0; j < 3; j++) {
            origin[i] += dcosIn[j][i] * (inputSpacing[j] * ((numVoxels[j]-1)/2.) );
        }
    }

    //  Now calculate the NEW tlc: 
    //  this->targetDcos 
    //  this->newSpacing
    //  this->newNumVoxels
    double newTlc[3]; 
    for (int i = 0; i < 3; i++) {
        newTlc[i] = origin[i];  
        for (int j = 0; j < 3; j++) {
            newTlc[i] -= targetDcos[j][i] * (this->newSpacing[j] * ((this->newNumVoxels[j] - 1)/2.) );
        }
    }
    double reslicedTlc[3]; 
    this->reslicedImage->GetOrigin( reslicedTlc );

    this->reslicedImage->SetOrigin( newTlc );
    this->reslicedImage->SetOrigin( newTlc );

    int numSlices = this->GetOutput()->GetDimensions()[2];
    int numTimepoints = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();
    int numCoils = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfCoils();
    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        newTlc, this->newSpacing, this->targetDcos, numSlices, numTimepoints, numCoils
    );

    delete[] tlc0;
    delete[] inputSpacing;
}



/*!
 *  Reset the DICOM orientation 
 */
void svkObliqueReslice::SetReslicedHeaderOrientation()
{
    string orientationString;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ostringstream dcosValue;
            dcosValue << this->targetDcos[i][j];
            orientationString.append( dcosValue.str() ) ;
            if (i < 2) {
                orientationString.append( "\\") ;
            }
        }
    }

    this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        orientationString,
        "SharedFunctionalGroupsSequence",
        0
    );

        //  Determine whether the data is ordered with or against the slice normal direction.
    double normal[3];
    this->reslicedImage->GetDcmHeader()->GetNormalVector(normal);
    svkDcmHeader::DcmDataOrderingDirection dataSliceOrder = svkDcmHeader::SLICE_ORDER_UNDEFINED;

    double dcosSliceOrder[3];
    for (int i = 0; i < 3; i++) {
        dcosSliceOrder[i] = this->targetDcos[2][i];
    }

    //  Use the scalar product to determine whether the data in the .cmplx 
    //  file is ordered along the slice normal or antiparalle to it. 
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
    this->reslicedImage->GetDcmHeader()->SetSliceOrder( dataSliceOrder );
    math->Delete();

}


/*
 *  Applies calculated rotation matrix to rotate coordinate system 
 *  vectors (dcos, with matrix with rows representing axes), needs
 *  to be transposed to apply rotation
 */
void svkObliqueReslice::RotateAxes(double axesIn[3][3], double rotatedAxes[3][3])
{
    //double axesInTsp[3][3]; 
    //vtkMath::Transpose3x3(axesIn, axesInTsp); 

    //double rotatedAxes[3][3]; 
    vtkMath::Multiply3x3(this->rotation, axesIn, rotatedAxes); 

    //vtkMath::Transpose3x3(rotatedAxesTsp, rotatedAxes); 
}



/*!
 *
 */
void svkObliqueReslice::Print3x3(double matrix[3][3], string name) 
 {
    cout << name << "1:  " << matrix[0][0] << "    " << matrix[0][1]<<"    "<<matrix[0][2] <<endl; 
    cout << name << "2:  " << matrix[1][0] << "    " << matrix[1][1]<<"    "<<matrix[1][2] <<endl; 
    cout << name << "3:  " << matrix[2][0] << "    " << matrix[2][1]<<"    "<<matrix[2][2] <<endl << endl; 
}


/*!
 *  Oblique Reslice only works with image data.  Output will be of the same type as input 
 *  which is the default behavior for an svkImageAlgorithm. 
 */
int svkObliqueReslice::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}

