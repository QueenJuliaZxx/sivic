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


#ifndef SVK_DCM_ENHANCED_VOLUME_READER_H
#define SVK_DCM_ENHANCED_VOLUME_READER_H

#include <svkDcmVolumeReader.h>


namespace svk {


/*! 
 *  Reads DICOM MR Enhanced Objects SOPClassUID = 1.2.840.10008.5.1.4.1.1.4.1.
 */
class svkDcmEnhancedVolumeReader : public svkDcmVolumeReader 
{

    public:

        static svkDcmEnhancedVolumeReader* New();
        vtkTypeRevisionMacro( svkDcmEnhancedVolumeReader, svkDcmVolumeReader );

        void UseDoublePrecision( bool useDoublePrecision );
        void SetRescalePixels( bool rescalePixels );

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "DICOM MR Enhanced Object";
        }

        //  Enum reader type
        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::DICOM_ENHANCED_MRI;
        }

        //  Methods:
        virtual int CanReadFile(const char* fname);

    protected:

        svkDcmEnhancedVolumeReader();
        ~svkDcmEnhancedVolumeReader();

		void         GetPixelTransform(double& intercept, double& slope, svkDcmHeader* header);
		virtual bool IsDataFloatingPoint( );
		virtual bool ArePixelsScaled( );
		void         GetRescaledPixels(double* doublePixels, unsigned short* shortPixels, double intercept, double slope, int numberOfValues );
		void         GetRescaledPixels(float* doublePixels, unsigned short* shortPixels, double intercept, double slope, int numberOfValues );

		// Required from parent class
        virtual svkDcmHeader::DcmPixelDataFormat GetFileType();
        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual void                            ExecuteInformation();

    private:
        
        bool useDoublePrecision;
        bool rescalePixels;

        virtual void    LoadData(svkImageData* data); 
        virtual void    InitPrivateHeader(); 

};


}   //svk


#endif //SVK_DCM_ENHANCED_VOLUME_READER_H

