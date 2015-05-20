/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <svkDCEBaseHeight.h>
#include <svkMathUtils.h>

#include <cmath>
#include <algorithm>

using namespace svk;


vtkCxxRevisionMacro(svkDCEBaseHeight, "$Rev$");
vtkStandardNewMacro(svkDCEBaseHeight);


/*!
 *
 */
svkDCEBaseHeight::svkDCEBaseHeight()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");
    //  Output Port 0 = base height map
    this->SetNumberOfOutputPorts(1); 

    this->normalize = false; 
    this->baselineMean = 0.;
    this->baselineStdDeviation = 0.; 
}



/*!
 *
 */
svkDCEBaseHeight::~svkDCEBaseHeight()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}



/*! 
 *  Integrate real spectra over specified limits. 
 */
void svkDCEBaseHeight::GenerateMaps()
{

    this->InitializeInjectionPoint();
    this->InitializeBaseline(); 

    int numVoxels[3]; 
    this->GetOutput(0)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //  Get the data array to initialize.  
    vtkDataArray* dceMapBaseHtArray;
    dceMapBaseHtArray = this->GetOutput(0)->GetPointData()->GetArray(0); 

    //  Add the output volume array to the correct array in the svkMriImageData object
    string arrayNameString("pixels");
    dceMapBaseHtArray->SetName( arrayNameString.c_str() );

    double voxelValue;
    for (int i = 0; i < totalVoxels; i++ ) {

        vtkFloatArray* perfusionDynamics = vtkFloatArray::SafeDownCast( 
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(i) 
        ); 
        float* dynamicVoxelPtr = perfusionDynamics->GetPointer(0);

        this->InitializeOutputVoxelValues( dynamicVoxelPtr, i ); 

    }

    if ( this->normalize ) {

        double nawmValue = this->GetNormalizationFactor(); 
        for (int i = 0; i < totalVoxels; i++ ) {
            voxelValue = dceMapBaseHtArray->GetTuple1( i );
            voxelValue /= nawmValue; 
            dceMapBaseHtArray->SetTuple1(i, voxelValue);
        }

    }

}


/*!  
 *  For multi-volume data modifies header's per frame functional group sequence:
 */
void svkDCEBaseHeight::InitializeOutputVoxelValues( float* dynamicVoxelPtr, int voxelIndex ) 
{

    double voxelBaseHt; 
    int    filterWindow = 5;
    int    arrayLength = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();

    svkMathUtils::MedianFilter1D( dynamicVoxelPtr, arrayLength, filterWindow);
    this->GetBaseParams( dynamicVoxelPtr, &voxelBaseHt); 

    //  Get the data array to initialize.  
    vtkDataArray* dceMapBaseHtArray;
    dceMapBaseHtArray = this->GetOutput(0)->GetPointData()->GetArray(0); 
    dceMapBaseHtArray->SetTuple1(  voxelIndex, voxelBaseHt );
}



/*!
 *  Compute the mean baseline and std dev as the mean of the first 
 *  time point over all spatial points in the volume 
 */
void svkDCEBaseHeight::InitializeBaseline()
{

    this->baselineMean = this->GetTimePointMean(0); 

    vtkDataArray* timePoint0Pixels = this->GetOutput(0)->GetPointData()->GetArray( 0 ); 
    int numSpatialPixels = timePoint0Pixels->GetNumberOfTuples(); 
    this->baselineStdDeviation = this->GetStandardDeviation( timePoint0Pixels, this->baselineMean, numSpatialPixels); 
}


/*
 * Compute the stdandard deviation of the array up to the specified endPt. 
 */
double svkDCEBaseHeight::GetStandardDeviation( vtkDataArray* array, float mean, int endPt) 
{
    double sumOfSquareDiffs = 0.; 
    for ( int i = 0; i < endPt; i++ ) {
        double diff = ( array->GetTuple1(i) - mean ); 
        sumOfSquareDiffs += diff * diff; 
    }
    
    double variance = sumOfSquareDiffs / endPt;
    return sqrt(variance);
}


/*!
 *  Compute the mean value over all spatial locations for the specified time point. 
 */
double svkDCEBaseHeight::GetTimePointMean( int timePoint )
{

    vtkDataArray* timePointPixels = this->GetImageDataInput(0)->GetPointData()->GetArray( timePoint ); 

    double sum = 0.; 

    int numSpatialPixels = timePointPixels->GetNumberOfTuples(); 
    for ( int i = 0; i < numSpatialPixels; i++ ) {
        sum += timePointPixels->GetTuple1(i); 
    }
    double mean = sum / numSpatialPixels; 

    return mean;     
}

/*!
 *  Compute the mean baseline and std dev as the mean of the first 
 *  time point over all spatial points in the volume 
 */
void svkDCEBaseHeight::InitializeInjectionPoint()
{

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader();
    int numberOfTimePoints = hdr->GetNumberOfTimePoints();

    //  Create a vtkDataArray to hold the average time kinetic 
    //  trace over the entire volume: 
    vtkFloatArray* time0Pixels = 
            static_cast<vtkFloatArray*>(
                svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(0) 
            ); 
    vtkFloatArray* averageTrace = vtkFloatArray::New();
    averageTrace->DeepCopy( time0Pixels );

    //  For each time point compute the mean value over all spatial points  
    //  Essentially 1 voxel that represents the DCE average DCE trace.
    for ( int timePt = 0; timePt < numberOfTimePoints; timePt++ ) {
        double timeSpatialMean = this->GetTimePointMean( timePt ); 
        averageTrace->SetTuple1( timePt, timeSpatialMean ); 
    }

    //  Now determine the injection point. 
    this->injectionPoint = 2;
    float runningSum     = 0.0;
    float runningAvg     = 0.0;
    float nextBaseline; 
    float basefactor;
    double runningStdDev;
    //  number of std devs above baseline for detection of the injection point. 
    float  injectionPointSDThreshold = 2.5; 
    for ( int timePt = 0; timePt < numberOfTimePoints; timePt++ ) {

        runningSum   += averageTrace->GetTuple1( timePt );
        runningAvg    = runningSum/( timePt + 1.0 );
        runningStdDev = this->GetStandardDeviation( averageTrace, runningAvg, timePt ); 

        nextBaseline = averageTrace->GetTuple1( timePt + 1 );
        basefactor   = runningAvg + injectionPointSDThreshold * runningStdDev;
        if (nextBaseline > basefactor) {
            this->injectionPoint = timePt;
        }
    }

}


/*!  
 *  Determines baseline value 
 */
void svkDCEBaseHeight::GetBaseParams( float* dynamicVoxelPtr, double* voxelBaseHt )
{
    // get base value
    int   injectionPoint = this->injectionPoint; 
    float baselineVal    = 0;
    for ( int pt = 1; pt < 5; pt++) {
    //for ( int pt = 3; pt < injectionPoint; pt++) {
        baselineVal += dynamicVoxelPtr[ pt ];
    }
    baselineVal = baselineVal / 4;
    //if ( injectionPoint > 1 ) {
    //    baselineVal = baselineVal / ( injectionPoint - 1 );
    //}
    *voxelBaseHt = baselineVal;
}
