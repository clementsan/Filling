#include <iostream>

#include <itkImage.h>
#include <itkImageFileReader.h> 
#include <itkImageFileWriter.h>

using namespace itk;
using namespace std;

const int Dimension = 3;
typedef unsigned short ImagePixelType;
typedef Image<ImagePixelType,Dimension>  ImageType;

typedef ImageFileReader<ImageType> VolumeReaderType;
typedef ImageFileWriter<ImageType> VolumeWriterType;

static void clear_edge(unsigned short *image, int *dims, int clear_label);
static void draw_fill_inside_image(unsigned short *image, int *dim, int new_label);

int main(int argc, const char* argv[])
{

  ImageType::Pointer image ;
  VolumeReaderType::Pointer labelReader = VolumeReaderType::New();

  // Reading image
  VolumeReaderType::Pointer imageReader = VolumeReaderType::New();
  imageReader->SetFileName(argv[1]) ;
  try
    {
      imageReader->Update() ;
    }
  catch (ExceptionObject err)
    {
      cerr<<"Exception object caught!"<<std::endl;
      cerr<<err<<std::endl;
      exit(0) ;
    }
  image = imageReader->GetOutput() ;

  ImageType::IndexType nullIndex;
  nullIndex[0] = 0;
  nullIndex[1] = 0;
  nullIndex[2] = 0;
  
  ImagePixelType *data = &((*image)[nullIndex]);
  ImageType::RegionType imageRegion = image->GetBufferedRegion();
  int dim[3];
  dim[0] = imageRegion.GetSize(0);
  dim[1] = imageRegion.GetSize(1);
  dim[2] = imageRegion.GetSize(2);

  clear_edge(data, dim, 0);
  draw_fill_inside_image(data, dim, 1);


  VolumeWriterType::Pointer writer = VolumeWriterType::New();
  writer->SetFileName(argv[2]);
  writer->SetInput(image);
  try
    {
      writer->Update();
    }
  catch (itk::ExceptionObject & err)
    {
      std::cerr<<"Exception object caught!"<<std::endl;
      std::cerr<<err<<std::endl;
    }
  return EXIT_SUCCESS;
}



static void clear_edge(unsigned short *image, int *dims, int clear_label)
  // clears the edge of the image
{
  int size_plane = dims[0]*dims[1];
  int size_line = dims[0];

  for (int z = 0; z < dims[2]; z++) {
    for (int y = 0; y < dims[1]; y++) {
      if ( (y == 0) || (y == dims[1]-1) ||
        (z == 0) || (z == dims[2]-1) ) { // draw whole plane
        for (int x = 0; x < dims[0] ; x++) 
          image[x +  size_line * y + size_plane * z] = clear_label;
        } else { // draw edges of x
        image[0 +  size_line * y + size_plane * z] = clear_label;
        image[size_line - 1 +  size_line * y + size_plane * z] = clear_label;
      }
    }
  }

}

static void 
draw_fill_inside_image(unsigned short *image, int *dims, int new_label)
  // Fill the 'inside' part of an image (closed objects that do not touch the
  // image edges)
{
  int size_plane = dims[0]*dims[1];
  int size_line = dims[0];

  unsigned short label;
  if (new_label > 1) label = new_label-1;
  else label = new_label+1;
  
  unsigned short background = 0;

  // fill image edges -> won't work if object touches the edge !!
  for (int z = 0; z < dims[2]; z++) {
    for (int y = 0; y < dims[1]; y++) {
      if ( (y == 0) || (y == dims[1]-1) ||
        (z == 0) || (z == dims[2]-1) ) { // draw whole plane
     for (int x = 0; x < dims[0] ; x++) 
       image[x +  size_line * y + size_plane * z] = label;
      } else { // draw edges of x
     image[0 +  size_line * y + size_plane * z] = label;
     image[size_line - 1 +  size_line * y + size_plane * z] = label;
      }
    }
  }
  
  // forward propagation
  for (int z = 1; z < dims[2]-1; z++) {
    for (int y = 1; y < dims[1]-1; y++) {
      for (int x = 1; x < dims[0]-1; x++) {
     int index = x +  size_line * y + size_plane * z;
     if (image[index] == background &&    // check past neighborhood
         (image[index - 1] == label || 
          image[index + size_line] == label || 
          image[index - size_plane] == label 
          )) {
       image[index] = label;
     }
      }
    }
  }

  // backward propagation
  for (int z = dims[2]-2; z > 0; z--) {
    for (int y = dims[1]-2; y > 0; y--) {
      for (int x = dims[0]-2; x > 0; x--) {
     int index = x +  size_line * y + size_plane * z;
     if (image[index] == background &&    // check past neighborhood
         (image[index + 1] == label || 
          image[index + size_line] == label || 
          image[index + size_plane] == label 
          )) {
       image[index] = label;
     }
      }
    }
  }

  // reassign labels
  for (int i = 0; i < dims[2]*dims[1]*dims[0]; i++) {
    if (image[i] == label) image[i] = background;
    else if (image[i] == background) image[i] = new_label;
  }

}
