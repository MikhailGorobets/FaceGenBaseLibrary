//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Functions for reading and writing images to various file formats.
//

#ifndef FGIMAGEIO_HPP
#define FGIMAGEIO_HPP

#include "FgStdExtensions.hpp"
#include "FgImage.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

// Load an image from any supported format:
void    loadImage_(Ustring const & fname,ImgC4UC & img);
void    loadImage_(Ustring const & fname,ImgF & img);
void    loadImage_(Ustring const & fname,ImgUC & img);

ImgC4UC
loadImage(Ustring const & fname);

// Save an image to any supported format:
void
saveImage(Ustring const & fname,ImgC4UC const & img);

void
saveImage(Ustring const & fname,const ImgUC & img);

struct  ImgFileFormat
{
    String      description;
    Strings     extensions;     // Usually 1 but can be 2 (eg. 'jpg', 'jpeg'). Preferred listed first.
};
typedef Svec<ImgFileFormat>     ImgFileFormats;

// List of image file formats supported by above in order of common use in upper case:
ImgFileFormats
imgFileFormats();

// List all supported image file format extensions in lower case, including synonyms:
Strings
imgFileExtensions();

// Command-line options string of the formats above:
String
imgFileExtensionsDescription();

bool
hasImgExtension(Ustring const & fname);

// Returns a list of image extensions for which there are readable files 'baseName.ext':
Strings
imgFindFiles(Ustring const & baseName);

// Look for an image file in any common format starting with 'baseName' and load it if found.
// Return true if found and false otherwise.
bool
imgFindLoadAnyFormat(Ustring const & baseName,ImgC4UC & img);

void
saveJfif(
    ImgC4UC const &  img,        // Alpha channel will be ignored
    Ustring const & fname,
    uint            quality);   // [1,100]

// data must be 4 channel RGBA of size wid*hgt*4:
Uchars
imgEncodeJpeg(uint wid,uint hgt,uchar const * data,int quality);

// Encode to JFIF format blob (can be dumped to .jpg file):
Uchars
imgEncodeJpeg(ImgC4UC const & img,int quality=100);

// Decode from JFIF format blob (can be read from JFIF format .jpg file):
ImgC4UC
imgDecodeJpeg(Uchars const & jfifBlob);

}

#endif
