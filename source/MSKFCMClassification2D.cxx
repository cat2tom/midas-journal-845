#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkSimpleFilterWatcher.h"

#include "itkMSKFCMClassifierInitializationImageFilter.h"
#include "itkFuzzyClassifierImageFilter.h"

int
main(int argc, char * argv[])
{

  if (argc < 11)
    {
      std::cerr << "usage: " << argv[0] << " input output nmaxIter error m P Q"
        "numThreads numClasses { centroids_1,...,centroid_numClusters } sigma"
        "radius [ -f valBackground ]" << std::endl;
      exit(1);
    }

  const int dim = 2;
  typedef signed short IPixelType;

  typedef unsigned char OPixelType;

  typedef itk::Image<IPixelType, dim> IType;
  typedef itk::Image<OPixelType, dim> OType;

  typedef itk::ImageFileReader<IType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(argv[1]);

  typedef itk::FuzzyClassifierInitializationImageFilter<IType>
      TFuzzyClassifier2D;
  typedef itk::MSKFCMClassifierInitializationImageFilter<IType>
      TClassifierMSKFCM;

  TClassifierMSKFCM::Pointer classifier = TClassifierMSKFCM::New();
  classifier->SetMaximumNumberOfIterations(atoi(argv[3]));
  classifier->SetMaximumError(atof(argv[4]));
  classifier->SetM(atof(argv[5]));
  classifier->SetP(atof(argv[6]));
  classifier->SetQ(atof(argv[7]));
  classifier->SetNumberOfThreads(atoi(argv[8]));

  int numClasses = atoi(argv[9]);
  classifier->SetNumberOfClasses(numClasses);

  TFuzzyClassifier2D::CentroidArrayType centroidsArray;

  int argvIndex = 10;
  for (int i = 0; i < numClasses; i++)
    {
      centroidsArray.push_back(atof(argv[argvIndex]));
      ++argvIndex;
    }

  classifier->SetCentroids(centroidsArray);
  //itk::SimpleFilterWatcher watcher(classifier, "MSKFCM classifier");

  typedef TFuzzyClassifier2D::CentroidType TCentroid;

  typedef itk::Statistics::RBFKernelInducedDistanceMetric<TCentroid>
      RBFKernelType;
  RBFKernelType::Pointer kernelDistancePtr = RBFKernelType::New();
  kernelDistancePtr->SetA(2.0);
  kernelDistancePtr->SetB(1.0);
  kernelDistancePtr->SetSigma(atoi(argv[argvIndex]));
  classifier->SetKernelDistanceMetric(static_cast<
      TClassifierMSKFCM::KernelDistanceMetricPointer >( kernelDistancePtr));

  typedef itk::FlatStructuringElement<dim> StructuringElement2DType;
  StructuringElement2DType::RadiusType elementRadius;
  for (int i = 0; i < dim; i++)
    {
      ++argvIndex;
      elementRadius[i] = atoi(argv[argvIndex]);
    }
  StructuringElement2DType structuringElement = StructuringElement2DType::Box(
      elementRadius);
  classifier->SetStructuringElement(structuringElement);

  if ( (argc-1 > argvIndex ) && (strcmp(argv[argvIndex + 1], "-f") == 0) )
    {
      classifier->SetIgnoreBackgroundPixels(true);
      classifier->SetBackgroundPixel(atof(argv[argvIndex + 2]));
    }

  classifier->SetInput(reader->GetOutput());

  typedef itk::FuzzyClassifierImageFilter<TClassifierMSKFCM::OutputImageType>
      TLabelClassifier2D;
  TLabelClassifier2D::Pointer labelClass = TLabelClassifier2D::New();
  labelClass->SetInput(classifier->GetOutput());

  typedef itk::ImageFileWriter<OType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(labelClass->GetOutput());
  writer->SetFileName(argv[2]);

  try
    {
    writer->Update();
    }
  catch (itk::ExceptionObject & excp)
    {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
