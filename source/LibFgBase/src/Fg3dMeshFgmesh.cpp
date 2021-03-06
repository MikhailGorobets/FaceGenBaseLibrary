//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Mesh format using little-endian int32, uint32, IEEE 754 single-precision and UTF-8 data
//

#include "stdafx.h"

#include "Fg3dMeshIo.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "Fg3dDisplay.hpp"
#include "FgStdStream.hpp"

using namespace std;

namespace Fg {

template<uint dim>
void
fgReadp(std::istream & is,FacetInds<dim> & fi)
{
    fgReadp(is,fi.posInds);
    fgReadp(is,fi.uvInds);
}

template<uint dim>
void
fgWritep(std::ostream & os,const FacetInds<dim> & fi)
{
    fgWritep(os,fi.posInds);
    fgWritep(os,fi.uvInds);
}

void
fgReadp(std::istream & is,SurfPoint & sp)
{
    fgReadp(is,sp.triEquivIdx);
    fgReadp(is,sp.weights);
    fgReadp(is,sp.label);
}

void
fgWritep(std::ostream & os,SurfPoint const & sp)
{
    fgWritep(os,sp.triEquivIdx);
    fgWritep(os,sp.weights);
    fgWritep(os,sp.label);
}

void
fgReadp(std::istream & is,Surf & s)
{
    fgReadp(is,s.name);
    fgReadp(is,s.tris);
    fgReadp(is,s.quads);
    fgReadp(is,s.surfPoints);
}

void
fgWritep(std::ostream & os,Surf const & s)
{
    fgWritep(os,s.name);
    fgWritep(os,s.tris);
    fgWritep(os,s.quads);
    fgWritep(os,s.surfPoints);
}

void
fgReadp(std::istream & is,MarkedVert & m)
{
    fgReadp(is,m.idx);
    fgReadp(is,m.label);
}

void
fgWritep(std::ostream & os,const MarkedVert & m)
{
    fgWritep(os,m.idx);
    fgWritep(os,m.label);
}

void
fgReadp(std::istream & is,Morph & m)
{
    fgReadp(is,m.name);
    fgReadp(is,m.verts);
}

void
fgWritep(std::ostream & os,Morph const & m)
{
    fgWritep(os,m.name);
    fgWritep(os,m.verts);
}

void
fgReadp(std::istream & is,IndexedMorph & m)
{
    fgReadp(is,m.name);
    fgReadp(is,m.baseInds);
    fgReadp(is,m.verts);
}

void
fgWritep(std::ostream & os,const IndexedMorph & m)
{
    fgWritep(os,m.name);
    fgWritep(os,m.baseInds);
    fgWritep(os,m.verts);
}

void
fgReadp(std::istream & is,Mesh & mesh)
{
    fgReadp(is,mesh.verts);
    fgReadp(is,mesh.uvs);
    fgReadp(is,mesh.surfaces);
    fgReadp(is,mesh.deltaMorphs);
    fgReadp(is,mesh.targetMorphs);
    fgReadp(is,mesh.markedVerts);
}

void
fgWritep(std::ostream & os,Mesh const & mesh)
{
    fgWritep(os,mesh.verts);
    fgWritep(os,mesh.uvs);
    fgWritep(os,mesh.surfaces);
    fgWritep(os,mesh.deltaMorphs);
    fgWritep(os,mesh.targetMorphs);
    fgWritep(os,mesh.markedVerts);
}

Mesh
loadFgmesh(Ustring const & fname)
{
    Mesh        ret;
    Ifstream      ifs(fname);
    if (fgReadpT<string>(ifs) != "FgMesh01")
        fgThrow("Not a valid FGMESH file",fname);
    fgReadp(ifs,ret);
    return ret;
}

void
saveFgmesh(Ustring const & fname,Mesh const & mesh)
{
    Ofstream          ofs(fname);
    fgWritep(ofs,string("FgMesh01"));
    fgWritep(ofs,mesh);
}

void
saveFgmesh(Ustring const & fname,Meshes const & meshes)
{saveFgmesh(fname,mergeMeshes(meshes)); }

void
fgSaveFgmeshTest(CLArgs const & args)
{
    FGTESTDIR;
    saveFgmesh("Mouth.tri",loadTri(dataDir()+"base/Mouth.tri"));
    viewMesh(loadFgmesh("Mouth.tri"));
}

}
