//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:		Andrew Beatty
// Created:		March 9, 2009
//

#include "stdafx.h"

#include "FgStdStream.hpp"
#include "FgDepGraph.hpp"

using namespace std;

void
fgLnkNoop(
    const std::vector<const FgVariant*> &,
    const std::vector<FgVariant*> &)
{}

string
fgDepGraph2Dot(
    const FgLinkGraph<FgDepNode,FgLink> &   lg,
    const string &                          label,
    const vector<uint> &                    paramInds)
{
    ostringstream    ret;
    ret << "digraph DepGraph\n{\n";
    ret << "  graph [label=\"" << label << "\"];\n  {\n    node [shape=box]\n";
    for (size_t ii=0; ii<paramInds.size(); ++ii)
        ret << "    \"" << lg.nodeData(paramInds[ii]).name(paramInds[ii]) << "\" [shape=doubleoctagon]\n";
    for (uint ii=0; ii<lg.numLinks(); ii++) {
        vector<uint>    sources = lg.linkSources(ii);
        vector<uint>    sinks = lg.linkSinks(ii);
        // Skip stubs (used by GUI) as they obsfucate:
        if ((sinks.size() == 1) && (lg.nodeData(sinks[0]).label == "stub"))
            continue;
        ret << "    L" << ii << " [shape=oval];\n";
        for (uint jj=0; jj<sources.size(); jj++)
            ret << "    \"" << lg.nodeData(sources[jj]).name(sources[jj]) 
                << "\" -> L" << ii << ";\n";
        for (uint jj=0; jj<sinks.size(); jj++)
            ret << "    L" << ii << " -> \""
                << lg.nodeData(sinks[jj]).name(sinks[jj]) << "\";\n";
    }
    ret << "  }\n";
    ret << "}\n";
    return ret.str();
}

void
fgDotToPdf(
    const std::string &     dotFile,
    const std::string &     pdfFile)
{
    // Uses 'dot.exe' from Graphviz (2.38 as of last test use):
    string      cmd = "dot -Tpdf -o" + pdfFile + " " + dotFile;
    system(cmd.c_str());
}

void
fgDepGraph2Pdf(
    const FgLinkGraph<FgDepNode,FgLink> &   lg,
    const std::string &                     rootName,
    const vector<uint> &                    paramInds)
{
    FgOfstream      ofs(rootName+".dot");
    ofs << fgDepGraph2Dot(lg,"",paramInds);
    ofs.close();
    fgDotToPdf(rootName+".dot",rootName+".pdf");
}

