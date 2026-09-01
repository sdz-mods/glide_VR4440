/*
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
*/

#include <3dfx.h>
#include <sst1vid.h>
#include "init.h"
#include <init96.h>
#include "fxinit.h"

/* hack */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <gdebug.h>

extern FxBool FX_CALL initAT3DGammaRGB(InitRegisterDesc *rd,
                                       double r, double g, double b);
extern FxBool FX_CALL initAT3DGammaTable(InitRegisterDesc *rd, FxU32 nentries,
                                         FxU32 *r, FxU32 *g, FxU32 *b);
extern void FX_CALL initAT3DRestoreGamma(InitRegisterDesc *rd);

static FxBool setVideo( FxU32                   hWnd,
                        GrScreenResolution_t    sRes,
                        GrScreenRefresh_t       vRefresh,
                        InitColorFormat_t       cFormat,
                        InitOriginLocation_t    yOrigin,
                        int                     nColBuffers,
                        int                     nAuxBuffers,
                        int                     *xres,
                        int                     *yres,
                        int                     *fbStride,
                        sst1VideoTimingStruct   *vidTimings) {

    return init96SetVideo( hWnd, sRes, vRefresh, cFormat, yOrigin,
                           nColBuffers, nAuxBuffers,
                           &context->info.regs,
                           xres, yres, fbStride );
}

static void restoreVideo( void ) {
    if (context->info.hwDep.vg96Info.vgaChip == 1)
        initAT3DRestoreGamma(&context->info.regs);
    init96RestoreVideo( &context->info.regs );
}

static FxBool enableTransport( InitFIFOData *info ) {
    return init96EnableTransport( info, &context->info.regs );
}

static void disableTransport( void ) {
    init96DisableTransport();
}

static /*InitSwapType_t*/ void swapBuffers( FxU32 code ) {
    if ( context && context->writeMethod )
        init96Swap( code, &context->info.regs, context->writeMethod );
/*  return INIT_SWAP_FLIP;*/
}

static FxU32 status( void ) {
  return *(context->info.regs.hwDep.VG96RegDesc.serialStatus);
}

static FxBool busy(void) {
  FxBool rv;
  rv = (status() & 0x1) ? FXTRUE : FXFALSE;
  return rv;
}

static void idle( void ) {
    init96Idle(context->writeMethod);
}

static void *getBufferPtr( InitBuffer_t buffer, int *strideBytes ) {
    return init96GetBufferPtr( buffer, strideBytes );
}

static void renderBuffer( InitBuffer_t buffer ) {
    init96RenderBuffer( buffer, context->writeMethod );
}

static void origin( InitOriginLocation_t origin ) {
    init96Origin( origin, context->writeMethod );
}

static void ioCtl( FxU32 token, void *argument ) {
}

static FxBool control( FxU32 code ) {
    FxBool rv = FXFALSE;
    GDBG_INFO((80, "vg96drvr: control: code = %d\n", code));
    if (context) 
        rv = init96Control( code, &context->info.regs);
    return rv;
}

static FxBool wrapFIFO(InitFIFOData *fd) {
    return init96WrapFIFO(&(context->info.regs), fd);
}

static FxBool gammaValue(const char *name, double *value) {
  const char *envp = myGetenv(name);
  double parsed;

  if (envp == NULL || envp[0] == '\0')
    return FXFALSE;

  parsed = atof(envp);
  if (parsed <= 0.0 || parsed > 16.0)
    return FXFALSE;

  *value = parsed;
  return FXTRUE;
}

static void sst96gamma( double gamma ) {
  static FxBool calledBefore = FXFALSE;
  static FxBool overrideR = FXFALSE;
  static FxBool overrideG = FXFALSE;
  static FxBool overrideB = FXFALSE;
  static double gammaR, gammaG, gammaB;
  double unified;

  if (context->info.hwDep.vg96Info.vgaChip != 1)
    return;

  if (!overrideR) gammaR = gamma;
  if (!overrideG) gammaG = gamma;
  if (!overrideB) gammaB = gamma;

  if (!calledBefore) {
    calledBefore = FXTRUE;

    overrideR = gammaValue("SST_RGAMMA", &gammaR);
    overrideG = gammaValue("SST_GGAMMA", &gammaG);
    overrideB = gammaValue("SST_BGAMMA", &gammaB);
    if (gammaValue("SST_GAMMA", &unified)) {
      gammaR = gammaG = gammaB = unified;
      overrideR = overrideG = overrideB = FXTRUE;
    }

    if (gammaValue("SST96_RGAMMA", &gammaR)) overrideR = FXTRUE;
    if (gammaValue("SST96_GGAMMA", &gammaG)) overrideG = FXTRUE;
    if (gammaValue("SST96_BGAMMA", &gammaB)) overrideB = FXTRUE;
    if (gammaValue("SST96_GAMMA", &unified)) {
      gammaR = gammaG = gammaB = unified;
      overrideR = overrideG = overrideB = FXTRUE;
    }
  }

  initAT3DGammaRGB(&context->info.regs, gammaR, gammaG, gammaB);
}

static void sliPciOwner( FxU32 *regbase, FxU32 owner ) {
}

static FxBool gammargb( double r, double g, double b ) {
  if (context->info.hwDep.vg96Info.vgaChip != 1)
    return FXFALSE;
  return initAT3DGammaRGB(&context->info.regs, r, g, b);
}

static FxBool gammatable( FxU32 nentries, FxU32 *r, FxU32 *g, FxU32 *b ) {
  if (context->info.hwDep.vg96Info.vgaChip != 1)
    return FXFALSE;
  return initAT3DGammaTable(&context->info.regs, nentries, r, g, b);
}

static sst1VideoTimingStruct *findvidtiming( GrScreenResolution_t sRes, GrScreenRefresh_t vRefresh) {
  return NULL;
}

void vg96DriverInit( InitContext *context ) {
    context->setVideo         = setVideo;
    context->restoreVideo     = restoreVideo;
    context->enableTransport  = enableTransport;
    context->disableTransport = disableTransport;
    context->swapBuffers      = swapBuffers;
    context->status           = status;
    context->busy             = busy;
    context->idle             = idle;
    context->getBufferPtr     = getBufferPtr;
    context->renderBuffer     = renderBuffer;
    context->origin           = origin;
    context->ioCtl            = ioCtl;
    context->control          = control;
    context->wrapFIFO         = wrapFIFO;

    context->gamma            = sst96gamma;
    context->sliPciOwner      = sliPciOwner;

    context->gammaRGB         = gammargb;
    context->initGammaTable   = gammatable;
    context->findVidTimingStruct = findvidtiming;
}
