//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: June 16, 2017
//
// Test C++ behaviour
//

#include "stdafx.h"

#include "FgCmd.hpp"
#include "FgSyntax.hpp"
#include "FgMetaFormat.hpp"
#include "FgImage.hpp"
#include "FgTestUtils.hpp"
#include "FgBuild.hpp"
#include "FgVersion.hpp"
#include "FgTime.hpp"

using namespace std;

static size_t rvoCount;

struct  FgTestmRvo
{
    FgDbls      data;
    FgTestmRvo() {}
    FgTestmRvo(double v) : data(1,v) {}
    FgTestmRvo(const FgTestmRvo & rhs) : data(rhs.data) {++rvoCount; }
};

FgTestmRvo
fgTestmRvo()
{
    FgDbls      t0(1,3.14159);
    return FgTestmRvo(t0[0]);
}

FgTestmRvo
fgTestmRvoMr(bool sel)
{
    FgDbls      t0(1,2.71828),
                t1(1,3.14159);
    if (sel)
        return FgTestmRvo(t0[0]);
    else
        return FgTestmRvo(t1[0]);
}

FgTestmRvo
fgTestmNrvo(bool sel)
{
    FgDbls      t0(1,2.71828),
                t1(1,3.14159);
    FgTestmRvo  ret;
    if (sel)
        ret = FgTestmRvo(t0[0]);
    else
        ret = FgTestmRvo(t1[0]);
    return ret;
}

FgTestmRvo
fgTestmNrvoMr(bool sel)
{
    FgDbls      t0(1,2.71828),
                t1(1,3.14159);
    FgTestmRvo  ret;
    if (sel) {
        ret = FgTestmRvo(t0[0]);
        return ret;
    }
    else {
        ret = FgTestmRvo(t1[0]);
        return ret;
    }
}

static
void
rvo(const FgArgs &)
{
    FgTestmRvo      t;
    rvoCount = 0;
    t = fgTestmRvo();
    fgout << fgnl << "RVO copies: " << rvoCount;
    rvoCount = 0;
    t = fgTestmRvoMr(true);
    t = fgTestmRvoMr(false);
    fgout << fgnl << "RVO with multiple returns (both paths) copies: " << rvoCount;
    rvoCount = 0;
    t = fgTestmNrvo(true);
    t = fgTestmNrvo(false);
    fgout << fgnl << "NRVO copies (both assignment paths): " << rvoCount;
    rvoCount = 0;
    t = fgTestmNrvoMr(true);
    t = fgTestmNrvoMr(false);
    fgout << fgnl << "NRVO with multiple returns (both paths) copies: " << rvoCount;
}

static
void
speedExp(const FgArgs &)
{
    double      val = 0,
                inc = 2.718281828,
                mod = 3.141592653,
                acc = 0;
    size_t      reps = 10000000;
    FgTimer     tm;
    for (size_t ii=0; ii<reps; ++ii) {
        acc += exp(-val);
        val += inc;
        if (val > mod)
            val -= mod;
    }
    fgout << fgnl << "exp() time: " << 1000000.0 * tm.readMs() / reps << " ns  (dummy val: " << acc << ")";
}

static
void
fgexp(const FgArgs &)
{
    double      maxRel = 0,
                totRel = 0;
    size_t      cnt = 0;
    for (double dd=0; dd<5; dd+=0.001) {
        double  baseline = exp(-dd),
                test = fgExpFast(-dd),
                meanVal = (test+baseline) * 0.5,
                relDel = (test-baseline) / meanVal;
        maxRel = fgMax(maxRel,relDel);
        totRel += relDel;
        ++cnt;
    }
    fgout << "Max Rel Del: " << maxRel << " mean rel del: " << totRel / cnt;
    double      val = 0,
                inc = 2.718281828,
                mod = 3.141592653,
                acc = 0;
    size_t      reps = 10000000;
    FgTimer     tm;
    for (size_t ii=0; ii<reps; ++ii) {
        acc += fgExpFast(-val);
        val += inc;
        if (val > mod)
            val -= mod;
    }
    fgout << fgnl << "exp() time: " << 1000000.0 * tm.readMs() / reps << " ns  (dummy val: " << acc << ")";
}

static
void
any(const FgArgs &)
{
    boost::any      v0 = 42,
                    v1 = v0;
    int *           v0Ptr = boost::any_cast<int>(&v0);
    *v0Ptr = 7;
    fgout << fgnl << "Original small value: " << *v0Ptr << " but copy remains at " << boost::any_cast<int>(v1);
    // Now try with a heavy object that is not subject to small value optimization (16 bytes) onto the stack:
    v0 = FgMat44D(42);
    v1 = v0;
    FgMat44D *      v0_ptr = boost::any_cast<FgMat44D>(&v0);
    (*v0_ptr)[0] = 7;
    fgout << fgnl << "Original big value: " << (*v0_ptr)[0] << " but copy remains at " << boost::any_cast<FgMat44D>(v1)[0];
}

