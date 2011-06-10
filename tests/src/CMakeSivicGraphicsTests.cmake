# 
#   Copyright © 2009-2011 The Regents of the University of California.
#   All Rights Reserved.
# 
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#   •   Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#   •   Redistributions in binary form must reproduce the above copyright notice,
#       this list of conditions and the following disclaimer in the documentation
#       and/or other materials provided with the distribution.
#   •   None of the names of any campus of the University of California, the name
#       "The Regents of the University of California," or the names of any of its
#       contributors may be used to endorse or promote products derived from this
#       software without specific prior written permission.
# 
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#   IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
#   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
#   OF SUCH DAMAGE.
#

#
#   $URL$
#   $Rev$
#   $Author$
#   $Date$
#
#   Authors:
#       Jason C. Crane, Ph.D.
#       Beck Olson

#SET(MEMORYCHECK_SUPPRESSIONS_FILE /home/bolson/Projects/sivic/trunk/tests/src/suppressions/renderSuppressions)
#INCLUDE(CTest)
#ENABLE_TESTING()
#############################################################
#   Paths to binary applications and scripts
#############################################################
SET( DEDICATED_TEST_BIN_PATH ${CMAKE_BINARY_DIR}/trunk/tests/${PLATFORM})

#############################################################
#   Location where output files from individual tests 
#   are written to. 
#############################################################
SET( TEST_RESULTS_ROOT ${SVK_TEST_ROOT}/results_tmp)

#############################################################
#   Flags for diff to avoid errors from minor differences in 
#   paths and rootnames in header files. 
#############################################################
SET( DIFF_OPT --ignore-matching-lines=SVK_CMD --ignore-matching-lines=root)

#############################################################
# This is a basic test to see if VTK is working
#############################################################
SET( TEST_NAME VTK_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/vtk_baseline)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/vtkBaselineTest ${TEST_RESULTS_PATH}/out.jpeg )

SET( TEST_NAME VTK_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )

#############################################################
# Check to see if you can render spectra from a phantom 
#############################################################
SET( TEST_NAME PHANTOM_SPECTRA_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ddf_files/ddf_to_ddf)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t RenderingTest --spectra ${TEST_CASE_ROOT}/input/20x_1.ddf -p ${TEST_RESULTS_PATH}
 )

SET( TEST_NAME PHANTOM_SPECTRA_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${PLATFORM} )

#############################################################
# Check to see if you can render an image from a phantom 
#############################################################
SET( TEST_NAME PHANTOM_IMAGE_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/idf_files/idf_to_idf)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t RenderingTest --image ${TEST_CASE_ROOT}/input/vol.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PHANTOM_IMAGE_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_1/${PLATFORM} )

#############################################################
# Check to see if you can render an image and spectra from a 
# phantom.
#############################################################
SET( TEST_NAME PHNTM_IMG_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/idf_files/idf_to_idf)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t RenderingTest --image ${TEST_CASE_ROOT}/input/vol.idf --spectra ${SVK_TEST_ROOT}/ddf_files/ddf_to_ddf/input/20x_1.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PHNTM_IMG_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_2/${PLATFORM} )

#############################################################
# Check to see if you can render an image with spectra and an 
# overlay from a phantom.
#############################################################
SET( TEST_NAME OVERLAY_MET_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t RenderingTest --image ${TEST_CASE_ROOT}/input/refImage.idf --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME OVERLAY_MET_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_3/${PLATFORM} )

#############################################################
# Check to see if you can render spectra and an overlay from
# a phantom.
#############################################################
SET( TEST_NAME PLOT_GRID_MET_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t RenderingTest --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PLOT_GRID_MET_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_5/${PLATFORM} )

#############################################################
# Check to see if the validation will catch the origin shift
# in the overlay-- no overlay should be rendered. 
#############################################################
SET( TEST_NAME PLOT_GRID_MET_SHIFT_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t RenderingTest --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met_shifted.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PLOT_GRID_MET_SHIFT_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_6/${PLATFORM} )


#############################################################
# Check to see if the we can render our phantom in multiple orientations. 
#############################################################
SET( TEST_NAME ORIENTATION_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t OrientationTest --image ${TEST_CASE_ROOT}/input/refImage.idf --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME ORIENTATION_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )


#############################################################
# Check to see if you can render spectra and an overlay from
# a phantom in different orientations.
#############################################################
SET( TEST_NAME ORIENTATION_SPEC_MET_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t OrientationTest --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME ORIENTATION_SPEC_MET_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )


#############################################################
# Check to see if the we can render a sagittal phantom  image and spectra in multiple orientations. 
#############################################################
SET( TEST_NAME SAG_IMAGE_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/orientations)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t OrientationTest --image ${TEST_CASE_ROOT}/input/sag_phantom.idf --spectra ${TEST_CASE_ROOT}/input/sag_recon.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME SAG_IMAGE_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )


#############################################################
# Check to see if you can render sagittal spectra
#############################################################
SET( TEST_NAME SAG_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/orientations)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t OrientationTest --spectra ${TEST_CASE_ROOT}/input/sag_recon.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME SAG_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )


#############################################################
# Check to see if the we can render a coronal phantom  image and spectra in multiple orientations. 
#############################################################
SET( TEST_NAME COR_IMAGE_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/orientations)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t OrientationTest --image ${TEST_CASE_ROOT}/input/cor_phantom.idf --spectra ${TEST_CASE_ROOT}/input/cor_phased.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME COR_IMAGE_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )


#############################################################
# Check to see if you can render coronal spectra
#############################################################
SET( TEST_NAME COR_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/orientations)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t OrientationTest --spectra ${TEST_CASE_ROOT}/input/cor_phased.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME COR_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )


#############################################################
# Check to see if you can render spectra from a phantom with sat bands
#############################################################
SET( TEST_NAME PHANTOM_SPECTRA_SAT_BANDS_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/sat_bands)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t SatBandTest --spectra ${TEST_CASE_ROOT}/input/sag_recon.ddf -p ${TEST_RESULTS_PATH}
 )

SET( TEST_NAME PHANTOM_SPECTRA_SAT_BANDS_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )


#############################################################
# Check to see if you can render an image from a phantom with sat bands
#############################################################
SET( TEST_NAME PHANTOM_IMAGE_SAT_BANDS_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/sat_bands)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t SatBandTest --image ${TEST_CASE_ROOT}/input/sag_phantom.idf --spectra ${TEST_CASE_ROOT}/input/sag_recon.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PHANTOM_IMAGE_SAT_BANDS_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
