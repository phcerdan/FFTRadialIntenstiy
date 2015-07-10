/* SAXSsim, Apply to a microscopy image, returning a I-q data set, allowing comparisson with Small Angle X-ray Scattering experiments.
Copyright (C) 2015 Pablo Hernandez-Cerdan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef SAXSsim_H_
#define SAXSsim_H_

#include "itkImage.h"
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <array>
#include <utility>
#include <algorithm>
#include <cmath>
#include <chrono>
#ifdef ENABLE_PARALLEL
#include <omp.h>
#endif
#ifdef ENABLE_QT
#include "../QT/Q_DebugStream.h"
#endif

class SAXSsim
{
public:
    // ITK typedefs
    const static unsigned int Dimension = 2;
    typedef unsigned int                                        InputPixelType;
    typedef double                                              RealPixelType;
    typedef itk::Image< InputPixelType, Dimension>              InputImageType;
    typedef itk::Image< RealPixelType, Dimension>               RealImageType;
    typedef itk::Image< std::complex<RealPixelType>, Dimension> ComplexImageType;
    typedef InputImageType::Pointer                             InputTypeP;
    typedef RealImageType::Pointer                              RealTypeP;
    typedef ComplexImageType::Pointer                           ComplexTypeP;
    // For Output(show and write) purposes of FFT.
    typedef unsigned short                                      OutputPixelType;
    typedef itk::Image< OutputPixelType, Dimension >            OutputImageType;
    typedef OutputImageType::Pointer                            OutputTypeP;

    using Intensities =  std::vector<std::vector<double> > ;

public:
    SAXSsim() = default;
    SAXSsim(const std::string imgName, std::string outputPlotName = "",
             int num_threads = 1, bool saveToFile=1);
    virtual ~SAXSsim ();
    void Initialize();
    void SetInputParameters(std::string inputName, std::string outputName, int numThreads, bool saveToFile);
#ifdef ENABLE_QT
    void SetQDebugStream(Q_DebugStream* input);
    Q_DebugStream* m_debugStream;
#endif
    InputTypeP  Read();
    ComplexTypeP  & FFT();
    RealTypeP & FFTModulusSquare();
    void SaveIntensityProfile(const std::string & fname);
    void GeneratePDF(const std::string & resultfile, double image_resolution);
    void WriteFFT( const RealTypeP & inputFFT,  const std::string &outputFilename);
    OutputTypeP RescaleFFT(const RealTypeP& inputFFT);

    Intensities & ComputeRadialIntensity();
#ifdef ENABLE_PARALLEL
    Intensities & ParallelComputeRadialIntensity();
#endif
    Intensities intensities_;
    std::vector<double>&  MeanIntensities();
    std::vector<double> intensitiesMean_;

    InputTypeP inputImg_;
    ComplexTypeP fftImg_;
    RealTypeP fftModulusSquare_;
    //For visualization only:
    RealTypeP WindowingFFT(
        const RealTypeP& modulusFFT,
        RealPixelType maxInputValue,
        RealPixelType maxOuputValue = 255 );
    RealTypeP &LogFFTModulusSquare(const RealTypeP & modulo);
    void ScaleForVisualization();
    std::vector<double>  intensitiesVisualization_;
    RealTypeP fftVisualization_;

    std::pair< int, int> midSize_;
    std::pair< int, int> imgSize_;
    std::pair<bool, bool> evenFlag_;
    int fMax_{0};
protected:
    void CheckEqualDimension();
    void InitializeSizeMembers();

    std::string inputName_;
    std::string outputName_;
    int numThreads_{1};
    bool saveToFile_{ true };

    template<typename T = double>
    inline T Modulo (const T &a, const T& b){
        return sqrt(a*a + b*b);
    }
};
#endif