static
void
parr(const FgArgs &)
{
    // Generate data and put in both parallel and packed arrays:
    size_t          N = 100000,
                    A = 8;
    FgDblss         pins(A),
                    pouts(A);
    for (FgDbls & pin : pins)
        pin = fgGenerate<double>(fgRandNormal,N);
    for (FgDbls & pout : pouts)
        pout.resize(N,0);
    FgDbls          sins,
                    souts(N*A,0.0);
    for (size_t ii=0; ii<N; ++ii)
        for (size_t jj=0; jj<A; ++jj)
            sins.push_back(pins[jj][ii]);

    // Parallel -> Parallel
    FgTimer         tm;
    for (size_t rr=0; rr<100; ++rr) {
        for (size_t ii=0; ii<N; ++ii) {
            pouts[0][ii] += pins[0][ii]*pins[1][ii] + pins[2][ii]*pins[3][ii];
            pouts[1][ii] += pins[1][ii]*pins[2][ii] + pins[3][ii]*pins[4][ii];
            pouts[2][ii] += pins[2][ii]*pins[3][ii] + pins[4][ii]*pins[5][ii];
            pouts[3][ii] += pins[3][ii]*pins[4][ii] + pins[5][ii]*pins[6][ii];
            pouts[4][ii] += pins[4][ii]*pins[5][ii] + pins[6][ii]*pins[7][ii];
            pouts[5][ii] += pins[5][ii]*pins[6][ii] + pins[7][ii]*pins[0][ii];
            pouts[6][ii] += pins[6][ii]*pins[7][ii] + pins[0][ii]*pins[1][ii];
            pouts[7][ii] += pins[7][ii]*pins[0][ii] + pins[1][ii]*pins[2][ii];
        }
    }
    size_t          time = tm.readMs();
    double          val = 0;
    for (const FgDbls & outs : pouts)
        val += fgSum(outs);
    fgout << fgnl << "Paral arrays in, paral arrays out: " << time << "ms. (" << val << ")";

    // Packed -> Packed
    tm.start();
    for (size_t rr=0; rr<100; ++rr) {
        for (size_t ii=0; ii<N; ++ii) {
            // Inlining this value in every use made no speed difference; the compiler
            // takes care of this automatically. We only leave it for clarity:
            size_t          idx = ii*8;
            souts[idx+0] += sins[idx+0]*sins[idx+1] + sins[idx+2]*sins[idx+3];
            souts[idx+1] += sins[idx+1]*sins[idx+2] + sins[idx+3]*sins[idx+4];
            souts[idx+2] += sins[idx+2]*sins[idx+3] + sins[idx+4]*sins[idx+5];
            souts[idx+3] += sins[idx+3]*sins[idx+4] + sins[idx+5]*sins[idx+6];
            souts[idx+4] += sins[idx+4]*sins[idx+5] + sins[idx+6]*sins[idx+7];
            souts[idx+5] += sins[idx+5]*sins[idx+6] + sins[idx+7]*sins[idx+0];
            souts[idx+6] += sins[idx+6]*sins[idx+7] + sins[idx+0]*sins[idx+1];
            souts[idx+7] += sins[idx+7]*sins[idx+0] + sins[idx+1]*sins[idx+2];
        }
    }
    time = tm.readMs();
    val = fgSum(souts);
    fgout << fgnl << "Packed array in, packed array out: " << time << "ms. (" << val << ")";

    // Packed -> Parallel
    // I have no idea why this is faster than packed->packed. I looked at the MSVC17 x64 O2
    // disassembler (on Goldbolt using just 4 vals and a single mult) and the multiply, add, store
    // operands were identical ...
    for (FgDbls & pout : pouts)
        fgFill(pout,0.0);
    tm.start();
    for (size_t rr=0; rr<100; ++rr) {
        for (size_t ii=0; ii<N; ++ii) {
            // Inlining this value in every use made no speed difference; the compiler
            // takes care of this automatically. We only leave it for clarity:
            size_t          idx = ii*8;
            pouts[0][ii] += sins[idx+0]*sins[idx+1] + sins[idx+2]*sins[idx+3];
            pouts[1][ii] += sins[idx+1]*sins[idx+2] + sins[idx+3]*sins[idx+4];
            pouts[2][ii] += sins[idx+2]*sins[idx+3] + sins[idx+4]*sins[idx+5];
            pouts[3][ii] += sins[idx+3]*sins[idx+4] + sins[idx+5]*sins[idx+6];
            pouts[4][ii] += sins[idx+4]*sins[idx+5] + sins[idx+6]*sins[idx+7];
            pouts[5][ii] += sins[idx+5]*sins[idx+6] + sins[idx+7]*sins[idx+0];
            pouts[6][ii] += sins[idx+6]*sins[idx+7] + sins[idx+0]*sins[idx+1];
            pouts[7][ii] += sins[idx+7]*sins[idx+0] + sins[idx+1]*sins[idx+2];
        }
    }
    time = tm.readMs();
    val = 0;
    for (const FgDbls & outs : pouts)
        val += fgSum(outs);
    fgout << fgnl << "Packed array in, paral arrays out: " << time << "ms. (" << val << ")";



}

void
fgCmdTestmCpp(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(rvo,"rvo","Return value optimization / copy elision"));
    cmds.push_back(FgCmd(speedExp,"exp","Measure the speed of library exp(double)"));
    cmds.push_back(FgCmd(fgexp,"fgexp","Test and mesaure speed of interal optimized exp"));
    cmds.push_back(FgCmd(any,"any","Test boost any copy semantics"));
    cmds.push_back(FgCmd(parr,"parr","Test speedup of switching from parallel to packed arrays"));
    fgMenu(args,cmds);
}

// */
